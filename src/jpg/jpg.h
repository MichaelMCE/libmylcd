
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



#ifndef _JPG_H_
#define _JPG_H_

int read_JPEG_file (TFRAME *frame, const wchar_t *filename, const int resize, const int sizeRestrict, const int widthMax, const int heightMax);
int write_JPEG_file (TFRAME *frame, const wchar_t * filename, const int width, const int height, const int quality);
int read_JPEG_buffer (TFRAME *frame, uint8_t *inbuffer, const uint64_t insize, const int resize, const int sizeRestrict, const int widthMax, const int heightMax);
int read_JPEG_metrics (FILE *hFile, int *width, int *height);



#endif


