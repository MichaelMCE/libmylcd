
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


#ifndef _BDF_H_
#define _BDF_H_



#define BDFLINELEN 255
#define CHODELTA 2048

int initBdf (THWD *hw);
void closeBdf (THWD *hw);


int BDFOpen (TWFONT *font);
int BDFClose (TWFONT *font);
int BDFLoadGlyphs (unsigned int *glist, int total, TWFONT *font);
int BDFReadFile (TWFONT *Font, const wchar_t *path);
int BDFReadData (THWD *hw, TWFONT *Font, ubyte **lines);


#endif

