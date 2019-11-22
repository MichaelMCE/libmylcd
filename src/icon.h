
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


#ifndef _ICON_H_
#define _ICON_H_




int loadIco (TFRAME *frame, const int flags, const wchar_t *filename, const int ox, const int height);
//int saveIco (TFRAME *frame, const wchar_t* filename, int width, int height, int flags);


int icoGetMetrics (const wchar_t *filename, int *width, int *height, int *bpp);

#endif


