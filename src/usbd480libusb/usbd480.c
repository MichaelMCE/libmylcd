
// USBD480, for the Sharp LQ043 LCD (as found in the PSP-1000) @ www.lcdinfo.com
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


// usbd480 driven through libusb0

#include "mylcd.h"

#if (__BUILD_USBD480LIBUSB__)

#include <lusb0_usb.h>

#include "../frame.h"
#include "../device.h"
#include "../memory.h"
#include "../lstring.h"
#include "../pixel.h"
#include "../convert.h"
#include "../sync.h"
#include "../misc.h"
#include "usbd480.h"
#include "libusbd480/libusbd480.h"


typedef int (*lpTouchCB) (TTOUCHCOORD *pos, int flags, intptr_t *userPtr);

typedef struct{
	TUSBD480 *dev;
	TTHRDSYNCCTRL hTouchThread;
	int threadState;
	lpTouchCB TouchCB;
	intptr_t *userPtr;
	unsigned int *convertBuffer;
}TUSBD480DI;


int usbd480_OpenDisplay (TDRIVER *drv);
int usbd480_CloseDisplay (TDRIVER *drv);
int usbd480_ClearDisplay (TDRIVER *drv);
int usbd480_Refresh (TDRIVER *drv, TFRAME *frm);
int usbd480_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static int usbd480_setOption (TDRIVER *drv, int option, intptr_t *value);
static int usbd480_getOption (TDRIVER *drv, int option, intptr_t *value);

void usbd480_shutdown (TDRIVER *drv);
static void clearDisplay (TDRIVER *drv);
int usbd480_update (TDRIVER *drv, TFRAME *frm);
int usbd480_setBrightness (TDRIVER *drv, int level);

unsigned int __stdcall usbd480_InputHandler (TUSBD480DI *di);




int initUSBD480 (TREGISTEREDDRIVERS *rd)
{
	TDISPLAYDRIVER dd;
	l_strcpy(dd.name,"USBD480:LIBUSB");
	l_strcpy(dd.comment,"USBD480 (libusb0)");
	dd.open = usbd480_OpenDisplay;
	dd.close = usbd480_CloseDisplay;
	dd.clear = usbd480_ClearDisplay;
	dd.refresh = usbd480_Refresh;
	dd.refreshArea = usbd480_RefreshArea;
	dd.setOption = usbd480_setOption;
	dd.getOption = usbd480_getOption;
	dd.status = LDRV_CLOSED;
	dd.optTotal = 5;
	
	int dnumber = registerDisplayDriver(rd, &dd);
	setDefaultDisplayOption(rd, dnumber, lOPT_USBD480_STRUCT, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_USBD480_BRIGHTNESS, 140);
	setDefaultDisplayOption(rd, dnumber, lOPT_USBD480_TOUCHPOSITION, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_USBD480_TOUCHCB, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_USBD480_TOUCHCBUSERPTR, 0);
	return (dnumber > 0);
}

void closeUSBD480 (TREGISTEREDDRIVERS *rd)
{
	return;
}

static TUSBD480DI *usbd480_getStruct (TDRIVER *drv)
{
	TUSBD480DI *usbd480di = NULL;
	drv->dd->getOption(drv, lOPT_USBD480_STRUCT, (void*)&usbd480di);
	return usbd480di;
}

static int usbd480_getOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (drv && value){
		if (option < drv->dd->optTotal){
			if (option == lOPT_USBD480_STRUCT){
				const int *opt = (int*)drv->dd->opt;
				*value = opt[option];
				return 1;
			}else if (option == lOPT_USBD480_TOUCHPOSITION){
				return libusbd480_GetTouchPosition(usbd480_getStruct(drv)->dev, (TTOUCHCOORD*)value);
			}
		}
	}
	return 0;
}

static int initTouchReporting (TUSBD480DI *di)
{
	int ret = libusbd480_SetConfigValue(di->dev, CFG_TOUCH_MODE, 4);
	libusbd480_SetConfigValue(di->dev, CFG_TOUCH_DEBOUNCE_VALUE, 20);
	libusbd480_SetConfigValue(di->dev, CFG_TOUCH_SKIP_SAMPLES, 20);
	libusbd480_SetConfigValue(di->dev, CFG_TOUCH_PRESSURE_LIMIT_LO, 50);
	libusbd480_SetConfigValue(di->dev, CFG_TOUCH_PRESSURE_LIMIT_HI, 126);
	return ret;
}

static int usbd480_setOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (!drv)
		return 0;
	if (option >= drv->dd->optTotal)
		return 0;

	intptr_t *opt = drv->dd->opt;

	if (option == lOPT_USBD480_STRUCT){
		opt[lOPT_USBD480_STRUCT] = (intptr_t)value;
		return 1;
		
	}else if (option == lOPT_USBD480_BRIGHTNESS){
		opt[lOPT_USBD480_BRIGHTNESS] = *value;
		
		libusbd480_SetBrightness(usbd480_getStruct(drv)->dev, *value&0xFF);
		return 1;
		
	}else if (option == lOPT_USBD480_TOUCHCB){
		opt[lOPT_USBD480_TOUCHCB] = (intptr_t)value;
		
		TUSBD480DI *di = usbd480_getStruct(drv);
		if (!di->TouchCB && value && !di->threadState){
			if (initTouchReporting(di)){
				di->TouchCB = (lpTouchCB)value;
				return newThread(&di->hTouchThread, usbd480_InputHandler, di);
			}else{
				return 0;
			}
		}else if (value == NULL){
			libusbd480_SetConfigValue(di->dev, CFG_TOUCH_MODE, 0);	// switch off
		}
		di->TouchCB = (lpTouchCB)value;
		return 1;
	}else if (option == lOPT_USBD480_TOUCHCBUSERPTR){
		opt[lOPT_USBD480_TOUCHCBUSERPTR] = (intptr_t)value;
		TUSBD480DI *di = usbd480_getStruct(drv);
		di->userPtr = value;
		return 1;
	}
	return 0;
}

void usbd480_shutdown (TDRIVER *drv)
{
	
	TUSBD480DI *di = usbd480_getStruct(drv);
	if (di != NULL){
		libusbd480_CloseDisplay(di->dev);
		l_free(di->convertBuffer);
		l_free(di->dev);
		l_free(di);
		drv->dd->setOption(drv, lOPT_USBD480_STRUCT, 0);
	}
}

int usbd480_OpenDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status == LDRV_CLOSED){
			//drv->dd->back = _newFrame(drv->dd->hw, drv->dd->width, drv->dd->height, 1, LFRM_BPP_16);
			//if (drv->dd->back){
				TUSBD480DI *usbd480di = (TUSBD480DI*)l_calloc(1, sizeof(TUSBD480DI));
				if (usbd480di){
					const int fsize = calcFrameSize(drv->dd->width, drv->dd->height, LFRM_BPP_16);
					usbd480di->convertBuffer = (unsigned int*)l_malloc(fsize);
					usbd480di->dev = (TUSBD480*)l_calloc(1, sizeof(TUSBD480));
					if (usbd480di->dev){
						if (libusbd480_OpenDisplay(usbd480di->dev, drv->dd->port)){
							usbd480_setOption(drv, lOPT_USBD480_STRUCT, (intptr_t*)usbd480di);
							usbd480_setOption(drv, lOPT_USBD480_TOUCHCBUSERPTR, 0);
							usbd480_setOption(drv, lOPT_USBD480_TOUCHCB, 0);
							drv->dd->status = LDRV_READY;
							drv->dd->clear(drv);
							return 1;
						}else{
							l_free(usbd480di->convertBuffer);
							l_free(usbd480di->dev);
							l_free(usbd480di);
							mylog("usbd480_OpenDisplay(): device #%i unavailable\n", drv->dd->port);
							return 0;
						}
					}
				}
			//}
		}
	}
	return 0;
}

int usbd480_CloseDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status != LDRV_CLOSED){
			drv->dd->status = LDRV_CLOSED;
			
			TUSBD480DI *di = usbd480_getStruct(drv);
			if (di){
				if (di->threadState){
					di->threadState = 0;
					joinThread(&di->hTouchThread);
					closeThreadHandle(&di->hTouchThread);
				}
				usbd480_shutdown(drv);
				//if (drv->dd->back)
					//deleteFrame(drv->dd->back);
				return 1;
			}
		}
	}
	return 0;
}

int usbd480_ClearDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status == LDRV_READY){
			clearDisplay(drv);
			return 1;
		}
	}
	return 0;
}

int usbd480_Refresh (TDRIVER *drv, TFRAME *frame)
{
	if (frame == NULL || drv == NULL){
		return 0;
	}else{
		return usbd480_update(drv, frame);
	}
	return 0;
}

int usbd480_RefreshArea (TDRIVER *drv, TFRAME *frame, int x1, int y1, int x2, int y2)
{
	return usbd480_Refresh(drv, frame);
}

int usbd480_update (TDRIVER *drv, TFRAME *frame)
{
	TUSBD480DI *di = usbd480_getStruct(drv);
	if (di != NULL){
		if (frame->bpp == LFRM_BPP_16){
			return libusbd480_DrawScreen(di->dev, l_getPixelAddress(frame,0,0), frame->frameSize);
		}else{
			pConverterFn converter = getConverter(frame->bpp, LFRM_BPP_16);
			if (converter){
				converter(frame, di->convertBuffer);
				const int fsize = calcFrameSize(drv->dd->width, drv->dd->height, LFRM_BPP_16);
				return libusbd480_DrawScreen(di->dev, (void*)di->convertBuffer, fsize);
			}
		}
	}
	return 0;
}

static void clearDisplay (TDRIVER *drv)
{
	TUSBD480DI *di = usbd480_getStruct(drv);
	if (di != NULL){
		ubyte *data = (ubyte*)l_calloc(2, di->dev->Width * di->dev->Height);
		if (data){
			libusbd480_DrawScreen(di->dev, data, di->dev->Width * di->dev->Height*2);
			l_free(data);
			return;
		}
	}
	return;
}

unsigned int __stdcall usbd480_InputHandler (TUSBD480DI *di)
{
	mylog("USBD480 Input handler started\n");
	
	di->threadState = 1;
	TTOUCHCOORD pos, prev;
	unsigned int id = 0;
	int prevFlags = 0;
	l_memset(&prev, 0, sizeof(TTOUCHCOORD));
	pos.time = getTicks();
	int ret = 0;

	
	while(di->threadState){
		ret = libusbd480_GetTouchPosition(di->dev, &pos);
		if (!di->threadState) break;
		if (!ret){
			di->threadState = 0;
			break;
		}else if (ret < 0){
			// -1 = timeout
			// -2 = invalid location
			continue;
		}else if (ret == 2){	// dragging in effect
			if (di->TouchCB && di->threadState){
				l_memcpy(&prev, &pos, sizeof(TTOUCHCOORD));
				pos.id = id;
				prevFlags = 0x01;
				di->TouchCB(&pos, 0x01, di->userPtr);
			}
		}else if (ret == 1){	// pen down - a single filtered debounced press
			if (di->TouchCB && di->threadState){
				l_memcpy(&prev, &pos, sizeof(TTOUCHCOORD));
				pos.id = ++id;
				prevFlags = 0x00;
				di->TouchCB(&pos, 0x00, di->userPtr);
			}
		}else if (ret == 3){	// pen up
			if (di->TouchCB && di->threadState){
				pos.id = id;
				prev.pen = 1;
				prev.dt = 0;
				di->TouchCB(&prev, prevFlags|0x02, di->userPtr);
			}
		}
	}
	mylog("libmylcd: USBD480 Input handler exited\n");
	_endthreadex(1);
	return ret;
}

#else

int initUSBD480(void *rd){return 1;}
void closeUSBD480(void *rd){return;}

#endif

