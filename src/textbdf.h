
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


#ifndef _TEXTBDF_H_
#define _TEXTBDF_H_



int textRender (TFRAME *frm, THWD *hw, const UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, int style);
int textGlueRender (TFRAME *frm, THWD *hw, const char *str, TLPRINTR *rect, int fontid, int flags, int style);
int textWrapRender (TFRAME *frm, THWD *hw, const UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, int style);

int cacheCharacterBuffer (THWD *hw, const UTF32 *glist, int total, int fontid);
int getTextMetricsList (THWD *hw, const UTF32 *glist, int first, int last, int flags, int fontid, TLPRINTR *rect);
int setRenderEffect (THWD *hw, const int mode);
void renderEffectReset (THWD *hw, const int fontid, const int mode);

#endif

