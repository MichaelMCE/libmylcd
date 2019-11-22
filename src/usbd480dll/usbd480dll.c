
// USBD480, for the Sharp LQ043 LCD (as found in the PSP-1000) @ www.lcdinfo.com

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

// usbd480 driven through USBD480_lib.dll

#include "mylcd.h"

#if (__BUILD_USBD480DLL__)

#include "../frame.h"
#include "../device.h"
#include "../memory.h"
#include "../lstring.h"
#include "../pixel.h"
#include "../convert.h"
#include "../sync.h"
#include "../misc.h"
#include "USBD480_lib.h"
#include "usbd480dll.h"
#include "touch.h"



int usbd480DLL_OpenDisplay (TDRIVER *drv);
int usbd480DLL_CloseDisplay (TDRIVER *drv);
int usbd480DLL_ClearDisplay (TDRIVER *drv);
int usbd480DLL_Refresh (TDRIVER *drv, TFRAME *frm);
int usbd480DLL_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
int usbd480DLL_setOption (TDRIVER *drv, int option, intptr_t *value);
int usbd480DLL_getOption (TDRIVER *drv, int option, intptr_t *value);

void usbd480DLL_shutdown (TDRIVER *drv);
void usbd480DLLclearDisplay (TDRIVER *drv);
int usbd480DLL_update (TDRIVER *drv, TFRAME *frm);
int usbd480DLL_setBrightness (TDRIVER *drv, int level);
unsigned int __stdcall usbd480dll_InputHandler (TUSBD480DIDLL *di);





int initUSBD480DLL (TREGISTEREDDRIVERS *rd)
{
	TDISPLAYDRIVER dd;

	l_strcpy(dd.name,"USBD480:DLL");
	l_strcpy(dd.comment,"USBD480 (USBD480_lib.dll)");
	dd.open = usbd480DLL_OpenDisplay;
	dd.close = usbd480DLL_CloseDisplay;
	dd.clear = usbd480DLL_ClearDisplay;
	dd.refresh = usbd480DLL_Refresh;
	dd.refreshArea = usbd480DLL_RefreshArea;
	dd.setOption = usbd480DLL_setOption;
	dd.getOption = usbd480DLL_getOption;
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

void closeUSBD480DLL (TREGISTEREDDRIVERS *rd)
{
	return;
}

static TUSBD480DIDLL *usbd480DLL_getStruct (TDRIVER *drv)
{
	TUSBD480DIDLL *usbd480 = NULL;
	drv->dd->getOption(drv, lOPT_USBD480_STRUCT, (intptr_t*)&usbd480);
	return usbd480;
}

int usbd480DLL_getOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (drv && value){
		if (option < drv->dd->optTotal){
			if (option == lOPT_USBD480_STRUCT){
				const intptr_t *opt = drv->dd->opt;
				*value = opt[option];
				return 1;
			}else if (option == lOPT_USBD480_TOUCHPOSITION){
				return usbd480DLL_GetTouchPosition(usbd480DLL_getStruct(drv), (TTOUCHCOORD*)value);
			}
		}
	}
	return 0;
}

int usbd480DLL_setOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (drv == NULL)
		return 0;
	if (option >= drv->dd->optTotal)
		return 0;

	intptr_t *opt = drv->dd->opt;

	if (option == lOPT_USBD480_STRUCT){
		opt[lOPT_USBD480_STRUCT] = (intptr_t)value;
		return 1;
		
	}else if (option == lOPT_USBD480_BRIGHTNESS){
		opt[lOPT_USBD480_BRIGHTNESS] = *value;
		if (opt[lOPT_USBD480_STRUCT]){
			TUSBD480DIDLL *usbd480 = usbd480DLL_getStruct(drv);
			USBD480_SetBrightness(&usbd480->di, (*value)&0xFF);
			return 1;
		}else{
			return 0;
		}
	}else if (option == lOPT_USBD480_TOUCHCB){
		opt[lOPT_USBD480_TOUCHCB] = (intptr_t)value;
		
		TUSBD480DIDLL *di = usbd480DLL_getStruct(drv);
		if (!di->TouchCB && value && !di->threadState){
			if (USBD480_SetTouchMode(&di->di, 4)){
				di->TouchCB = (lpTouchCBdll)value;
				return newThread(&di->hTouchThread, usbd480dll_InputHandler, di);
			}else{
				return 0;
			}
		}else if (value == 0){
			USBD480_SetTouchMode(&di->di, 0);
		}
		di->TouchCB = (lpTouchCBdll)value;
		return 1;
	}else if (option == lOPT_USBD480_TOUCHCBUSERPTR){
		opt[lOPT_USBD480_TOUCHCBUSERPTR] = (intptr_t)value;
		TUSBD480DIDLL *di = usbd480DLL_getStruct(drv);
		di->userPtr = value;
		return 1;
	}
	return 0;
}

void usbd480DLL_shutdown (TDRIVER *drv)
{
	TUSBD480DIDLL *usbd480 = usbd480DLL_getStruct(drv);
	if (usbd480 != NULL){
		drv->dd->setOption(drv, lOPT_USBD480_STRUCT, 0);
		USBD480_Close(&usbd480->di);
		l_free(usbd480->convertBuffer);
		l_free(usbd480);
	}
}

int usbd480DLL_OpenDisplay (TDRIVER *drv)
{
	
	if (drv){
		if (drv->dd->status == LDRV_CLOSED){
			//drv->dd->back = _newFrame(drv->dd->hw, drv->dd->width, drv->dd->height, 1, LFRM_BPP_16);
			//if (drv->dd->back){
				TUSBD480DIDLL *usbd480 = (TUSBD480DIDLL*)l_calloc(1, sizeof(TUSBD480DIDLL));
				if (usbd480){
					int dtotal = USBD480_GetNumberOfDisplays();
					if (!dtotal){
						mylog("usbd480DLL_OpenDisplay: USBD480 not found\n");
						l_free(usbd480);
						return 0;
					}
					const int fsize = calcFrameSize(drv->dd->width, drv->dd->height, LFRM_BPP_16);
					usbd480->convertBuffer = (unsigned int*)l_malloc(fsize);
					
					USBD480_GetDisplayConfiguration(0, &usbd480->di);
					if (USBD480_Open(&usbd480->di, 0)){
						USBD480_SetAddress(&usbd480->di, 0);
						USBD480_SetFrameStartAddress(&usbd480->di, 0);
						usbd480DLL_setOption(drv, lOPT_USBD480_STRUCT, (intptr_t*)usbd480);
						usbd480->Width = usbd480->di.Width;
						usbd480->Height = usbd480->di.Height;
						drv->dd->status = LDRV_READY;
						drv->dd->clear(drv);
						return 1;
					}else{
						l_free(usbd480->convertBuffer);
						l_free(usbd480);
						mylog("usbd480DLL_OpenDisplay: USBD480 #%i unavailable\n", drv->dd->port);
					}
				}
			//}
		}
	}
	return 0;
}

int usbd480DLL_CloseDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status != LDRV_CLOSED){
			drv->dd->status = LDRV_CLOSED;
			TUSBD480DIDLL *di = usbd480DLL_getStruct(drv);
			if (di){
				if (di->threadState){
					di->threadState = 0;
					usbd480DLL_shutdown(drv);
					joinThread(&di->hTouchThread);
					closeThreadHandle(&di->hTouchThread);
				}else{
					usbd480DLL_shutdown(drv);
				}
				//if (drv->dd->back)
					//deleteFrame(drv->dd->back);
				return 1;
			}
		}
	}
	return 0;
}

int usbd480DLL_ClearDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status == LDRV_READY){
			usbd480DLLclearDisplay(drv);
			return 1;
		}
	}
	return 0;
}

int usbd480DLL_Refresh (TDRIVER *drv, TFRAME *frame)
{
	if (frame == NULL || drv == NULL){
		return 0;
	}else{
		return usbd480DLL_update(drv, frame);
	}
	return 0;
}

int usbd480DLL_RefreshArea (TDRIVER *drv, TFRAME *frame, int x1, int y1, int x2, int y2)
{
	return usbd480DLL_Refresh(drv, frame);
}

int usbd480DLL_update (TDRIVER *drv, TFRAME *frame)
{
	TUSBD480DIDLL *usbd480 = usbd480DLL_getStruct(drv);
	if (usbd480 != NULL){
		if (frame->bpp == LFRM_BPP_16){
			return USBD480_DrawFullScreen(&usbd480->di, frame->pixels); 
		}else{
			pConverterFn converter = getConverter(frame->bpp, LFRM_BPP_16);
			if (converter){
				converter(frame, usbd480->convertBuffer);
				return USBD480_DrawFullScreen(&usbd480->di, (unsigned char*)usbd480->convertBuffer);
			}
		}
	}
	return 0;
}

void usbd480DLLclearDisplay (TDRIVER *drv)
{
	TUSBD480DIDLL *usbd480 = usbd480DLL_getStruct(drv);
	if (usbd480 != NULL){
		ubyte *data = (ubyte*)l_calloc(2, usbd480->Width*usbd480->Height);
		if (data){
			USBD480_DrawFullScreen(&usbd480->di, data);
			l_free(data);
			return;
		}
	}
	return;
}

unsigned int __stdcall usbd480dll_InputHandler (TUSBD480DIDLL *di)
{
	mylog("\nusbd480dll_InputHandler started\n");
	di->threadState = 1;
	TTOUCHCOORD pos, prev;
	unsigned int id = 0;
	int prevFlags = 0;
	l_memset(&prev, 0, sizeof(TTOUCHCOORD));
	pos.time = getTicks();
	
	initTouchDll(di, &di->touch);
	
	while(di->threadState){
		int ret = usbd480DLL_GetTouchPosition(di, &pos);
		if (!di->threadState) break;
		if (!ret){
			di->threadState = 0;
			break;
		}else if (ret < 0){
			// -1 = timeout
			// -2 = invalid location
			lSleep(1);
			continue;
		}else if (ret == 2){	// dragging in effect
			if (di->TouchCB && di->threadState){
				l_memcpy(&prev, &pos, sizeof(TTOUCHCOORD));
				pos.id = id;
				prevFlags = 0x01;
				di->TouchCB(&pos, 0x01, di->userPtr);
			}
		}else if (ret == 1){	// a single filtered debounced press
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
	mylog("usbd480: usbd480dll_InputHandler exited\n");
	_endthreadex(1);
	return 1;
}


#else

int initUSBD480DLL(void *rd){return 1;}
void closeUSBD480DLL(void *rd){return;}

#endif

