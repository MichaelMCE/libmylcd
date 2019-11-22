
// KSO108 and compatibles controller driver.

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

#if (__BUILD_KS0108__)

#include "../frame.h"
#include "../memory.h"
#include "../utils.h"
#include "../pixel.h"
#include "../device.h"
#include "../display.h"
#include "../lstring.h"
#include "../copy.h"
#include "ks0108.h"


static void KS0108_sendCommand (TDRIVER *drv, const ubyte value,const ubyte cs);
static void KS0108_setStartLine (TDRIVER *drv, const ubyte line);
static void KS0108_setPage (TDRIVER *drv, const ubyte p);
static void KS0108_setColumn (TDRIVER *drv, const ubyte c);
static void KS0108_displayON (TDRIVER *drv);
static void KS0108_displayOFF (TDRIVER *drv);
static void KS0108_displayReset (TDRIVER *drv);
static void KS0108_sendByte (TDRIVER *drv, const ubyte *values, const int tbytes);
static INLINE void KS0108_delay (TDRIVER *drv, int microseconds);

static int KS0108_Refresh (TDRIVER *drv, TFRAME *frm);
static int KS0108_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static int KS0108_OpenDisplay (TDRIVER *drv);
static int KS0108_ClearDisplay (TDRIVER *drv);
static int KS0108_CloseDisplay (TDRIVER *drv);
static int KS0108_setOption (TDRIVER *drv, int option, intptr_t *value);
static int KS0108_getOption (TDRIVER *drv, int option, intptr_t *value);

static INLINE void uploadquad (TDRIVER *drv, TFRAME *frm, int x, int y);
static INLINE int quadrefresh (TDRIVER *drv, TFRAME *frm);
static INLINE int cmpquad (TFRAME *a, TFRAME *b, int x, int y);


// KS0108 commands
#define DISPLAY_ON			0x3f	//0011 1111
#define DISPLAY_OFF			0x3e	//0011 1110
#define DISPLAY_STARTLINE	0xc0	//1100 0000
#define DISPLAY_PAGE_SET	0xb8	//1011 1000
#define DISPLAY_COLUMN_SET	0x40	//0100 0000


// lpt control port pins
#define PIN1	1		// Strobe
#define PIN14	0		// Line (inverted)
#define PIN16	6		// Init  ?
#define PIN17	8		// Select

#define LCD_ENABLE		PIN1		//0001
#define LCD_CS1			PIN14		//0000
#define LCD_CS2			PIN16		//0110
#define LCD_CD			PIN17		//1000
//#define LCD_CS3		0		//0010


// default delay between read/writes of the ks0108 controller
// use 'KS0108_setOption(drv, lOPT_KS0108_DELAY, delaytime);
static intptr_t DELAYTIME = 2;



int initKS0108 (TREGISTEREDDRIVERS *rd)
{
	TDISPLAYDRIVER dd;
	
	// register a KS0108 driver	
	l_strcpy(dd.name, "KS0108");
	l_strcpy(dd.comment, "KS0108 display driver.");
	dd.open = KS0108_OpenDisplay;
	dd.close = KS0108_CloseDisplay;
	dd.clear = KS0108_ClearDisplay;
	dd.refresh = KS0108_Refresh;
	dd.refreshArea = KS0108_RefreshArea;
	dd.setOption = KS0108_setOption;
	dd.getOption = KS0108_getOption;
	dd.status = LDRV_CLOSED;
	dd.optTotal = 1;
	
	int dnumber = registerDisplayDriver(rd, &dd);
	setDefaultDisplayOption(rd, dnumber, lOPT_KS0108_DELAY, DELAYTIME);
	return (dnumber > 0);
}

void closeKS0108 (TREGISTEREDDRIVERS *rd)
{
	return;
}

static int KS0108_setOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (drv){
		intptr_t *opt = drv->dd->opt;
		opt[option] = (intptr_t)value;
		return 1;
	}else
		return 0;
}

static int KS0108_getOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (drv && value){
		intptr_t *opt = drv->dd->opt;
		*value = (opt[option]);
		return 1;
	}else
		return 0;
}

static int KS0108_OpenDisplay (TDRIVER *drv)
{
	
	if (drv){
		if (drv->dd->status == LDRV_CLOSED){
			drv->dd->back = _newFrame(drv->dd->hw, drv->dd->width, drv->dd->height, 1, LFRM_BPP_1);
			if (drv->dd->back){
				drv->dd->status = LDRV_READY;
				KS0108_displayReset(drv);
				KS0108_displayON(drv);
				return 1;
			}
		}
	}

	return 0;
}

static int KS0108_CloseDisplay (TDRIVER *drv)
{

	if (drv){
		if (drv->dd->status != LDRV_CLOSED){
			drv->dd->status = LDRV_CLOSED;
			KS0108_displayReset(drv);
			if (drv->dd->back)
				deleteFrame(drv->dd->back);
			return 1;
		}
	}

	return 0;
}

static int KS0108_ClearDisplay (TDRIVER *drv)
{
	if (!drv){
		return 0;
	}
	
	if (drv->dd->status != LDRV_READY)
		return 0;
				
	KS0108_getOption(drv, lOPT_KS0108_DELAY, &DELAYTIME);
	ubyte *zero = l_malloc(drv->dd->width);
	if (!zero){
		return 0;
	}

	l_memset (zero, 0, drv->dd->width);
	int x;
	for (x=0;x<8;x++){
		KS0108_setPage(drv, x);
		KS0108_setColumn(drv, 0);
		KS0108_sendByte(drv, zero, drv->dd->width);
    }
	l_free (zero);

	KS0108_setStartLine(drv, 0);
	KS0108_setColumn(drv, 0);
	KS0108_setPage(drv, 0);

	if (drv->dd->hw->caps[CAP_BACKBUFFER]==CAP_STATE_ON)
		clearFrame(drv->dd->back);

	return 1;
}

static int KS0108_Refresh (TDRIVER *drv, TFRAME *frm)
{
	if (!drv||!frm)
		return 0;
	else{
		if (drv->dd->status == LDRV_READY){
			KS0108_getOption(drv, lOPT_KS0108_DELAY, &DELAYTIME);
			if (frm->hw->caps[CAP_BACKBUFFER]==CAP_STATE_ON)
				return quadrefresh(drv, frm);
			else
				return KS0108_RefreshArea(drv, frm, 0, 0, frm->width-1, frm->height-1);
		}
	}

	return 0;
}


static int KS0108_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	if (!frm)
		return 0;
	if ((x2<x1) || (y2<y1))
		return 0;

	if (drv->dd->status != LDRV_READY)
		return 0;
		
	// update back buffer
	if (frm->hw->caps[CAP_BACKBUFFER])
		copyArea(frm, drv->dd->back, x1, y1, x1, y1, x2, y2);

	x2 = MIN(MIN(frm->width, drv->dd->width),x2+1);
	y2 = MIN(MIN(frm->height, drv->dd->height),y2+1);
	x1 = MAX(x1,0);
	y1 = MAX(y1,0);

	ubyte b[(x2-x1)+1];
	int o=y1&7;
	int p=y1>>3;
	int x,y=0;
	int bct;
	y2 +=o;
	int y_frm_yoffset_o2;

	for (y = y1; y < y2; y += 8){
		bct=0;
		for (x=x1; x< x2; x++){
			y_frm_yoffset_o2 = y-o;
			
			b[bct]  = getPixel_NB(frm, x, y_frm_yoffset_o2);
			if (++y_frm_yoffset_o2 < frm->height){
				b[bct] |= getPixel_NB(frm, x, y_frm_yoffset_o2) << 1;
				if (++y_frm_yoffset_o2 < frm->height){
					b[bct] |= getPixel_NB(frm, x, y_frm_yoffset_o2) << 2;
					if (++y_frm_yoffset_o2 < frm->height){
						b[bct] |= getPixel_NB(frm, x, y_frm_yoffset_o2) << 3;
						if (++y_frm_yoffset_o2 < frm->height){
							b[bct] |= getPixel_NB(frm, x, y_frm_yoffset_o2) << 4;
							if (++y_frm_yoffset_o2 < frm->height){
								b[bct] |= getPixel_NB(frm, x, y_frm_yoffset_o2) << 5;
								if (++y_frm_yoffset_o2 < frm->height){
									b[bct] |= getPixel_NB(frm, x, y_frm_yoffset_o2) << 6;
									if (++y_frm_yoffset_o2 < frm->height)
										b[bct]|=getPixel_NB(frm, x, y_frm_yoffset_o2) << 7;
								}
							}
						}
					}
				}
			}
			bct++;
		}
		KS0108_setPage(drv, p++);
		KS0108_setColumn(drv, x1);
		KS0108_sendByte(drv, b, bct);
	}
	return 1;
}


static void KS0108_setPage (TDRIVER *drv, const ubyte x)
{
	KS0108_sendCommand(drv, DISPLAY_PAGE_SET | x, LCD_CS1);
	KS0108_sendCommand(drv, DISPLAY_PAGE_SET | x, LCD_CS2);
	//KS0108_sendCommand(drv, DISPLAY_PAGE_SET | x, LCD_CS3);
	drv->dd->currentRow = x;
}


static void KS0108_setStartLine (TDRIVER *drv, const ubyte line)
{
	KS0108_sendCommand(drv, DISPLAY_STARTLINE | (line & 63), LCD_CS1);
	KS0108_sendCommand(drv, DISPLAY_STARTLINE | (line & 63), LCD_CS2);
	//KS0108_sendCommand(drv, DISPLAY_STARTLINE | (line & 63), LCD_CS3);
}

static void KS0108_setColumn (TDRIVER *drv, const ubyte x)
{
	drv->dd->currentColumn = x;
	
	if (x<64){
		KS0108_sendCommand(drv, DISPLAY_COLUMN_SET | x, LCD_CS1);
		KS0108_sendCommand(drv, DISPLAY_COLUMN_SET | 0, LCD_CS2);
		//KS0108_sendCommand(drv, DISPLAY_COLUMN_SET | 0, LCD_CS3);
	}else if (x<128){
		KS0108_sendCommand(drv, DISPLAY_COLUMN_SET | 0, LCD_CS1);
		KS0108_sendCommand(drv, DISPLAY_COLUMN_SET | (x-64), LCD_CS2);
		//KS0108_sendCommand(drv, DISPLAY_COLUMN_SET | 0, LCD_CS3);
	}
//	else if (x<192){
//		KS0108_sendCommand(drv, DISPLAY_COLUMN_SET | 0, LCD_CS1);
//		KS0108_sendCommand(drv, DISPLAY_COLUMN_SET | 0, LCD_CS2);
//		KS0108_sendCommand(drv, DISPLAY_COLUMN_SET | (x-128), LCD_CS3);
//	}
 
}

static void KS0108_displayReset (TDRIVER *drv)
{
	KS0108_displayOFF(drv);
	KS0108_displayON(drv);
	KS0108_setStartLine(drv, 0);
	KS0108_setColumn(drv, 0);
	KS0108_setPage(drv, 0);
}

static void KS0108_displayOFF (TDRIVER *drv)
{
	KS0108_sendCommand(drv, DISPLAY_OFF,LCD_CS1);
	KS0108_sendCommand(drv, DISPLAY_OFF,LCD_CS2);
	//KS0108_sendCommand(drv,DISPLAY_OFF,LCD_CS3);
}

static void KS0108_displayON (TDRIVER *drv)
{
	KS0108_sendCommand(drv, DISPLAY_ON,LCD_CS1);
	KS0108_sendCommand(drv, DISPLAY_ON,LCD_CS2);
	//KS0108_sendCommand(drv,DISPLAY_ON,LCD_CS3);
}

static void KS0108_sendCommand (TDRIVER *drv, const ubyte value, const ubyte cs)
{
	drv->pd->write8(drv->dd->port, value);
	KS0108_delay(drv, DELAYTIME);
	drv->pd->write8(drv->dd->port+2,			  cs | LCD_CD);
	KS0108_delay(drv, DELAYTIME);
	drv->pd->write8(drv->dd->port+2, LCD_ENABLE | cs | LCD_CD);
	KS0108_delay(drv, DELAYTIME);
}

static void KS0108_sendByte (TDRIVER *drv, const ubyte *values, const int tbytes)
{
    if (drv->dd->currentColumn > (drv->dd->width-1)) return;
    	
	ubyte cs;
	int i;

	for (i=0;i<tbytes;i++){
    	if (drv->dd->currentColumn<64) cs = LCD_CS1;
    	else if (drv->dd->currentColumn<128) cs = LCD_CS2;
    	else return; //drv->dd->currentColumn<192) cs = LCD_CS3

		drv->pd->write8(drv->dd->port,values[i]);
		KS0108_delay(drv, DELAYTIME);
		drv->pd->write8(drv->dd->port+2,cs);
		KS0108_delay(drv, DELAYTIME);
		drv->pd->write8(drv->dd->port+2,LCD_ENABLE | cs);

    	if (drv->dd->currentColumn++ > (drv->dd->width-1)) return;
    	
		KS0108_delay(drv, MAX(DELAYTIME-1,0));
 	}
}

static INLINE void KS0108_delay (TDRIVER *drv, const int microseconds)
{
	if (!microseconds){
		drv->pd->read(drv->dd->port+1);
	}else{
		int i;
		for (i=microseconds;i--;)
			drv->pd->read(drv->dd->port+1);
	}
}

static INLINE void uploadquad (TDRIVER *drv, TFRAME *frm, int x, int y)
{
	KS0108_RefreshArea(drv, frm,x,y,x+quadBoxW-1,y+quadBoxH-1);
}

static INLINE int cmpquad (TFRAME *a, TFRAME *b, int x, int y)
{
	int i,j;
	int xPLUS8=x+quadBoxW;
	int yPLUS8=y+quadBoxH;
	
	for (i=x;i<xPLUS8; i++)
		for (j=y;j<yPLUS8; j++)
			if (getPixel_NB(a,i,j) ^ getPixel_NB(b,i,j))
				return 0;
	return 1;
}

static INLINE int quadrefresh (TDRIVER *drv, TFRAME *frm)
{
	int x2 = MIN(frm->width, drv->dd->width);
	int y2 = MIN(frm->height, drv->dd->height);
	int x1 = 0;
	int y1 = 0;
	int updates=0;
	int x,y;
	
	for (y=y1;y<y2; y+=quadBoxH){
		for (x=x1;x<x2; x+=quadBoxW){
			if (!cmpquad(frm, drv->dd->back, x, y)){
				//copyquad (frm,frm->hw->back,x,y);
				uploadquad(drv, frm,x,y);
				updates++;
			}
		}
	}
	return updates;
}


#else

int initKS0108(void *rd){return 1;}
void closeKS0108(void *rd){return;}
#endif
