
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


#ifndef _IMAGE_H_
#define _IMAGE_H_



TFRAME *_newImage (THWD *hw, const wchar_t *filename, const int type, const int frameBPP);

int saveImage (TFRAME *frame, const wchar_t *filename, const int img_type, int w, int h);
TFRAME *newImage (THWD *hw, const wchar_t *filename, const int frameBPP);
int loadImage (TFRAME *frame, const wchar_t *filename);
int loadImageEx (TFRAME *frame, const wchar_t *filename, const int flags, const int ox, const int oy);


int imageGetMetrics (const wchar_t *filename, int type, int *_width, int *_height, int *bpp);


#endif


