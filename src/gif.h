
// libmylcd - http://mylcd.sourceforge.net/
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


#ifndef _GIF_H_
#define _GIF_H_



int gifGetSize (const wchar_t *filename, int *width, int *height, int *bpp);
void *gifGetImageBuffer (const wchar_t *filename, int *width, int *height);

int loadGif (TFRAME *frame, const int flags, const wchar_t *filename, int ox, int oy);
int saveGif (TFRAME *frame, const wchar_t *filename, int w, int h, const int flags);


#endif


