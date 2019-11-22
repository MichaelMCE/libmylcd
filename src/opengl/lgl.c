
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

#include <GL/glfw.h>
#include "../memory.h"
#include "../utils.h"
#include "../misc.h"
#include "lgl.h"


TGLWIN *gl_open (const char *title, int width, int height)
{
	
	TGLWIN *glwin = l_calloc(1, sizeof(TGLWIN));
	if (!glwin){
		mylog("gl_open(): no memory\n");
		return NULL;
	}

    if (!glfwInit()){
        mylog("gl_open(): failed to initialize GLFW\n");
		l_free(glwin);
		return NULL;
    }

    // Open a window and create its OpenGL context
    if (!glfwOpenWindow(width, height, 8, 8, 8, 0, 8, 0, GLFW_WINDOW)){
        mylog("gl_open(): failed to open GLFW window\n");
		l_free(glwin);
		return NULL;
    }

	glfwGetWindowSize(&glwin->width, &glwin->height);
	if (width < glwin->width) width = glwin->width;
	if (height < glwin->height) height = glwin->height;
	
	glwin->out = l_calloc(4, width * height);
	glwin->buffer = l_calloc(4, width * height);
	if (!glwin->buffer){
		l_free(glwin);
		mylog("gl_open(): no memory for buffer\n");
		return NULL;
	}

    glfwSetWindowTitle(title);
    glfwSwapInterval(0);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	return glwin;
}

static void ABGRToARGBFlip (const unsigned int *restrict buffer, const int width, const int height, unsigned int *restrict out)
{
	int r, g, b;
	int i = 0;
	int xd = 0;
	int y_width;
	
	for (int y = height-1; y >= 0; y--){
		xd = 0;
		y_width = y*width;
		
		for (int x = width-1; x >=0; x--){
			r = (buffer[i]&0x0000FF)<<16;
			g = (buffer[i]&0x00FF00);
			b = (buffer[i]&0xFF0000)>>16;
			i++;
		
			out[y_width+xd] = r|g|b;
			xd++;
		}
	}
}

int gl_update (TGLWIN *glwin, const int bpp, void *buffer)
{
	//if (bpp == LFRM_BPP_24)
	//	glDrawPixels(glwin->width, glwin->height, GL_RGB, GL_UNSIGNED_BYTE, buffer);
	//else if (bpp == LFRM_BPP_32 || bpp == LFRM_BPP_32A)
	
	ABGRToARGBFlip(buffer, glwin->width, glwin->height, glwin->out);
	glDrawPixels(glwin->width, glwin->height, GL_RGBA, GL_UNSIGNED_BYTE, glwin->out);
				
	glfwSwapBuffers();
	return 1;
}

void gl_close (TGLWIN *glwin)
{
	if (glwin){
		if (glwin->buffer)
			l_free(glwin->buffer);
		if (glwin->out)
			l_free(glwin->out);
						
		l_free(glwin);
	}
	glfwCloseWindow();
}


#endif
