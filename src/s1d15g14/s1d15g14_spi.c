
// S1D15G14 driver via USB13700. www.lcdinfo.com for details on the USB board.
// Found in Nokia 3510i LCD
// 

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
#include "s1d15g14_spi.h"



static int s1d15g14_OpenDisplay (TDRIVER *drv);
static int s1d15g14_CloseDisplay (TDRIVER *drv);
static int s1d15g14_ClearDisplay (TDRIVER *drv);
int s1d15g14_Refresh (TDRIVER *drv, TFRAME *frm);
static int s1d15g14_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static int s1d15g14_updateArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
int s1d15g14_updateArea_12bpp (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
int s1d15g14_updateArea_8bpp (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static void s1d15g14_initiate (TDRIVER *drv);
static void s1d15g14_shutdown (TDRIVER *drv);
static int s1d15g14_setOption (TDRIVER *drv, int option, intptr_t *value);
static int s1d15g14_getOption (TDRIVER *drv, int option, intptr_t *value);

static INLINE void s1d15g14_set_contrast (TDRIVER *drv, ubyte contrast);
static INLINE void s1d15g14_set_updateArea (TDRIVER *drv, ubyte x1, ubyte y1, ubyte x2, ubyte y2);
static INLINE void s1d15g14clearDisplay (TDRIVER *drv);

static INLINE int quadrefresh (TDRIVER *drv, TFRAME *frm);
//static INLINE int cmpquad (TFRAME *a, TFRAME *b, int x, int y);
static INLINE void s1d15g14_copyArea (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2);


static void send_data (TDRIVER *drv, ubyte data);
static void send_cmd (TDRIVER *drv, ubyte cmd);
static void set_ctrl (TDRIVER *drv, ubyte ctrl);
static void s1d15g14_sendQueue (TDRIVER *drv);

static int initOnce = 0;



#define RESET		BIT0
#define	CS			BIT1		/* CE */


#define quadBoxW	6
#define quadBoxH	6



int initS1D15G14_SPI (TREGISTEREDDRIVERS *rd)
{
	TDISPLAYDRIVER dd;

	l_strcpy(dd.name, "S1D15G14:SPI");
	l_strcpy(dd.comment, "S1D15G14. CS = USB13700 EXP Pin 5, RS = Pin 6");
	dd.open = s1d15g14_OpenDisplay;
	dd.close = s1d15g14_CloseDisplay;
	dd.clear = s1d15g14_ClearDisplay;
	dd.refresh = s1d15g14_Refresh;
	dd.refreshArea = s1d15g14_RefreshArea;
	dd.setOption = s1d15g14_setOption;
	dd.getOption = s1d15g14_getOption;
	dd.status = LDRV_CLOSED;
	dd.optTotal = 2;
	
	int dnumber = registerDisplayDriver(rd, &dd);
	setDefaultDisplayOption(rd, dnumber, lOPT_U13700D_STRUCT, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_S1D15G14_CONTRAST, 71);
	return (dnumber > 0);
}


void closeS1D15G14_SPI (TREGISTEREDDRIVERS *rd)
{
	return;
}

static void send_palette (TDRIVER *drv)
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
	send_data(drv, 13);
	send_data(drv, 15);

	send_data(drv, 0);		// blue
	send_data(drv, 5);
	send_data(drv, 10);
	send_data(drv, 15);
}


static int s1d15g14_getOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (drv && value){
		if (option < drv->dd->optTotal){
			intptr_t *opt = drv->dd->opt;
			*value = opt[option];
			return 1;
		}
	}
	return 0;
}

static int s1d15g14_setOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (!drv)
		return 0;
	if (option >= drv->dd->optTotal)
		return 0;

	intptr_t *opt = drv->dd->opt;

	if (option == lOPT_U13700D_STRUCT){
		opt[lOPT_U13700D_STRUCT] = (intptr_t)value;

	}else if (option == lOPT_S1D15G14_CONTRAST){
		opt[lOPT_S1D15G14_CONTRAST] = *value;
		s1d15g14_set_contrast(drv, *value);

	}else{
		return 0;
	}
	return 1;
}

static void send_cmd (TDRIVER *drv, ubyte cmd)
{
	drv->pd->write16(drv->dd->port, cmd & ~0x100);
}

static void send_data (TDRIVER *drv, ubyte data)
{
	drv->pd->write16(drv->dd->port, data|0x100);
}

static void set_ctrl (TDRIVER *drv, ubyte ctrl)
{
	drv->pd->write8(drv->dd->port|0x02, ctrl);
}

static void s1d15g14_sendQueue (TDRIVER *drv)
{
	drv->pd->flush(drv->pd);
}

static int s1d15g14_OpenDisplay (TDRIVER *drv)
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
				s1d15g14_initiate(drv);
				s1d15g14_ClearDisplay(drv);
				return 1;
			}
		}
	}
	return 0;
}

static int s1d15g14_CloseDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status != LDRV_CLOSED){
			drv->dd->status = LDRV_CLOSED;
			s1d15g14_shutdown(drv);
			if (drv->dd->back)
				deleteFrame(drv->dd->back);
			return 1;
		}
	}
	return 0;
}

static int s1d15g14_ClearDisplay (TDRIVER *drv)
{	
	if (!drv){
		return 0;
	}else{
		if (drv->dd->status == LDRV_READY){
			s1d15g14clearDisplay(drv);
			return 1;
		}
	}
	return 0;
}

int s1d15g14_Refresh (TDRIVER *drv, TFRAME *frm)
{	
	if (!drv||!frm){
		return 0;
		
	}else{
		if (drv->dd->status == LDRV_READY){
			if (frm->hw->caps[CAP_BACKBUFFER]==CAP_STATE_ON){
				//set_ctrl(drv, RESET);
				if (quadrefresh(drv, frm))
					s1d15g14_sendQueue(drv);
				//set_ctrl(drv, RESET|CS);
				return 1;
			}else{
				//set_ctrl(drv, RESET);
				if (s1d15g14_RefreshArea(drv, frm, 0, 0, drv->dd->width-1, drv->dd->height-1)){
					s1d15g14_sendQueue(drv);
					//set_ctrl(drv, RESET|CS);
					return 1;
				}
			}
		}
	}
	return 0;
}

static int s1d15g14_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	if (!frm || !drv || (unsigned int)x2<x1 || (unsigned int)y2<y1)
		return 0;
	
	if (drv->dd->status != LDRV_READY)
		return 0;

	x2 = MIN(MIN(frm->width, drv->dd->width), (unsigned int)x2);
	y2 = MIN(MIN(frm->height, drv->dd->height), (unsigned int)y2);
	x1 = MAX(x1,0);
	y1 = MAX(y1,0);
	
	return s1d15g14_updateArea(drv, frm, x1, y1, x2, y2);
}

static int s1d15g14_updateArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{	
	// update back buffer
	if (frm->hw->caps[CAP_BACKBUFFER]==CAP_STATE_ON)
		s1d15g14_copyArea(frm, drv->dd->back, x1, y1, x1, y1, x2, y2);

	if (frm->bpp == LFRM_BPP_12)
		return s1d15g14_updateArea_12bpp(drv, frm, x1, y1, x2, y2);
	else /*if (frm->bpp == LFRM_BPP_8)*/
		return s1d15g14_updateArea_8bpp(drv, frm, x1, y1, x2, y2);
}

int s1d15g14_updateArea_12bpp (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	#if 1
	if (!(x2&0x01)){
		if (x2 < frm->width-1)
			x2++;
		else
			x1--;
	}
	#endif

	s1d15g14_set_updateArea(drv, x1, y1 , x2++, y2++);
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
	return 1;
}

int s1d15g14_updateArea_8bpp (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	#if 0
	if (!(x2&0x01)){
		if (x2 < frm->width-1)
			x2++;
		else
			x1--;
	}
	#endif

	s1d15g14_set_updateArea(drv, x1, y1 , x2++, y2++);
	send_cmd(drv, 0x2C);

	int x,y;
	for (y=y1; y<y2; y++){
		for (x=x1; x<x2; x++){
			send_data(drv, l_getPixel_NB(frm, x, y));
		}
	}
	return 1;
}

static INLINE void s1d15g14clearDisplay (TDRIVER *drv)
{
	s1d15g14_set_updateArea(drv, 0, 0 , drv->dd->width-1, drv->dd->height-1);
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

	s1d15g14_sendQueue(drv);
}

static INLINE void s1d15g14_set_updateArea (TDRIVER *drv, ubyte x1, ubyte y1, ubyte x2, ubyte y2)
{
	send_cmd(drv, 0x2A);
	send_data(drv, x1);
	send_data(drv, x2);
	//s1d15g14_sendQueue(drv);
	
	send_cmd(drv, 0x2B);
	send_data(drv, y1);
	send_data(drv, y2);
	//s1d15g14_sendQueue(drv);
}

static INLINE void s1d15g14_set_contrast (TDRIVER *drv, ubyte contrast)
{
	send_cmd(drv, 0x25);
	send_data(drv, contrast&0x7F);
	s1d15g14_sendQueue(drv);
}

static void s1d15g14_initiate (TDRIVER *drv)
{

	// power up
	if (!initOnce){
		initOnce = 0xFF;
		set_ctrl(drv, RESET|CS);
		lSleep(5);
		set_ctrl(drv, CS);
		lSleep(5);
		set_ctrl(drv, RESET|CS);
	}

	set_ctrl(drv, RESET);

	send_cmd(drv, 0x01);	// LCD Software Reset
	s1d15g14_sendQueue(drv);
	lSleep(5);
	
	send_cmd(drv, 0xC6);	// Initialize internal settings
	s1d15g14_sendQueue(drv);

	send_cmd(drv, 0xB9);		// Refresh set
	send_data(drv, 63);
	s1d15g14_sendQueue(drv);	

	send_cmd(drv, 0xB6);		// Display Control
	send_data(drv, 128);
	send_data(drv, 128);
	send_data(drv, 129);
	send_data(drv, 84);
	send_data(drv, 69);
	send_data(drv, 82);
	send_data(drv, 67);
	s1d15g14_sendQueue(drv);

	send_cmd(drv, 0xB3);		// Gray Scale Position
	send_data(drv, 0);
	send_data(drv, 3);
	send_data(drv, 6);
	send_data(drv, 10);
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
	s1d15g14_sendQueue(drv);
	
	send_cmd(drv, 0xB4);		// Gray Scale Position
	send_data(drv, 0);
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
	s1d15g14_sendQueue(drv);
	
		
	send_cmd(drv, 0xB5);		// Gamma Curve Set
	send_data(drv, 1);
	s1d15g14_sendQueue(drv);
	
	send_cmd(drv, 0xBE);		// Power Control
	send_data(drv, 4);
	s1d15g14_sendQueue(drv);
	
	send_cmd(drv, 0x11);		// Sleep out
	s1d15g14_sendQueue(drv);
	
	send_cmd(drv, 0xB7);		// Temperature gradient set
	int i;
	for(i=0; i<14; i++)
		send_data(drv, 0);
	s1d15g14_sendQueue(drv);
	
	send_cmd(drv, 0x03);		// Booster Voltage ON
	s1d15g14_sendQueue(drv);
	lSleep(40);
	
	send_cmd(drv, 0x21);		// Inversion control

	s1d15g14_set_contrast(drv, 71); // Write contrast

	send_cmd(drv, 0x3A);			// set pixel format
	if (drv->dd->bpp == LFRM_BPP_12){
		send_data(drv, 0x03);		// 12bits per pixel
	}else{
		send_data(drv, 0x02);		// 8bits RGB 
		send_palette(drv);
	}

	s1d15g14_sendQueue(drv);

	send_cmd(drv, 0x29);		// Display On
	s1d15g14_sendQueue(drv);

	s1d15g14_sendQueue(drv);
	lSleep(10);

}

static void s1d15g14_shutdown (TDRIVER *drv)
{
	if (initOnce){
		initOnce = 0;
		drv->pd->write8(drv->dd->port, RESET|CS);
		s1d15g14_sendQueue(drv);
	}
}

static INLINE int cmpquad (TFRAME *a, TFRAME *b, int x, int y, int *i)
{
	int xPLUS8 = MIN(x+quadBoxW-1, a->width-1);
	int yPLUS8 = MIN(y+quadBoxH, a->height);
	int j;
	x--;
	for (*i=xPLUS8;*i>x;(*i)--){
		for (j=y;j<yPLUS8; j++){
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
	
	for (y=y1;y<y2; y+= quadBoxH){
		for (x=x1;x<x2; x+= quadBoxW){
			if (!cmpquad(frm, back, x, y, &i)){
				s1d15g14_updateArea(drv, frm, x, y, i, y+quadBoxH-1);
				updates++;
			}
		}
	}
	return updates;
}

static INLINE void s1d15g14_copyArea (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2)
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
		for (x=x1;x<x2;x++,xx++){
			l_setPixel_NB(to, xx, dy, l_getPixel_NB(from,x,y));
		}
	}
	return;
}


#else

int initS1D15G14_SPI(void *rd){return 1;}
void closeS1D15G14_SPI(void *rd){return;}

#endif

