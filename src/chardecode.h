
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


#ifndef _CHARDECODE_H_
#define _CHARDECODE_H_


int setCharacterEncodingTable (THWD *hw, int TableID);

UTF32 remapChar (THWD *hw, const UTF32 enc);
int isCharRef (THWD *hw, const char *str, UTF32 *ch);
int decodeMultiChar (THWD *hw, const ubyte *str, UTF32 *ch, const int cmap);
int getCharEncoding (THWD *hw);
void resetCharDecodeFlags (THWD *hw);

int countCharacters (THWD *hw, const char *buffer);

int decodeCharacterCode (THWD *hw, const ubyte *buffer, UTF32 *ch);
int decodeCharacterBuffer (THWD *hw, const char *buffer, UTF32 *glist, int total);

int createCharacterList (THWD *hw, const char *str, UTF32 *glist, int total);

void htmlCharRefEnable (THWD *hw);
void htmlCharRefDisable (THWD *hw);
void combinedCharEnable (THWD *hw);
void combinedCharDisable (THWD *hw);

#endif

