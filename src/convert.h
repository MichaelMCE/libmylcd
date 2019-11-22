
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




#ifndef _CONVERT_H_
#define _CONVERT_H_


#define PIXSET    ((140<<16)|(140<<8)|140)
#define PIXCLEAR  ((240<<16)|(240<<8)|240)


int convertFrameArea (TFRAME *src, TFRAME *des, int dx, int dy, int x1, int y1, int x2, int y2);
int convertFrame (TFRAME *src, TFRAME *des);


unsigned int frame1To1BPP (TFRAME *frame, void *des);
unsigned int frame1To8BPP (TFRAME *frame, void *des, const ubyte clrLow, const ubyte clrHigh);
unsigned int frame1ToRGB555 (TFRAME *frame, void *des, const int clrLow, const int clrHigh);
unsigned int frame1ToRGB565 (TFRAME *frame, void *des, const int clrLow, const int clrHigh);
unsigned int frame1ToRGB444 (TFRAME *frame, void *des, const int clrLow, const int clrHigh);
unsigned int frame1ToRGB888 (TFRAME *frame, void *des, const int clrLow, const int clrHigh);
unsigned int frame1ToARGB8888 (TFRAME *frame, void *des, const int clrLow, const int clrHigh);

/*
void frame1ToBuffer32 (TFRAME *frm, unsigned int *buffer);
void frame8ToBuffer32 (TFRAME *frm, unsigned int *buffer);
void frame12ToBuffer32 (TFRAME *frm, unsigned int *buffer);
void frame15ToBuffer32 (TFRAME *frm, unsigned int *buffer);
void frame16ToBuffer32 (TFRAME *frm, unsigned int *buffer);
void frame24ToBuffer32 (TFRAME *frm, unsigned int *buffer);
void frame32ToBuffer32 (TFRAME *frm, unsigned int *buffer);
*/
void ABGRToARGB (unsigned int *buffer, size_t totalPixels);
pConverterFn getConverter(const int src_bpp, const int des_bpp);
unsigned int frame1BPPToRGB (TFRAME *frame, void *des, int des_bpp, const int clrLow, const int clrHigh);


void buffer32ToFrame32 (unsigned int * restrict buffer, const int width, const int height, const int pitch, TFRAME *frm);


#endif

