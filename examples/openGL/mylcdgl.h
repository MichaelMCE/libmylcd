
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
//  GNU LIBRARY GENERAL PUBLIC LICENSE for details.

#ifndef _MYLCDGL_H_
#define _MYLCDGL_H_

#include <gl/gl.h>
#include <GL/glu.h>
#include <GL/glfw.h>
#include <GL/openglut.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <conio.h>
#include <windows.h>
#include <fcntl.h>
#include <mylcd.h>

#define DRAW_TO_BITMAP	0
#define SHOW_WINDOW		0


#define MYGL_CREATECONTEXT	0x01
#define MYGL___				0x02
#define MYGL_MESSAGEQUEUE	0x04
#define MYGL_SHOWWINDOW		0x08
#define MYGL_DRAWCURSOR		0x10
#define MYGL_DRAWCROSSHAIR	0x20

extern int DWIDTH;
extern int DHEIGHT;
extern THWD *hw;
extern TFRAME *frame;



int openglInit (int flags, int width, int height);
void EnableOpenGL (HDC hdc, HDC * hDC, HGLRC * hRC);
void DisableOpenGL (HWND hWnd, HDC hDC, HGLRC hRC);

void mylcdgl_SetWindowTitle (const char *title);
void mylcdgl_SetSize (int width, int height);
int mylcdgl_setoption (int option, intptr_t *value);
void mylcdgl_shutdown ();
void mylcdgl_sleep (int ms);
int mylcdgl_update (int flags, int x, int y);

void getGLFrame (unsigned char *data, TFRAME *frame, int glwidth, int glheight);

int mylcdgl_draw (TFRAME *frame, int flags, int x, int y);

#endif

