
// USBD480, for the USBD480 control board with the Sharp LQ043 LCD (as found in the PSP-1000) @ www.lcdinfo.com
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

#if (__BUILD_USBD480__ && __WIN32__)


#include "../frame.h"
#include "../device.h"
#include "../memory.h"
#include "../lstring.h"
#include "../pixel.h"
#include "../convert.h"
#include "../sync.h"
#include "../misc.h"
#include "usbd480hid.h"
#include "../usbd480libusb/libusbd480/libusbd480.h"
#include "../usbd480libusb/libusbd480/hid.h"


typedef int (*lpTouchCB) (TTOUCHCOORD *pos, int flags, void *userPtr);


typedef struct{
	TUSBD480 *dev;
	TTHRDSYNCCTRL hTouchThread;
	int threadState;
	lpTouchCB TouchCB;
	void *userPtr;
	TUSBD480HID *hid;
	TFRAME *convertFrame;
	unsigned int *convertBuffer;
	
}TUSBD480DI;


int usbd480h_OpenDisplay (TDRIVER *drv);
int usbd480h_CloseDisplay (TDRIVER *drv);
int usbd480h_ClearDisplay (TDRIVER *drv);
int usbd480h_Refresh (TDRIVER *drv, TFRAME *frm);
int usbd480h_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
int usbd480h_setOption (TDRIVER *drv, int option, intptr_t *value);
int usbd480h_getOption (TDRIVER *drv, int option, intptr_t *value);

void usbd480h_shutdown (TDRIVER *drv);
static void clearDisplay (TDRIVER *drv);
int usbd480h_update (TDRIVER *drv, TFRAME *frm);
int usbd480h_setBrightness (TDRIVER *drv, int level);
unsigned int __stdcall usbd480h_InputHandler (TUSBD480DI *di);



int initUSBD480HID (TREGISTEREDDRIVERS *rd)
{
	//printf("initUSBD480HID\n");
	
	TDISPLAYDRIVER dd;

	l_strcpy(dd.name,"USBD480:LIBUSBHID");
	l_strcpy(dd.comment,"USBD480 (libusb0, HID)");
	dd.open = usbd480h_OpenDisplay;
	dd.close = usbd480h_CloseDisplay;
	dd.clear = usbd480h_ClearDisplay;
	dd.refresh = usbd480h_Refresh;
	dd.refreshArea = usbd480h_RefreshArea;
	dd.setOption = usbd480h_setOption;
	dd.getOption = usbd480h_getOption;
	dd.status = LDRV_CLOSED;
	dd.optTotal = 5;
	
	int dnumber = registerDisplayDriver(rd, &dd);
	setDefaultDisplayOption(rd, dnumber, lOPT_USBD480_STRUCT, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_USBD480_BRIGHTNESS, 140);
	setDefaultDisplayOption(rd, dnumber, lOPT_USBD480_TOUCHPOSITION, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_USBD480_TOUCHCB, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_USBD480_TOUCHCBUSERPTR, 0);
	
	//printf("initUSBD480HID %i\n", dnumber);
	
	return (dnumber > 0);
}

void closeUSBD480HID (TREGISTEREDDRIVERS *rd)
{
	//printf("closeUSBD480HID\n");
	
	return;
}

static TUSBD480DI *usbd480_getStruct (TDRIVER *drv)
{
	TUSBD480DI *usbd480di = NULL;
	drv->dd->getOption(drv, lOPT_USBD480_STRUCT, (void*)&usbd480di);
	return usbd480di;
}

static int initTouchReporting (TUSBD480DI *di)
{
	
	int ret = libusbd480_SetConfigValue(di->dev, CFG_TOUCH_MODE, 1);
	libusbd480_SetConfigValue(di->dev, CFG_TOUCH_DEBOUNCE_VALUE, 20);
	libusbd480_SetConfigValue(di->dev, CFG_TOUCH_SKIP_SAMPLES, 20);
	libusbd480_SetConfigValue(di->dev, CFG_TOUCH_PRESSURE_LIMIT_LO, 50);
	libusbd480_SetConfigValue(di->dev, CFG_TOUCH_PRESSURE_LIMIT_HI, 126);
	return ret;
}

int usbd480h_getOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (drv && value){
		if (option < drv->dd->optTotal){
			if (option == lOPT_USBD480_STRUCT){
				const intptr_t *opt = drv->dd->opt;
				*value = opt[option];
				return 1;
			}else if (option == lOPT_USBD480_TOUCHPOSITION){
				return libusbd480_GetTouchPosition(usbd480_getStruct(drv)->dev, (TTOUCHCOORD*)value);
			}
		}
	}
	return 0;
}

int usbd480h_setOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (!drv)
		return 0;
	if (option >= drv->dd->optTotal)
		return 0;

	intptr_t *opt = drv->dd->opt;
	if (option == lOPT_USBD480_STRUCT){
		
		//printf("usbd480h_setOption %p %i %p, %p %p\n", drv, option, value, drv->dd, drv->dd->opt);
		
		opt[lOPT_USBD480_STRUCT] = (intptr_t)value;
		return 1;
		
	}else if (option == lOPT_USBD480_BRIGHTNESS){
		opt[lOPT_USBD480_BRIGHTNESS] = *value;
		libusbd480_SetBrightness(usbd480_getStruct(drv)->dev, (*value)&0xFF);
		return 1;
		
	}else if (option == lOPT_USBD480_TOUCHCB){
		opt[lOPT_USBD480_TOUCHCB] = (intptr_t)value;
		TUSBD480DI *di = usbd480_getStruct(drv);
		if (!di->TouchCB && value && !di->threadState){
			if (initTouchReporting(di)){
				di->TouchCB = (lpTouchCB)value;
				return newThread(&di->hTouchThread, usbd480h_InputHandler, di);
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

void usbd480h_shutdown (TDRIVER *drv)
{
	TUSBD480DI *di = usbd480_getStruct(drv);
	if (di != NULL){
		libusbd480_CloseDisplay(di->dev);
		lDeleteFrame(di->convertFrame);
		//l_free(di->convertBuffer);
		l_free(di->dev);
		l_free(di);
		drv->dd->setOption(drv, lOPT_USBD480_STRUCT, NULL);
	}
}

int usbd480h_OpenDisplay (TDRIVER *drv)
{
	//printf("usbd480h_OpenDisplay\n");
	
	if (drv){
		if (drv->dd->status == LDRV_CLOSED){
			//drv->dd->back = _newFrame(drv->dd->hw, drv->dd->width, drv->dd->height, 1, LFRM_BPP_16);
			//if (drv->dd->back){
				TUSBD480DI *usbd480di = (TUSBD480DI*)l_calloc(1, sizeof(TUSBD480DI));
				if (usbd480di){
					//const int fsize = calcFrameSize(drv->dd->width, drv->dd->height, LFRM_BPP_16);
					//usbd480di->convertBuffer = (unsigned int*)l_malloc(fsize);
					
					usbd480di->convertFrame = _newFrame(drv->dd->hw, drv->dd->width, drv->dd->height, 1, LFRM_BPP_16);
					usbd480di->convertBuffer = lGetPixelAddress(usbd480di->convertFrame, 0, 0);
					
					usbd480di->dev = (TUSBD480*)l_calloc(1, sizeof(TUSBD480));
					if (usbd480di->dev){
						usbd480di->hid = (TUSBD480HID*)l_calloc(1, sizeof(TUSBD480HID));
						if (usbd480di->hid){
							if (libusbd480_OpenDisplay(usbd480di->dev, drv->dd->port)){
								usbd480h_setOption(drv, lOPT_USBD480_STRUCT, (intptr_t*)usbd480di);
								usbd480h_setOption(drv, lOPT_USBD480_TOUCHCBUSERPTR, NULL);
								usbd480h_setOption(drv, lOPT_USBD480_TOUCHCB, NULL);
								drv->dd->status = LDRV_READY;
								drv->dd->clear(drv);
								return 1;
							}else{
								mylog("usbd480h_OpenDisplay(): device #%i unavailable\n", drv->dd->port);
							}
							l_free(usbd480di->hid);
						}
						l_free(usbd480di->dev);
					}
					//l_free(usbd480di->convertBuffer);
					lDeleteFrame(usbd480di->convertFrame);
					l_free(usbd480di);
				}
			//}
		}
	}
	return 0;
}

int usbd480h_CloseDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status != LDRV_CLOSED){
			drv->dd->status = LDRV_CLOSED;
			TUSBD480DI *di = usbd480_getStruct(drv);
			if (di){
				if (di->threadState){
					di->threadState = 0;
					hid_SignalCloseReady(di->hid);
					joinThread(&di->hTouchThread);
					hid_CloseUSBD480(di->hid);
					closeThreadHandle(&di->hTouchThread);
				}
				l_free(di->hid);
				usbd480h_shutdown(drv);
				//if (drv->dd->back)
					//deleteFrame(drv->dd->back);
				return 1;
			}
		}
	}
	return 0;
}

int usbd480h_ClearDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status == LDRV_READY){
			clearDisplay(drv);
			return 1;
		}
	}
	return 0;
}

int usbd480h_Refresh (TDRIVER *drv, TFRAME *frame)
{
	if (frame == NULL || drv == NULL){
		return 0;
	}else{
		return usbd480h_update(drv, frame);
	}
	return 0;
}

void frame32ToBuffer16 (TFRAME *frm, TFRAME *des, int x1, int y1, int x2, int y2)
{
	uint16_t * restrict out = (uint16_t*)lGetPixelAddress(des, x1, y1);
	TRGBA * restrict in = (TRGBA*)lGetPixelAddress(frm, x1, y1);
	TRGBA * restrict p = in;
	const int w = (x2-x1)+1;
	const int h = (y2-y1)+1;
	const int cpitch = des->pitch;
	const int fpitch = frm->pitch>>2;
	
	for (int y = 0; y < h; y++){
		for (int x = 0; x < w; x++)
			*(out+x) = (p[x].r&0xF8)>>3|(p[x].g&0xFC)<<3|(p[x].b&0xF8)<<8;

		p += fpitch;
		out = (uint16_t*)((uint8_t*)out+cpitch);
	}
}

int usbd480h_RefreshArea (TDRIVER *drv, TFRAME *frame, int x1, int y1, int x2, int y2)
{
	TUSBD480DI *di = usbd480_getStruct(drv);
	if (di != NULL){
		if (frame->bpp == LFRM_BPP_32 || frame->bpp == LFRM_BPP_32A){
			//printf(":: %i %i %i %i\n", x1, y1, x2, y2);

			TLPOINTEX region = {x1, y1, x2, y2};
			frame32ToBuffer16(frame, di->convertFrame, x1, y1, x2, y2);
			libusbd480_DrawScreenAreaX700(di->dev, (uint8_t*)di->convertFrame->pixels, frame->frameSize, &region);

		}else if (frame->bpp == LFRM_BPP_16){
			TLPOINTEX region = {x1, y1, x2, y2};
			libusbd480_DrawScreenAreaX700(di->dev, frame->pixels, frame->frameSize, &region);

		}else{
			usbd480h_Refresh(drv, frame);
		}
	}
	return 1;
}

int usbd480h_update (TDRIVER *drv, TFRAME *frame)
{
	
	TUSBD480DI *di = usbd480_getStruct(drv);
	if (di != NULL){
		if (frame->bpp == LFRM_BPP_16){
			return libusbd480_DrawScreen(di->dev, frame->pixels, frame->frameSize);
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

unsigned int __stdcall usbd480h_InputHandler (TUSBD480DI *di)
{
	mylog("usbd480: usbd480h_InputHandler started\n");
	di->threadState = 1;
	TTOUCHCOORD pos, prev;
	unsigned int id = 0;
	int prevFlags = 0;
	l_memset(&prev, 0, sizeof(TTOUCHCOORD));
	pos.time = getTicks();
	int ret = 1;
	
	if (!hid_FindUSBD480(di->hid, di->dev->Serial, 1)){
		printf("USBD480 HID interface not found\n");
		ret = 0;
		goto threadend;
	}

	if (!hid_OpenUSBD480(di->hid)){
		mylog("unable to open usbd480 HID interface\n");
		hid_CloseUSBD480(di->hid);
		ret = 0;
		goto threadend;
	} 

	while(di->threadState){
		int ret = hid_GetTouchPosition(di->dev, di->hid, &pos);
		if (!di->threadState) break;
		if (!ret){
			di->threadState = 0;
			ret = 1;
			break;

		}else if (ret < 0){
			// -1 = timeout
			// -2 = invalid location
			hid_ClearQueue(di->hid);
			//libusbd480_CleanQueue(di->dev);
			//lSleep(0);
			continue;
			
		}else if (ret == 2){	// dragging in effect
			if (di->threadState){
				l_memcpy(&prev, &pos, sizeof(TTOUCHCOORD));
				pos.id = id;
				prevFlags = 0x01;
				
				if (di->TouchCB)
					di->TouchCB(&pos, 0x01, di->userPtr);
			}
		}else if (ret == 1){	// pen down - a single filtered debounced press
			if (di->threadState){
				l_memcpy(&prev, &pos, sizeof(TTOUCHCOORD));
				pos.id = ++id;
				prevFlags = 0x00;
				hid_ClearQueue(di->hid);
				
				if (di->TouchCB)
					di->TouchCB(&pos, 0x00, di->userPtr);
			}
		}else if (ret == 3){	// pen up
			if (di->threadState){
				pos.id = id;
				prev.pen = 1;
				prev.dt = 0;
				hid_ClearQueue(di->hid);
				
				if (di->TouchCB)
					di->TouchCB(&prev, prevFlags|0x02, di->userPtr);
			}
		}
	}
	
	hid_CloseUSBD480(di->hid);
		
threadend:
	di->threadState = 0;
	mylog("usbd480: usbd480h_InputHandler exited\n");
	_endthreadex(ret);
	return ret;
}

#else

int initUSBD480HID(void *rd){return 1;}
void closeUSBD480HID(void *rd){return;}

#endif

