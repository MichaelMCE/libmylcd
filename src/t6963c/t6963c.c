
// T6963C and compatibles controller driver.
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

#if (__BUILD_T6963C__)


#include "../memory.h"
#include "../utils.h"
#include "../pixel.h"
#include "../device.h"
#include "../display.h"
#include "../lstring.h"
#include "t6963c.h"


static int t6963c_displayInit (TDRIVER *drv);
static int t6963c_Refresh (TDRIVER *drv, TFRAME *frm);
static int t6963c_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static int t6963c_Refresh_areahook (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static int t6963c_OpenDisplay (TDRIVER *drv);
static int t6963c_ClearDisplay (TDRIVER *drv);
static int t6963c_CloseDisplay (TDRIVER *drv);
static int t6963c_setOption (TDRIVER *drv, int option, intptr_t *value);
static int t6963c_getOption (TDRIVER *drv, int option, intptr_t *value);

static INLINE void t6963c_sendCommand (TDRIVER *drv, const ubyte cmd);
static INLINE void t6963c_sendData (TDRIVER *drv, const ubyte data);
static INLINE void t6963c_delay (TDRIVER *drv, int microseconds);


// set control port pins, defaults to Pollin wiring
#define PIN1		1		// Strobe
#define PIN14		2		// Line (inverted)
#define PIN15		4		// Init
#define PIN17		8		// Select

#define LCD_RW		PIN1
#define LCD_RD		PIN17
#define LCD_CD		PIN15
#define LCD_CE		PIN14

#define G_BASE		0x0100
#define T_BASE		0x1600

#define quadBoxW	16
#define quadBoxH	8

// default delay between read/writes of the t6963c controller
static int DELAYTIME = 2; // us


int initT6963C (TREGISTEREDDRIVERS *rd)
{
	TDISPLAYDRIVER dd;
	
	// register T6963C driver	
	l_strcpy(dd.name, "T6963C");
	l_strcpy(dd.comment, "T6963C display driver. Pin 1->WR 17->RD 16->A0 14->CS");
	dd.open = t6963c_OpenDisplay;
	dd.close = t6963c_CloseDisplay;
	dd.clear = t6963c_ClearDisplay;
	dd.refresh = t6963c_Refresh;
	dd.refreshArea = t6963c_Refresh_areahook; //t6963c_RefreshArea;
	dd.setOption = t6963c_setOption;
	dd.getOption = t6963c_getOption;
	dd.status = LDRV_CLOSED;
	dd.optTotal = 1;
	
	int dnumber = registerDisplayDriver(rd, &dd);
	setDefaultDisplayOption(rd, dnumber, lOPT_T6963C_DELAY, DELAYTIME);
	return (dnumber > 0);
}

void closeT6963C (TREGISTEREDDRIVERS *rd)
{
	return;
}

static int t6963c_setOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (drv){
		intptr_t *opt = drv->dd->opt;
		if (option == lOPT_T6963C_DELAY){
			DELAYTIME = MAX(*value, 0);
			opt[option] = DELAYTIME;
			return 1;
		}
	}
	return 0;
}

static int t6963c_getOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (drv && value){
		intptr_t *opt = drv->dd->opt;
		*value = opt[option];
		return 1;
	}else
		return 0;
}

static int t6963c_OpenDisplay (TDRIVER *drv)
{
	
	if (drv){
		if (drv->dd->status == LDRV_CLOSED){
			drv->dd->back = NULL;
			drv->dd->status = LDRV_READY;
			t6963c_displayInit(drv);
			return 1;
		}
	}

	return 0;
}

static int t6963c_CloseDisplay (TDRIVER *drv)
{

	if (drv){
		if (drv->dd->status != LDRV_CLOSED){
			t6963c_sendCommand(drv, 0xb3);			// reset auto write mode
			t6963c_sendCommand(drv, 0);
			drv->dd->status = LDRV_CLOSED;
			return 1;
		}
	}else{
		return 0;
	}

	return 0;
}

static int t6963c_ClearDisplay (TDRIVER *drv)
{
	if (!drv){
		return 0;
	}
	
	if (drv->dd->status != LDRV_READY)
		return 0;

	// clear gfx ram
	t6963c_sendData(drv, G_BASE & 0xFF);
	t6963c_sendData(drv, G_BASE >> 8);
  	t6963c_delay(drv,1);
  	t6963c_sendCommand(drv, 0x24);
	t6963c_delay(drv,DELAYTIME);
	t6963c_sendCommand(drv, 0xb0);	// auto write mode on

	int i;
	for (i=5120; i--;)
		t6963c_sendData(drv, drv->dd->clr);
		
	t6963c_sendCommand(drv, 0xb2);	// auto mode off


	// clear text ram
	t6963c_sendData(drv, T_BASE & 0xFF);
	t6963c_sendData(drv, T_BASE >> 8);
  	t6963c_delay(drv,1);
  	t6963c_sendCommand(drv, 0x24);
	t6963c_delay(drv,DELAYTIME);
	t6963c_sendCommand(drv, 0xb0);	// auto write mode on

	for (i=1280; i--;)
		t6963c_sendData(drv, 0);

	t6963c_sendCommand(drv, 0xb2);	// auto mode off

	return 1;
}

static int t6963c_Refresh (TDRIVER *drv, TFRAME *frm)
{
	if (!drv||!frm)
		return 0;
	else if (drv->dd->status == LDRV_READY)
			return t6963c_RefreshArea(drv, frm, 0, 0, frm->width-1, frm->height-1);

	return 0;
}


static int t6963c_Refresh_areahook (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	return drv->dd->refresh(drv,frm);
}


static int t6963c_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	if (!frm){
		return 0;
	}
	if ((x2<x1) || (y2<y1)){
		return 0;
	}

	if (drv->dd->status != LDRV_READY)
		return 0;

	x2 = MIN(MIN(frm->width, drv->dd->width),x2+1);
	y2 = MIN(MIN(frm->height, drv->dd->height),y2+1);
	x1 = MAX(x1,0);
	y1 = MAX(y1,0);

	int x,y;
	ubyte data;

	if (frm->width == drv->dd->WIDTH){
  		t6963c_sendData(drv, G_BASE&0xFF);
  		t6963c_sendData(drv, G_BASE>>8);
  		t6963c_delay(drv,1);
  		t6963c_sendCommand(drv, 0x24);
		t6963c_delay(drv,DELAYTIME);
		t6963c_sendCommand(drv, 0xb0);	// auto write mode on

		for (y = y1; y < y2; y++){
			for (x = x1; x < x2; ){
				data = getPixel_NB(frm, x++, y) << 7;
				data |= getPixel_NB(frm, x++, y) << 6;
				data |= getPixel_NB(frm, x++, y) << 5;
				data |= getPixel_NB(frm, x++, y) << 4;
				data |= getPixel_NB(frm, x++, y) << 3;
				data |= getPixel_NB(frm, x++, y) << 2;
				data |= getPixel_NB(frm, x++, y) << 1;
				data |= getPixel_NB(frm, x++, y);
				t6963c_sendData(drv, data);
			}
		}
		t6963c_sendCommand(drv, 0xb3);	// auto write mode off
		
	}else{	
		int addr;
		int pitch = drv->dd->WIDTH>>3;
		int col = (x1>>3);
		
		for (y = y1; y < y2; y++){

			addr = G_BASE + (y*pitch) + col;
	  		t6963c_sendData(drv, addr&0xFF);
	  		t6963c_sendData(drv, addr>>8);
  			t6963c_sendCommand(drv, 0x24);
  			t6963c_delay(drv,1);
			t6963c_sendCommand(drv, 0xb0);	// auto write mode on
			
			for (x = x1; x < x2; x++){
				data = getPixel_NB(frm, x++, y) << 7;
				data |= getPixel_NB(frm, x++, y) << 6;
				data |= getPixel_NB(frm, x++, y) << 5;
				data |= getPixel_NB(frm, x++, y) << 4;
				data |= getPixel_NB(frm, x++, y) << 3;
				data |= getPixel_NB(frm, x++, y) << 2;
				data |= getPixel_NB(frm, x++, y) << 1;
				data |= getPixel_NB(frm, x, y);
				t6963c_sendData(drv, data);
			}
			t6963c_sendCommand(drv, 0xb3);	// auto write mode off
		}
	}
	return 1;
}


static int t6963c_displayInit (TDRIVER *drv)
{
	
	t6963c_sendCommand(drv, 0xb3);				// switch off auto writes
	
	t6963c_sendData(drv, T_BASE & 0xFF);		// Data1: LowAddress
	t6963c_sendData(drv, T_BASE >> 8);			// Data2: HighAddress
	t6963c_sendCommand(drv, 0x40);				// Command: 0x40 -> 01000000

	t6963c_sendData(drv, drv->dd->WIDTH>>3);	// Data1: Colums
	t6963c_sendData(drv, 0);					// Data2: 0
	t6963c_sendCommand(drv, 0x41);				// Command: 0x41 -> 01000001

	t6963c_sendData(drv, G_BASE & 0xFF);		// Data1: LowAddress
	t6963c_sendData(drv, G_BASE >> 8);			// Data2: HighAddress
	t6963c_sendCommand(drv, 0x42);				// Command: 0x42 -> 01000010

	t6963c_sendData(drv, 30);	// Data1: Colums
	//t6963c_sendData(drv, drv->dd->width>>3);	// Data1: Colums
	t6963c_sendData(drv, 0);					// Data2: 0
	t6963c_sendCommand(drv, 0x43);				// Command: 0x43 -> 01000011

	t6963c_sendData(drv, 0);					// Data2: 0
	t6963c_sendData(drv, 0);					// Data2: 0
	t6963c_sendCommand(drv, 0x22);				// Command: 0x22 -> 00100010

	t6963c_sendCommand(drv, 0x88);				// OR Mode
	t6963c_sendCommand(drv, 0x90);				// gfx off, text off, cursor off
	lSleep(10);

	t6963c_sendCommand(drv, 0x98);				// gfx on, text off, cursor off
	lSleep(10);

	return 1;
}

static INLINE void t6963c_sendCommand (TDRIVER *drv, const ubyte cmd)
{

	drv->pd->write8(drv->dd->port, cmd);
	t6963c_delay(drv,DELAYTIME);
	drv->pd->write8(drv->dd->port+2, 7);		// 1010 CE low
	t6963c_delay(drv,DELAYTIME);
	drv->pd->write8(drv->dd->port+2, 0);		// 0111 C/D low 
}

static INLINE void t6963c_sendData (TDRIVER *drv, const ubyte data)
{
	drv->pd->write8(drv->dd->port+2, 3);		// 0010 CE low
	drv->pd->write8(drv->dd->port, data);
	t6963c_delay(drv,DELAYTIME);
	drv->pd->write8(drv->dd->port+2, 0);		// 0111 WR high 
}


static INLINE void t6963c_delay (TDRIVER *drv, int microseconds)
{
	while(microseconds--)
		drv->pd->read(drv->dd->port+1);
}
#else

int initT6963C(void *rd){return 1;}
void closeT6963C(void *rd){return;}
#endif
