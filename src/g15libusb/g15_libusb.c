
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

#if (__BUILD_G15LIBUSB__)


#include "../memory.h"
#include "../frame.h"
#include "../device.h"
#include "../lstring.h"
#include "../misc.h"
#include "../sync.h"
#include "libg15/libg15.h"
#include "g15_libusb.h"
#include "startframe.h"
#include "endframe.h"




int g15_libusb_OpenDisplay (TDRIVER *drv);
int g15_libusb_CloseDisplay (TDRIVER *drv);
int g15_libusb_ClearDisplay (TDRIVER *drv);
int g15_libusb_Refresh (TDRIVER *drv, TFRAME *frm);
int g15_libusb_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
int g15_libusb_setOption (TDRIVER *drv, int option, intptr_t *value);
int g15_libusb_getOption (TDRIVER *drv, int option, intptr_t *value);


int g15_libusb_update (ubyte *data);
unsigned int __stdcall g15_libusb_keyListener (TMYLCDG15LU *mylcdg15);


static ubyte rbittable[256];
static void create_rbittable ();



int initG15_libusb (TREGISTEREDDRIVERS *rd)
{
	TDISPLAYDRIVER dd;

	l_strcpy(dd.name,"G15:LIBUSB");
	l_strcpy(dd.comment,"G15 display (libUSB)");
	dd.open = g15_libusb_OpenDisplay;
	dd.close = g15_libusb_CloseDisplay;
	dd.clear = g15_libusb_ClearDisplay;
	dd.refresh = g15_libusb_Refresh;
	dd.refreshArea = g15_libusb_RefreshArea;
	dd.setOption = g15_libusb_setOption;
	dd.getOption = g15_libusb_getOption;
	dd.status = LDRV_CLOSED;
	dd.optTotal = 6;
	
	int drvnum = registerDisplayDriver(rd, &dd);
	setDefaultDisplayOption(rd, drvnum, lOPT_G15LU_STRUCT, 0);
	setDefaultDisplayOption(rd, drvnum, lOPT_G15LU_SOFTKEYCB, 0);
	setDefaultDisplayOption(rd, drvnum, lOPT_G15LU_MLEDS, 0);
	setDefaultDisplayOption(rd, drvnum, lOPT_G15LU_KBBRIGHTNESS, 0);
	setDefaultDisplayOption(rd, drvnum, lOPT_G15LU_LCDBRIGHTNESS, 0);
	setDefaultDisplayOption(rd, drvnum, lOPT_G15LU_LCDCONTRAST, 0);
	
	create_rbittable();
	return (drvnum > 0);
}

void closeG15_libusb (TREGISTEREDDRIVERS *rd)
{
	return;
}

static void create_rbittable ()
{
	int i = 256;
	while(i--){
		rbittable[i] = (i&0x80)>>7;
		rbittable[i] |= (i&0x40)>>5;
		rbittable[i] |= (i&0x20)>>3;
		rbittable[i] |= (i&0x10)>>1;
		rbittable[i] |= (i&0x08)<<1;
		rbittable[i] |= (i&0x04)<<3;
		rbittable[i] |= (i&0x02)<<5;
		rbittable[i] |= (i&0x01)<<7;
	}
}

int g15_libusb_setOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (!drv)
		return 0;
	if (option >= drv->dd->optTotal)
		return 0;

	intptr_t *opt = (intptr_t*)drv->dd->opt;

	if (option == lOPT_G15LU_STRUCT){
		opt[lOPT_G15LU_STRUCT] = (intptr_t)value;
		
	}else if (option == lOPT_G15LU_MLEDS){
		opt[lOPT_G15LU_MLEDS] = *value;
		setLEDs(*value);
		
	}else if (option == lOPT_G15LU_KBBRIGHTNESS){
		opt[lOPT_G15LU_KBBRIGHTNESS] = *value;
		setKBBrightness(*value);
	
	}else if (option == lOPT_G15LU_LCDBRIGHTNESS){	
		opt[lOPT_G15LU_LCDBRIGHTNESS] = *value;
		setLCDBrightness(*value);

	}else if (option == lOPT_G15LU_LCDCONTRAST){
		opt[lOPT_G15LU_LCDCONTRAST] = *value;
		setLCDContrast(*value);
		
	}else if (option == lOPT_G15LU_SOFTKEYCB){
		opt[lOPT_G15LU_SOFTKEYCB] = *value;
		TMYLCDG15LU *mylcdg15 = (TMYLCDG15LU*)opt[lOPT_G15LU_STRUCT];
		mylcdg15->softkeycb = (lpsoftkeycb)value;
	}else{
		return 1;
	}
	return 1;
}

int g15_libusb_getOption (TDRIVER *drv, int option, intptr_t *value)
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

int g15_libusb_OpenDisplay (TDRIVER *drv)
{
	if (drv){
		if ((drv->dd->WIDTH != G15_LCD_WIDTH) || (drv->dd->HEIGHT != G15_LCD_HEIGHT)){
			mylog("libmylcd: G15 LCD resolution is %i by %i pixels which is not optional\n", G15_LCD_WIDTH, G15_LCD_HEIGHT);
			drv->dd->WIDTH = G15_LCD_WIDTH;
			drv->dd->HEIGHT = G15_LCD_HEIGHT;
		}
				
		if (drv->dd->status == LDRV_CLOSED){
			TMYLCDG15LU *mylcdg15 = (TMYLCDG15LU*)l_calloc(1, sizeof(TMYLCDG15LU));
			if (mylcdg15){
				g15_libusb_setOption(drv, lOPT_G15LU_STRUCT, (intptr_t*)mylcdg15);
			}else{
				mylog("mylcd: open g15_libusb: malloc error\n");
				return 0;
			}
			drv->dd->back = _newFrame(drv->dd->hw, drv->dd->width, drv->dd->height, 1, drv->dd->bpp);
			if (drv->dd->back){
				if (initLibG15() == G15_NO_ERROR){
					if (g15NumberOfConnectedDevices()){
						libg15Debug(0/*255*/);
						mylcdg15->updateCt = 0;
						newThread(&mylcdg15->hThreadKey, (void*)g15_libusb_keyListener, (void*)mylcdg15);
						drv->dd->status = LDRV_READY;
						//drv->dd->clear(drv);
						g15_libusb_update((ubyte*)startframe160x43);
						return 1;
					}
					exitLibG15();
				}
				mylog("libmylcd: G15 libusb: device not found\n");
				l_free(mylcdg15);
				deleteFrame(drv->dd->back);
			}
		}
	}
	return 0;
}

int g15_libusb_CloseDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status != LDRV_CLOSED){
			drv->dd->status = LDRV_CLOSED;

			TMYLCDG15LU *mylcdg15 = NULL;
			drv->dd->getOption(drv, lOPT_G15LU_STRUCT, (void*)&mylcdg15);
			if (mylcdg15){
				g15_libusb_update((ubyte*)endframe160x43);
				mylcdg15->threadState = 0;
				exitLibG15();
				joinThread(&mylcdg15->hThreadKey);
				closeThreadHandle(&mylcdg15->hThreadKey);
				l_free(mylcdg15);
			}else{
				exitLibG15();
			}
			if (drv->dd->back){
				deleteFrame(drv->dd->back);
			}
			return 1;
		}
	}
	return 0;
}

int g15_libusb_ClearDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status == LDRV_READY){
			unsigned char *data = (unsigned char *)l_calloc(1, G15_FRAMESIZE);
			if (data){
				writePixmapToLCD(data);
				l_free(data);
				if (drv->dd->back)
					clearFrame(drv->dd->back);
				return 1;
			}
		}
	}
	return 0;
}

void g15_libusb_softButtonCB (TMYLCDG15LU *mylcdg15, int keys)
{
	if (mylcdg15 && keys){
		if ((intptr_t)mylcdg15->softkeycb){
			mylcdg15->softkeycb(0, keys>>23, (struct TMYLCDG15LU *)mylcdg15);
		}
	}
	return;
}

unsigned int __stdcall g15_libusb_keyListener (TMYLCDG15LU *mylcdg15)
{
	mylog("g15:libusb g15_libusb_keyListener started\n");
	mylcdg15->threadState = 1;
	unsigned int keys;
	int ret;
	
	while(mylcdg15->threadState){
		keys = 0;
		ret = getPressedKeys(&keys, 1000);
		if (mylcdg15->threadState){
			if (!ret){
				g15_libusb_softButtonCB(mylcdg15, keys);
			}else /*if (ret != -116)*/{
				lSleep(1);
			}
		}
	}
	mylog("g15:libusb g15_libusb_keyListener exited\n");
	return 1;
}

static void byteReverseFrame (ubyte *data)
{
	int i;
	for (i = 0; i < G15_FRAMESIZE; i++){
		*data = rbittable[*data];
		data++;
	}
}

int g15_libusb_update (ubyte *data)
{
	byteReverseFrame(data);
	writePixmapToLCD(data);
	return 1;
}

static int signalFrameRefreshBlock (TMYLCDG15LU *mylcdg15, TFRAME *frm)
{
	l_memcpy(mylcdg15->data, frm->pixels, frm->frameSize);
	return g15_libusb_update(mylcdg15->data);
}

int g15_libusb_Refresh (TDRIVER *drv, TFRAME *frm)
{
	if (!frm || !drv)
		return 0;

	if (drv->dd->status != LDRV_READY)
		return 0;

	TMYLCDG15LU *mylcdg15 = (TMYLCDG15LU*)((intptr_t*)drv->dd->opt)[lOPT_G15LU_STRUCT];
	return signalFrameRefreshBlock(mylcdg15, frm);
}

int g15_libusb_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	return g15_libusb_Refresh(drv, frm);
}

#else

int initG15_libusb(void *rd){return 1;}
void closeG15_libusb(void *rd){return;}

#endif

