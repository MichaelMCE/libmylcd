
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


#ifndef _COPY_H_
#define _COPY_H_


#include "utils.h"


int copyArea (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2);
int copyAreaA (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2, float alpha);
int copyAreaEx (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2, int scaleh, int scalev, int style);
int copyAreaScaled (TFRAME *from, TFRAME *to, const int src_x, const int src_y, const int src_width, const int src_height, const int dest_x, const int dest_y, const int dest_width, const int dest_height, const int mode);

int flipFrame (TFRAME *src, TFRAME *des, int flag);
int copyFrame (TFRAME *from, TFRAME *to);
int moveArea (TFRAME *frm, int x1, int y1, int x2, int y2, int pixels, const int mode, const int dir);


#if 1

#include <string.h>	

static INLINE void copyArea_fast8 (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2)
{
	if ((dx>to->width-1) || (dy>to->height-1))
		return; 

	y2 = MIN(y2, from->height-1);
	x2 = MIN(x2, from->width-1);
	y1 = MIN(y1, from->height-1);
	x1 = MIN(x1, from->width-1);

	if (!dx && !dy){
		if (from->width == to->width && from->height == to->height && x2+1 == from->width && y2+1 == from->height){
			if (from->bpp == to->bpp && from->frameSize == to->frameSize){
				void *top = to->pixels;
				void *fromp = from->pixels;
				size_t framesize = from->frameSize;
				l_memcpy(top, fromp, framesize);
				return;
			}
		}
	}

	int y;
	if (((x1&7) || (1+(x2-x1))&7)){
		int x,xx,row;
		x2++; y2++;
		for (y=y1; y<y2; y++,dy++){
			xx = dx;
			row = y*from->pitch;
			for (x=x1; x<x2; x++, xx++)
				setPixel_NB(to,xx,dy, getPixel_NBr(from,x,row));
		}
	}else{
		ubyte * restrict s,* restrict d;
		int tbytes;
		const int n = (1+(x2-x1))>>3;
		y2++;
		for (y=y1; y<y2; y++,dy++){
			s = l_getPixelAddress(from, x1, y);
			d = l_getPixelAddress(to, dx, dy);
			tbytes = n;
			while (tbytes--)
				*d++ = *s++;
		}
	}
	return;
}
#endif


#endif

