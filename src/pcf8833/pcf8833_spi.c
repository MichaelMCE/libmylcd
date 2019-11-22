
// PCF8833 driver via USB13700. www.lcdinfo.com for details on the USB board.

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

#if (__BUILD_PCF8833__)

#include "../frame.h"
#include "../device.h"
#include "../display.h"
#include "../memory.h"
#include "../pixel.h"
#include "../copy.h"
#include "../lstring.h"
#include "pcf8833_spi.h"



int pcf8833_OpenDisplay (TDRIVER *drv);
int pcf8833_CloseDisplay (TDRIVER *drv);
static int pcf8833_ClearDisplay (TDRIVER *drv);
int pcf8833_Refresh (TDRIVER *drv, TFRAME *frm);
static int pcf8833_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static int pcf8833_updateArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static void pcf8833_initiate (TDRIVER *drv);
static void pcf8833_shutdown (TDRIVER *drv);
static int pcf8833_setOption (TDRIVER *drv, int option, intptr_t *value);
static int pcf8833_getOption (TDRIVER *drv, int option, intptr_t *value);

static INLINE void pcf8833_set_contrast (TDRIVER *drv, int contrast);
static INLINE void pcf8833_set_updateArea (TDRIVER *drv, int x1, int y1, int x2, int y2);
static INLINE void pcf8833clearDisplay (TDRIVER *drv);

static INLINE int quadrefresh (TDRIVER *drv, TFRAME *frm);
static INLINE void pcf8833_copyArea (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2);


static void send_data (TDRIVER *drv, ubyte data);
static void send_cmd (TDRIVER *drv, ubyte cmd);
static void set_ctrl (TDRIVER *drv, ubyte ctrl);
static void pcf8833_sendQueue (TDRIVER *drv);

static int initOnce = 0;



#define RESET		BIT0
#define	CS			BIT1		/* CE */


#define quadBoxW	6
#define quadBoxH	6



int initPCF8833_USB (TREGISTEREDDRIVERS *rd)
{
	TDISPLAYDRIVER dd;

	l_strcpy(dd.name, "PCF8833:SPI");
	l_strcpy(dd.comment, "PCF8833:SPI. CS = USB13700 EXP Pin 5, RS = Pin 6");
	dd.open = pcf8833_OpenDisplay;
	dd.close = pcf8833_CloseDisplay;
	dd.clear = pcf8833_ClearDisplay;
	dd.refresh = pcf8833_Refresh;
	dd.refreshArea = pcf8833_RefreshArea;
	dd.setOption = pcf8833_setOption;
	dd.getOption = pcf8833_getOption;
	dd.status = LDRV_CLOSED;
	dd.optTotal = 2;
	
	int dnumber = registerDisplayDriver(rd, &dd);
	setDefaultDisplayOption(rd, dnumber, lOPT_U13700D_STRUCT, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_PCF8833_CONTRAST, 58);
	return (dnumber > 0);
}


void closePCF8833_USB (TREGISTEREDDRIVERS *rd)
{
	return;
}

static int pcf8833_getOption (TDRIVER *drv, int option, intptr_t *value)
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

static int pcf8833_setOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (!drv)
		return 0;
	if (option >= drv->dd->optTotal)
		return 0;

	intptr_t *opt = drv->dd->opt;

	if (option == lOPT_U13700D_STRUCT){
		opt[lOPT_U13700D_STRUCT] = (intptr_t)value;
		
	}else if (option == lOPT_PCF8833_CONTRAST){
		opt[lOPT_PCF8833_CONTRAST] = *value;
		pcf8833_set_contrast(drv, *value);
		
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

static void pcf8833_sendQueue (TDRIVER *drv)
{
	drv->pd->flush(drv->pd);
}

int pcf8833_OpenDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status == LDRV_CLOSED){
			if (drv->dd->bpp == LFRM_BPP_16){
				drv->dd->back = _newFrame(drv->dd->hw, drv->dd->width, drv->dd->height, 1, LFRM_BPP_16);
				drv->dd->clr = 0x0000;
			}else if (drv->dd->bpp == LFRM_BPP_12){
				drv->dd->back = _newFrame(drv->dd->hw, drv->dd->width, drv->dd->height, 1, LFRM_BPP_12);
				drv->dd->clr = 0x000;
			}else{
				drv->dd->back = _newFrame(drv->dd->hw, drv->dd->width, drv->dd->height, 1, LFRM_BPP_8);
				drv->dd->clr = 0x00;
			}
			if (drv->dd->back){
				drv->dd->status = LDRV_READY;
				pcf8833_initiate(drv);
				pcf8833_ClearDisplay(drv);
				return 1;
			}
		}
	}
	return 0;
}

int pcf8833_CloseDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status != LDRV_CLOSED){
			drv->dd->status = LDRV_CLOSED;
			pcf8833_shutdown(drv);
			if (drv->dd->back)
				deleteFrame(drv->dd->back);
			return 1;
		}
	}
	return 0;
}

static int pcf8833_ClearDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status == LDRV_READY){
			pcf8833clearDisplay(drv);
			return 1;
		}
	}
	return 0;
}

int pcf8833_Refresh (TDRIVER *drv, TFRAME *frm)
{	
	if (!drv||!frm){
		return 0;
		
	}else{
		if (drv->dd->status == LDRV_READY){
			if (frm->hw->caps[CAP_BACKBUFFER]==CAP_STATE_ON){
				//set_ctrl(drv, RESET);
				if (quadrefresh(drv, frm))
					pcf8833_sendQueue(drv);
				//set_ctrl(drv, RESET|CS);
				return 1;
			}else{
				//set_ctrl(drv, RESET);
				if (pcf8833_RefreshArea(drv, frm, 0, 0, drv->dd->width-1, drv->dd->height-1)){
					pcf8833_sendQueue(drv);
					//set_ctrl(drv, RESET|CS);
					return 1;
				}
			}
		}
	}
	return 0;
}

static int pcf8833_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{	
	if (!frm || !drv || x2<x1 || y2<y1)
		return 0;
	
	if (drv->dd->status != LDRV_READY)
		return 0;

	x2 = MIN(MIN(frm->width, drv->dd->width), (unsigned int)x2);
	y2 = MIN(MIN(frm->height, drv->dd->height), (unsigned int)y2);
	x1 = MAX(x1,0);
	y1 = MAX(y1,0);
	
	return pcf8833_updateArea(drv, frm, x1, y1, x2, y2);
}

int pcf8833_updateArea_8bpp (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	#if 0
	if (!(x2&0x01)){
		if (x2 < frm->width-1)
			x2++;
		else
			x1--;
	}
	#endif

	pcf8833_set_updateArea(drv, x1, y1 , x2++, y2++);
	send_cmd(drv, 0x2C);

	int x,y;
	for (y=y1; y<y2; y++){
		for (x=x1; x<x2; x++){
			send_data(drv, l_getPixel_NB(frm, x, y));
		}
	}
	return 1;
}

int pcf8833_updateArea_12bpp (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	#if 1
	if (!(x2&0x01)){
		if (x2 < frm->width-1)
			x2++;
		else
			x1--;
	}
	#endif

	pcf8833_set_updateArea(drv, x1, y1 , x2++, y2++);
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

int pcf8833_updateArea_16bpp (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	#if 0
	if (!(x2&0x01)){
		if (x2 < frm->width-1)
			x2++;
		else
			x1--;
	}
	#endif

	pcf8833_set_updateArea(drv, x1, y1 , x2++, y2++);
	send_cmd(drv, 0x2C);

	int x,y,c16;
	for (y=y1; y<y2; y++){
		for (x=x1; x<x2; x++){
			//x++;
			c16 = l_getPixel_NB(frm, x, y);
			send_data(drv, (c16>>8)&0xFF);
			send_data(drv, c16&0xFF);
		}
	}
	//send_cmd(drv, 00);
	return 1;
}

static int pcf8833_updateArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{	
	// update back buffer
	if (frm->hw->caps[CAP_BACKBUFFER]==CAP_STATE_ON)
		pcf8833_copyArea(frm, drv->dd->back, x1, y1, x1, y1, x2, y2);

	if (frm->bpp == LFRM_BPP_12)
		return pcf8833_updateArea_12bpp(drv, frm, x1, y1, x2, y2);
	else if (frm->bpp == LFRM_BPP_16)
		return pcf8833_updateArea_16bpp(drv, frm, x1, y1, x2, y2);
	else /*if (frm->bpp == LFRM_BPP_8)*/
		return pcf8833_updateArea_8bpp(drv, frm, x1, y1, x2, y2);
}

static INLINE void pcf8833clearDisplay (TDRIVER *drv)
{
	pcf8833_set_updateArea(drv, 0, 0 , drv->dd->width-1, drv->dd->height-1);
	lDrawRectangleFilled(drv->dd->back, 0, 0, drv->dd->back->width-1, drv->dd->back->height-1, drv->dd->clr);
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
	}else if (drv->dd->bpp == LFRM_BPP_16){
		int x, y,c = drv->dd->clr&0xFFFF;
		
		for (y=0; y<drv->dd->height; y++){
			for (x=0; x<drv->dd->width; x++){
				send_data(drv, (c>>8)&0xFF);
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
	//send_cmd(drv, 0x00);
	pcf8833_sendQueue(drv);
}

static INLINE void pcf8833_set_updateArea (TDRIVER *drv, int x1, int y1, int x2, int y2)
{
	//drv->dd->currentColumn = x;
	//drv->dd->currentRow = y;
	
	x1++;
	x2++;
	y1++;
	y2++;
	
	send_cmd(drv, 0x2A);
	send_data(drv, x1);
	send_data(drv, x2);
	//pcf8833_sendQueue(drv);
	
	send_cmd(drv, 0x2B);
	send_data(drv, y1);
	send_data(drv, y2);
	//pcf8833_sendQueue(drv);
}

static INLINE void pcf8833_set_contrast (TDRIVER *drv, int contrast)
{
	send_cmd(drv, 0x25);
	send_data(drv, contrast&0x3F);
	pcf8833_sendQueue(drv);
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

	//pcf8833_sendQueue(drv);
}

static void pcf8833_initiate (TDRIVER *drv)
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

	send_cmd(drv, 0x01);		// LCD Software Reset
	pcf8833_sendQueue(drv);
	lSleep(5);

	send_cmd(drv, 0x11);		// sleep out
	send_cmd(drv, 0x20);		// inversion
	send_cmd(drv, 0x3A);		// set pixel format
	if (drv->dd->bpp == LFRM_BPP_16){
		send_data(drv, 0x05);		// 16bits per pixel
	}else if (drv->dd->bpp == LFRM_BPP_12){
		send_data(drv, 0x03);		// 12bits per pixel
	}else{
		send_data(drv, 0x02);		// 8bits RGB 
		send_palette(drv);
	}

	//send_cmd(drv, 0x36);		// MADCTL
	//send_data(drv, 0xC8);

	pcf8833_set_contrast(drv, 58);
	
	send_cmd(drv, 0x29);		// display on
	pcf8833_sendQueue(drv);
	lSleep(10);

}

static void pcf8833_shutdown (TDRIVER *drv)
{
	if (initOnce){
		initOnce = 0;
		drv->pd->write8(drv->dd->port, RESET|CS);
		pcf8833_sendQueue(drv);
	}
}

static INLINE int cmpquad (TFRAME *a, TFRAME *b, int x, int y, int *i)
{
	int xPLUS8 = MIN(x+quadBoxW-1, a->width-1);
	int yPLUS8 = MIN(y+quadBoxH, a->height);
	int j;
	x--;
	for (*i=xPLUS8;*i>x;(*i)--)
		for (j=y;j<yPLUS8; j++)
			if (l_getPixel_NB(a,*i,j) ^ l_getPixel_NB(b,*i,j))
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
				pcf8833_updateArea(drv, frm, x, y, i, y+quadBoxH-1);
				updates++;
			}
		}
	}
	return updates;
}

static INLINE void pcf8833_copyArea (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2)
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

int initPCF8833_USB(void *rd){return 1;}
void closePCF8833_USB(void *rd){return;}

#endif

