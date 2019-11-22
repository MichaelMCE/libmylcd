
// sed1335 and compatibles controller driver.
// defaults to 'Polin' wiring

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

#if (__BUILD_SED1335__)


#include "../memory.h"
#include "../frame.h"
#include "../utils.h"
#include "../pixel.h"
#include "../device.h"
#include "../display.h"
#include "../lstring.h"
#include "sed1335.h"



static int sed1335_Refresh (TDRIVER *drv, TFRAME *frm);
static int sed1335_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static int sed1335_OpenDisplay (TDRIVER *drv);
static int sed1335_ClearDisplay (TDRIVER *drv);
static int sed1335_CloseDisplay (TDRIVER *drv);
static int sed1335_setOption (TDRIVER *drv, int option, intptr_t *value);
static int sed1335_getOption (TDRIVER *drv, int option, intptr_t *value);

static int sed1335_displayInit (TDRIVER *drv);

static INLINE void sed1335_sendCommand (TDRIVER *drv, const ubyte cmd);
static INLINE void sed1335_sendData (TDRIVER *drv, const ubyte data);
static INLINE void sed1335_delay (TDRIVER *drv, int microseconds);
static INLINE void sed1335_copyArea (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2);
static INLINE int quadrefresh (TDRIVER *drv, TFRAME *frm);
static INLINE int cmpquad (TFRAME *a, TFRAME *b, int x, int y, int *col);



// set control port pins, defaults to Pollin wiring
#define LPT_PIN1	1		// Strobe
#define LPT_PIN14	2		// Line (inverted)
#define LPT_PIN15	4		// Init
#define LPT_PIN17	8		// Select

#define LCD_RW		LPT_PIN1
#define LCD_RD		LPT_PIN17
#define LCD_CD		LPT_PIN15
#define LCD_CE		LPT_PIN14

#define quadBoxW	32	// scan by 32 pixels across
#define quadBoxH	16	// by 16 down


// default delay between read/writes of the sed1335 controller
static int DELAYTIME = 0; // us

// layer control lookup
int layer[]={4,16,64};	// layers 1, 2 and 3



int initSED1335 (TREGISTEREDDRIVERS *rd)
{
	TDISPLAYDRIVER dd;
	
	// register sed1335 driver	
	dd.open = sed1335_OpenDisplay;
	dd.close = sed1335_CloseDisplay;
	dd.clear = sed1335_ClearDisplay;
	dd.refresh = sed1335_Refresh;
	dd.refreshArea = sed1335_RefreshArea;
	dd.setOption = sed1335_setOption;
	dd.getOption = sed1335_getOption;
	dd.status = LDRV_CLOSED;
	dd.optTotal = 4;

	l_strcpy(dd.name, "SED1335");
	l_strcpy(dd.comment, "SED133x display driver. LPT Pin 1->WR, 17->RD, 16->A0, 14->CS\n\t\t6.0Mhz crystal");
	int drvID = registerDisplayDriver(rd, &dd);
	setDefaultDisplayOption(rd, drvID, lOPT_SED1335_DELAY, DELAYTIME);
	setDefaultDisplayOption(rd, drvID, lOPT_SED1335_XTAL, 6000000);
	setDefaultDisplayOption(rd, drvID, lOPT_SED1335_TLAYER, 1);
	setDefaultDisplayOption(rd, drvID, lOPT_SED1335_ALAYER, 0);

	l_strcpy(dd.name, "SED1335:10");
	l_strcpy(dd.comment, "SED133x display driver. LPT Pin 1->WR, 17->RD, 16->A0, 14->CS\n\t\t10.0Mhz crystal");
	drvID = registerDisplayDriver(rd, &dd);
	setDefaultDisplayOption(rd, drvID, lOPT_SED1335_DELAY, DELAYTIME);
	setDefaultDisplayOption(rd, drvID, lOPT_SED1335_XTAL, 10000000);
	setDefaultDisplayOption(rd, drvID, lOPT_SED1335_TLAYER, 1);
	setDefaultDisplayOption(rd, drvID, lOPT_SED1335_ALAYER, 0);
	
	l_strcpy(dd.name, "SED1335:11");
	l_strcpy(dd.comment, "SED133x display driver. LPT Pin 1->WR, 17->RD, 16->A0, 14->CS\n\t\t11.0592Mhz crystal");
	drvID = registerDisplayDriver(rd, &dd);
	setDefaultDisplayOption(rd, drvID, lOPT_SED1335_DELAY, DELAYTIME);
	setDefaultDisplayOption(rd, drvID, lOPT_SED1335_XTAL, 11059200);
	setDefaultDisplayOption(rd, drvID, lOPT_SED1335_TLAYER, 1);
	setDefaultDisplayOption(rd, drvID, lOPT_SED1335_ALAYER, 0);
	
	return (drvID > 0);
}

void closeSED1335 (TREGISTEREDDRIVERS *rd)
{
	return;
}

static int sed1335_setOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (drv){
		intptr_t *opt = drv->dd->opt;
		
		if (option == lOPT_SED1335_DELAY){
			DELAYTIME = MAX(*value,0);
			opt[option] = DELAYTIME;
			return 1;
		}else if (option == lOPT_SED1335_XTAL){
			opt[option] = *value;
			sed1335_displayInit(drv);
			return 1;
		}else if (option == lOPT_SED1335_TLAYER){ 
			if (*value && (*value < 4)){
				opt[option] = *value;
				sed1335_displayInit(drv);
				return 1;
			}
		}else if (option == lOPT_SED1335_ALAYER){
			if (*value < 3){
				opt[option] = *value;
				 drv->dd->currentRow = *value * ((drv->dd->WIDTH>>3) * drv->dd->HEIGHT);
				return 1;
			}
		}		
		
	}
	return 0;
}

static int sed1335_getOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (drv && value){
		intptr_t *opt = drv->dd->opt;
		*value = (opt[option]);
		return 1;
	}else
		return 0;
}

static int sed1335_OpenDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status == LDRV_CLOSED){
			drv->dd->back = _newFrame(drv->dd->hw, drv->dd->width, drv->dd->height, 1, LFRM_BPP_1);
			if (drv->dd->back){
				drv->dd->status = LDRV_READY;
				sed1335_displayInit(drv);
				return 1;
			}
		}
	}

	return 0;
}

static int sed1335_CloseDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status != LDRV_CLOSED){
			sed1335_sendCommand(drv, 0);
			sed1335_sendData(drv, 0);
			if (drv->dd->back){
				deleteFrame(drv->dd->back);
				drv->dd->back = NULL;
			}
			drv->dd->status = LDRV_CLOSED;
			return 1;
		}
	}

	return 0;
}

static int sed1335_ClearDisplay (TDRIVER *drv)
{
	if (!drv){
		
		return 0;
	}
	
	if (drv->dd->status != LDRV_READY)
		return 0;

	sed1335_sendCommand(drv, 0x46);
	sed1335_sendData(drv, 0x00);
	sed1335_sendData(drv, 0x00);
	sed1335_sendCommand(drv, 0x42);

	intptr_t layers;
	sed1335_getOption (drv, lOPT_SED1335_TLAYER, &layers);
	int i = layers * ((drv->dd->WIDTH>>3)*drv->dd->HEIGHT);
	while (i--)
		sed1335_sendData(drv, drv->dd->clr);
		
	return 1;
}

static int sed1335_Refresh (TDRIVER *drv, TFRAME *frm)
{
	if (!drv||!frm){
		return 0;
	}else if (drv->dd->status == LDRV_READY){
		if (frm->hw->caps[CAP_BACKBUFFER] == CAP_STATE_ON){
			quadrefresh(drv, frm);
			drv->pd->flush(drv->pd);
			return 1;
		}else{
			sed1335_RefreshArea(drv, frm, 0, 0, frm->width-1, frm->height-1);
			drv->pd->flush(drv->pd);
			return 1;
		}
	}

	return 0;
}

static int sed1335_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	if (!frm)
		return 0;

	//if ((x2<x1) || (y2<y1))
		return 0;

	if (drv->dd->status != LDRV_READY)
		return 0;

	// sync back buffer with front
	if (frm->hw->caps[CAP_BACKBUFFER] == CAP_STATE_ON)
		sed1335_copyArea(frm, drv->dd->back, x1, y1, x1, y1, x2, y2);

	x2 = MIN(MIN(frm->width, drv->dd->width),x2+1);
	y2 = MIN(MIN(frm->height, drv->dd->height),y2+1);
	x1 = MAX(x1,0);
	y1 = MAX(y1,0);

	ubyte data;
	int addr;
	int pitch = drv->dd->WIDTH>>3;
	int col = (x1>>3) + drv->dd->currentRow; // currentRow = memory offset for active layer
	int x,y;
	int row;

	for (y = y1; y < y2; y++){
		sed1335_sendCommand(drv, 0x46);		// set memory position
		addr = (y*pitch) + col;
		sed1335_sendData(drv, addr&0xFF);	// low byte
		sed1335_sendData(drv, addr>>8);		// high byte
		sed1335_sendCommand(drv, 0x42);		// write to memory command
		row = y*frm->pitch;
		
		for (x = x1; x < x2; x++){
			data = getPixel_NBr(frm, x++, row) << 7;
			data |= getPixel_NBr(frm, x++, row) << 6;
			data |= getPixel_NBr(frm, x++, row) << 5;
			data |= getPixel_NBr(frm, x++, row) << 4;
			data |= getPixel_NBr(frm, x++, row) << 3;
			data |= getPixel_NBr(frm, x++, row) << 2;
			data |= getPixel_NBr(frm, x++, row) << 1;
			data |= getPixel_NBr(frm, x, row);
			sed1335_sendData(drv, data);
		}
	}
	return 1;
}

static int sed1335_displayInit (TDRIVER *drv)
{
	int i;	
	int addr=0;
	intptr_t value;
	
	
	// SYSTEM SET
	drv->dd->getOption(drv, lOPT_SED1335_XTAL, &value);
	sed1335_sendCommand(drv, 0x40);						// SYSTEM SET COMMAND
	sed1335_sendData(drv, 0x30);						// P1   -> PRT=0, IV=1, W/S=0, M0-M2=0
	sed1335_sendData(drv, 0x87);						// FX   -> WF=1, FX=7
	sed1335_sendData(drv, 0x00);						// FY   -> FY=7
	sed1335_sendData(drv, (drv->dd->WIDTH/8)-1);		// C/R  -> Char per line - 1
	sed1335_sendData(drv, (value/70/drv->dd->HEIGHT)/9);	// TC/R -> ( f_osc / f_frame / [L/F] - 1 ) / 9
	sed1335_sendData(drv, drv->dd->HEIGHT-1);			// L/F  -> Line per graphic screen - 1 (127)
	sed1335_sendData(drv, drv->dd->WIDTH/8);			// APL  -> Virtual screen low byte ( char per line)
	sed1335_sendData(drv, 00);							// APH  -> Virtual screen low byte 

	sed1335_getOption (drv, lOPT_SED1335_TLAYER, &value);
	sed1335_sendCommand(drv, 0x44);						// set total layers	and address
	for (i = 0; i < value; i++){
		addr = i * ((drv->dd->WIDTH>>3) * drv->dd->HEIGHT);
		sed1335_sendData(drv, addr&0xFF);				// first Layer Low Byte (0x0000)
		sed1335_sendData(drv, addr>>8);					// first Layer High Byte
		sed1335_sendData(drv, drv->dd->HEIGHT);			// number of rows		
	}

	// Char generator address, to follow gfx
	addr = value * ((drv->dd->WIDTH>>3) * drv->dd->HEIGHT);
	sed1335_sendCommand(drv, 0x5c);						// char generator address (4b00)
	sed1335_sendData(drv, addr&0xFF);					// low byte
	sed1335_sendData(drv, addr>>8);						// high byte

	// HORIZONTAL SCROLL POSITION
	sed1335_sendCommand(drv, 0x5a);						// HORIZONTAL SCROLL POSITION
	sed1335_sendData(drv, 0x00);						// no scroll offset

	// OVERLAY
	sed1335_sendCommand(drv, 0x5b);						// OVERLAY COMMAND
	if (value==3)										// lOPT_SED1335_TLAYER
		sed1335_sendData(drv, 4+8+16);					// 3 layer composition
	else
		sed1335_sendData(drv, 4+8);						// 2 layer - default
	
 	//DISPLAY OFF
	sed1335_sendCommand(drv, 0x58);						// DISPLAY OFF COMMAND
	sed1335_sendData(drv, 0);							// all layers off  

	sed1335_ClearDisplay(drv);							// clear display (all layers)
	if (drv->dd->back)
		clearFrame(drv->dd->back);

	sed1335_getOption (drv, lOPT_SED1335_ALAYER, &value);
	sed1335_sendCommand(drv, 0x59);						// DISPLAY ON COMMAND
	sed1335_sendData(drv, layer[value]);				// active Layer ON  
	// set current active memory address
	drv->dd->currentRow = value * ((drv->dd->WIDTH>>3) * drv->dd->HEIGHT);

	// CURSOR DIRECTION COMMAND
	sed1335_sendCommand(drv, 0x4c);						// CURSOR DIRECTION COMMAND (SHIFT RIGHT)
	return 1;
}

static INLINE void sed1335_sendCommand (TDRIVER *drv, const ubyte cmd)
{

	drv->pd->write8(drv->dd->port, cmd);
	sed1335_delay(drv,DELAYTIME);
	drv->pd->write8(drv->dd->port+2, 7);		// 1010 CE low
	sed1335_delay(drv,DELAYTIME);
	drv->pd->write8(drv->dd->port+2, 0);		// 0111 C/D low 
}

static INLINE void sed1335_sendData (TDRIVER *drv, const ubyte data)
{
	drv->pd->write8(drv->dd->port+2, 3);		// 0010 CE low
	drv->pd->write8(drv->dd->port, data);
	sed1335_delay(drv,DELAYTIME);
	drv->pd->write8(drv->dd->port+2, 0);		// 0111 WR high 
}


static INLINE void sed1335_delay (TDRIVER *drv, int microseconds)
{
	while(microseconds--)
		drv->pd->read(drv->dd->port+1);
}

static INLINE int cmpquad (TFRAME *a, TFRAME *b, int x, int y, int *i)
{
	int j;
	int xPLUS8 = MIN(x+quadBoxW-1, a->width-1);
	int yPLUS8 = MIN(y+quadBoxH, a->height);

	x--;
	for (*i=xPLUS8;*i>x;(*i)--){
		for (j=y; j<yPLUS8; j++){
			if (getPixel_NB(a,*i,j) ^ getPixel_NB(b,*i,j))
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
				sed1335_RefreshArea(drv, frm, x, y, i, y+quadBoxH-1);
				updates++;
			}
		}
	}
	return updates;
}


static INLINE void sed1335_copyArea (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2)
{
	if ((dx>to->width-1) || (dy>to->height-1))
		return; 
	
	int x,y;
	int row;
	int xx=dx;
	y2=MIN(y2+1, from->height);
	x2=MIN(x2+1, from->width);
	y1=MIN(y1, from->height-1);
	x1=MIN(x1, from->width-1);
	
	for (y = y1; y<y2; y++,dy++){
		xx = dx;
		row = y * from->pitch;
		for (x = x1; x<x2; x++,xx++)
			setPixel_NB(to, xx, dy, getPixel_NBr(from, x, row));
	}
	return;
}

#else

int initSED1335(void *rd){return 1;}
void closeSED1335(void *rd){return;}

#endif
