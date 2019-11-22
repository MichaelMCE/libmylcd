
// PCD8544 display driver
// found in Nokia 3310, 5210, 8210, 6210, 3210 and many others.

/*
Using LCDInfo wiring: http://forum.lcdinfo.com/viewtopic.php?p=2456
SCLK	-> D0 (LPT pin 2)
SDA		-> D1 (LPT pin 3)
RESET	-> D2 (LPT pin 4)
CS/CE	-> D3 (LPT pin 5)
DC		-> D4 (LPT pin 6)
POWER	-> D5 (LPT pin 7)

PCD8854 driver can also be configured to simultaneously drive three displays.
Connect 2nd or 3rd CS line to D6 or D7 - LPT pins 8 and 9
*/


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
#include "pcd8544_sio.h"



static int initOnce = 0;
static void pcd8544_shutdown (TDRIVER *drv);

int pcd8544_OpenDisplay (TDRIVER *drv);
int pcd8544_CloseDisplay (TDRIVER *drv);
int pcd8544_ClearDisplay (TDRIVER *drv);
int pcd8544_Refresh (TDRIVER *drv, TFRAME *frm);
int pcd8544_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
int pcd8544_setOption (TDRIVER *drv, int option, intptr_t *value);
int pcd8544_getOption (TDRIVER *drv, int option, intptr_t *value);
int pcd8544_updateAreaFlippedH (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
int pcd8544_updateArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
void pcd8544_initiate (TDRIVER *drv);

void pcd8544_write_data (TDRIVER *drv, ubyte data);
void pcd8544_write_command (TDRIVER *drv, ubyte data);
void pcd8544_set_xaddr (TDRIVER *drv, ubyte x);
void pcd8544_set_yaddr (TDRIVER *drv, ubyte y);

static INLINE void writeBytes (TDRIVER *drv, ubyte *values,int tbytes);

static INLINE int quadrefresh (TDRIVER *drv, TFRAME *frm);
static INLINE int cmpquad (TFRAME *a, TFRAME *b, int x, int y, int *col);
static INLINE void pcd8544_copyArea (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2);


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
#define DC			PIN6
#define POWER		PIN7
#define	CS_1		PIN8|PIN9		// is using pin5 which means CS 2 and 3 should be disabled (high)
#define	CS_2		PIN5|PIN9		// is using pin8 which means CS 1 and 3 should be disabled
#define	CS_3		PIN5|PIN8		// is using pin9 which means CS 1 and 2 should be disabled
#define	CS_ALL		PIN5|PIN8|PIN9
#define RESET_POWER RESET|POWER


int initPCD8544 (TREGISTEREDDRIVERS *rd)
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
	dd.optTotal = 3;

	l_strcpy(dd.name, "PCD8544:SIO");
	l_strcpy(dd.comment, "PCD8544 display driver. CS:1 = LPT Pin 5 (D3)");
	dnumber = registerDisplayDriver(rd, &dd);
	setDefaultDisplayOption(rd, dnumber, lOPT_PCD8544_CS, CS_1);
	setDefaultDisplayOption(rd, dnumber, lOPT_PCD8544_HFLIP, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_PCD8544_INVERT, 0);
	
	l_strcpy(dd.name, "PCD8544:SIO:2");
	l_strcpy(dd.comment, "PCD8544 display driver. CS:2 = LPT Pin 8 (D6)");
	dnumber = registerDisplayDriver(rd, &dd);
	setDefaultDisplayOption(rd, dnumber, lOPT_PCD8544_CS, CS_2);
	setDefaultDisplayOption(rd, dnumber, lOPT_PCD8544_HFLIP, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_PCD8544_INVERT, 0);
		
	l_strcpy(dd.name, "PCD8544:SIO:3");
	l_strcpy(dd.comment, "PCD8544 display driver. CS:3 = LPT Pin 9 (D7)");
	dnumber = registerDisplayDriver(rd, &dd);
	setDefaultDisplayOption(rd, dnumber, lOPT_PCD8544_CS, CS_3);
	setDefaultDisplayOption(rd, dnumber, lOPT_PCD8544_HFLIP, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_PCD8544_INVERT, 0);

	return (dnumber > 0);
}

void closePCD8544 (TREGISTEREDDRIVERS *rd)
{
	return;
}

int pcd8544_setOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (drv){
		intptr_t *opt = drv->dd->opt;
		opt[option] = *value;
		return 1;
	}else{
		return 0;
	}
}

int pcd8544_getOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (drv && value){
		intptr_t *opt = drv->dd->opt;
		*value = opt[option];
		return 1;
	}else{
		return 0;
	}
}

int pcd8544_OpenDisplay (TDRIVER *drv)
{
	
	if (drv){
		if (drv->dd->status == LDRV_CLOSED){
			drv->dd->back = _newFrame(drv->dd->hw, drv->dd->width, drv->dd->height, 1, LFRM_BPP_1);
			if (drv->dd->back){
				drv->dd->status = LDRV_READY;
				pcd8544_initiate(drv);
				pcd8544_ClearDisplay(drv);
				return 1;
			}
		}
	}
	return 0;
}

int pcd8544_CloseDisplay (TDRIVER *drv)
{

	if (drv){
		if (drv->dd->status == LDRV_READY){
			pcd8544_ClearDisplay(drv);
			pcd8544_shutdown(drv);
			if (drv->dd->back)
				deleteFrame(drv->dd->back);
			drv->dd->status = LDRV_CLOSED;
			return 1;
		}
	}
	return 0;
}

int pcd8544_ClearDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status == LDRV_READY){
			pcd8544_set_xaddr(drv, 0);
			pcd8544_set_yaddr(drv, 0);

			int buffersize = (drv->dd->WIDTH * drv->dd->HEIGHT)>>3;
			while(buffersize--)
				pcd8544_write_data(drv, drv->dd->clr);
		
			pcd8544_set_xaddr(drv, 0);
			pcd8544_set_yaddr(drv, 0);
			return 1;
		}
	}
	return 0;
}


void pcd8544_write_data (TDRIVER *drv, ubyte data)
{

	intptr_t cs=0;
	pcd8544_getOption(drv, lOPT_PCD8544_CS, &cs);
	int i=8;

	while(i--){
		if (data&0x80){
			data <<= 1;	
			drv->pd->write8(drv->dd->port, cs | SDA |        DC | RESET_POWER);
			drv->pd->write8(drv->dd->port, cs | SDA | SCLK | DC | RESET_POWER);
		}else{
			data <<= 1;	
			drv->pd->write8(drv->dd->port, cs |        DC | RESET_POWER);
			drv->pd->write8(drv->dd->port, cs | SCLK | DC | RESET_POWER);
		}
	}

	// bringing chip select high disengages controller
//	drv->pd->write8(drv->dd->port, CS_ALL | DC | RESET_POWER);
}

void pcd8544_write_command (TDRIVER *drv, ubyte data)
{

	intptr_t cs=0;
	pcd8544_getOption(drv, lOPT_PCD8544_CS, &cs);
	int i=8;
	
	while(i--){
		if (data&0x80){
			data <<= 1;	
			drv->pd->write8(drv->dd->port, cs | SDA |        RESET_POWER);
			drv->pd->write8(drv->dd->port, cs | SDA | SCLK | RESET_POWER);
		}else{
			data <<= 1;	
			drv->pd->write8(drv->dd->port, cs |        RESET_POWER);
			drv->pd->write8(drv->dd->port, cs | SCLK | RESET_POWER);
		}
	}

	// bringing chip select high disengages controller
//	drv->pd->write8(drv->dd->port, CS_ALL | RESET_POWER);
}

int pcd8544_Refresh (TDRIVER *drv, TFRAME *frm)
{
	if (!frm||!drv)
		return 0;
	else if (drv->dd->status == LDRV_READY){
		if (frm->hw->caps[CAP_BACKBUFFER] == CAP_STATE_ON){
			quadrefresh(drv, frm);
			drv->pd->flush(drv->pd);
			return 1;
		}else{
			pcd8544_RefreshArea(drv, frm, 0, 0, frm->width-1, frm->height-1);
			drv->pd->flush(drv->pd);
			return 1;
		}
	}
	return 0;
}

int pcd8544_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{

	if (!frm||!drv){
		return 0;
	}
	
	if (drv->dd->status != LDRV_READY)
		return 0;
		
	//if ((x2<x1) || (y2<y1))
	//	return 0;

	// sync back buffer with front
	if (frm->hw->caps[CAP_BACKBUFFER]==CAP_STATE_ON)
		pcd8544_copyArea(frm, drv->dd->back, x1, y1, x1, y1, x2, y2);

	x2 = MIN(MIN(frm->width, drv->dd->width), x2+1);
	y2 = MIN(MIN(frm->height, drv->dd->height), y2+1);
	x1 = MAX(x1,0);
	y1 = MAX(y1,0);
	
	int flip=0;
	pcd8544_getOption(drv, lOPT_PCD8544_HFLIP, (intptr_t*)&flip);
	if (flip)
		return pcd8544_updateAreaFlippedH(drv, frm,x1,y1,x2,y2);
	else
		return pcd8544_updateArea(drv,frm,x1,y1,x2,y2);
}


//static int lastx = -1;

int pcd8544_updateArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	
	ubyte b[(x2-x1)+4];
	int o=y1&7;
	int p=y1>>3;
	int x,y=0;
	int bct;
	int y_o;
	y2 +=o;

	if (y2 > frm->height-1)
		y2 = frm->height-1;

	for (y=y1; y<y2; y=y+8){
		bct=0;
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
		pcd8544_set_yaddr(drv, p++);
		
		//if (x1 != lastx)
			pcd8544_set_xaddr(drv, x1);
		//lastx = x1+bct;	
		writeBytes(drv, b, bct);
	}
	return 1;
}

int pcd8544_updateAreaFlippedH (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{

	ubyte b[(x2-x1)+1];
	int o=y1&7;
	int p=y1>>3;
	int x,y=0;
	int bct;
	int y_o;
	int height = frm->height;
	int w = drv->dd->width;
	y2 +=o;


	for (y=y1; y<y2; y=y+8){
		bct=0;
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

		pcd8544_set_yaddr(drv, p);
		pcd8544_set_xaddr(drv, w - x1 - bct);
		writeBytes(drv, b, bct);
	}

	return 1;
}

static INLINE void writeBytes (TDRIVER *drv, ubyte *values, int tbytes)
{
//	if (drv->dd->currentColumn > (drv->dd->width-1)) return;

	while(tbytes--){
		pcd8544_write_data(drv, *(values++));
//		if (drv->dd->currentColumn++ > (drv->dd->width-1))
//			return;
 	}
}

void pcd8544_set_xaddr (TDRIVER *drv, ubyte x)
{
//	drv->dd->currentColumn = x;
	pcd8544_write_command(drv, 0x80 | (x&0x7F));
}

void pcd8544_set_yaddr (TDRIVER *drv, ubyte y)
{
//	drv->dd->currentRow = y;
	pcd8544_write_command(drv, 0x40 | (y&0x07));	
}

void pcd8544_initiate (TDRIVER *drv)
{
	if (!initOnce){
		initOnce = 0xFF;
		// Power On
		drv->pd->write8(drv->dd->port, RESET_POWER);
		drv->pd->write8(drv->dd->port, POWER);
		lSleep(1);
		drv->pd->write8(drv->dd->port, RESET_POWER);
	}

	// command sequence borrowed from LCDInfo
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
		
	//let display settle
	//lSleep(300);
}

static void pcd8544_shutdown (TDRIVER *drv)
{
	if (initOnce){
		initOnce = 0;
		drv->pd->write8(drv->dd->port, POWER);
		lSleep(1);
		drv->pd->write8(drv->dd->port, 0);
	}
}

static INLINE int cmpquad (TFRAME *a, TFRAME *b, int x, int y, int *i)
{
	int j;
	int xPLUS8 = MIN(x+quadBoxW-1, a->width-1);
	int yPLUS8 = MIN(y+quadBoxH, a->height);

	x--;
	for (*i=xPLUS8;*i>x;(*i)--)
		for (j=y;j<yPLUS8; j++)
			if (getPixel_NB(a,*i,j) ^ getPixel_NB(b,*i,j))
				return 0;
	return 1;
}

static INLINE int quadrefresh (TDRIVER *drv, TFRAME *frm)
{
	int x2 = MIN(frm->width, drv->dd->width);
	int y2 = MIN(frm->height, drv->dd->height);
	int x1 = 0, y1 = 0;
	int updates = 0;
	int x,y,i;
	TFRAME *back = drv->dd->back;
	
	
	for (y=y1;y<y2; y+= quadBoxH){
		for (x=x1;x<x2; x+= quadBoxW){
			if (!cmpquad(frm, back, x, y, &i)){
				pcd8544_RefreshArea(drv, frm, x, y, i, y+quadBoxH-1);
				updates++;
			}
		}
	}
	return updates;
}


static INLINE void pcd8544_copyArea (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2)
{
	if ((dx>to->width-1) || (dy>to->height-1))
		return; 
	
	int x,y;
	int xx=dx;
	y2=MIN(y2+1, from->height);
	x2=MIN(x2+1, from->width);
	y1=MIN(y1, from->height);
	x1=MIN(x1, from->width);
	
	for (y=y1;y<y2;y++,dy++){
		xx=dx;
		for (x=x1;x<x2;x++,xx++)
			setPixel_NB(to, xx, dy, getPixel_NB(from,x,y));
	}
	return;
}


#else

int initPCD8544(void *rd){return 1;}
void closePCD8544(void *rd){return;}

#endif

