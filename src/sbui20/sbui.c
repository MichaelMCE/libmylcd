
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

// disabled. now using direct access. refer to sbui153 
#if (0 && __BUILD_SBUI__ && __WIN32__)

#include "../memory.h"
#include "../frame.h"
#include "../utils.h"
#include "../fileio.h"
#include "../misc.h"
#include "../pixel.h"
#include "../device.h"
#include "../lstring.h"
#include "../convert.h"
#include "../sync.h"
#include <windows.h>
#include <SBUI_20/SwitchBlade.h>
#include "sbui.h"


#define ENABLE_SBUINOTFOUNDWARNING	0
#define ENABLE_ADDRNOTFOUND			0

#define SBSDKDLL "RzSwitchbladeSDK2.dll"


DLL_DECLARE(HRESULT, , RzSBStart, (void));
DLL_DECLARE(HRESULT, , RzSBStop, (void));
//DLL_DECLARE(HRESULT, , RzSBQueryCapabilities, (PRZSBSDK_QUERYCAPABILITIES));
DLL_DECLARE(HRESULT, , RzSBRenderBuffer, (int, void *));
DLL_DECLARE(HRESULT, , RzSBSetImageDynamicKey, (int, int, void*));
//DLL_DECLARE(HRESULT, , RzSBSetImageTouchpad, (void*));
DLL_DECLARE(HRESULT, , RzSBEnableGesture, (int, int));
DLL_DECLARE(HRESULT, , RzSBEnableOSGesture, (int, int));
DLL_DECLARE(HRESULT, , RzSBDynamicKeySetCallback, (DynamicKeyCallbackFunctionType));
DLL_DECLARE(HRESULT, , RzSBGestureSetCallback, (TouchpadGestureCallbackFunctionType));
DLL_DECLARE(HRESULT, , RzSBAppEventSetCallback, (AppEventCallbackType));


typedef struct{
	void *buffer;

	TSBGESTURE sbg;	
	pgesturecb gestCB;
	pdkcb dkCB;
	void *udata_ptr;

	TTHRDSYNCCTRL s;
	uint64_t t0;
}TSBCTX;


// we need to do this because RzSBGestureSetCallback() does not provide us with a context/instance user pointer
static TSBCTX *g_sbctx = NULL;
static int initOnce = 0;

static int sbui_OpenDisplay (TDRIVER *drv);
static int sbui_CloseDisplay (TDRIVER *drv);
static int sbui_Clear (TDRIVER *drv);
static int sbui_Refresh (TDRIVER *drv, TFRAME *frm);
static int sbui_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static int sbui_getOption (TDRIVER *drv, int option, intptr_t *value);
static int sbui_setOption (TDRIVER *drv, int option, intptr_t *value);



static inline void dll_load_error (const char *fn, const char *dll, const int err)
{
#if (ENABLE_ADDRNOTFOUND)
	char buffer[MAX_PATH+1];
	
	if (err == -1)
		snprintf(buffer, MAX_PATH, "LoadLibrary(): File not found\n\n'%s'", dll);
	else if (err == -2)
		snprintf(buffer, MAX_PATH, "GetProcAddress(): Symbol '%s' not found in '%s'", fn, dll);
	else
		return;

	MessageBoxA(NULL, buffer, dll, MB_SYSTEMMODAL|MB_ICONSTOP|MB_OK);
#endif
}

static inline int sbuiLoadDll (const char *lib)
{
	if (initOnce) return 1;
	
	DLL_LOAD(lib, RzSBStart, 1);
	DLL_LOAD(lib, RzSBStop, 1);
	//DLL_LOAD(lib, RzSBQueryCapabilities, 1);
	DLL_LOAD(lib, RzSBRenderBuffer, 1);
	
	DLL_LOAD(lib, RzSBSetImageDynamicKey, 1);
	//DLL_LOAD(lib, RzSBSetImageTouchpad, 1);
	
	DLL_LOAD(lib, RzSBEnableGesture, 1);
	DLL_LOAD(lib, RzSBEnableOSGesture, 1);
	
	DLL_LOAD(lib, RzSBDynamicKeySetCallback, 1);
	DLL_LOAD(lib, RzSBGestureSetCallback, 1);
	DLL_LOAD(lib, RzSBAppEventSetCallback, 1);

	initOnce = 1;
		
	return 1;
}

int sbui20Init (TREGISTEREDDRIVERS *rd)
{
	//if (sbuiLoadDll(SBSDKDLL) <= 0)
	//	return 0;

	g_sbctx = NULL;
	TDISPLAYDRIVER dd;
	l_strcpy(dd.comment, "SwitchBlade UI v2.0");
		
	dd.open = sbui_OpenDisplay;
	dd.close = sbui_CloseDisplay;
	dd.clear = sbui_Clear;
	dd.refresh = sbui_Refresh;
	dd.refreshArea = sbui_RefreshArea;
	dd.setOption = sbui_setOption;
	dd.getOption = sbui_getOption;
	dd.status = LDRV_CLOSED;
	dd.optTotal = 7;
	
	l_strcpy(dd.name, "SBUI20");
	int dnumber = registerDisplayDriver(rd, &dd);
	setDefaultDisplayOption(rd, dnumber, lOPT_SBUI_STRUCT, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_SBUI_GESTURECB, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_SBUI_DKCB, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_SBUI_UDATAPTR, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_SBUI_GESTURECFG, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_SBUI_SETDK, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_SBUI_RECONNECT, 0);
	
	return (dnumber > 0);
}

void sbui20Close (TREGISTEREDDRIVERS *rd)
{
	g_sbctx = NULL;
	initOnce = 0;
}

static inline void * drvToCtx (TDRIVER *drv)
{	
	if (drv){
		TSBCTX *sbctx = NULL;
		drv->dd->getOption(drv, lOPT_SBUI_STRUCT, (void*)&sbctx);
		return sbctx;
	}else{
		return (TSBCTX*)g_sbctx;
	}
}

static inline int rzsb_success (int ret, const wchar_t *str)
{
	if (RZSB_SUCCESS(ret)){
		return 1;
	}else{
#if (ENABLE_SBUINOTFOUNDWARNING)
		wchar_t buffer[4096];
		wchar_t *hLocal = NULL;

		int error = ret;
		int gle = GetLastError();
		if (gle) error = gle;
		
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, 0, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&hLocal, 0, 0);
		LocalLock(hLocal);
		
		snwprintf(buffer, sizeof(buffer), 
			L"%s() failed with error 0x%X/%i (GLE 0x%X/%i)\n\n%s",
			str, ret, ret&0xFFFF, gle, gle&0xFFFF, hLocal
		);
		
		LocalFree(hLocal);
		
		if (gle == ERROR_ALREADY_EXISTS)
			l_wcscat(buffer, L"\nNote:\nTry closing any other SBUI program then try again.");


		MessageBoxW(NULL, buffer, L"SwitchBlade32-20", MB_SYSTEMMODAL|MB_ICONSTOP|MB_OK);
#endif
		return 0;
	}
}

static int sbui_CloseDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status != LDRV_CLOSED){
			drv->dd->status = LDRV_CLOSED;
			rzsb_success(_RzSBStop(), L"RzSBStop");
			
			TSBCTX *sbctx = (TSBCTX*)drvToCtx(drv);
			if (sbctx){
				drv->dd->setOption(drv, lOPT_SBUI_UDATAPTR, NULL);
				drv->dd->setOption(drv, lOPT_SBUI_GESTURECB, NULL);
				lock_delete(&sbctx->s);
				l_free(sbctx->buffer);
				l_free(sbctx);
				g_sbctx = NULL;
				//initOnce = 0;
			}
			return 1;
		}
	}
	return 0;
}

static CALLBACK HRESULT touchpadGestureCB (RZSBSDK_GESTURETYPE type, DWORD params, WORD x, WORD y, WORD z)
{
	//printf("touchpadGestureCB: %i %i %i %i %i\n", (int)type, (int)params, (int)x, (int)y, (int)z);
	
	if (!g_sbctx || !ValidGesture(type)){
	//	printf("..invalid gesture\n");
		return S_OK;
	}

	TSBCTX *sbctx = (TSBCTX*)drvToCtx(NULL/*drv*/);
	if (sbctx){
		if (lock(&sbctx->s)){
			TSBGESTURE *sbg = &sbctx->sbg;

			sbg->timePrev = sbg->time;
			sbg->time = getTicks();
			sbg->dt = sbg->time - sbg->timePrev;
			sbg->ct++;
			sbg->type = type;
			sbg->params = params;
			sbg->x = x;
			sbg->y = y;
			sbg->z = z;

			if (sbctx->gestCB)
				sbctx->gestCB(sbg, sbctx->udata_ptr);
			
			if (type == RZSBSDK_GESTURE_PRESS && !params) sbg->id++;
			if (type == RZSBSDK_GESTURE_TAP && params) sbg->id++;
			if (type == RZSBSDK_GESTURE_FLICK && params) sbg->id++;
			
			unlock(&sbctx->s);
		}
	}
	
	return S_OK;
}

static int dkLast = -1;
static int dkLastState = -1;
static int64_t dkLastTime = 0;

static CALLBACK HRESULT dynamicKeyCB (RZSBSDK_DKTYPE key, RZSBSDK_KEYSTATETYPE state)
{
	if (!g_sbctx) return S_OK;
	
	const int64_t t1 = GetTickCount();
	
	if (key != dkLast || state != dkLastState){
		dkLast = key;
		dkLastState = state;
		dkLastTime = t1;
	}else{
		unsigned int dt = t1 - dkLastTime;
		dkLastTime = t1;
		if (dt < 100) return S_OK;
		
		dkLast = key;
		dkLastState = state;
		dkLastTime = t1;
	}

	//printf("dynamicKeyCB key:%i, state:%i, %i\n", key, state, (int)GetCurrentThreadId());

	TSBCTX *sbctx = (TSBCTX*)drvToCtx(NULL/*drv*/);
	if (sbctx){
		if (lock(&sbctx->s)){
			if (sbctx->dkCB)
				sbctx->dkCB(key, state, sbctx->udata_ptr);
			unlock(&sbctx->s);
		}
	}
	return S_OK;
}


static CALLBACK HRESULT appEventCallback (RZSBSDK_EVENTTYPETYPE event, DWORD mode, DWORD processId)
{
	//printf("appEventCallback: event:%i, mode:%i, %i\n", (int)event, (int)mode, (int)processId);
	
	if (!g_sbctx) return S_OK;
	if (event == RZSBSDK_EVENT_NONE) return S_OK;

	TSBCTX *sbctx = (TSBCTX*)drvToCtx(NULL/*drv*/);
	if (!sbctx) return S_OK;
	
	if (lock(&sbctx->s)){
		if (sbctx->dkCB){
			if (event == RZSBSDK_EVENT_CLOSE)
				sbctx->dkCB(SBUI_DK_CLOSE, SBUI_DK_DOWN, sbctx->udata_ptr);
			else if (event == RZSBSDK_EVENT_EXIT)
				sbctx->dkCB(SBUI_DK_EXIT, SBUI_DK_DOWN, sbctx->udata_ptr);
			else if (event == RZSBSDK_EVENT_ACTIVATED)
				sbctx->dkCB(SBUI_DK_ACTIVATE, SBUI_DK_DOWN, sbctx->udata_ptr);
			else if (event == RZSBSDK_EVENT_DEACTIVATED)
				sbctx->dkCB(SBUI_DK_DEACTIVATE, SBUI_DK_DOWN, sbctx->udata_ptr);
		}
		unlock(&sbctx->s);
	}

	return S_OK;
}

static int sbuiStart (TDRIVER *drv, TSBCTX *sbctx)
{
	HRESULT ret = _RzSBStart();
	if (rzsb_success(ret, L"RzSBStart")){

#if 1
		rzsb_success(_RzSBEnableGesture(RZSBSDK_GESTURE_ALL, 0), L"_RzSBEnableGesture");
		rzsb_success(_RzSBEnableOSGesture(RZSBSDK_GESTURE_ALL, 0), L"RzSBEnableOSGesture");
#else
		// SBSDK default is to have all gestures enabled
		/*rzsb_success(_RzSBEnableGesture(RZSBSDK_GESTURE_PRESS, 0), L"RzSBEnableGesture");
		rzsb_success(_RzSBEnableGesture(RZSBSDK_GESTURE_TAP, 0), L"RzSBEnableGesture");
		rzsb_success(_RzSBEnableGesture(RZSBSDK_GESTURE_FLICK, 0), L"RzSBEnableGesture");
		rzsb_success(_RzSBEnableGesture(RZSBSDK_GESTURE_ZOOM, 0), L"RzSBEnableGesture");
		rzsb_success(_RzSBEnableGesture(RZSBSDK_GESTURE_ROTATE, 0), L"RzSBEnableGesture");*/
		rzsb_success(_RzSBEnableGesture(RZSBSDK_GESTURE_ALL, 0), L"RzSBEnableGesture");


		// forwarded to OS only what it needs and we don't want
		rzsb_success(_RzSBEnableOSGesture(RZSBSDK_GESTURE_PRESS, 0), L"RzSBEnableOSGesture");
		rzsb_success(_RzSBEnableOSGesture(RZSBSDK_GESTURE_TAP, 0), L"RzSBEnableOSGesture");
		rzsb_success(_RzSBEnableOSGesture(RZSBSDK_GESTURE_FLICK, 0), L"RzSBEnableOSGesture");
		rzsb_success(_RzSBEnableOSGesture(RZSBSDK_GESTURE_ZOOM, 0), L"RzSBEnableOSGesture");
		rzsb_success(_RzSBEnableOSGesture(RZSBSDK_GESTURE_ROTATE, 0), L"RzSBEnableOSGesture");
		rzsb_success(_RzSBEnableOSGesture(RZSBSDK_GESTURE_MOVE, 0), L"RzSBEnableOSGesture");
		rzsb_success(_RzSBEnableOSGesture(RZSBSDK_GESTURE_HOLD, 0), L"RzSBEnableOSGesture");
		rzsb_success(_RzSBEnableOSGesture(RZSBSDK_GESTURE_RELEASE, 0), L"RzSBEnableOSGesture");
		rzsb_success(_RzSBEnableOSGesture(RZSBSDK_GESTURE_SCROLL, 0), L"RzSBEnableOSGesture");

#endif

		rzsb_success(_RzSBAppEventSetCallback(appEventCallback), L"RzSBAppEventSetCallback");
		rzsb_success(_RzSBDynamicKeySetCallback(dynamicKeyCB), L"RzSBDynamicKeySetCallback");
		rzsb_success(_RzSBGestureSetCallback(touchpadGestureCB), L"RzSBGestureSetCallback");


		if (sbctx)
			sbctx->t0 = GetTickCount();
		return 1;
	}
	return 0;
}

static inline int sbui_getOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (drv && value){
		switch (option){
		  case lOPT_SBUI_RSTATS:
		  	return 0;

		  case lOPT_SBUI_RSTATSRESET:
		  	return 0;

		  case lOPT_SBUI_SETDK:
		  	if (value) *value = 0;
		  	return 1;
		  case lOPT_SBUI_RECONNECT:
		  case lOPT_SBUI_STRUCT:
		  case lOPT_SBUI_GESTURECB:
		  case lOPT_SBUI_DKCB:
		  case lOPT_SBUI_UDATAPTR:{
			intptr_t *opt = drv->dd->opt;
			*value = opt[option];
			return 1;
		  }
		};
	}
	return 0;
}

static int sbui_setOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (!drv) return 0;
	intptr_t *opt = drv->dd->opt;
	
	if (option == lOPT_SBUI_STRUCT){
		opt[option] = (intptr_t)value;
		g_sbctx = (TSBCTX*)value;
		return 1;
		
	}else if (option == lOPT_SBUI_SETDK){
		if (value){
			TSBGESTURESETDK *sbdk = (TSBGESTURESETDK*)value;
			if (sbdk->size == sizeof(TSBGESTURESETDK)){
				opt[option] = (intptr_t)value;
				return rzsb_success(_RzSBSetImageDynamicKey(sbdk->dk, sbdk->state, sbdk->path), L"RzSBSetImageDynamicKey");
			}
		}
		return 0;
		
	}else if (option == lOPT_SBUI_RECONNECT){
		opt[option] = 0;
		
		TSBCTX *sbctx = (TSBCTX*)opt[lOPT_SBUI_STRUCT];
		if (sbctx){
			rzsb_success(_RzSBStop(), L"RzSBStop");
			return sbuiStart(drv, sbctx);
		}
		return 0;
	}else if (option == lOPT_SBUI_DKCB){
		opt[option] = (intptr_t)value;
		
		TSBCTX *sbctx = (TSBCTX*)opt[lOPT_SBUI_STRUCT];
		if (sbctx){
			sbctx->dkCB = (pdkcb)value;
			return 1;
		}
	}else if (option == lOPT_SBUI_GESTURECB){
		opt[option] = (intptr_t)value;
		
		TSBCTX *sbctx = (TSBCTX*)opt[lOPT_SBUI_STRUCT];
		if (sbctx){
			sbctx->gestCB = (pgesturecb)value;
			return 1;
		}
	}else if (option == lOPT_SBUI_RSTATSRESET){
	  	return 0;

	}else if (option == lOPT_SBUI_RSTATS){
		return 1;

	}else if (option == lOPT_SBUI_UDATAPTR){
		opt[option] = (intptr_t)value;
		TSBCTX *sbctx = (TSBCTX*)opt[lOPT_SBUI_STRUCT];
		if (sbctx){
			sbctx->udata_ptr = (void*)value;
			return 1;
		}
	}else if (option == lOPT_SBUI_GESTURECFG){
		if (value){
			const TSBGESTURECBCFG *sbcfg = (TSBGESTURECBCFG*)value;
			if (sbcfg && sbcfg->op && sbcfg->gesture){
				switch (sbcfg->op){
			  	case SBUICB_OP_GestureEnable:
					return rzsb_success(_RzSBEnableGesture(sbcfg->gesture, sbcfg->state), L"RzSBEnableGesture");
			  	case SBUICB_OP_GestureSetNotification:
					return rzsb_success(_RzSBEnableGesture(sbcfg->gesture, sbcfg->state), L"RzSBEnableGesture");
			  	case SBUICB_OP_GestureSetOSNotification:
					return rzsb_success(_RzSBEnableOSGesture(sbcfg->gesture, sbcfg->state), L"RzSBEnableOSGesture");
				};
			}
		}
	}
	return 0;
}

static int sbui_OpenDisplay (TDRIVER *drv)
{
	if (!drv)
		return 0;

	if (drv->dd->status != LDRV_CLOSED)
		return 0;

	if (sbuiLoadDll(SBSDKDLL) <= 0){
		drv->dd->status = LDRV_CLOSED;
		return 0;
	}

	if (sbuiStart(drv, NULL)){
		const int fsize = calcFrameSize(drv->dd->width, drv->dd->height, LFRM_BPP_16);
		TSBCTX *sbctx = l_calloc(1, sizeof(TSBCTX));
		if (sbctx){
			lock_create(&sbctx->s);
			sbctx->sbg.time = getTicks();
			
			sbctx->buffer = (unsigned int*)l_malloc(fsize);
			if (sbctx->buffer){
				drv->dd->setOption(drv, lOPT_SBUI_STRUCT, (intptr_t*)sbctx);
				drv->dd->status = LDRV_READY;
				return 1;
			}
		}
	}
	
	g_sbctx = NULL;
	initOnce = 0;
	drv->dd->status = LDRV_CLOSED;
	return 0;
}

static int sbui_update (TDRIVER *drv, TSBCTX *sbctx, void *buffer, const int fsize)
{
	//printf("sbui_update\n");
	
	RZSBSDK_BUFFERPARAMS bp; 
	//memset(&bp, 0, sizeof(RZSB_BUFFERPARAMS));
	bp.pData = (BYTE*)buffer;
	bp.DataSize = fsize;
	bp.PixelType = RGB565;

	HRESULT ret = _RzSBRenderBuffer(RZSBSDK_DISPLAY_WIDGET, &bp);
	if (RZSB_FAILED(ret)){
		//printf("SBUI: RzSBRenderBuffer() failed, 0x%X\n", (unsigned int)ret);
		
		if (ret == 0x800706BA){	// == RPC_S_SERVER_UNAVAILABLE
			if (GetTickCount() - sbctx->t0 > 1500){
				_RzSBStop();
				sbuiStart(drv, sbctx);
			}
		}
	}
	
	return (int)ret;
}

static int sbui_Clear (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status == LDRV_READY){
			TSBCTX *sbctx = (TSBCTX*)drvToCtx(drv);
			const int fsize = drv->dd->width * drv->dd->height * 2;
			l_memset(sbctx->buffer, drv->dd->clr, fsize);
			sbui_update(drv, sbctx, sbctx->buffer, fsize);
			return 1;
		}
	}
	return 0;
}

static int sbui_Refresh (TDRIVER *drv, TFRAME *frame)
{
	if (!frame || !drv || drv->dd->status != LDRV_READY)
		return 0;

	TSBCTX *sbctx = drvToCtx(drv);
	if (sbctx){
		if (frame->bpp == LFRM_BPP_16){
			sbui_update(drv, sbctx, frame->pixels, frame->frameSize);
			return 1;
		}else{
			pConverterFn converter = getConverter(frame->bpp, LFRM_BPP_16);
			if (converter){
				converter(frame, sbctx->buffer);
				const int fsize = calcFrameSize(drv->dd->width, drv->dd->height, LFRM_BPP_16);
				return sbui_update(drv, sbctx, sbctx->buffer, fsize);
			}
		}
	}	
	return 0;
}

static int sbui_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	return sbui_Refresh(drv, frm);
}

#else

int sbui20Init (void *rd){return 0;}
void sbui20Close (void *rd){return;}

#endif



