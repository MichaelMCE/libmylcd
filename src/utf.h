
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


#ifndef _UTF_H_
#define _UTF_H_



int JISX0213ToUTF32 (const ubyte *buffer, UTF32 *ch);
int ISO2022JPToUTF32 (THWD *hw, const ubyte *buffer, UTF32 *ch);
int EUCJPToUTF32 (const ubyte *buffer, UTF32 *ch);

int ISO2022KRToUTF32 (THWD *hw, const ubyte *buffer, UTF32 *wc);
int EUCKRToUTF32 (THWD *hw, const ubyte *buffer, UTF32 *ch);

int EUCCNToUTF32 (THWD *hw, const ubyte *buffer, UTF32 *ch);
int EUCTWToUTF32 (THWD *hw, const ubyte *buffer, UTF32 *ch);
int HZToUTF32 (const ubyte *buffer, UTF32 *ch);
int GB18030ToUTF32 (THWD *hw, const ubyte *buffer, UTF32 *wc);
int BIG5ToUTF32 (THWD *hw, const ubyte *buffer, UTF32 *ch);
int GBKToUTF32 (THWD *hw, const ubyte *buffer, UTF32 *wc);

int TIS620ToUTF32 (const ubyte *buffer, UTF32 *wc);

int UTF16ToUTF32 (const ubyte *buffer, UTF32 *wc);
int UTF16BEToUTF32 (const ubyte *buffer, UTF32 *ch);
int UTF16LEToUTF32 (const ubyte *buffer, UTF32 *ch);
int UTF8ToUTF32 (const UTF8 *buffer, UTF32 *wc);



int eucjp2sjis (const ubyte *in, UTF32 *out);
int JaToSJIS (const ubyte *in, ubyte *out);

#endif


