
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


#ifndef _TGA_H_
#define _TGA_H_


#define TGATRESHOLD		(unsigned char)(150)

typedef struct {
    ubyte			identsize;          // size of ID field that follows 18 byte header (0 usually)
    ubyte			colourmaptype;      // type of colour map 0=none, 1=has palette
    ubyte			imagetype;          // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed
    unsigned short	colourmapstart;     // first colour map entry in palette
    unsigned short	colourmaplength;    // number of colours in palette
    ubyte			colourmapbits;      // number of bits per palette entry 15,16,24,32
    unsigned short	xstart;             // image x origin
    unsigned short	ystart;             // image y origin
    unsigned short 	width;              // image width in pixels
    unsigned short	height;			    // image height in pixels
    ubyte			bpp;				// image bits per pixel 8,16,24,32
    ubyte			descriptor;         // image descriptor bits (vh flip bits)
}TTGA;		// pixel data follows header

int loadTga (TFRAME *frm, const int flags, const wchar_t *filename, int ox, int oy);
int saveTga (TFRAME *frm, const wchar_t *filename, int width, int height);


int tgaGetMetrics (const wchar_t *filename, int *width, int *height, int *bpp);



#endif

