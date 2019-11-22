
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


#ifndef _FONTS_H_
#define _FONTS_H_



#define WFONT_ACTIVE			0x0001
#define WFONT_BUILT				0x0002
#define WFONT_BDF_HEADER		0x0004	// bdf header. loaded flag
#define WFONT_DEFAULTCHARBUILT	0x0008
#define WFONT_CHRPOSITIONSREAD	0x0010


void registerInternalFonts (THWD *hw, const wchar_t *fontPath);

wchar_t *getFontPath (THWD *hw);


TFONT *fontIDToFontA (THWD *hw, int id);
TWFONT *fontIDToFontW (THWD *hw, int id);

TWFONT *buildBDFFont (TWFONT *font);
TWFONT *buildBDFGlyphs (THWD *hw, const char *str, int fontid);
TWFONT *buildBDFGlyph (THWD *hw, UTF32 ch, int fontid);

int cacheCharactersAll (THWD *hw, int fontid);
int cacheCharacters (THWD *hw, const char *str, int fontid);
int cacheCharacterList (THWD *hw, UTF32 *glist, int gtotal, int fontid);
int cacheCharacterRange (THWD *hw, UTF32 min, UTF32 max, int fontid);
int stripCharacterList (THWD *hw, UTF32 *glist, int gtotal, TWFONT *font);

int getFontCharacterSpacing (THWD *hw, int fontid);
int setFontCharacterSpacing (THWD *hw, int fontid, int pixels);
int getFontLineSpacing (THWD *hw, int fontid);
int setFontLineSpacing (THWD *hw, int fontid, int pixels);
int setFontZeroWidthSpacing (THWD *hw, int fontid, int pixels);

TWCHAR * getGlyph (THWD *hw, const char *c, UTF32 ch, int fontid);
int getCachedGlyphs (THWD *hw, UTF32 *glist, int gtotal, int fontid);
int cacheGlyphs (UTF32 *glist, int gtotal, TWFONT *font);
int mergeFontW (THWD *hw, int fontid1, int fontid2);

void *fontIDToFont (THWD *hw, int fontid);
void deleteFonts (THWD *hw);
int flushFont (THWD *hw, int fontid);

int registerFontW (THWD *hw, const wchar_t *filename, TWFONT *f);
int registerFontA (THWD *hw, const wchar_t *filename, TFONT *f);

int unregisterFontW (THWD *hw, int fontid);
int unregisterFontA (THWD *hw, int fontid);

TENUMFONT *enumFontsBeginW (THWD *hw);
TENUMFONT *enumFontsBeginA (THWD *hw);

int enumFontsNextW (TENUMFONT *enf);
int enumFontsNextA (TENUMFONT *enf);

void enumFontsDeleteW (TENUMFONT *enf);
void enumFontsDeleteA (TENUMFONT *enf);

#endif


