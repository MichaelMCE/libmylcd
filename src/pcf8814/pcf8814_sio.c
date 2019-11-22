
// PCF8814 driver via USB13700. www.lcdinfo.com for details on the USB board.
// Found in Nokia 3510, 6310, 5310 and others
// Can also drive PCF8811 based displays

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

#if (__BUILD_PCF8814__)

#include "../frame.h"
#include "../device.h"
#include "../display.h"
#include "../memory.h"
#include "../pixel.h"
#include "../copy.h"
#include "../lstring.h"
#include "pcf8814_sio.h"



static void pcf8814_shutdown (TDRIVER *drv);

static int pcf8814_OpenDisplay (TDRIVER *drv);
static int pcf8814_CloseDisplay (TDRIVER *drv);
static int pcf8814_ClearDisplay (TDRIVER *drv);
static int pcf8814_Refresh (TDRIVER *drv, TFRAME *frm);
static int pcf8814_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static int pcf8814_setOption (TDRIVER *drv, int option, intptr_t *value);
static int pcf8814_getOption (TDRIVER *drv, int option, intptr_t *value);
static int pcf8814_updateAreaFlippedH (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static int pcf8814_updateArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static void pcf8814_initiate (TDRIVER *drv);

static void pcf8814_write_data (TDRIVER *drv, ubyte data);
static void pcf8814_write_command (TDRIVER *drv, ubyte data);
static void pcf8814_set_xaddr (TDRIVER *drv, ubyte x);
static void pcf8814_set_yaddr (TDRIVER *drv, ubyte y);

static INLINE void writeBytes (TDRIVER *drv, ubyte *values,int tbytes);
static void pcf8814_flushdata (TDRIVER *drv);


static int initOnce = 0;


// lpt data port
#define PIN2		1	//d0
#define PIN3		2	//d1
#define PIN4		4	//d2
#define PIN5		8	//d3
#define PIN6		16	//d4
#define PIN7		32	//d5
#define PIN8		64	//d6
#define PIN9		128	//d7

#define SCLK		PIN2
#define SDA			PIN3
#define RESET		PIN4
#define POWER		PIN7
#define	CS_1		PIN8|PIN9		// is using pin5 which means CS 2 and 3 should be disabled (high)
#define	CS_2		PIN5|PIN9		// is using pin8 which means CS 1 and 3 should be disabled
#define	CS_3		PIN5|PIN8		// is using pin9 which means CS 1 and 2 should be disabled
#define	CS_ALL		PIN5|PIN8|PIN9
#define RESET_POWER RESET|POWER


int initPCF8814 (TREGISTEREDDRIVERS *rd)
{
	TDISPLAYDRIVER dd;
	int dnumber;
	
	dd.open = pcf8814_OpenDisplay;
	dd.close = pcf8814_CloseDisplay;
	dd.clear = pcf8814_ClearDisplay;
	dd.refresh = pcf8814_Refresh;
	dd.refreshArea = pcf8814_RefreshArea;
	dd.setOption = pcf8814_setOption;
	dd.getOption = pcf8814_getOption;
	dd.status = LDRV_CLOSED;
	dd.optTotal = 4;

	l_strcpy(dd.name, "PCF8814:SIO");
	l_strcpy(dd.comment, "PCF8814:SIO");
	dnumber = registerDisplayDriver(rd, &dd);
	setDefaultDisplayOption(rd, dnumber, lOPT_PCF8814_CS, CS_1);
	setDefaultDisplayOption(rd, dnumber, lOPT_PCF8814_HFLIP, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_PCF8814_INVERT, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_PCF8814_STRUCT, 0);

	return (dnumber > 0);
}

void closePCF8814 (TREGISTEREDDRIVERS *rd)
{
	return;
}

static int pcf8814_setOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (drv){
		intptr_t *opt = drv->dd->opt;
		opt[option] = *value;

		if (option == lOPT_PCF8814_INVERT){
			if (*value&0x01)
				pcf8814_write_command(drv, 0xA7);
			else
				pcf8814_write_command(drv, 0xA6);
			pcf8814_flushdata(drv);
		}
		return 1;
	}else{
		return 0;
	}
}

static int pcf8814_getOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (drv && value){
		intptr_t *opt = drv->dd->opt;
		*value = opt[option];
		return 1;
	}else{
		return 0;
	}
}

static int pcf8814_OpenDisplay (TDRIVER *drv)
{
	
	if (drv){
		if (drv->dd->status == LDRV_CLOSED){
			//drv->dd->back = _newFrame(drv->dd->hw, drv->dd->width, drv->dd->height, 1, LFRM_BPP_1);
			//if (drv->dd->back){
				drv->dd->status = LDRV_READY;
				pcf8814_initiate(drv);
				pcf8814_ClearDisplay(drv);
				return 1;
			//}
		}
	}

	return 0;
}

static int pcf8814_CloseDisplay (TDRIVER *drv)
{

	if (drv){
		if (drv->dd->status == LDRV_READY){
			//pcf8814_ClearDisplay(drv);
			pcf8814_shutdown(drv);
			//if (drv->dd->back)
				//deleteFrame(drv->dd->back);
			drv->dd->status = LDRV_CLOSED;
			return 1;
		}
	}
	return 0;
}

static int pcf8814_ClearDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status == LDRV_READY){
			pcf8814_set_xaddr(drv, 0);
			pcf8814_set_yaddr(drv, 0);
		
			int buffersize = ((drv->dd->height/8)*drv->dd->width)+drv->dd->WIDTH;
			while(buffersize--)
				pcf8814_write_data(drv, drv->dd->clr);
		
			pcf8814_set_xaddr(drv, 0);
			pcf8814_set_yaddr(drv, 0);
			pcf8814_flushdata(drv);
			return 1;
		}
	}
	return 0;
}

static void pcf8814_flushdata (TDRIVER *drv)
{
	drv->pd->flush(drv->pd);
}

static void pcf8814_write_command (TDRIVER *drv, ubyte cmd)
{

	drv->pd->write8(drv->dd->port,         RESET_POWER);
	drv->pd->write8(drv->dd->port,  SCLK | RESET_POWER);
	
	ubyte i = 8;
	while(i--){
		if (cmd&0x80){
			cmd <<= 1;
			drv->pd->write8(drv->dd->port, SDA |        RESET_POWER);
			drv->pd->write8(drv->dd->port, SDA | SCLK | RESET_POWER);
		}else{
			cmd <<= 1;
			drv->pd->write8(drv->dd->port,        RESET_POWER);
			drv->pd->write8(drv->dd->port, SCLK | RESET_POWER);
		}
	}
}

static void pcf8814_write_data (TDRIVER *drv, ubyte data)
{	
	drv->pd->write8(drv->dd->port, SDA |        RESET_POWER);
	drv->pd->write8(drv->dd->port, SDA | SCLK | RESET_POWER);

	ubyte i = 8;
	while(i--){
		if (data&0x80){
			data <<= 1;
			drv->pd->write8(drv->dd->port, SDA |        RESET_POWER);
			drv->pd->write8(drv->dd->port, SDA | SCLK | RESET_POWER);
		}else{
			data <<= 1;
			drv->pd->write8(drv->dd->port,        RESET_POWER);
			drv->pd->write8(drv->dd->port, SCLK | RESET_POWER);
		}
	}
}

static int pcf8814_Refresh (TDRIVER *drv, TFRAME *frm)
{
	if (!frm || !drv)
		return 0;
	else if (drv->dd->status == LDRV_READY){
		pcf8814_RefreshArea(drv, frm, 0, 0, frm->width-1, frm->height-1);
		drv->pd->flush(drv->pd);
		return 1;
	}
	return 0;
}

static int pcf8814_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{

	if (!frm || !drv)
		return 0;
	if (drv->dd->status != LDRV_READY)
		return 0;

	intptr_t flip = 0;
	pcf8814_getOption(drv, lOPT_PCF8814_HFLIP, &flip);
	if (!flip)
		return pcf8814_updateArea(drv, frm, 0, 0, frm->width-1, frm->height-1);
	else
		return pcf8814_updateAreaFlippedH(drv, frm, 0, 0, frm->width-1, frm->height-1);
}

static int pcf8814_updateArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	ubyte b[((frm->height/8)*frm->width)+frm->width];
	int y_o;
	int x,y=0;
	int bct=0;
	int o = y2&7;
	y2++, x2++;
	y2 += o;

	for (y=y1; y<y2; y+=8){
		for (x=x1; x<x2; x++){
			y_o = y-o;
			b[bct] = getPixel_NB(frm, x, y_o);
			if (++y_o  < frm->height){
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

	// not required as display ram address resets to 0,0 after each full frame update
	#if 0
	pcf8814_set_yaddr(drv, 0);
	pcf8814_set_xaddr(drv, 0);
	#endif

	writeBytes(drv, b, bct);
	pcf8814_flushdata(drv);
	return 1;
}

static int pcf8814_updateAreaFlippedH (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	ubyte b[((frm->height/8)*frm->width)+frm->width];
	const int height = frm->height;
	int y_o;
	int o=y1&7;
	int x,y=0;
	int bct=0;
	y2++;
	y2 +=o;

	for (y=y1; y<y2; y=y+8){
		for (x=x2; x>=x1; x--){
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

	// not required as display ram address resets to 0,0 after each full frame update
	#if 0
	pcf8814_set_yaddr(drv, 0);
	pcf8814_set_xaddr(drv, 0);
	#endif

	writeBytes(drv, b, bct);
	pcf8814_flushdata(drv);
	return 1;
}

static INLINE void writeBytes (TDRIVER *drv, ubyte *values, int tbytes)
{
	while(tbytes--)
		pcf8814_write_data(drv, *(values++));
}

static void pcf8814_set_xaddr (TDRIVER *drv, ubyte x)
{
//	drv->dd->currentColumn = x;
	pcf8814_write_command(drv, x & 0x0F);
	pcf8814_write_command(drv, 0x10 | (x >> 4));
}

static void pcf8814_set_yaddr (TDRIVER *drv, ubyte y)
{
//	drv->dd->currentRow = y;
	pcf8814_write_command(drv, 0xB0 | (y&0x0F));	// set page
}

static void pcf8814_initiate (TDRIVER *drv)
{
	drv->pd->write8(drv->dd->port, RESET_POWER);
	
	if (!initOnce){
		initOnce = 0xFF;
		// Power On
		drv->pd->write8(drv->dd->port, RESET_POWER);
		drv->pd->write8(drv->dd->port, POWER);
		drv->pd->flush(drv->pd);
		lSleep(5);
		drv->pd->write8(drv->dd->port, RESET_POWER);
		drv->pd->flush(drv->pd);
		lSleep(5);
	}

	pcf8814_write_command(drv, 0xE2);	/* reset */ 
	pcf8814_write_command(drv, 0xE1);	/* power save mode off */
	pcf8814_write_command(drv, 0x62);	/* 4x multiplier */ 
	pcf8814_write_command(drv, 0x81);
	pcf8814_write_command(drv, 0x3F);
	pcf8814_write_command(drv, 0x2C);	// switch on charge pump
	pcf8814_write_command(drv, 0x25);
	pcf8814_write_command(drv, 0x67);
	pcf8814_write_command(drv, 0xA4);
	
	pcf8814_write_command(drv, (31+drv->dd->HEIGHT));		// set start line
	
	pcf8814_write_command(drv, 0x1A);	/* p is automatic */ 
	pcf8814_write_command(drv, 0xC8);	/* mirror y */
	
	pcf8814_write_command(drv, 0xA8);	/* set data order (msb/lsb)*/

	pcf8814_write_command(drv, 0xAF);	/* switch on display*/


	intptr_t invert=0;
	pcf8814_getOption(drv, lOPT_PCF8814_INVERT, &invert);
	if (invert)
		pcf8814_write_command(drv, 0xA7);
	else
		pcf8814_write_command(drv, 0xA6);
	pcf8814_flushdata(drv);
}

static void pcf8814_shutdown (TDRIVER *drv)
{
	if (initOnce){
		initOnce = 0;
		drv->pd->write8(drv->dd->port, POWER);
		drv->pd->flush(drv->pd);
		lSleep(1);
		drv->pd->write8(drv->dd->port, 0);
		drv->pd->flush(drv->pd);
		lSleep(1);
	}
}

#else

int initPCF8814(void *rd){return 1;}
void closePCF8814(void *rd){return;}

#endif

