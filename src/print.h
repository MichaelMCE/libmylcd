
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


#ifndef _PRINT_H_
#define _PRINT_H_

int getCharMetrics (THWD *hw, const char *c, int fontid, int *w, int *h);
int getFontMetrics (THWD *hw, int fontid, int *w, int *h, int *a, int *d, int *tchars);
int getTextMetrics (THWD *hw, const char *txt, int flags, int fontid, int *w, int *h);

int _print (TFRAME *frm, const char *buffer, int x, int y, int font, int style);
int _printf (TFRAME *frm, int x, int y, int font, int style, const char *string, va_list *ap);
int _printEx (TFRAME *frm, TLPRINTR *rect, int font, int flags, int style, const char *string, va_list *ap);
int _printList (TFRAME *frm, const UTF32 *glist, int first, int total, TLPRINTR *rect, int fontid, int flags, int style);


TFRAME *newString (THWD *hw, const int lbpp, const int flags, const int font, const char *string, va_list *ap);
TFRAME * newStringEx (THWD *hw, const TMETRICS *metrics, const int lbpp, const int flags, const int font, const char *string, va_list *ap);
TFRAME * newStringList (THWD *hw, const int lbpp, const int flags, const int font, const UTF32 *glist, const int gtotal);
TFRAME * newStringListEx (THWD *hw, const TMETRICS *metrics, const int lbpp, const int flags, const int font, const UTF32 *glist, const int gtotal);

#endif

