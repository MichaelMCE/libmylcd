
// Display driver for the logitech G19 keyboard LCD

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

#if ((__BUILD_G19DISPLAY__) && (__BUILD_WIN32__))


#include "../memory.h"
#include "../frame.h"
#include "../device.h"
#include "../sync.h"
#include "../convert.h"
#include "../lstring.h"
#include "../misc.h"
#include "../g15/lglcd.dll/lg_lcd.h"
#include "g19display.h"
#include "g19.h"


#define APPLETNAME "G19 applet <update me>"

 

int g19Display_OpenDisplay (TDRIVER *drv);
int g19Display_CloseDisplay (TDRIVER *drv);
int g19Display_ClearDisplay (TDRIVER *drv);
int g19Display_Refresh (TDRIVER *drv, TFRAME *frm);
int g19Display_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
int g19Display_getOption (TDRIVER *drv, int option, intptr_t *value);
int g19Display_setOption (TDRIVER *drv, int option, intptr_t *value);

static int g19Display_update (TDRIVER *drv, TFRAME *frm);
static void clearDisplay (TDRIVER *drv);
static int openG19 (TDRIVER *drv);
static void closeG19 (TDRIVER *drv);
DWORD WINAPI softButtonCB (int device, DWORD dwButtons, const PVOID pContext);
DWORD WINAPI OnLCDNotificationCallback (int connection, const PVOID pContext, DWORD notificationCode,
	DWORD notifyParm1, DWORD notifyParm2, DWORD notifyParm3, DWORD notifyParm4);
void printG19ErrorCode (int code);



int initG19Display (TREGISTEREDDRIVERS *rd)
{
	TDISPLAYDRIVER dd;
	int dnumber;

	
	dd.open = g19Display_OpenDisplay;
	dd.close = g19Display_CloseDisplay;
	dd.clear = g19Display_ClearDisplay;
	dd.refresh = g19Display_Refresh;
	dd.refreshArea = g19Display_RefreshArea;
	dd.setOption = g19Display_setOption;
	dd.getOption = g19Display_getOption;
	dd.status = LDRV_CLOSED;
	dd.optTotal = 3;
	
	l_strcpy(dd.name, "G19");
	l_strcpy(dd.comment, "Logitech G19 (LCDMon)");
	dnumber = registerDisplayDriver(rd, &dd);
	setDefaultDisplayOption(rd, dnumber, lOPT_G19_STRUCT, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_G19_PRIORITY, G19_PRIORITY_ASYNC);
	setDefaultDisplayOption(rd, dnumber, lOPT_G19_SOFTKEYCB, 0);
	//setDefaultDisplayOption(rd, dnumber, lOPT_G19_DEVICE, 0);
			
	return (dnumber > 0);
}

void closeG19Display (TREGISTEREDDRIVERS *rd)
{
	return;
}

int g19Display_getOption (TDRIVER *drv, int option, intptr_t *value)
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

int g19Display_setOption (TDRIVER *drv, int option, intptr_t *value)
{
	
	if (!drv)
		return 0;
	if (option >= drv->dd->optTotal)
		return 0;

	intptr_t *opt = (intptr_t*)drv->dd->opt;

	if (option == lOPT_G19_STRUCT){
		opt[lOPT_G19_STRUCT] = (intptr_t)value;
		
	}else if (option == lOPT_G19_PRIORITY){
		TMYLCDG19 *mylcdg19 = (TMYLCDG19 *)opt[lOPT_G19_STRUCT];
		opt[lOPT_G19_PRIORITY] = *value;
		mylcdg19->priority = *value;
		
	}else if (option == lOPT_G19_SOFTKEYCB){
		TMYLCDG19 *mylcdg19 = (TMYLCDG19 *)opt[lOPT_G19_STRUCT];
		opt[lOPT_G19_SOFTKEYCB] = (intptr_t)value;
		mylcdg19->softkeycb = (psoftkeyg19cb)value;
		
	//}else if (option == lOPT_G19_DEVICE){
	//	opt[lOPT_G19_DEVICE] = *value;
		
	}else{
		return 0;
	}
	return 1;
}

int g19Display_OpenDisplay (TDRIVER *drv)
{
	
	if (drv){
		if ((drv->dd->WIDTH != LGLCD_QVGA_BMP_WIDTH) || (drv->dd->HEIGHT != LGLCD_QVGA_BMP_HEIGHT)){
			mylog("libmylcd: G19 LCD resolution is %i by %i pixels which is not optional\n", LGLCD_QVGA_BMP_WIDTH, LGLCD_QVGA_BMP_HEIGHT);
			drv->dd->WIDTH = LGLCD_QVGA_BMP_WIDTH;
			drv->dd->HEIGHT = LGLCD_QVGA_BMP_HEIGHT;
		}
		if (drv->dd->status == LDRV_CLOSED){
			TMYLCDG19 *mylcdg19 = (TMYLCDG19*)l_calloc(1, sizeof(TMYLCDG19));
			if (mylcdg19){
				mylcdg19->lg = (TLOGITECHG19*)l_calloc(1, sizeof(TLOGITECHG19));
				if (mylcdg19->lg){
					g19Display_setOption(drv, lOPT_G19_STRUCT, (intptr_t*)mylcdg19);
					if (openG19(drv)){
						int errorcode = (int)lg_LcdSetAsLCDForegroundApp(mylcdg19->lg->openContext.device, LGLCD_LCD_FOREGROUND_APP_YES);
						if (errorcode != ERROR_SUCCESS && errorcode != ERROR_LOCK_FAILED){
							mylog("lg_LcdSetAsLCDForegroundApp(): ");
							printG19ErrorCode(errorcode);
							closeG19(drv);
							l_free(mylcdg19->lg);
							l_free(mylcdg19);
							drv->dd->status = LDRV_CLOSED;
							return 0;
						}else{
							drv->dd->status = LDRV_READY;
							drv->dd->clear(drv);
							return 1;
						}
					}else{
						l_free(mylcdg19->lg);
						l_free(mylcdg19);
						g19Display_setOption(drv, lOPT_G19_STRUCT, NULL);
						drv->dd->status = LDRV_CLOSED;
					}
				}
			}
		}
	}
	return 0;
}

int g19Display_CloseDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status != LDRV_CLOSED){
			drv->dd->status = LDRV_CLOSED;
			if (drv->dd->back)
				deleteFrame(drv->dd->back);

			closeG19(drv);
			void *ptr = NULL;
			drv->dd->getOption(drv, lOPT_G19_STRUCT, (void*)&ptr);
			if (ptr){
				if (((TMYLCDG19*)ptr)->lg)
					l_free(((TMYLCDG19*)ptr)->lg);
				l_free(ptr);
			}
			return 1;
		}
	}
	return 0;
}

int g19Display_ClearDisplay (TDRIVER *drv)
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

int g19Display_Refresh (TDRIVER *drv, TFRAME *frm)
{
	if (frm == NULL || drv == NULL)
		return 0;
	else if (drv->dd->status != LDRV_READY)
		return 0;
	else
		return g19Display_update(drv, frm);
}

int g19Display_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	return g19Display_Refresh(drv,frm);
}

static int signalFrameRefresh (TMYLCDG19 *mylcdg19, TFRAME *frm)
{
	if (mylcdg19->lg->contextStatus){
		pConverterFn converter = getConverter(frm->bpp, LFRM_BPP_32);
		if (converter){
			converter(frm, (unsigned int*)&mylcdg19->lg->bmp.pixels);
			return (ERROR_SUCCESS == lg_LcdUpdateBitmap(mylcdg19->lg->openContext.device, &mylcdg19->lg->bmp.hdr, mylcdg19->priority));
		}
	}
	return 0;
}

static int g19Display_update (TDRIVER *drv, TFRAME *frm)
{
	intptr_t *opt = (intptr_t*)drv->dd->opt;
	TMYLCDG19 *mylcdg19 = (TMYLCDG19*)opt[lOPT_G19_STRUCT];
  	return signalFrameRefresh(mylcdg19, frm);
}

static void clearDisplay (TDRIVER *drv)
{
	if (drv){
		intptr_t *opt = (intptr_t*)drv->dd->opt;
		TMYLCDG19 *mylcdg19 = (TMYLCDG19 *)opt[lOPT_G19_STRUCT];
		if (mylcdg19->lg->contextStatus){
  			l_memset(&mylcdg19->lg->bmp.pixels, drv->dd->clr, sizeof(mylcdg19->lg->bmp.pixels));
   			lg_LcdUpdateBitmap(mylcdg19->lg->openContext.device, &mylcdg19->lg->bmp.hdr, mylcdg19->priority);
   		}
	}
}

static void closeG19 (TDRIVER *drv)
{
	TMYLCDG19 *mylcdg19 = NULL;
	drv->dd->getOption(drv, lOPT_G19_STRUCT, (void*)&mylcdg19);
	if (mylcdg19 != NULL){
    	lg_LcdClose(mylcdg19->lg->openContext.device);
    	lg_LcdDisconnect(mylcdg19->lg->connectContext.connection);
    	lg_LcdDeInit();
    }
}

void printG19ErrorCode (int code)
{
#ifdef __DEBUG__
	switch(code){
		case ERROR_SERVICE_NOT_ACTIVE:
			mylog("libmylcd: G19: ERROR_SERVICE_NOT_ACTIVE (lg_LcdInit() has not been called)\n");
			break;
		case ERROR_ALREADY_EXISTS:
			mylog("libmylcd: G19: ERROR_ALREADY_EXISTS\n");
			break;
		case ERROR_INVALID_PARAMETER:
			mylog("libmylcd: G19: ERROR_INVALID_PARAMETER\n");
			break;
		case ERROR_FILE_NOT_FOUND:
			mylog("libmylcd: G19: ERROR_FILE_NOT_FOUND (LCDMon is not running)\n");
			break;
		case RPC_X_WRONG_PIPE_VERSION:
			mylog("libmylcd: G19: RPC_X_WRONG_PIPE_VERSION\n");
			break;
		case ERROR_NO_MORE_ITEMS:
			mylog("libmylcd: G19: ERROR_NO_MORE_ITEMS\n");
			break;
		case ERROR_DEVICE_NOT_CONNECTED:
			mylog("libmylcd: G19: ERROR_DEVICE_NOT_CONNECTED\n");
			break;
		case ERROR_ACCESS_DENIED:
			mylog("libmylcd: G19: ERROR_ACCESS_DENIED\n");
			break;
		case ERROR_LOCK_FAILED:
			mylog("libmylcd: G19: ERROR_LOCK_FAILED\n");
			break;
		case ERROR_OLD_WIN_VERSION:
			mylog("libmylcd: G19: ERROR_OLD_WIN_VERSION\n");
			break;
		case ERROR_NO_SYSTEM_RESOURCES:
			mylog("libmylcd: G19: ERROR_NO_SYSTEM_RESOURCES\n");
			break;
		case RPC_S_SERVER_UNAVAILABLE: 
			mylog("libmylcd: G19: RPC_S_SERVER_UNAVAILABLE\n");
			break;
		case ERROR_ALREADY_INITIALIZED:
			mylog("libmylcd: G19: ERROR_ALREADY_INITIALIZED\n");
			break;
		case ERROR_SUCCESS:
			mylog("libmylcd: G19: No error\n");
			break;
		default:
			mylog("libmylcd: G19: Unknown or unexpected error ocde %i\n", code);
	};
#endif
}

static int openG19 (TDRIVER *drv)
{
	
	int errorcode = (int)lg_LcdInit();
    if (ERROR_SUCCESS != errorcode && ERROR_ALREADY_INITIALIZED != errorcode){
    	mylog("lg_LcdInit(): ");
		printG19ErrorCode(errorcode);
    	return 0;
    }

	TMYLCDG19 *mylcdg19 = NULL;
	drv->dd->getOption(drv, lOPT_G19_STRUCT, (void*)&mylcdg19);
	if (mylcdg19 == NULL){
		mylog("libmylcd: could not obtain TMYLCDG19 handle\n");
		lg_LcdDeInit();
		return 0;
	}

	mylcdg19->lg->bmp.hdr.Format = LGLCD_BMP_FORMAT_QVGAx32;
	mylcdg19->lg->contextStatus = 1;
	mylcdg19->softkeycb = NULL;
	mylcdg19->intUser = 0;
	mylcdg19->ptrUser = NULL;
	drv->dd->getOption(drv, lOPT_G19_PRIORITY, (intptr_t*)&mylcdg19->priority);	

	memset(&mylcdg19->lg->connectContext, 0, sizeof(lgLcdConnectContextExA));

    mylcdg19->lg->connectContext.appFriendlyName = (char*)APPLETNAME;
	mylcdg19->lg->connectContext.connection = LGLCD_INVALID_CONNECTION;
	mylcdg19->lg->connectContext.dwAppletCapabilitiesSupported = LGLCD_APPLET_CAP_QVGA;
	mylcdg19->lg->connectContext.isAutostartable = FALSE;
	mylcdg19->lg->connectContext.isPersistent = FALSE;
	mylcdg19->lg->connectContext.onConfigure.configCallback = NULL;
	mylcdg19->lg->connectContext.onConfigure.configContext = NULL;
	mylcdg19->lg->connectContext.onNotify.notificationCallback = OnLCDNotificationCallback;
	mylcdg19->lg->connectContext.onNotify.notifyContext = drv;
	errorcode = (int)lg_LcdConnectEx(&mylcdg19->lg->connectContext);
	if (ERROR_SUCCESS != errorcode){
		mylog("lg_LcdConnect(): ");
		printG19ErrorCode(errorcode);
		lg_LcdDeInit();
		return 0;
	}	
	
	//int devicei = 0;
	//drv->dd->getOption(drv, lOPT_G19_DEVICE, &devicei);
    
    l_memset(&mylcdg19->lg->openContext, 0, sizeof(lgLcdOpenByTypeContext));
    // Let's attempt to open up a colour device
	//mylcdg19->lg->openContext.index = devicei;
    mylcdg19->lg->openContext.connection = mylcdg19->lg->connectContext.connection;
    mylcdg19->lg->openContext.deviceType = LGLCD_DEVICE_QVGA;
    // we have no softbutton notification callback
    mylcdg19->lg->openContext.onSoftbuttonsChanged.softbuttonsChangedCallback = softButtonCB;
    mylcdg19->lg->openContext.onSoftbuttonsChanged.softbuttonsChangedContext = drv;
    // the "device" member will be returned upon return
    mylcdg19->lg->openContext.device = LGLCD_INVALID_DEVICE;

    errorcode = lg_LcdOpenByType(&mylcdg19->lg->openContext);
	if (ERROR_SUCCESS != errorcode){
		mylog("lg_LcdOpenByType(): ");
		printG19ErrorCode(errorcode);
		lg_LcdDisconnect(mylcdg19->lg->connectContext.connection);
		lg_LcdDeInit();
		return 0;
	}else{
		return 1;
	}
}


DWORD WINAPI softButtonCB (int device, DWORD dwButtons, const PVOID pContext)
{
	if (pContext != NULL){
		TDRIVER *drv = (TDRIVER *)pContext;
		TMYLCDG19 *mylcdg19 = NULL;
		drv->dd->getOption(drv, lOPT_G19_STRUCT, (intptr_t*)&mylcdg19);
		if (mylcdg19){
			if ((intptr_t)mylcdg19->softkeycb)
				mylcdg19->softkeycb(device, dwButtons, (struct TMYLCDG19*)mylcdg19);
		}
	}
	return 0;
}


DWORD WINAPI OnLCDNotificationCallback (int connection, const PVOID pContext, DWORD notificationCode,
	DWORD notifyParm1, DWORD notifyParm2, DWORD notifyParm3, DWORD notifyParm4)
{
	
	TMYLCDG19 *mylcdg19 = NULL;
	
	if (pContext != NULL){
		TDRIVER *drv = (TDRIVER*)pContext;
		mylcdg19 = NULL;
		drv->dd->getOption(drv, lOPT_G19_STRUCT, (intptr_t*)&mylcdg19);
		if (mylcdg19 == NULL)
			return 1;
	}else{
	 	return 1;
	}
	
	switch (notificationCode){
      case LGLCD_NOTIFICATION_DEVICE_ARRIVAL:
      case LGLCD_NOTIFICATION_APPLET_ENABLED:
		mylcdg19->lg->contextStatus = 1;
		break;
        
      case LGLCD_NOTIFICATION_DEVICE_REMOVAL:
      case LGLCD_NOTIFICATION_APPLET_DISABLED:
      case LGLCD_NOTIFICATION_CLOSE_CONNECTION:
      case LGLCD_NOTIFICATION_TERMINATE_APPLET:
		mylcdg19->lg->contextStatus = 0;
		break;

      default:
		break;
	}

    return 1;
}

#else

int initG19Display(void *rd){return 1;}
void closeG19Display(void *rd){return;}

#endif

