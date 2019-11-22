
// Display driver for the logitech G15 keyboard LCD

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

#if ((__BUILD_G15DISPLAY__) && (__BUILD_WIN32__) && (!__BUILD_2432BITONLY_SUPPORT__))

#include "../memory.h"
#include "../frame.h"
#include "../device.h"
#include "../sync.h"
#include "../convert.h"
#include "../lstring.h"
#include "../misc.h"
#include "lglcd.dll/lg_lcd.h"
#include "g15display.h"
#include "g15.h"
 

int g15Display_OpenDisplay (TDRIVER *drv);
int g15Display_CloseDisplay (TDRIVER *drv);
int g15Display_ClearDisplay (TDRIVER *drv);
int g15Display_Refresh (TDRIVER *drv, TFRAME *frm);
int g15Display_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
int g15Display_getOption (TDRIVER *drv, int option, intptr_t *value);
int g15Display_setOption (TDRIVER *drv, int option, intptr_t *value);

static int g15Display_update (TDRIVER *drv, TFRAME *frm);
static void clearDisplay (TDRIVER *drv);
static int openG15 (TDRIVER *drv);
static void closeG15 (TDRIVER *drv);
static DWORD WINAPI softButtonCB (int device, DWORD dwButtons, const PVOID pContext);
unsigned int __stdcall g15_frameDispatcher (TMYLCDG15 *mylcdg15);
void printG15ErrorCode (int code);



int initG15Display (TREGISTEREDDRIVERS *rd)
{
	TDISPLAYDRIVER dd;
	int dnumber;
	
	dd.open = g15Display_OpenDisplay;
	dd.close = g15Display_CloseDisplay;
	dd.clear = g15Display_ClearDisplay;
	dd.refresh = g15Display_Refresh;
	dd.refreshArea = g15Display_RefreshArea;
	dd.setOption = g15Display_setOption;
	dd.getOption = g15Display_getOption;
	dd.status = LDRV_CLOSED;
	dd.optTotal = 4;
	
	l_strcpy(dd.name, "G15");
	l_strcpy(dd.comment, "Logitech G15 (LCDMon)");
	dnumber = registerDisplayDriver(rd, &dd);
	setDefaultDisplayOption(rd, dnumber, lOPT_G15_DEVICE, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_G15_STRUCT, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_G15_PRIORITY, G15_PRIORITY_ASYNC);
	setDefaultDisplayOption(rd, dnumber, lOPT_G15_SOFTKEYCB, 0);
	return (dnumber > 0);
}

void closeG15Display (TREGISTEREDDRIVERS *rd)
{
	return;
}

int g15Display_getOption (TDRIVER *drv, int option, intptr_t *value)
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

int g15Display_setOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (!drv)
		return 0;
	if (option >= drv->dd->optTotal)
		return 0;

	intptr_t *opt = (intptr_t*)drv->dd->opt;

	if (option == lOPT_G15_STRUCT){
		opt[lOPT_G15_STRUCT] = (intptr_t)value;
		
	}else if (option == lOPT_G15_PRIORITY){
		TMYLCDG15 *mylcdg15 = (TMYLCDG15 *)opt[lOPT_G15_STRUCT];
		opt[lOPT_G15_PRIORITY] = *value;
		mylcdg15->priority = *value;
		
	}else if (option == lOPT_G15_SOFTKEYCB){
		TMYLCDG15 *mylcdg15 = (TMYLCDG15 *)opt[lOPT_G15_STRUCT];
		opt[lOPT_G15_SOFTKEYCB] = (intptr_t)value;
		mylcdg15->softkeycb = (psoftkeycb)value;
		
	}else if (option == lOPT_G15_DEVICE){
		opt[lOPT_G15_DEVICE] = *value;
		
	}else{
		return 0;
	}
	return 1;
}

int g15Display_OpenDisplay (TDRIVER *drv)
{
	if (drv){
		if ((drv->dd->WIDTH != LGLCD_BMP_WIDTH) || (drv->dd->HEIGHT != LGLCD_BMP_HEIGHT)){
			mylog("libmylcd: G15 LCD resolution is %i by %i pixels which is not optional\n", LGLCD_BMP_WIDTH, LGLCD_BMP_HEIGHT);
			drv->dd->WIDTH = LGLCD_BMP_WIDTH;
			drv->dd->HEIGHT = LGLCD_BMP_HEIGHT;
		}
				
		if (drv->dd->status == LDRV_CLOSED){
			TMYLCDG15 *mylcdg15 = (TMYLCDG15*)l_calloc(1, sizeof(TMYLCDG15));
			if (mylcdg15){
				mylcdg15->lg = (TLOGITECHG*)l_calloc(1, sizeof(TLOGITECHG));
				if (mylcdg15->lg){
					g15Display_setOption(drv, lOPT_G15_STRUCT, (intptr_t*)mylcdg15);
					intptr_t devicei = drv->dd->port;
					g15Display_setOption(drv, lOPT_G15_DEVICE, (intptr_t*)&devicei);
					
					if (openG15(drv)){
						int errorcode = (int)lg_LcdSetAsLCDForegroundApp(mylcdg15->lg->openContext.device, LGLCD_LCD_FOREGROUND_APP_YES);
						if (errorcode != ERROR_SUCCESS && errorcode != ERROR_LOCK_FAILED){
							mylog("lg_LcdSetAsLCDForegroundApp(): ");
							printG15ErrorCode(errorcode);
							closeG15(drv);
							l_free(mylcdg15->lg);
							l_free(mylcdg15);
							drv->dd->status = LDRV_CLOSED;
							return 0;
						}else{
						
							drv->dd->back = _newFrame(drv->dd->hw, drv->dd->width, drv->dd->height, 1, LFRM_BPP_1);
							mylcdg15->back = _newFrame(drv->dd->hw, drv->dd->width, drv->dd->height, 1, LFRM_BPP_1);
							if (drv->dd->back == NULL){
								drv->dd->status = LDRV_CLOSED;
								return 0;
							}
							drv->dd->status = LDRV_READY;
							drv->dd->clear(drv);
							return 1;
						}
					}else{
						l_free(mylcdg15->lg);
						l_free(mylcdg15);
						g15Display_setOption(drv, lOPT_G15_STRUCT, 0);
						drv->dd->status = LDRV_CLOSED;
					}
				}
			}
		}
	}
	return 0;
}

int g15Display_CloseDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status != LDRV_CLOSED){
			drv->dd->status = LDRV_CLOSED;
			if (drv->dd->back)
				deleteFrame(drv->dd->back);

			closeG15(drv);
			void *ptr = NULL;
			drv->dd->getOption(drv, lOPT_G15_STRUCT, (void*)&ptr);
			if (ptr){
				if (((TMYLCDG15*)ptr)->back)
					deleteFrame(((TMYLCDG15*)ptr)->back);				
				if (((TMYLCDG15*)ptr)->lg)
					l_free(((TMYLCDG15*)ptr)->lg);
				l_free(ptr);
			}
			return 1;
		}
	}
	return 0;
}

int g15Display_ClearDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status != LDRV_CLOSED){
			clearDisplay(drv);
			if (drv->dd->back)
				clearFrame(drv->dd->back);
			return 1;
		}
	}
	return 0;
}

int g15Display_Refresh (TDRIVER *drv, TFRAME *frm)
{
	if (!frm || !drv)
		return 0;

	if (drv->dd->status != LDRV_READY)
		return 0;

	return g15Display_update(drv, frm);
}

int g15Display_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	if (!frm || !drv)
		return 0;

	if (drv->dd->status != LDRV_READY)
		return 0;
	return g15Display_update(drv, frm);
}

static int signalFrameRefresh (TMYLCDG15 *mylcdg15, TFRAME *frm)
{
	frame1To8BPP(frm, &mylcdg15->lg->bmp.pixels, 0, 255);
	return (ERROR_SUCCESS == lg_LcdUpdateBitmap(mylcdg15->lg->openContext.device, &mylcdg15->lg->bmp.hdr, mylcdg15->priority));
}

static int g15Display_update (TDRIVER *drv, TFRAME *frm)
{
	intptr_t *opt = (intptr_t*)drv->dd->opt;
	TMYLCDG15 *mylcdg15 = (TMYLCDG15*)opt[lOPT_G15_STRUCT];

  	return signalFrameRefresh(mylcdg15, frm);
}

static void clearDisplay (TDRIVER *drv)
{
	if (drv){
		intptr_t *opt = (intptr_t*)drv->dd->opt;
		TMYLCDG15 *mylcdg15 = (TMYLCDG15 *)opt[lOPT_G15_STRUCT];
		
  		l_memset(&mylcdg15->lg->bmp.pixels, drv->dd->clr, sizeof(mylcdg15->lg->bmp.pixels));
   		lg_LcdUpdateBitmap(mylcdg15->lg->openContext.device, &mylcdg15->lg->bmp.hdr, mylcdg15->priority);
	}
}

void printG15ErrorCode (int code)
{
#ifdef __DEBUG__
	switch(code){
		case ERROR_SERVICE_NOT_ACTIVE:
			mylog("libmylcd: G15: ERROR_SERVICE_NOT_ACTIVE (lg_LcdInit() has not been called)\n");
			break;
		case ERROR_ALREADY_EXISTS:
			mylog("libmylcd: G15: ERROR_ALREADY_EXISTS\n");
			break;
		case ERROR_INVALID_PARAMETER:
			mylog("libmylcd: G15: ERROR_INVALID_PARAMETER\n");
			break;
		case ERROR_FILE_NOT_FOUND:
			mylog("libmylcd: G15: ERROR_FILE_NOT_FOUND (LCDMon is not running)\n");
			break;
		case RPC_X_WRONG_PIPE_VERSION:
			mylog("libmylcd: G15: RPC_X_WRONG_PIPE_VERSION\n");
			break;
		case ERROR_NO_MORE_ITEMS:
			mylog("libmylcd: G15: ERROR_NO_MORE_ITEMS\n");
			break;
		case ERROR_DEVICE_NOT_CONNECTED:
			mylog("libmylcd: G15: ERROR_DEVICE_NOT_CONNECTED\n");
			break;
		case ERROR_ACCESS_DENIED:
			mylog("libmylcd: G15: ERROR_ACCESS_DENIED\n");
			break;
		case ERROR_LOCK_FAILED:
			mylog("libmylcd: G15: ERROR_LOCK_FAILED\n");
			break;
		case ERROR_OLD_WIN_VERSION:
			mylog("libmylcd: G15: ERROR_OLD_WIN_VERSION\n");
			break;
		case ERROR_NO_SYSTEM_RESOURCES:
			mylog("libmylcd: G15: ERROR_NO_SYSTEM_RESOURCES\n");
			break;
		case RPC_S_SERVER_UNAVAILABLE: 
			mylog("libmylcd: G15: RPC_S_SERVER_UNAVAILABLE\n");
			break;
		case ERROR_ALREADY_INITIALIZED:
			mylog("libmylcd: G15: ERROR_ALREADY_INITIALIZED\n");
			break;
		case ERROR_SUCCESS:
			mylog("libmylcd: G15: No error\n");
			break;
		default:
			mylog("libmylcd: G15: Unknown or unexpected error ocde %i\n", code);
	};
#endif
}

static int openG15 (TDRIVER *drv)
{
	int errorcode = (int)lg_LcdInit();
    if (ERROR_SUCCESS != errorcode && ERROR_ALREADY_INITIALIZED != errorcode){
    	mylog("lg_LcdInit(): ");
		printG15ErrorCode(errorcode);
    	return 0;
    }

	TMYLCDG15 *mylcdg15 = NULL;
	drv->dd->getOption(drv, lOPT_G15_STRUCT, (void*)&mylcdg15);
	if (mylcdg15 == NULL){
		mylog("libmylcd: could not obtain TMYLCDG15 handle\n");
		lg_LcdDeInit();
		return 0;
	}

	mylcdg15->lg->bmp.hdr.Format = LGLCD_BMP_FORMAT_160x43x1;
	mylcdg15->softkeycb = NULL;
	mylcdg15->intUser = 0;
	mylcdg15->ptrUser = NULL;
	drv->dd->getOption(drv, lOPT_G15_PRIORITY, (intptr_t*)&mylcdg15->priority);

	l_memset(&mylcdg15->lg->connectContext, 0, sizeof(lgLcdConnectContext));
    mylcdg15->lg->connectContext.appFriendlyName = (char *)"g15 applet";
	mylcdg15->lg->connectContext.isAutostartable = FALSE;
	mylcdg15->lg->connectContext.isPersistent = TRUE;
	mylcdg15->lg->connectContext.onConfigure.configCallback = NULL;
	mylcdg15->lg->connectContext.onConfigure.configContext = NULL;
	mylcdg15->lg->connectContext.connection = LGLCD_INVALID_CONNECTION;
	
	errorcode = (int)lg_LcdConnect(&mylcdg15->lg->connectContext);
	if (ERROR_SUCCESS != errorcode){
		mylog("lg_LcdConnect(): ");
		printG15ErrorCode(errorcode);
		lg_LcdDeInit();
		return 0;
	}	
	
	intptr_t devicei = 0;
	g15Display_getOption(drv, lOPT_G15_DEVICE, (intptr_t*)&devicei);

	errorcode = (int)lg_LcdEnumerate(mylcdg15->lg->connectContext.connection, devicei, &mylcdg15->lg->deviceDescription);
	if (ERROR_SUCCESS != errorcode){
		mylog("lg_LcdEnumerate(): ");
		printG15ErrorCode(errorcode);
		lg_LcdDisconnect(mylcdg15->lg->connectContext.connection);
		lg_LcdDeInit();
		return 0;
	}
	if ((mylcdg15->lg->deviceDescription.Width != LGLCD_BMP_WIDTH) && (mylcdg15->lg->deviceDescription.Height != LGLCD_BMP_HEIGHT)){
		mylog("libmylcd: invalid width or height\n");
		lg_LcdDisconnect(mylcdg15->lg->connectContext.connection);
       	lg_LcdDeInit();
		return 0;
	}

    l_memset(&mylcdg15->lg->openContext, 0, sizeof(lgLcdOpenContext));
    mylcdg15->lg->openContext.index = devicei;
    mylcdg15->lg->openContext.connection = mylcdg15->lg->connectContext.connection;
    mylcdg15->lg->openContext.onSoftbuttonsChanged.softbuttonsChangedCallback = softButtonCB;
    mylcdg15->lg->openContext.onSoftbuttonsChanged.softbuttonsChangedContext = drv;
    mylcdg15->lg->openContext.device = LGLCD_INVALID_DEVICE;
    
    errorcode = (int)lg_LcdOpen(&mylcdg15->lg->openContext);
	if (ERROR_SUCCESS != errorcode){
		mylog("lg_LcdOpen(): ");
		printG15ErrorCode(errorcode);
		lg_LcdDisconnect(mylcdg15->lg->connectContext.connection);
		lg_LcdDeInit();
		return 0;
	}else{
		return 1;
	}
}

static DWORD WINAPI softButtonCB (int device, DWORD dwButtons, const PVOID pContext)
{
	if (pContext != NULL){
		TDRIVER *drv = (TDRIVER *)pContext;
		TMYLCDG15 *mylcdg15 = NULL;
		drv->dd->getOption(drv, lOPT_G15_STRUCT, (void*)&mylcdg15);
		if (mylcdg15){
			if ((intptr_t)mylcdg15->softkeycb)
				mylcdg15->softkeycb(device, dwButtons, (struct TMYLCDG15 *)mylcdg15);
		}
	}
	return 0;
}

static void closeG15 (TDRIVER *drv)
{
	TMYLCDG15 *mylcdg15 = NULL;
	drv->dd->getOption(drv, lOPT_G15_STRUCT, (void*)&mylcdg15);
	if (mylcdg15 != NULL){
    	lg_LcdClose(mylcdg15->lg->openContext.device);
    	lg_LcdDisconnect(mylcdg15->lg->connectContext.connection);
    	lg_LcdDeInit();
    }
}

#else

int initG15Display(void *rd){return 1;}
void closeG15Display(void *rd){return;}

#endif

