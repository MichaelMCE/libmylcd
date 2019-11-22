
// SED1565 (KS0723/S6B1713) display driver, found in the Nokia 7110
// http://sandiding.tripod.com/nokialcd7110.html

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

#if (__BUILD_SED1565__)

#include "../frame.h"
#include "../device.h"
#include "../display.h"
#include "../memory.h"
#include "../pixel.h"
#include "../copy.h"
#include "../lstring.h"
#include "sed1565sio.h"


#define SED1565_X_OFFSET	18		// 18 is required by Nokia 7110 displays, otherwise set to 0
#define KS0713_X_OFFSET		4		// Samsung KS0713/1713 offset, TODO

static int initOnce = 0;

static int sed1565_OpenDisplay (TDRIVER *drv);
static int sed1565_CloseDisplay (TDRIVER *drv);
static int sed1565_ClearDisplay (TDRIVER *drv);
static int sed1565_Refresh (TDRIVER *drv, TFRAME *frm);
static int sed1565_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static int sed1565_setOption (TDRIVER *drv, int option, intptr_t *value);
static int sed1565_getOption (TDRIVER *drv, int option, intptr_t *value);
static int sed1565_updateArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static void sed1565_initiate (TDRIVER *drv);
static void sed1565_shutdown (TDRIVER *drv);

static INLINE void sed1565_write_data (TDRIVER *drv, ubyte data);
static INLINE void sed1565_write_command (TDRIVER *drv, ubyte data);
static INLINE void sed1565_set_xaddr (TDRIVER *drv, ubyte x);
static INLINE void sed1565_set_yaddr (TDRIVER *drv, ubyte y);
static INLINE void sed1565_set_contrast (TDRIVER *drv, ubyte contrast);
static INLINE void sed1565_set_startLine (TDRIVER *drv, ubyte line);
static INLINE void sed1565_writeBytes (TDRIVER *drv, ubyte *values,int tbytes);

static INLINE void uploadquad (TDRIVER *drv, TFRAME *frm, int x, int y);
static INLINE int quadrefresh (TDRIVER *drv, TFRAME *frm);
static INLINE int cmpquad (TFRAME *a, TFRAME *b, int x, int y, int *col);
static INLINE void sed1565_copyArea (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2);


#define quadBoxW 8
#define quadBoxH 8

// lpt data port
#define PIN2		1	//d0
#define PIN3		2	//d1
#define PIN4		4	//d2
#define PIN5		8	//d3
#define PIN6		16	//d4
#define PIN7		32	//d5
#define PIN8		64	//d6
#define PIN9		128	//d7
#define NONE		0

#define RESET		PIN2
#define	CE			PIN3
#define SI			PIN4
#define DC			PIN5
#define SCLK		PIN6
#define POWER		NONE
#define RESET_POWER RESET|POWER


int initSED1565sio (TREGISTEREDDRIVERS *rd)
{
	TDISPLAYDRIVER dd;

	l_strcpy(dd.name, "SED1565:S");
	l_strcpy(dd.comment, "SED1565 display driver. Using serial interface.");
	dd.open = sed1565_OpenDisplay;
	dd.close = sed1565_CloseDisplay;
	dd.clear = sed1565_ClearDisplay;
	dd.refresh = sed1565_Refresh;
	dd.refreshArea = sed1565_RefreshArea;
	dd.setOption = sed1565_setOption;
	dd.getOption = sed1565_getOption;
	dd.status = LDRV_CLOSED;
	dd.optTotal = 1;
	
	int dnumber = registerDisplayDriver(rd, &dd);
	setDefaultDisplayOption(rd, dnumber, lOPT_SED1565_HFLIP, 0);
	return (dnumber > 0);
}


void closeSED1565sio (TREGISTEREDDRIVERS *rd)
{
	return;
}

static int sed1565_setOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (!drv) return 0;
	
	intptr_t *opt = drv->dd->opt;
		
	if (option == lOPT_SED1565_HFLIP){
		opt[option] = *value;

		if (!*value)
			sed1565_write_command(drv, 0xA1);
		else
			sed1565_write_command(drv, 0xA0);

		drv->pd->flush(drv->pd);
		
		return 1;
	}else
		return 0;
}

static int sed1565_getOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (drv && value){
		const intptr_t *opt = drv->dd->opt;
		*value = (opt[option]);
		return 1;
	}else
		return 0;
}

static int sed1565_OpenDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status == LDRV_CLOSED){
			drv->dd->back = _newFrame(drv->dd->hw, drv->dd->width, drv->dd->height, 1, LFRM_BPP_1);
			if (drv->dd->back){
				drv->dd->status = LDRV_READY;
				sed1565_initiate(drv);
				return sed1565_ClearDisplay(drv);
			}
		}
	}
	return 0;
}


static int sed1565_CloseDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status != LDRV_CLOSED){
			drv->dd->status = LDRV_CLOSED;
			sed1565_shutdown(drv);
			if (drv->dd->back)
				deleteFrame(drv->dd->back);
			return 1;
		}
	}
	return 0;
}

static int sed1565_ClearDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status == LDRV_READY){
			int x,y;
			int pages = drv->dd->height>>3;

			for (y=0; y < pages; y++){
				sed1565_set_yaddr(drv, y);
				sed1565_set_xaddr(drv, 0);
				for (x=0; x<drv->dd->width; x++)
					sed1565_write_data(drv, drv->dd->clr);
			}

			if (drv->dd->height&7){
				sed1565_set_yaddr(drv, pages);
				sed1565_set_xaddr(drv, 0);
				for (x=0; x<drv->dd->width; x++)
					sed1565_write_data(drv, drv->dd->clr);		
			}

			sed1565_write_command(drv, 0xE3);
			return 1;
		}
	}
	return 0;
}

static int sed1565_Refresh (TDRIVER *drv, TFRAME *frm)
{
	
	if (!drv||!frm)
		return 0;
	else{
		if (drv->dd->status == LDRV_READY){
			if (frm->hw->caps[CAP_BACKBUFFER]==CAP_STATE_ON){
				quadrefresh(drv, frm);
				drv->pd->flush(drv->pd);
				return 1;
			}else{
				sed1565_RefreshArea(drv, frm, 0, 0, drv->dd->width-1, drv->dd->height-1);
				drv->pd->flush(drv->pd);
				return 1;
			}
		}
	}
	return 0;
}

static int sed1565_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	
	if (!frm||!drv){
		return 0;
	}
	
	if (drv->dd->status != LDRV_READY)
		return 0;
		
	//if (x2 < x1 || y2 < y1)
		//return 0;

	// update back buffer
	if (frm->hw->caps[CAP_BACKBUFFER]==CAP_STATE_ON)
		sed1565_copyArea(frm, drv->dd->back, x1, y1, x1, y1, x2, y2);

	x2 = MIN(MIN(frm->width, drv->dd->width), x2+1);
	y2 = MIN(MIN(frm->height, drv->dd->height), y2+1);
	x1 = MAX(x1,0);
	y1 = MAX(y1,0);
	
	return sed1565_updateArea(drv, frm,x1,y1,x2,y2);
}


//static int lastx = -1;
//static int lasty = -1;

static int sed1565_updateArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{

	ubyte b[(x2-x1)+1];
	int o=y1&7;
	int p=y1>>3;
	int x,y=0;
	int bct;
	int y_o;
	int height = frm->height;
	y2 += o;


	for (y=y1; y<y2; y=y+8){
		bct=0;
		for (x=x1; x<x2; x++){
			y_o = y-o;

			b[bct] = getPixel_NB(frm, x, y_o);
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

		//if (p != lasty)
			sed1565_set_yaddr(drv, p++);
		//lasty = p++;
		
		//if (x1 != lastx)
			sed1565_set_xaddr(drv, x1);
		//lastx = x1+bct;	

		sed1565_writeBytes(drv, b, bct);
	}
	
	return 1;
}

static INLINE void sed1565_writeBytes (TDRIVER *drv, ubyte *values, int tbytes)
{
	if (drv->dd->currentColumn > (drv->dd->width-1)) return;

	while(tbytes--){
		sed1565_write_data(drv, *(values++));
		if (drv->dd->currentColumn++ > (drv->dd->width-1))
			return;
 	}
}


static INLINE void sed1565_write_data (TDRIVER *drv, ubyte data)
{

	//int cs=0;
	//pcd8544_getOption(drv, lOPT_PCD8544_CS, &cs);
	int i=8;

	while(i--){
		if (data&0x80){
			data <<= 1;	
			drv->pd->write8(drv->dd->port, SI |        DC | RESET_POWER);
			drv->pd->write8(drv->dd->port, SI | SCLK | DC | RESET_POWER);
		}else{
			data <<= 1;	
			drv->pd->write8(drv->dd->port,        DC | RESET_POWER);
			drv->pd->write8(drv->dd->port, SCLK | DC | RESET_POWER);
		}
	}

	// bringing chip select high disengages controller
	drv->pd->write8(drv->dd->port, CE | DC | RESET_POWER);
}

static INLINE void sed1565_write_command (TDRIVER *drv, ubyte data)
{

	//int cs=0;
	//pcd8544_getOption(drv, lOPT_PCD8544_CS, &cs);
	int i=8;

	while(i--){
		if (data&0x80){
			data <<= 1;	
			drv->pd->write8(drv->dd->port, SI |        RESET_POWER);
			drv->pd->write8(drv->dd->port, SI | SCLK | RESET_POWER);
		}else{
			data <<= 1;	
			drv->pd->write8(drv->dd->port,        RESET_POWER);
			drv->pd->write8(drv->dd->port, SCLK | RESET_POWER);
		}
	}

	// bringing chip select high disengages controller
	drv->pd->write8(drv->dd->port, CE | RESET_POWER);
}

static INLINE void sed1565_set_xaddr (TDRIVER *drv, ubyte x)
{
	drv->dd->currentColumn = x;
	x += SED1565_X_OFFSET;
	sed1565_write_command(drv, 0x10 | (x >> 4));
	sed1565_write_command(drv,x&0x0F);
	
}

static INLINE void sed1565_set_yaddr (TDRIVER *drv, ubyte y)
{
	drv->dd->currentRow = y;
	sed1565_write_command(drv, 0xB0 | (y&0x0F));
}

static INLINE void sed1565_set_contrast (TDRIVER *drv, ubyte contrast)
{
	sed1565_write_command(drv, 0x81);
	sed1565_write_command(drv, contrast&0x3F);
}

static INLINE void sed1565_set_startLine (TDRIVER *drv, ubyte line)
{
	sed1565_write_command(drv, 0x40 | (line & 0x3f));
}

static void sed1565_initiate (TDRIVER *drv)
{

	// power up
	if (!initOnce){
		initOnce = 0xFF;
		// Power On
		drv->pd->write8(drv->dd->port, RESET_POWER);
		drv->pd->write8(drv->dd->port,       POWER);
		lSleep(1);
		drv->pd->write8(drv->dd->port, RESET_POWER);
		lSleep(1);
	}

	sed1565_write_command(drv, 0xE2);	// soft reset
    sed1565_write_command(drv, 0xA3);	// 1/7 bias
    sed1565_write_command(drv, 0xA1);	// ADC reverse
    sed1565_write_command(drv, 0xC0);	// common output direction normal
    sed1565_write_command(drv, 0x22);	// V5 ratio
    sed1565_write_command(drv, 0x2F);	// Power Control Set Command(page 34): booster+voltage regulator+voltage follower
    sed1565_write_command(drv, 0xE3);	// nop
	sed1565_write_command(drv, 0xAF);	// display on
	sed1565_set_startLine(drv, 0);
	sed1565_set_contrast (drv, 51);
    sed1565_write_command(drv, 0xE3);	// nop
	lSleep(15);
}

static void sed1565_shutdown (TDRIVER *drv)
{
	if (initOnce){
		initOnce = 0;
		drv->pd->write8(drv->dd->port, POWER);
		lSleep(1);
		drv->pd->write8(drv->dd->port, 0);
	}
}


static INLINE void uploadquad (TDRIVER *drv, TFRAME *frm, int x, int y)
{
	sed1565_RefreshArea(drv, frm,x,y,x+quadBoxW-1,y+quadBoxH-1);
}

static INLINE int cmpquad (TFRAME *a, TFRAME *b, int x, int y, int *i)
{
	int xPLUS8 = MIN(x+quadBoxW-1, a->width-1);
	int yPLUS8 = MIN(y+quadBoxH, a->height);
	int j;

	x--;
	for (*i=xPLUS8; *i>x; (*i)--)
		for (j=y; j<yPLUS8; j++)
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
	int i = 0;
	TFRAME *back = drv->dd->back;

	for (int y = y1; y < y2; y += quadBoxH){
		for (int x = x1; x < x2; x += quadBoxW){
			if (!cmpquad(frm, back, x, y, &i)){
				sed1565_RefreshArea(drv, frm, x, y, i, y+quadBoxH-1);
				updates++;
			}
		}
	}
	
	return updates;
}

static INLINE void sed1565_copyArea (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2)
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

int initSED1565sio(void *rd){return 1;}
void closeSED1565sio(void *rd){return;}

#endif

