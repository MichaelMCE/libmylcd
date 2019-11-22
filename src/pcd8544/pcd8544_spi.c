
// PCD8544 driver via USB13700. www.lcdinfo.com for details on the USB board.
// Found in Nokia 3310, 5210, 8210, 6210, 3210 and many others.

// libmylcd
// An LCD framebuffer library
// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2009  Michael McElligott
// 
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU LIBRARY GENERAL PUBLIC LICENSE for more details.


#include "mylcd.h"

#if (__BUILD_PCD8544__)

#include "../frame.h"
#include "../device.h"
#include "../display.h"
#include "../memory.h"
#include "../pixel.h"
#include "../copy.h"
#include "../lstring.h"
#include "pcd8544_spi.h"


static int initOnce = 0;
static void pcd8544_shutdown (TDRIVER *drv);

static int pcd8544_OpenDisplay (TDRIVER *drv);
static int pcd8544_CloseDisplay (TDRIVER *drv);
static int pcd8544_ClearDisplay (TDRIVER *drv);
static int pcd8544_Refresh (TDRIVER *drv, TFRAME *frm);
static int pcd8544_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static int pcd8544_setOption (TDRIVER *drv, int option, intptr_t *value);
static int pcd8544_getOption (TDRIVER *drv, int option, intptr_t *value);
static int pcd8544_updateAreaFlippedH (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static int pcd8544_updateArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static void pcd8544_initiate (TDRIVER *drv);

static void pcd8544_write_data (TDRIVER *drv, ubyte data);
static void pcd8544_write_command (TDRIVER *drv, ubyte data);
static void pcd8544_set_xaddr (TDRIVER *drv, ubyte x);
static void pcd8544_set_yaddr (TDRIVER *drv, ubyte y);

static INLINE void writeBytes (TDRIVER *drv, ubyte *values,int tbytes);

static void set_ctrl (TDRIVER *drv, ubyte ctrl);
static void pcd8544_flushdata (TDRIVER *drv);



#define RESET		BIT0		/* active low */
#define	CS			BIT1		/* CE, active low */
#define DC			BIT2		/* command or data*/


/*
USB13700 
Exp connector - signal - LCD Conn
15/20			3.3v		1
11				SCK			2
10				SDA			3		MOSI
13				DC			4		EXP pin 13 should be pulled to 3.3v via a resistor
5				CS			5
1/2/8/19		GND			6
				Vout		7 		0.22uf to GND
6				Reset		8
*/


int initPCD8544_USB (TREGISTEREDDRIVERS *rd)
{
	TDISPLAYDRIVER dd;
	int dnumber;
	
	dd.open = pcd8544_OpenDisplay;
	dd.close = pcd8544_CloseDisplay;
	dd.clear = pcd8544_ClearDisplay;
	dd.refresh = pcd8544_Refresh;
	dd.refreshArea = pcd8544_RefreshArea;
	dd.setOption = pcd8544_setOption;
	dd.getOption = pcd8544_getOption;
	dd.status = LDRV_CLOSED;
	dd.optTotal = 4;

	l_strcpy(dd.name, "PCD8544:SPI");
	l_strcpy(dd.comment, "PCD8544. CS = EXP Pin 5");
	dnumber = registerDisplayDriver(rd, &dd);
	setDefaultDisplayOption(rd, dnumber, lOPT_PCD8544_CS, CS);
	setDefaultDisplayOption(rd, dnumber, lOPT_PCD8544_HFLIP, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_PCD8544_INVERT, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_PCD8544_STRUCT, 0);

	return (dnumber > 0);
}

void closePCD8544_USB (TREGISTEREDDRIVERS *rd)
{
	return;
}

static int pcd8544_setOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (drv){
		intptr_t *opt = drv->dd->opt;
		opt[option] = *value;
		
		if (option == lOPT_PCD8544_INVERT){
			set_ctrl(drv, RESET);
			if (*value)
				pcd8544_write_command(drv, 8 | 4 | 1);
			else
				pcd8544_write_command(drv, 8 | 4);
			pcd8544_flushdata(drv);
			set_ctrl(drv, RESET|DC);
		}
		return 1;
	}else{
		return 0;
	}
}

static int pcd8544_getOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (drv && value){
		intptr_t *opt = drv->dd->opt;
		*value = opt[option];
		return 1;
	}else{
		return 0;
	}
}

static int pcd8544_OpenDisplay (TDRIVER *drv)
{
	
	if (drv){
		if (drv->dd->status == LDRV_CLOSED){
			//drv->dd->back = _newFrame(drv->dd->hw, drv->dd->width, drv->dd->height, 1, LFRM_BPP_1);
			//if (drv->dd->back){
				drv->dd->status = LDRV_READY;
				pcd8544_initiate(drv);
				pcd8544_ClearDisplay(drv);
				return 1;
			//}
		}
	}

	return 0;
}

static int pcd8544_CloseDisplay (TDRIVER *drv)
{

	if (drv){
		if (drv->dd->status == LDRV_READY){
			//pcd8544_ClearDisplay(drv);
			pcd8544_shutdown(drv);
			//if (drv->dd->back)
				//deleteFrame(drv->dd->back);
			drv->dd->status = LDRV_CLOSED;
			return 1;
		}
	}
	return 0;
}

static int pcd8544_ClearDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status == LDRV_READY){
			set_ctrl(drv, RESET);
			pcd8544_set_xaddr(drv, 0);
			pcd8544_set_yaddr(drv, 0);
			pcd8544_flushdata(drv);
			set_ctrl(drv, RESET|DC);
	
			int buffersize = (drv->dd->WIDTH * drv->dd->HEIGHT)>>3;
			while(buffersize--)
				pcd8544_write_data(drv, drv->dd->clr);
		
			pcd8544_flushdata(drv);
			set_ctrl(drv, RESET);
			pcd8544_set_xaddr(drv, 0);
			pcd8544_set_yaddr(drv, 0);
			pcd8544_flushdata(drv);
			set_ctrl(drv, RESET|DC);
			return 1;
		}
	}
	return 0;
}

static void set_ctrl (TDRIVER *drv, ubyte ctrl)
{
	drv->pd->write8(drv->dd->port|0x02, ctrl);
}

static void pcd8544_flushdata (TDRIVER *drv)
{
	drv->pd->flush(drv->pd);
}

static void pcd8544_write_data (TDRIVER *drv, ubyte data)
{
	drv->pd->write8(drv->dd->port, data);
}

static void pcd8544_write_command (TDRIVER *drv, ubyte data)
{
	drv->pd->write8(drv->dd->port, data);
}

static int pcd8544_Refresh (TDRIVER *drv, TFRAME *frm)
{
	if (!frm || !drv)
		return 0;
	else if (drv->dd->status == LDRV_READY){
		pcd8544_RefreshArea(drv, frm, 0, 0, frm->width-1, frm->height-1);
		drv->pd->flush(drv->pd);
		return 1;
	}
	return 0;
}

static int pcd8544_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{

	if (!frm || !drv)
		return 0;
		
	if (drv->dd->status != LDRV_READY)
		return 0;

	intptr_t flip=0;
	pcd8544_getOption(drv, lOPT_PCD8544_HFLIP, &flip);
	if (flip)
		return pcd8544_updateAreaFlippedH(drv, frm, 0, 0, frm->width-1, frm->height-1);
	else
		return pcd8544_updateArea(drv, frm, 0, 0, frm->width-1, frm->height-1);
}

static int pcd8544_updateArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{

	ubyte b[(drv->dd->WIDTH * drv->dd->HEIGHT)>>3];
	int y_o;
	int x,y=0;
	int bct=0;
	int o=y1&7;
	y2++, x2++;
	y2 +=o;

	for (y=y1; y<y2; y=y+8){
		for (x=x1; x<x2; x++){
			y_o = y-o;
			
			b[bct] = getPixel_NB(frm, x, y_o);
			if (++y_o < frm->height){
				b[bct] |= getPixel_NB(frm, x, y_o) << 1;
				if (++y_o < frm->height){
					b[bct] |= getPixel_NB(frm, x, y_o) << 2;
					if (++y_o < frm->height){
						b[bct] |= getPixel_NB(frm, x, y_o) << 3;
						if (++y_o < frm->height){
							b[bct] |= getPixel_NB(frm, x, y_o) << 4;
							if (++y_o < frm->height){
								b[bct] |= getPixel_NB(frm, x, y_o) << 5;
								if (++y_o < frm->height){
									b[bct] |= getPixel_NB(frm, x, y_o) << 6;
									if (++y_o < frm->height)
										b[bct] |= getPixel_NB(frm, x, y_o) << 7;
								}
							}
						}
					}
				}
			}
			bct++;
		}
	}
/*
	set_ctrl(drv, RESET);
	pcd8544_set_yaddr(drv, 0);
	pcd8544_set_xaddr(drv, 0);
*/
//	set_ctrl(drv, RESET|DC);
	writeBytes(drv, b, bct);
	return 1;
}

static int pcd8544_updateAreaFlippedH (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	ubyte b[(drv->dd->WIDTH * drv->dd->HEIGHT)>>3];
	const int height = frm->height;
	int y_o;
	int o=y1&7;
	int x,y=0;
	int bct=0;
	y2++, x2++;
	y2 +=o;

	for (y=y1; y<y2; y=y+8){
		for (x=x2-1; x>=x1; x--){
			y_o = y-o;
			
			b[bct]  = getPixel_NB(frm, x, y_o);
			if (++y_o < height){
				b[bct] |= getPixel_NB(frm, x, y_o) << 1;
				if (++y_o < height){
					b[bct] |= getPixel_NB(frm, x, y_o) << 2;
					if (++y_o < height){
						b[bct] |= getPixel_NB(frm, x, y_o) << 3;
						if (++y_o < height){
							b[bct] |= getPixel_NB(frm, x, y_o) << 4;
							if (++y_o < height){
								b[bct] |= getPixel_NB(frm, x, y_o) << 5;
								if (++y_o < height){
									b[bct] |= getPixel_NB(frm, x, y_o) << 6;
									if (++y_o < height)
										b[bct] |= getPixel_NB(frm, x, y_o) << 7;
								}
							}
						}
					}
				}
			}
			bct++;
		}
	}
/*
	set_ctrl(drv, RESET);
	pcd8544_set_yaddr(drv, 0);
	pcd8544_set_xaddr(drv, 0);
*/
//	set_ctrl(drv, RESET|DC);
	writeBytes(drv, b, bct);

	return 1;
}

static INLINE void writeBytes (TDRIVER *drv, ubyte *values, int tbytes)
{
	while(tbytes--)
		pcd8544_write_data(drv, *(values++));
}

static void pcd8544_set_xaddr (TDRIVER *drv, ubyte x)
{
//	drv->dd->currentColumn = x;
	pcd8544_write_command(drv, 0x80 | (x&0x7F));
}

static void pcd8544_set_yaddr (TDRIVER *drv, ubyte y)
{
//	drv->dd->currentRow = y;
	pcd8544_write_command(drv, 0x40 | (y&0x07));	
}

static void pcd8544_initiate (TDRIVER *drv)
{
	if (!initOnce){
		initOnce = 0xFF;

		set_ctrl(drv, RESET);
		lSleep(25);
		set_ctrl(drv, CS);
		lSleep(25);
		set_ctrl(drv, RESET);
		lSleep(25);
	}

	// command sequence borrowed from LCDInfo
	set_ctrl(drv, RESET);
    pcd8544_write_command(drv, 0x21);	// LCD Extended Commands.
    pcd8544_write_command(drv, 197);	// Set LCD Vop (Contrast). (129-255)
    pcd8544_write_command(drv, 0x06);	// Set Temp coefficent. (4-7)
    pcd8544_write_command(drv, 0x13);	// LCD bias mode 1:48. (17-23)
    pcd8544_write_command(drv, 0x20);	// LCD Standard Commands, address mode.
    pcd8544_write_command(drv, 0x09);	// Activate all segments.
    pcd8544_write_command(drv, 0x08);	// Blank Display
	pcd8544_write_command(drv, 0x0C);	// LCD in normal mode.

	intptr_t invert=0;
	pcd8544_getOption(drv, lOPT_PCD8544_INVERT, &invert);
	if (invert)
		pcd8544_write_command(drv, 8 | 4 | 1);
	else
		pcd8544_write_command(drv, 8 | 4);

    pcd8544_flushdata(drv);
}

static void pcd8544_shutdown (TDRIVER *drv)
{
	if (initOnce){
		initOnce = 0;
		set_ctrl(drv, RESET|CS);
		set_ctrl(drv, CS);
		pcd8544_flushdata(drv);
	}
}

#else

int initPCD8544_USB(void *rd){return 1;}
void closePCD8544_USB(void *rd){return;}

#endif

