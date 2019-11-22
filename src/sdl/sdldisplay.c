
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

#if (__BUILD_SDL__)

#include "../memory.h"
#include "../utils.h"
#include "../pixel.h"
#include "../device.h"
#include "../lstring.h"
#include "../convert.h"
#include "sdldisplay.h"
#include "lsdl.h"


int createSDLWindow (TDRIVER *drv);
static int sdl_OpenDisplay (TDRIVER *drv);
static int sdl_CloseDisplay (TDRIVER *drv);
static int sdl_Clear (TDRIVER *drv);
int sdl_Refresh (TDRIVER *drv, TFRAME *frm);
int sdl_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static int sdl_getOption (TDRIVER *drv, int option, intptr_t *value);
static int sdl_setOption (TDRIVER *drv, int option, intptr_t *value);


int initSDLDisplay (TREGISTEREDDRIVERS *rd)
{
	TDISPLAYDRIVER dd;
	
	l_strcpy(dd.comment, "SDL virtual display");
	dd.open = sdl_OpenDisplay;
	dd.close = sdl_CloseDisplay;
	dd.clear = sdl_Clear;
	dd.refresh = sdl_Refresh;
	dd.refreshArea = sdl_RefreshArea;
	dd.setOption = sdl_setOption;
	dd.getOption = sdl_getOption;
	dd.status = LDRV_CLOSED;
	dd.optTotal = 1;
	
	l_strcpy(dd.name, "SDL");
	int dnumber = registerDisplayDriver(rd, &dd);
	setDefaultDisplayOption(rd, dnumber, lOPT_SDL_STRUCT, 0);

	/*const int sdlwintotal = 6;
	int i;
	for (i = 2; i <= sdlwintotal; i++){
		sprintf(dd.name, "SDL:%i", i);
		dnumber = registerDisplayDriver(rd, &dd);
		setDefaultDisplayOption(rd, dnumber, lOPT_SDL_STRUCT, 0);	
	}*/
	return (dnumber > 0);
}

void closeSDLDisplay (TREGISTEREDDRIVERS *rd)
{
	return;
}

 
static TSDLWIN *DrvToSDL (TDRIVER *drv)
{
	TSDLWIN *sdlwin = NULL;
	drv->dd->getOption(drv, lOPT_SDL_STRUCT, (void*)&sdlwin);
	return sdlwin;
}

static int sdl_getOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (drv && value){
		intptr_t *opt = drv->dd->opt;
		*value = opt[option];
		return 1;
	}else{
		return 0;
	}
}

static int sdl_setOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (!drv) return 0;
	intptr_t *opt = drv->dd->opt;
	
	if (option == lOPT_SDL_STRUCT){
		opt[lOPT_SDL_STRUCT] = (intptr_t)value;
		return 1;
	}else{
		return 0;
	}
}

static int sdl_CloseDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status != LDRV_CLOSED){
			drv->dd->status = LDRV_CLOSED;
			sdl_close(DrvToSDL(drv));
			return 1;
		}
	}
	return 0;
}

static int sdl_OpenDisplay (TDRIVER *drv)
{
	if (!drv)
		return 0;

	if (drv->dd->status != LDRV_CLOSED)
		return 0;

	if (createSDLWindow(drv)){
		drv->dd->status = LDRV_READY;
		return 1;
	}else{
		drv->dd->status = LDRV_CLOSED;
		return 0;
	}
}
 
int createSDLWindow (TDRIVER *drv)
{
	char name[lMaxDriverNameLength+8];
	l_strncpy(name, SDL_WINDOW_TITLE, lMaxDriverNameLength);
	l_strncpy(name+l_strlen(name), drv->dd->name, sizeof(name)-l_strlen(name));
	TSDLWIN *sdlwin = sdl_open(name, drv->dd->width, drv->dd->height);
	if (sdlwin != NULL){
		drv->dd->setOption(drv, lOPT_SDL_STRUCT, (intptr_t*)sdlwin);
		drv->dd->clear(drv);
		sdl_update(sdlwin, sdlwin->buffer);
	}
	return (sdlwin != NULL);
}

static int sdl_Clear (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status == LDRV_READY){
			TSDLWIN *sdlwin = DrvToSDL(drv);
			if (sdlwin){
				l_memset(sdlwin->buffer, drv->dd->clr, drv->dd->width * drv->dd->height * 4);
				sdl_update(sdlwin, sdlwin->buffer);
				return 1;
			}
		}
	}
	return 0;
}

int sdl_Refresh (TDRIVER *drv, TFRAME *frm)
{
	if (!frm)
		return 0;
	if (drv->dd->status != LDRV_READY)
		return 0;

	TSDLWIN *sdlwin = DrvToSDL(drv);
	if (sdlwin){
		if (frm->bpp == LFRM_BPP_32 || frm->bpp == LFRM_BPP_32A){
			sdl_update(sdlwin, frm->pixels);
			return 1;
		}else{
			pConverterFn converter = getConverter(frm->bpp, LFRM_BPP_32);
			if (converter){
				converter(frm, sdlwin->buffer);
				sdl_update(sdlwin, sdlwin->buffer);
				return 1;
			}
		}
	}	
	return 0;
}

int sdl_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	return sdl_Refresh(drv, frm);
}

#else

int initSDLDisplay(void *rd){return 1;}
void closeSDLDisplay(void *rd){return;}

#endif

