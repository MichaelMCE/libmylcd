
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

#if (__BUILD_OPENGL__)

#include "../memory.h"
#include "../utils.h"
#include "../misc.h"
#include "../pixel.h"
#include "../device.h"
#include "../lstring.h"
#include "../convert.h"
#include "gldisplay.h"
#include "lgl.h"


int createGLWindow (TDRIVER *drv);
static int gl_OpenDisplay (TDRIVER *drv);
static int gl_CloseDisplay (TDRIVER *drv);
static int gl_Clear (TDRIVER *drv);
int gl_Refresh (TDRIVER *drv, TFRAME *frm);
int gl_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
static int gl_getOption (TDRIVER *drv, int option, intptr_t *value);
static int gl_setOption (TDRIVER *drv, int option, intptr_t *value);


int initGLDisplay (TREGISTEREDDRIVERS *rd)
{
	TDISPLAYDRIVER dd;
	
	l_strcpy(dd.comment, "GL virtual display");
	dd.open = gl_OpenDisplay;
	dd.close = gl_CloseDisplay;
	dd.clear = gl_Clear;
	dd.refresh = gl_Refresh;
	dd.refreshArea = gl_RefreshArea;
	dd.setOption = gl_setOption;
	dd.getOption = gl_getOption;
	dd.status = LDRV_CLOSED;
	dd.optTotal = 1;
	
	l_strcpy(dd.name, "OpenGL");
	int dnumber = registerDisplayDriver(rd, &dd);
	setDefaultDisplayOption(rd, dnumber, lOPT_GL_STRUCT, 0);
	return (dnumber > 0);
}

void closeGLDisplay (TREGISTEREDDRIVERS *rd)
{
	return;
}
 
static TGLWIN *DrvToGL (TDRIVER *drv)
{
	TGLWIN *glwin = NULL;
	drv->dd->getOption(drv, lOPT_GL_STRUCT, (void*)&glwin);
	return glwin;
}

static int gl_getOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (drv && value){
		intptr_t *opt = drv->dd->opt;
		*value = opt[option];
		return 1;
	}else{
		return 0;
	}
}

static int gl_setOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (!drv) return 0;
	intptr_t *opt = drv->dd->opt;
	
	if (option == lOPT_GL_STRUCT){
		opt[lOPT_GL_STRUCT] = (intptr_t)value;
		return 1;
	}else{
		return 0;
	}
}

static int gl_CloseDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status != LDRV_CLOSED){
			drv->dd->status = LDRV_CLOSED;
			gl_close(DrvToGL(drv));
			return 1;
		}
	}
	return 0;
}

static int gl_OpenDisplay (TDRIVER *drv)
{
	if (!drv)
		return 0;

	if (drv->dd->status != LDRV_CLOSED)
		return 0;

	if (createGLWindow(drv)){
		drv->dd->status = LDRV_READY;
		return 1;
	}else{
		drv->dd->status = LDRV_CLOSED;
		return 0;
	}
}
 
int createGLWindow (TDRIVER *drv)
{
	char name[lMaxDriverNameLength+8];
	l_strncpy(name, GL_WINDOW_TITLE, lMaxDriverNameLength);
	l_strncpy(name+l_strlen(name), drv->dd->name, sizeof(name)-l_strlen(name));
	TGLWIN *glwin = gl_open(name, drv->dd->width, drv->dd->height);
	if (glwin != NULL){
		drv->dd->setOption(drv, lOPT_GL_STRUCT, (intptr_t*)glwin);
		drv->dd->clear(drv);
		gl_update(glwin, LFRM_BPP_32, glwin->buffer);
	}
	return (glwin != NULL);
}

static int gl_Clear (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status == LDRV_READY){
			TGLWIN *glwin = DrvToGL(drv);
			if (glwin){
				l_memset(glwin->buffer, drv->dd->clr, drv->dd->width * drv->dd->height * 4);
				gl_update(glwin, LFRM_BPP_32, glwin->buffer);
				return 1;
			}
		}
	}
	return 0;
}

int gl_Refresh (TDRIVER *drv, TFRAME *frm)
{
	if (!frm || !drv)
		return 0;
	if (drv->dd->status != LDRV_READY)
		return 0;

	TGLWIN *glwin = DrvToGL(drv);
	if (glwin){
		if (frm->bpp == LFRM_BPP_32 || frm->bpp == LFRM_BPP_32A /*|| frm->bpp == LFRM_BPP_24*/){
			gl_update(glwin, frm->bpp, frm->pixels);
			return 1;
		}else{
			pConverterFn converter = getConverter(frm->bpp, LFRM_BPP_32);
			if (converter){
				converter(frm, (void*)glwin->buffer);
				gl_update(glwin, LFRM_BPP_32, glwin->buffer);
				return 1;
			}
		}
	}	
	return 0;
}

int gl_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	return gl_Refresh(drv, frm);
}

#else

int initGLDisplay(void *rd){return 1;}
void closeGLDisplay(void *rd){return;}

#endif

