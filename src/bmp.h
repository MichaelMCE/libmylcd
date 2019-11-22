
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


#ifndef _BMP_H_
#define _BMP_H_


#define DIBTRESHOLD		(50)
#define BMPHEADERSIZE	(54)

typedef struct {
	short        fil;
	uint8_t		 bm[2];				// 2
	int			 fsize;				// 6
	int          temp;				// 10
	int			 dataOffset;		// 14
	int			 HInfo;				// 18
	int			 Width;				// 22
	int			 Height;			// 26
	short	 	 Planes;
	short		 bpp;
	unsigned int compression;        /* Compression type          */
	unsigned int imagesize;          /* Image size in bytes       */
	int			 xresolution;
	int			 yresolution;     	 /* Pixels per meter          */
	unsigned int ncolours;           /* Number of colours in palette       */
	unsigned int importantcolours;   /* Important colours         */
}__attribute__ ((packed))TBMP;


int loadBmp (TFRAME *frm, const int flags, const wchar_t *filename, int ox, int oy);
int saveBmp (TFRAME *frm, const wchar_t *filename, int width, int height);

int bmpGetMetrics (const wchar_t *filename, int *width, int *height, int *bpp);

#endif



