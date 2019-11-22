
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

#if ((__BUILD_DDRAW__) && (__BUILD_WIN32__))

#include "../memory.h"
#include "../utils.h"
#include "../pixel.h"
#include "../device.h"
#include "../lstring.h"
#include "../convert.h"
#include "ddrawdisplay.h"
#include "dd.h"


static int directDraw_OpenDisplay (TDRIVER *drv);
static int directDraw_CloseDisplay (TDRIVER *drv);
static int directDraw_Clear (TDRIVER *drv);
static int directDraw_Refresh (TDRIVER *drv, TFRAME *frm);
static int directDraw_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static int directDraw_setOption (TDRIVER *drv, int option, intptr_t *value);
static int directDraw_getOption (TDRIVER *drv, int option, intptr_t *value);
static int createDirectDrawWindow (TDRIVER *drv);




int initDDrawDisplay (TREGISTEREDDRIVERS *rd)
{
	if (!registerWC())
		return 0;

	TDISPLAYDRIVER dd;
	l_strcpy(dd.comment, "DirectDraw virtual display");
	dd.open = directDraw_OpenDisplay;
	dd.close = directDraw_CloseDisplay;
	dd.clear = directDraw_Clear;
	dd.refresh = directDraw_Refresh;
	dd.refreshArea = directDraw_RefreshArea;
	dd.setOption = directDraw_setOption;
	dd.getOption = directDraw_getOption;
	dd.status = LDRV_CLOSED;
	dd.optTotal = 5;

	l_strcpy(dd.name, "DDRAW");
	int dnumber = registerDisplayDriver(rd, &dd);
	
	setDefaultDisplayOption(rd, dnumber, lOPT_DDRAW_STRUCT, 0);
	setDefaultDisplayOption(rd, dnumber, lOPT_DDRAW_HWNDTARGET, 0);
	

	/*const int ddrawtotal = 6;
	int i;
	for (i = 2; i <= ddrawtotal; i++){
		sprintf(dd.name, "DDRAW:%i", i);
		dnumber = registerDisplayDriver(rd, &dd);
		setDefaultDisplayOption(rd, dnumber, lOPT_DDRAW_STRUCT, 0);	
	}*/
	return (dnumber > 0);
}

void closeDDrawDisplay (TREGISTEREDDRIVERS *rd)
{
	releaseDDLib();
}
 
static TMYLCDDDRAW *DrvToDDraw (TDRIVER *drv)
{
	TMYLCDDDRAW *mddraw = NULL;
	drv->dd->getOption(drv, lOPT_DDRAW_STRUCT, (void*)&mddraw);
	return mddraw;
}

static int directDraw_getOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (drv && value){
		if (option == lOPT_DDRAW_GETHWND){
			TMYLCDDDRAW *mddraw = DrvToDDraw(drv);
			if (mddraw){
				*value = (intptr_t)mddraw->wnd;
				return 1;
			}
		}
		intptr_t *opt = drv->dd->opt;
		*value = opt[option];
		return 1;
	}else{
		return 0;
	}
}

static int directDraw_setOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (!drv) return 0;
	intptr_t *opt = drv->dd->opt;
	
	if (option == lOPT_DDRAW_STRUCT){
		opt[lOPT_DDRAW_STRUCT] = (intptr_t)value;
		return 1;
		
	}else if (option == lOPT_DDRAW_SHOW){
		TMYLCDDDRAW *mddraw = DrvToDDraw(drv);
		if (mddraw){
			ShowWindow(mddraw->wnd, SW_RESTORE);
			ShowWindow(mddraw->wnd, SW_SHOW);
			SetForegroundWindow(mddraw->wnd);
			BringWindowToTop(mddraw->wnd);
			SetFocus(mddraw->wnd);
		}
			
	}else if (option == lOPT_DDRAW_HIDE){
		TMYLCDDDRAW *mddraw = DrvToDDraw(drv);
		if (mddraw)
			ShowWindow(mddraw->wnd, SW_HIDE);
			
	}else if (option == lOPT_DDRAW_HWNDTARGET){
		TMYLCDDDRAW *mddraw = DrvToDDraw(drv);
		if (mddraw){
			mddraw->hTargetWin = (HWND)value;
			opt[lOPT_DDRAW_HWNDTARGET] = (intptr_t)value;
			return 1;
		}
	}
	return 0;
}

static int directDraw_CloseDisplay (TDRIVER *drv)
{
	if (!drv){
		return 0;
	}else{
		if (drv->dd->status != LDRV_CLOSED){
			drv->dd->status = LDRV_CLOSED;
			
			TMYLCDDDRAW *mddraw = DrvToDDraw(drv);
			if (mddraw){
				//directDraw_close(mddraw);
				PostMessage(mddraw->wnd, WM_DD_CLOSE, 0, 0);
				WaitForSingleObject(mddraw->hThread, INFINITE);
				l_free(mddraw->buffer);
				l_free(mddraw);
			} 
			return 1;
		}
	}
	return 0;
}

static int directDraw_OpenDisplay (TDRIVER *drv)
{
	if (!drv)
		return 0;
	
	if (drv->dd->status != LDRV_CLOSED)
		return 0;

	if (createDirectDrawWindow(drv)){
		drv->dd->status = LDRV_READY;
		return 1;
	}else{
		drv->dd->status = LDRV_CLOSED;
		return 0;
	}
}

static int createDirectDrawWindow (TDRIVER *drv)
{
	TMYLCDDDRAW *mddraw = l_calloc(1, sizeof(TMYLCDDDRAW));
	if (mddraw){
		mddraw->bufferLength = drv->dd->width * drv->dd->height * sizeof(unsigned int);
		mddraw->buffer =(unsigned int *)l_calloc(drv->dd->width * drv->dd->height, sizeof(unsigned int));
		if (mddraw->buffer){
			drv->dd->setOption(drv, lOPT_DDRAW_STRUCT, (intptr_t*)mddraw);
			mddraw->drv = drv;
			l_memset(mddraw->buffer, drv->dd->clr, mddraw->bufferLength);

			char name[lMaxDriverNameLength+8];
			l_strncpy(name, DDRAW_WINDOW_TITLE, lMaxDriverNameLength);
			l_strncpy(name+l_strlen(name), drv->dd->name, sizeof(name)-l_strlen(name));

			if (directDraw_create(mddraw, name, drv->dd->width, drv->dd->height)){
				PostMessage(mddraw->wnd, WM_DD_PAINT, mddraw->bufferLength, (LPARAM)mddraw->buffer);
				return 1;
			}
			l_free(mddraw->buffer);
		}
		l_free(mddraw);
	}
	return 0;
}

static int directDraw_Clear (TDRIVER *drv)
{
	if (!drv){
		return 0;
	}else{
		if (drv->dd->status == LDRV_READY){
			TMYLCDDDRAW *mddraw = DrvToDDraw(drv);
			if (mddraw){
				if (mddraw->status){
					l_memset(mddraw->buffer, drv->dd->clr, mddraw->bufferLength);
					//directDraw_update(mddraw, mddraw->buffer, mddraw->bufferLength);
					SendMessage(mddraw->wnd, WM_DD_PAINT, mddraw->bufferLength, (LPARAM)mddraw->buffer);
				}
				return 1;
			}
		}
	}
	return 0;
}

static int directDraw_Refresh (TDRIVER *drv, TFRAME *frm)
{
	if (!frm||!drv)
		return 0;
	if (drv->dd->status != LDRV_READY)
		return 0;

	TMYLCDDDRAW *mddraw = DrvToDDraw(drv);
	if (mddraw){
		if (mddraw->status){
			if (frm->bpp == LFRM_BPP_32 || frm->bpp == LFRM_BPP_32A){
				//directDraw_update(mddraw, frm->pixels, frm->frameSize);
				PostMessage(mddraw->wnd, WM_DD_PAINT, frm->frameSize, (LPARAM)frm->pixels);
				return 1;
			}else{
				pConverterFn converter = getConverter(frm->bpp, LFRM_BPP_32);
				if (converter){
					converter(frm, mddraw->buffer);
					//directDraw_update(mddraw, mddraw->buffer, mddraw->bufferLength);
					PostMessage(mddraw->wnd, WM_DD_PAINT, mddraw->bufferLength, (LPARAM)mddraw->buffer);
					return 1;
				}
			}
		}
	}
	return 0;
}

static int directDraw_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	return directDraw_Refresh(drv, frm);
}


#else

int initDDrawDisplay(void *rd){return 1;}
void closeDDrawDisplay(void *rd){return;}

#endif
