
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
//
//	You should have received a copy of the GNU Library General Public
//	License along with this library; if not, write to the Free
//	Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "mylcd.h"

#if (__BUILD_PRINT_SUPPORT__)

#include <string.h>

#include "memory.h"
#include "utils.h"
#include "frame.h"
#include "fonts.h"
#include "lstring.h"
#include "bdf.h"
#include "print.h"
#include "draw.h"
#include "chardecode.h"
#include "textbdf.h"
#include "textbitmap.h"
#include "misc.h"
#include "api.h"


static inline int fontType (int fontid);
static inline int getACharMetrics (THWD *hw, const char *c, int fontid, int *w, int *h);
static inline int getATextMetrics (THWD *hw, const char *string, TLPRINTR *rect, int flags, int fontid, int *w, int *h);
static inline int getAFontMetrics (THWD *hw, int fontid, int *w, int *h, int *a, int *d, int *tchars);

static inline int getWCharMetrics (THWD *hw, const char *c, int fontid, int *w, int *h);
static inline int getWTextMetrics (THWD *hw, const char *string, TLPRINTR *rect, int flags, int fontid, int *w, int *h);
static inline int getWFontMetrics (THWD *hw, int fontid, int *w, int *h, int *a, int *d, int *tchars);

static inline int setupPrintRect (TFRAME *frm, TLPRINTR *rect, int flags);
static inline char *getFormattedVaListString (THWD *hw, const char *string, va_list *ap);



int getCharMetrics (THWD *hw, const char *c, int fontid, int *w, int *h)
{
	if (!fontType(fontid))
		return getACharMetrics(hw, c, fontid, w, h);
	else
		return getWCharMetrics(hw, c, fontid, w, h);
}


int getFontMetrics (THWD *hw, int fontid, int *w, int *h, int *a, int *d, int *tchars)
{
	if (!fontType(fontid))
		return getAFontMetrics(hw, fontid,w,h,a,d,tchars);
	else
		return getWFontMetrics(hw, fontid,w,h,a,d,tchars);
}

int getTextMetrics (THWD *hw, const char *txt, const int flags, int fontid, int *w, int *h)
{
	TLPRINTR rect = {0};
	rect.bx2 = rect.by2 = MAXFRAMESIZEW-1;

	if (!fontType(fontid))
		return getATextMetrics(hw, txt, &rect, flags, fontid, w, h);
	else
		return getWTextMetrics(hw, txt, &rect, flags, fontid, w, h);
}

int _print (TFRAME *frm, const char *buffer, int x, int y, int font, int style)
{
	TLPRINTR rect = {0};
	rect.bx1 = rect.sx = rect.ex = x;
	rect.by1 = rect.sy = rect.ey = y;
	rect.bx2 = frm->width-1;
	rect.by2 = frm->height-1;
	return _printEx(frm, &rect, font, PF_CLIPDRAW|PF_IGNOREFORMATTING, style, buffer, NULL);
}

int _printEx (TFRAME *frm, TLPRINTR *rect, int font, int flags, int style, const char *string, va_list *ap)
{
	if (!setupPrintRect(frm, rect, flags))
		return 0;

	int ret = 0;
	char *buffer = NULL;
	void *fbuffer = NULL;
		
	if (flags&PF_IGNOREFORMATTING)
		buffer = (char*)string;
	else
		buffer = fbuffer = getFormattedVaListString(frm->hw, string, ap);

	if (flags&PF_CLIPWRAP){
		if (!fontType(font))
			ret = textBitmapRenderWrap(frm, frm->hw, buffer, rect, font, flags|PF_CLIPTEXTH, style);
		else
			ret = textGlueRender(frm, frm->hw, buffer, rect, font, flags|PF_CLIPTEXTH, style);
	}else{
 		rect->ex = rect->sx = MAX(rect->sx,0);
		rect->ey = rect->sy = MAX(rect->sy,0);
		if (!fontType(font)){
			ret = textBitmapRender(frm, frm->hw, buffer, rect, font, flags, style);
			if (flags&PF_INVERTTEXTRECT)
				invertArea(frm, rect->sx, rect->sy, rect->ex-1, rect->ey-1);
		}else{
			ret = textGlueRender(frm, frm->hw, buffer, rect, font, flags, style);
			if (flags&PF_INVERTTEXTRECT)
				invertArea(frm, rect->sx, rect->sy, rect->ex-1, rect->ey-1);
		}

	/*if (flags&PF_INVERTTEXTRECT){
		invertArea(frm, rect->sx, rect->sy, rect->ex-rect->sx, rect->ey-rect->sy);
	}else*/
		if (flags&PF_TEXTBOUNDINGBOX)
			drawRectangleDotted(frm, rect->sx, rect->sy, rect->ex, rect->ey, getForegroundColour(frm->hw));
	}
	if (fbuffer)
		l_free(fbuffer);
	return ret;
}

int _printf (TFRAME *frm, int x, int y, int font, int style, const char *string, va_list *ap)
{
	int ret = 0;
	char *buffer = getFormattedVaListString(frm->hw, string, ap);
	if (buffer){
		ret = _print(frm, buffer, x, y, font, style);
		l_free(buffer);
	}
	return ret;
}

TFRAME * newStringEx (THWD *hw, const TMETRICS *metrics, const int lbpp, const int flags, const int font, const char *string, va_list *ap)
{
	char *buffer, *fbuffer = NULL;
	if (flags&PF_IGNOREFORMATTING)
		buffer = (char*)string;
	else
		buffer = fbuffer = getFormattedVaListString(hw, string, ap);

	int w = 0, h = 0;
	if (flags&PF_CLIPWRAP || flags&PF_WORDWRAP)
		w = abs(metrics->width);
	
	getTextMetrics(hw, buffer, flags, font, &w, &h);
	//printf("newStringEx 1: %i %i\n", w, h);
	//h += 50;
	
	w += 1;
	if (metrics->x > 0) w += (metrics->x*2);
	if (metrics->width < 0) w += abs(metrics->width);
	else if (metrics->width > 0 && w > metrics->width) w = metrics->width;		// set max width
	if (w < MINFRAMESIZEW) w = MINFRAMESIZEW;
			
	
	h += 1;
	if (metrics->y > 0) h += (metrics->y*2);
	if (metrics->height < 0) h += abs(metrics->height);
	else if (metrics->height > 0 &&  h > metrics->height) h = metrics->height;	// set max height
	if (h < MINFRAMESIZEH) h = MINFRAMESIZEH;

// testing..
#if 1
	int sy = 0;
	int y = metrics->y;
	if (y < 0){
		//h -= abs(y);
		sy = y;
		y = 0;
	}
#endif

	TLPRINTR rect = {metrics->x, y, w-1, h-1, 0, sy, 0, sy};
	TFRAME *frame = _newFrame(hw, w, h, 1, lbpp);

	if (frame){
		//printf("newStringEx 2: %i %i\n", w, h);
		
		int ret = _printEx(frame, &rect, font, flags|PF_IGNOREFORMATTING/*|PF_CLIPWRAP*/, LPRT_CPY, buffer, NULL);
		if (fbuffer) l_free(fbuffer);
		//printf("newStringEx 3: %i\n", ret);
		
		if (ret)
	 		return frame;
		else
			deleteFrame(frame);

	}else if (fbuffer){
		l_free(fbuffer);
	}
	return NULL;
}

TFRAME * newString (THWD *hw, const int lbpp, const int flags, const int font, const char *string, va_list *ap)
{
	char *buffer, *fbuffer = NULL;
	if (flags&PF_IGNOREFORMATTING)
		buffer = (char*)string;
	else
		buffer = fbuffer = getFormattedVaListString(hw, string, ap);

	int w = 0, h = 0;
	getTextMetrics(hw, buffer, flags, font, &w, &h);
	w += 1;
	if (w < MINFRAMESIZEW) w = MINFRAMESIZEW;
		
	h += 1;
	if (h < MINFRAMESIZEH) h = MINFRAMESIZEH;
	
	TLPRINTR rect = {0, 0, w-1, h-1, 0, 0, 0, 0};
	TFRAME *frame = _newFrame(hw, w, h, 1, lbpp);

	if (frame){
		//clearFrameClr(frame, getBackgroundColour(hw));
		
		int ret = _printEx(frame, &rect, font, flags|PF_IGNOREFORMATTING|PF_CLIPWRAP, LPRT_CPY, buffer, NULL);
		if (fbuffer) l_free(fbuffer);
		
		if (ret)
	 		return frame;
		else
			deleteFrame(frame);

	}else if (fbuffer){
		l_free(fbuffer);
	}
	return NULL;
}


TFRAME * newStringList (THWD *hw, const int lbpp, const int flags, const int font, const UTF32 *glist, const int gtotal)
{

	if (!glist || !gtotal) return NULL;

	TLPRINTR mrect = {0,0,0,0,0,0,0,0};
	getTextMetricsList(hw, glist, 0, gtotal-1, flags, font, &mrect);

	int w = abs(mrect.bx2 - mrect.bx1)+1;
	if (w < MINFRAMESIZEW) w = MINFRAMESIZEW;
	int h = abs(mrect.by2 - mrect.by1)+1;
	if (h < MINFRAMESIZEH) h = MINFRAMESIZEH;


	TLPRINTR rect = {0,0,w-1,h-1,0,0,0,0};
	TFRAME *frame = _newFrame(hw, w, h, 1, lbpp);

	if (frame){
		if (_printList(frame, glist, 0, gtotal, &rect, font, flags|PF_CLIPWRAP, LPRT_CPY)){
	 		return frame;
		}else{
			deleteFrame(frame);
		}
}
	return NULL;
}

TFRAME * newStringListEx (THWD *hw, const TMETRICS *metrics, const int lbpp, const int flags, const int font, const UTF32 *glist, const int gtotal)
{
	if (!glist || !gtotal) return NULL;


	int w = 0, h = 0;

	TLPRINTR mrect = {0,0,0,0,0,0,0,0};
	getTextMetricsList(hw, glist, 0, gtotal-1, flags, font, &mrect);
	
	if (flags&PF_CLIPWRAP || flags&PF_WORDWRAP)
		w = abs(metrics->width);
	else
		w = abs(mrect.bx2 - mrect.bx1)+1;
	if (w < MINFRAMESIZEW) w = MINFRAMESIZEW;
	h = abs(mrect.by2 - mrect.by1)+1;
	if (h < MINFRAMESIZEH) h = MINFRAMESIZEH;



	/*
	w += 1;
	if (metrics->x > 0) w += (metrics->x*2);
	if (metrics->width < 0) w += abs(metrics->width);
	else if (metrics->width > 0 && w > metrics->width) w = metrics->width;		// set max width
	if (w < MINFRAMESIZEW) w = MINFRAMESIZEW;
			
	
	h += 1;
	if (metrics->y > 0) h += (metrics->y*2);
	if (metrics->height < 0) h += abs(metrics->height);
	else if (metrics->height > 0 &&  h > metrics->height) h = metrics->height;	// set max height
	if (h < MINFRAMESIZEH) h = MINFRAMESIZEH;
	*/

// testing..
#if 1
	int sy = 0;
	int y = metrics->y;
	if (y < 0){
		//h -= abs(y);
		sy = y;
		y = 0;
	}
#endif

	TLPRINTR rect = {metrics->x, y, w-1, h-1, 0, sy, 0, sy};
	TFRAME *frame = _newFrame(hw, w, h, 1, lbpp);

	if (frame){
		if (_printList(frame, glist, 0, gtotal, &rect, font, flags|PF_CLIPWRAP, LPRT_CPY))
	 		return frame;
		else
			deleteFrame(frame);
	}
	return NULL;
}

int _printList (TFRAME *frm, const UTF32 *glist, int first, int total, TLPRINTR *rect, int fontid, int flags, int style)
{
	int ret = 0;
	TWFONT *font;

	if (!setupPrintRect(frm, rect, flags))
		return 0;


	if (!(font=fontIDToFontW(frm->hw, fontid)))
		return 0;

	if ((flags&PF_CLIPWRAP) && (font=fontIDToFontW(frm->hw, fontid))){
		ret = textWrapRender(frm, frm->hw, glist, first, first+total-1, rect, font, flags|PF_CLIPTEXTH, style);
	}else{
   		rect->ex = rect->sx = MAX(rect->sx,0);
		rect->ey = rect->sy = MAX(rect->sy,0);
		ret = textRender(frm, frm->hw, glist, first, first+total-1, rect, font, flags|PF_CLIPTEXTH, style);

		if (flags&PF_INVERTTEXTRECT){
			invertArea(frm, rect->sx, rect->sy, rect->ex-1, rect->ey-1);
		}else if (flags&PF_TEXTBOUNDINGBOX){
			//int tmp = frm->style;
			//frm->style = LSP_XOR;
			drawRectangleDotted(frm, rect->sx, rect->sy, rect->ex, rect->ey, getForegroundColour(frm->hw));
			//frm->style = tmp;
		}
	}

	return ret;
}

static inline int getWCharMetrics (THWD *hw, const char *c, int fontid, int *w, int *h)
{
	TWFONT *font = buildBDFGlyphs(hw, c, fontid);
	if (!font) return -1;

	UTF32 ch;
	int chars = decodeCharacterCode(hw, (ubyte*)c, &ch);

	if (chars){
		if ((ch>0) && (ch<font->maxCharIdx)){
			if (font->chr){
				if (font->chr[ch]){
					if (w) *w = font->chr[ch]->w;
					if (h) *h = font->chr[ch]->h;
					return chars;
				}
			}
		}
	}
	if (w) *w = 0;
	if (h) *h = 0;
	return -2;
}

static inline int getWFontMetrics (THWD *hw, int fontid, int *w, int *h, int *a, int *d, int *tchars)
{
	TWFONT *font = fontIDToFontW(hw, fontid);
	if (!font) return 0;
	
	if (!(font->flags&WFONT_BDF_HEADER)){
		BDFOpen(font);	// force font preprocessing
	 	BDFClose(font);
	}
	if (w) *w = font->QuadWidth;
	if (h) *h = font->QuadHeight;
	if (a) *a = font->fontAscent;
	if (d) *d = font->fontDescent;
	if (tchars) *tchars = font->GlyphsInFont;
	
	return font->CharsBuilt;
}

static inline int getWTextMetrics (THWD *hw, const char *str, TLPRINTR *rect, const int flags, int fontid, int *ww, int *hh)
{
	if (!str || !fontid || !rect) return 0;

	if ((flags&PF_CLIPWRAP || flags&PF_WORDWRAP) && ww)
		rect->bx2 = *ww;

	int pos = textGlueRender(NULL, hw, str, rect, fontid, flags|PF_CLIPWRAP|PF_GETTEXTBOUNDS|PF_DONTRENDER, 0);
	if (ww) *ww = (rect->bx2 - rect->bx1)+1;
	if (hh) *hh = (rect->by2 - rect->by1)+1;
	return pos;
}

static inline int getATextMetrics (THWD *hw, const char *str, TLPRINTR *rect, int flags, int fontid, int *ww, int *hh)
{
	if (!str||!fontid||!rect) return 0;

	int c = textBitmapRender(NULL, hw, str, rect, fontid, flags|PF_DONTRENDER, 0);
	if (c){
		if (ww) *ww = (rect->ex - rect->sx)+1;
		if (hh) *hh = (rect->ey - rect->sy)+1;
	}
	return c;
}

static inline int getACharMetrics (THWD *hw, const char *c, int id, int *w, int *h)
{
	if (!buildBitmapFont(hw, id)) return 0;
	TFONT *font = fontIDToFontA(hw, id);
	if (!font) return 0;

	if (c){
		if (w) *w = (font->c[(ubyte)*c].right-font->c[(ubyte)*c].left)+1;
		if (h) *h = (font->c[(ubyte)*c].btm-font->c[(ubyte)*c].top)+1;
	}
	return 1;
}

static inline int getAFontMetrics (THWD *hw, int id, int *w, int *h, int *a, int *d, int *tchars)
{
	if (w) *w = 0;
	if (h) *h = 0;
	if (tchars) *tchars = 0;

	if (!buildBitmapFont(hw, id)) return 0;
	TFONT *font = fontIDToFontA(hw, id);
	if (!font) return 0;

	if (w) *w = font->charW;
	if (h) *h = font->charH;
	if (a) *a = font->charH;
	if (d) *d = 0;
	if (tchars) *tchars = font->charsPerRow;
	return 1;
}

static inline char *getFormattedVaListString (THWD *hw, const char *string, va_list *ap)
{
	char *buffer = NULL;
	const int enc = getCharEncoding(hw);
	if (enc == CMT_UTF16 || enc == CMT_UTF16BE || enc == CMT_UTF16LE)
		l_vaswprintf((wchar_t**)&buffer, (wchar_t*)string, ap);
	else
		l_vasprintf(&buffer, string, ap);
	return buffer;
}

static inline int setupPrintRect (TFRAME *frm, TLPRINTR *rect, int flags)
{
    if (!rect->bx2) rect->bx2 = frm->width-1;
    if (!rect->by2) rect->by2 = frm->height-1;
    if (rect->bx1 > rect->bx2) rect->bx1 = 0;
    if (rect->by1 > rect->by2) rect->by1 = 0;
    if (flags&PF_RESETX) rect->sx = rect->bx1;
    if (flags&PF_RESETY) rect->sy = rect->by1;
    if (rect->sx < rect->bx1) rect->sx = rect->bx1;
    if (rect->sy < rect->by1) rect->sy = rect->by1;
	if ((rect->sx > rect->bx2) || (rect->sy > rect->by2)) return 0;
    if (rect->ex > rect->bx2) rect->ex = rect->sx;
    if (rect->ey > rect->by2) rect->ey = rect->sy;
    if (flags&PF_USELASTX) rect->sx = rect->ex;
    if (flags&PF_USELASTY) rect->sy = rect->ey;
    return 1;
}


// return 0 if TGA/image bitmap, 1 if BDF
static inline int fontType (int fontid)
{
	return (fontid<lBaseFontW) ? 0: 1;
}

#endif

