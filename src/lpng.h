
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


#ifndef _LPNG_H_
#define _LPNG_H_


#define PNG_READ_TRESHOLD	127
#define PNG_WRITE_LO		0
#define PNG_WRITE_HI		255



int loadPng (TFRAME *frame, const int flags, const wchar_t *filename, int ox, int oy);
int savePng (TFRAME *frame, const wchar_t *filename, int width, int height, int flags);
int pngGetMetrics (const wchar_t *filename, int *width, int *height, int *bpp);

#endif


