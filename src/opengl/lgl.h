
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


#ifndef _LGL_H_
#define _LGL_H_

typedef struct{
	int width;
	int height;
	void *buffer;
	unsigned int *out;
}TGLWIN;

TGLWIN *gl_open (const char *title, int width, int height);
int gl_update (TGLWIN *glwin, const int bpp, void *buffer);
void gl_close (TGLWIN *glwin);


#endif

