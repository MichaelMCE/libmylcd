
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


#include "mylcd.h"


#if (__BUILD_JPG_SUPPORT__)

#include "memory.h"
#include "convert.h"
#include "frame.h"
#include "jpg/jpg.h"
#include "ljpg.h"
#include "image.h"


int saveJpg (TFRAME *frame, const wchar_t *filename, int width, int height, int quality)
{
#if (__BUILD_JPG_WRITE_SUPPORT__)
	return write_JPEG_file(frame, filename, width, height, quality);
#else
	return 0;
#endif
}

int loadJpg (TFRAME *frame, const int flags, const wchar_t *filename, int widthMax, int heightMax)
{
#if (__BUILD_JPG_READ_SUPPORT__)
	return read_JPEG_file(frame, filename, flags&LOAD_RESIZE, flags&LOAD_SIZE_RESTRICT, widthMax, heightMax);
#else
	return 0;
#endif
}

#else

int loadJpg (TFRAME *frame, const int flags, const wchar_t *filename, int widthMax, int heightMax){return 0;}
int saveJpg (TFRAME *frame, const wchar_t *filename, int width, int height, int quality){return 0;}

#endif

