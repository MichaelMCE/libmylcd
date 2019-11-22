
// S1D15G14 display driver as found in LCD's from the Nokia 3510i
// incomplete

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

#if (__BUILD_S1D15G14__)

#include "../frame.h"
#include "../device.h"
#include "../display.h"
#include "../memory.h"
#include "../pixel.h"
#include "../draw.h"
#include "../copy.h"
#include "../lstring.h"
#include "s1d15g14_sio.h"

static int initOnce = 0;

static int nokia3510i_OpenDisplay (TDRIVER *drv);
static int nokia3510i_CloseDisplay (TDRIVER *drv);
static int nokia3510i_ClearDisplay (TDRIVER *drv);
static int nokia3510i_Refresh (TDRIVER *drv, TFRAME *frm);
static int nokia3510i_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static int nokia3510i_setOption (TDRIVER *drv, int option, intptr_t *value);
static int nokia3510i_getOption (TDRIVER *drv, int option, intptr_t *value);
static int nokia3510i_updateArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static void nokia3510i_initiate (TDRIVER *drv);
static void nokia3510i_shutdown (TDRIVER *drv);

static INLINE void nokia3510i_set_contrast (TDRIVER *drv, ubyte contrast);
static INLINE void nokia3510i_set_updateArea (TDRIVER *drv, ubyte x1, ubyte y1, ubyte x2, ubyte y2);
static int nokia3510i_updateArea_8bpp (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static int nokia3510i_updateArea_12bpp (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static INLINE void s1d15g14clearDisplay (TDRIVER *drv);

static INLINE int quadrefresh (TDRIVER *drv, TFRAME *frm);
static INLINE int cmpquad (TFRAME *a, TFRAME *b, int x, int y, int *col);
static INLINE void nokia3510i_copyArea (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2);
static INLINE void cs (TDRIVER *drv);




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

#define RESET		PIN2		// d0
#define	CE			PIN3		// d1
#define SI			PIN4		// d2, SDA
#define SCLK		PIN5		// d3
#define POWER		NONE
#define RESET_POWER RESET|POWER

#define quadBoxW	6
#define quadBoxH	6



int initS1D15G14 (TREGISTEREDDRIVERS *rd)
{
	TDISPLAYDRIVER dd;

	l_strcpy(dd.name, "S1D15G14:SIO");
	l_strcpy(dd.comment, "Nokia 3510i (s1d15g14) display driver");
	dd.open = nokia3510i_OpenDisplay;
	dd.close = nokia3510i_CloseDisplay;
	dd.clear = nokia3510i_ClearDisplay;
	dd.refresh = nokia3510i_Refresh;
	dd.refreshArea = nokia3510i_RefreshArea;
	dd.setOption = nokia3510i_setOption;
	dd.getOption = nokia3510i_getOption;
	dd.status = LDRV_CLOSED;
	dd.optTotal = 0;
	
	int dnumber = registerDisplayDriver(rd, &dd);
	//setDefaultDisplayOption(rd, dnumber, lOPT_N3510I_ , 0);
	return (dnumber > 0);
}


void closeS1D15G14 (TREGISTEREDDRIVERS *rd)
{
	return;
}

static void send_cmd (TDRIVER *drv, ubyte cmd)
{

	drv->pd->write8(drv->dd->port,         RESET_POWER);
	drv->pd->write8(drv->dd->port,  SCLK | RESET_POWER);
	
	ubyte i = 8;
	while(i--){
		if (cmd&0x80){
			cmd <<= 1;
			drv->pd->write8(drv->dd->port, SI |        RESET_POWER);
			drv->pd->write8(drv->dd->port, SI | SCLK | RESET_POWER);
		}else{
			cmd <<= 1;
			drv->pd->write8(drv->dd->port,        RESET_POWER);
			drv->pd->write8(drv->dd->port, SCLK | RESET_POWER);
		}
	}
}

static void send_data (TDRIVER *drv, ubyte data)
{
	
	drv->pd->write8(drv->dd->port, SI |        RESET_POWER);
	drv->pd->write8(drv->dd->port, SI | SCLK | RESET_POWER);

	ubyte i = 8;
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
}

static INLINE void cs (TDRIVER *drv)
{
	drv->pd->write8(drv->dd->port, CE | SCLK | RESET_POWER);
}

void colourSet (TDRIVER *drv)
{
	send_cmd(drv, 0x2D);	

	send_data(drv, 0);		// red
	send_data(drv, 2);
	send_data(drv, 4);
	send_data(drv, 6);
	send_data(drv, 8);
	send_data(drv, 10);
	send_data(drv, 13);
	send_data(drv, 15);

	send_data(drv, 0);		// green
	send_data(drv, 2);
	send_data(drv, 4);
	send_data(drv, 6);
	send_data(drv, 8);
	send_data(drv, 10);
	send_data(drv, 12);
	send_data(drv, 15);

	send_data(drv, 0);		// blue
	send_data(drv, 4);
	send_data(drv, 8);
	send_data(drv, 15);
	cs(drv);
}

int nokia3510i_setOption (TDRIVER *drv, int option, intptr_t *value)
{
	return 0;
}

int nokia3510i_getOption (TDRIVER *drv, int option, intptr_t *value)
{
	return 0;
}

int nokia3510i_OpenDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status == LDRV_CLOSED){
			if (drv->dd->bpp == LFRM_BPP_12){
				drv->dd->back = _newFrame(drv->dd->hw, drv->dd->width, drv->dd->height, 1, LFRM_BPP_12);
				drv->dd->clr = 0x000;
			}else{
				drv->dd->back = _newFrame(drv->dd->hw, drv->dd->width, drv->dd->height, 1, LFRM_BPP_8);
				drv->dd->clr = 0x00;
			}
			
			if (drv->dd->back){
				drv->dd->status = LDRV_READY;
				nokia3510i_initiate(drv);
				return nokia3510i_ClearDisplay(drv);
			}
		}
	}
	return 0;
}

int nokia3510i_CloseDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status != LDRV_CLOSED){
			drv->dd->status = LDRV_CLOSED;
			nokia3510i_shutdown(drv);
			if (drv->dd->back)
				deleteFrame(drv->dd->back);
			return 1;
		}
	}
	return 0;
}

int nokia3510i_ClearDisplay (TDRIVER *drv)
{
	if (!drv)
		return 0;
	else{
		if (drv->dd->status == LDRV_READY){
			s1d15g14clearDisplay(drv);
			return 1;
		}
	}
	return 0;
}

int nokia3510i_Refresh (TDRIVER *drv, TFRAME *frm)
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
				nokia3510i_RefreshArea(drv, frm, 0, 0, drv->dd->width-1, drv->dd->height-1);
				drv->pd->flush(drv->pd);
				return 1;
			}
		}
	}
	return 0;
}

static int nokia3510i_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	if (!frm || !drv || (unsigned int)x2<x1 || (unsigned int)y2<y1)
		return 0;
	
	if (drv->dd->status != LDRV_READY)
		return 0;

	x2 = MIN(MIN(frm->width, drv->dd->width), (unsigned int)x2+1);
	y2 = MIN(MIN(frm->height, drv->dd->height), (unsigned int)y2+1);
	x1 = MAX(x1,0);
	y1 = MAX(y1,0);
	
	return nokia3510i_updateArea(drv, frm,x1,y1,x2,y2);
}

int nokia3510i_updateArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	// update back buffer
	if (frm->hw->caps[CAP_BACKBUFFER]==CAP_STATE_ON)
		nokia3510i_copyArea(frm, drv->dd->back, x1, y1, x1, y1, x2, y2);

	if (frm->bpp == LFRM_BPP_12)
		return nokia3510i_updateArea_12bpp(drv, frm, x1, y1, x2, y2);
	else /*if (frm->bpp == LFRM_BPP_8)*/
		return nokia3510i_updateArea_8bpp(drv, frm, x1, y1, x2, y2);
}

static int nokia3510i_updateArea_12bpp (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	nokia3510i_set_updateArea(drv, x1, y1 , x2++, y2++);
	send_cmd(drv, 0x2C);

	unsigned int c1, c2;
	int x,y;

	for (y=y1; y<y2; y++){
		for (x=x1; x<x2; x++){
			c1 = l_getPixel_NB(frm, x++, y);
			c2 = l_getPixel_NB(frm, x, y);
			send_data(drv, (c1>>4)&0xFF);
			send_data(drv, (c1&0x0F)<<4 | ((c2>>8)&0x0F));
			send_data(drv, c2&0xFF);
		}
	}
	cs(drv);
	return 1;
}

static int nokia3510i_updateArea_8bpp (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	nokia3510i_set_updateArea(drv, x1, y1 , x2++, y2++);
	send_cmd(drv, 0x2C);

	int x,y;
	for (y=y1; y<y2; y++){
		for (x=x1; x<x2; x++){
			send_data(drv, l_getPixel_NB(frm, x, y));
		}
	}
	cs(drv);
	return 1;
}

static INLINE void s1d15g14clearDisplay (TDRIVER *drv)
{
	nokia3510i_set_updateArea(drv, 0, 0 , drv->dd->width-1, drv->dd->height-1);
	drawRectangleFilled(drv->dd->back, 0, 0, drv->dd->back->width-1, drv->dd->back->height-1, drv->dd->clr);
	send_cmd(drv, 0x2C);		// enable access to pixel ram

	if (drv->dd->bpp == LFRM_BPP_12){
		int x, y,c = drv->dd->clr&0xFFF;
		
		for (y=0; y<drv->dd->height; y++){
			for (x=0; x<drv->dd->width; x += 2){
				send_data(drv, (c>>4)&0xFF);
				send_data(drv, (c&0x0F)<<4 | ((c>>8)&0x0F));
				send_data(drv, c&0xFF);
			}
		}
	}else if (drv->dd->bpp == LFRM_BPP_8){
		int x, y,c = drv->dd->clr&0xFF;

		for (y=0; y<drv->dd->height; y++){
			for (x=0; x<drv->dd->width; x++)
				send_data(drv, c);
		}
	}
	cs(drv);
}

static INLINE void nokia3510i_set_updateArea (TDRIVER *drv, ubyte x1, ubyte y1, ubyte x2, ubyte y2)
{
	//drv->dd->currentColumn = x;
	//drv->dd->currentRow = y;
	send_cmd(drv, 0x2A);
	send_data(drv, x1);
	send_data(drv, x2);
	cs(drv);
	send_cmd(drv, 0x2B);
	send_data(drv, y1);
	send_data(drv, y2);
	cs(drv);
}

static INLINE void nokia3510i_set_contrast (TDRIVER *drv, ubyte contrast)
{
	send_cmd(drv, 0x25);
	send_data(drv, contrast);
	cs(drv);
}

void nokia3510i_initiate (TDRIVER *drv)
{

	// power up
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

	send_cmd(drv, 0x01);	// LCD Software Reset
	cs(drv);
	drv->pd->flush(drv->pd);
	lSleep(1);
	
	send_cmd(drv, 0xC6);	// Initial Escape
	cs(drv);
	
	send_cmd(drv, 0xB9);		// Refresh set
	send_data(drv, 8|16|32);
	cs(drv);
	
	send_cmd(drv, 0xB6);		// Display Control
	send_data(drv, 128);
	send_data(drv, 128);
	send_data(drv, 129);
	send_data(drv, 84);
	send_data(drv, 69);
	send_data(drv, 82);
	send_data(drv, 67);
	cs(drv);
	
	send_cmd(drv, 0xB3);		// Gray Scale Position
	send_data(drv, 1);
	send_data(drv, 2);
	send_data(drv, 4);
	send_data(drv, 8);
	send_data(drv, 16);
	send_data(drv, 30);
	send_data(drv, 40);
	send_data(drv, 50);
	send_data(drv, 60);
	send_data(drv, 70);
	send_data(drv, 80);
	send_data(drv, 90);
	send_data(drv, 100);
	send_data(drv, 110);
	send_data(drv, 127);
	cs(drv);
	
	send_cmd(drv, 0xB5);		// Gamma Curve Set
	send_data(drv, 1);
	cs(drv);
	
	send_cmd(drv, 0xBE);		// Power Control
	send_data(drv, 4);
	cs(drv);
	
	send_cmd(drv, 0x11);		// Sleep out
	cs(drv);
	
	send_cmd(drv, 0xB7);		// Temperature gradient set
	int i;
	for(i=0; i<14; i++)
		send_data(drv, 0);
	cs(drv);
	
	send_cmd(drv, 0x03);		// Booster Voltage ON
	cs(drv);
	drv->pd->flush(drv->pd);
	lSleep(40);
	
	send_cmd(drv, 0x21);		// Inversion control
	cs(drv);
	
	send_cmd(drv, 0x25);		// Write contrast
	send_data(drv, 90);
	cs(drv);

	send_cmd(drv, 0x3A);		// set pixel format
	//send_data(drv, 0x02);		// 8bit per pixel
	if (drv->dd->bpp == LFRM_BPP_12){
		send_data(drv, 0x03);		// 12bits per pixel
		cs(drv);
	}else{
		send_data(drv, 0x02);		// 8bits RGB 
		cs(drv);
		colourSet(drv);
	}

	send_cmd(drv, 0x29);		// Display On
	cs(drv);

	drv->pd->flush(drv->pd);
	lSleep(10);

}

void nokia3510i_shutdown (TDRIVER *drv)
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


static INLINE int cmpquad (TFRAME *a, TFRAME *b, int x, int y, int *i)
{
	int xPLUS8 = MIN(x+quadBoxW-1, a->width-1);
	int yPLUS8 = MIN(y+quadBoxH, a->height);
	int j;

	x--;
	for (*i=xPLUS8; *i>x; (*i)--){
		for (j=y; j<yPLUS8; j++){
			if (l_getPixel_NB(a,*i,j) ^ l_getPixel_NB(b,*i,j))
				return 0;
		}
	}
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

	for (y=y1; y<y2; y+= quadBoxH){
		for (x=x1; x<x2; x+= quadBoxW){
			if (!cmpquad(frm, back, x, y, &i)){
				nokia3510i_RefreshArea(drv, frm, x, y, i, y+quadBoxH-1);
				updates++;
			}
		}
	}
	
	return updates;
}


static INLINE void nokia3510i_copyArea (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2)
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
			l_setPixel(to, xx, dy, l_getPixel_NB(from,x,y));
	}
	return;
}


#else

int initS1D15G14(void *rd){return 1;}
void closeS1D15G14(void *rd){return;}

#endif

