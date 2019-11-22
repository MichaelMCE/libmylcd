
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

#if (__BUILD_BDF_FONT_SUPPORT__ && __BUILD_PRINT_SUPPORT__)

#include <string.h>
#include <assert.h>

#include "memory.h"
#include "apilock.h"
#include "pixel.h"
#include "copy.h"
#include "fonts.h"
#include "textbdf.h"
#include "draw.h"
#include "chardecode.h"
#include "combmarks.h"
#include "lmath.h"
#include "api.h"

#include "sync.h"
#include "misc.h"



static inline int _cacheCharacterBuffer (const UTF32 *clist, int ctotal, TWFONT *font)
{
	int total = 0, cached = 0;

	UTF32 *glist = (UTF32*)l_malloc(sizeof(UTF32)*ctotal);
	if (glist){
		l_memcpy(glist,clist,sizeof(UTF32)*ctotal);
		if ((total=stripCharacterList(font->hw, glist, ctotal, font)))
			cached = cacheGlyphs(glist, total, font);
		l_free(glist);
	}
	return cached;
}

int cacheCharacterBuffer (THWD *hw, const UTF32 *glist, int total, int fontid)
{
	TWFONT *font = fontIDToFontW(hw, fontid);
	return _cacheCharacterBuffer(glist, total, font);
}

// return dimensions of multiline text if rendered
static inline int textMetricsRect (THWD *hw, const UTF32 *glist, int first, int last, int flags, TWFONT *font, TLPRINTR *rect)
{
	rect->bx1 = rect->sx = rect->ex = 0;
	rect->by1 = rect->sy = rect->ey = 0;
	if (!rect->bx2)
		rect->bx2 = MAXFRAMESIZEW-1;
	if (!rect->by2)
		rect->by2 = MAXFRAMESIZEH-1;

	if (flags&PF_CLIPWRAP){
		return textWrapRender(NULL, hw, glist, first, last, rect, font, flags|PF_GETTEXTBOUNDS|PF_DONTRENDER, 0);
	}else{
		int pos = textRender(NULL, hw, glist, first, last, rect, font, flags|PF_DONTRENDER, 0);
		rect->bx2 = rect->ex;
		rect->by2 = rect->ey;
		return pos;
	}
}

int getTextMetricsList (THWD *hw, const UTF32 *glist, int first, int last, int flags, int fontid, TLPRINTR *rect)
{
	TWFONT *font = fontIDToFontW(hw, fontid);
	return textMetricsRect(hw, glist, first, last, flags|PF_CLIPTEXTH, font, rect);
}

int textGlueRender (TFRAME *frm, THWD *hw, const char *str, TLPRINTR *rect, int fontid, int flags, int style)
{
	int gindex = 0;

	int total = countCharacters(hw, str);
	if (total){
		UTF32 *glist = (UTF32*)l_malloc(sizeof(UTF32)*total);
		if (glist){
			TWFONT *font;
			decodeCharacterBuffer(hw, str, glist, total);
			if ((font=fontIDToFontW(hw, fontid))){
				_cacheCharacterBuffer(glist, total--, font);
				if (flags&PF_CLIPWRAP)
					gindex = textWrapRender(frm, hw, glist, 0, total, rect, font, flags, style);
				else
					gindex = textRender(frm, hw, glist, 0, total, rect, font, flags, style);
			}
			l_free(glist);
		}
	}
	return gindex;
}

static inline int textWrapRenderLJ (TFRAME *frame, THWD *hw, const UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, TLPOINTEX *tbound, int style)
{
	//printf("textWrapRenderLJ %p %i %i\n", frame, first, last);
	const int textboundflag = (flags&PF_GETTEXTBOUNDS) | (flags&PF_TEXTBOUNDINGBOX) | (flags&PF_INVERTTEXTRECT);
	int passcount = 1;
	int next = first;
			
	rect->ey = rect->sy;

	if (!frame){
		tbound->x1 = 0;//MAXFRAMESIZEW-1;
		tbound->y1 = MAXFRAMESIZEH-1;
	}else{
		tbound->x1 = frame->width-1;
		tbound->y1 = frame->height-1;
	}

	do{
		if (!passcount){
			rect->sy = rect->ey + font->LineSpace;	
		}else{
			passcount = 0;
			rect->sy = rect->ey;
		}

    	rect->ex = rect->sx = MAX(rect->sx, 0);
		//rect->ey = rect->sy = MAX(rect->sy, 0);
		first = next;
		next = textRender(frame, hw, glist, first, last, rect, font, flags, style);
		//printf("textRender %i %i %i, %i %i\n", next, first, last, rect->by2, rect->bx2);
		rect->sx = MIN(rect->sx, rect->bx1);

		if (&glist[first] == glist){
			if (glist[first] == '\n'){
				rect->ey += 2;
				rect->sy += 2;
			}
		}
		if (textboundflag){
			tbound->x1 = MIN(tbound->x1, rect->sx);
			tbound->y1 = MIN(tbound->y1, rect->sy);
			tbound->x2 = MAX(tbound->x2, rect->ex);
			tbound->y2 = MAX(tbound->y2, rect->ey);
		}
	}while((next > first) && (next <= last) && next);

	if (glist[last] == '\n'){
		rect->ey += (font->PixelSize - font->fontDescent);
		rect->sy += (font->PixelSize - font->fontDescent);
	}

	if (textboundflag){
		tbound->x1 = MIN(tbound->x1, rect->sx);
		tbound->y1 = MIN(tbound->y1, rect->sy);
		tbound->x2 = MAX(tbound->x2, rect->ex);
		tbound->y2 = MAX(tbound->y2, rect->ey);
	}
	return next;
}


static inline int textWrapRenderRJ (TFRAME *frame, THWD *hw, const UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, TLPOINTEX *tbound, int style)
{
	int width = 0;
	int passcount = 1;
	int next = first;
	int textboundflag = (flags&PF_GETTEXTBOUNDS) | (flags&PF_TEXTBOUNDINGBOX) | (flags&PF_INVERTTEXTRECT);
	rect->ey = rect->sy;

	if (!frame){
		tbound->x1 = 0;//MAXFRAMESIZEW-1;
		tbound->y1 = MAXFRAMESIZEH-1;
	}else{
		tbound->x1 = frame->width-1;
		tbound->y1 = frame->height-1;
	}


	do{
		if (!passcount){
			rect->sy = rect->ey + font->LineSpace;
		}else{
			passcount = 0;
			rect->sy = rect->ey;
		}

		rect->ex = rect->sx = 0;
		first = next;
		textRender(NULL, hw, glist, first, last, rect, font, flags|PF_DONTRENDER, style);

		if (glist[first] == '\n')
			rect->ey -= (font->PixelSize - font->fontDescent) - font->LineSpace;

		if (textboundflag){
			tbound->x2 = MAX(tbound->x2, rect->ex);
			tbound->y2 = MAX(tbound->y2, rect->ey);
		}
		width = (rect->ex - rect->sx);
		rect->ex = rect->sx = MAX(rect->bx2 - width, rect->bx1);
		//rect->ex = rect->sx = rect->bx2 - width;

		if (textboundflag){
			tbound->x1 = MIN(tbound->x1, rect->sx);
			tbound->y1 = MIN(tbound->y1, rect->sy);
		}
		next = textRender(frame, hw, glist, first, last, rect, font, flags, style);
	}while((next>first) && (next<=last) && next);

	if (glist[last] == '\n'){
		rect->ey += (font->PixelSize - font->fontDescent);
		tbound->y2 = MAX(tbound->y2, rect->ey);
		rect->sy += (font->PixelSize - font->fontDescent);
		tbound->y1 = MIN(tbound->y1, rect->sy);
	}
	return next;	
}

static inline int textWrapRenderMJ (TFRAME *frame, THWD *hw, const UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, TLPOINTEX *tbound, int style)
{
	int width = 0;
	int passcount = 1;
	int next = first;
	int rectwidth = (rect->bx2 - rect->bx1) + 1;
	int textboundflag = (flags&PF_GETTEXTBOUNDS) | (flags&PF_TEXTBOUNDINGBOX) | (flags&PF_INVERTTEXTRECT);
	rect->ey = rect->sy;

	if (!frame){
		tbound->x1 = 0;//MAXFRAMESIZEW-1;
		tbound->y1 = MAXFRAMESIZEH-1;
	}else{
		tbound->x1 = frame->width-1;
		tbound->y1 = frame->height-1;
	}

	do{
		if (!passcount){
			rect->sy = rect->ey + font->LineSpace;
		}else{
			passcount = 0;
			rect->sy = rect->ey;
		}
		
		rect->ex = rect->sx = 0;
		first = next;
		textRender(NULL, hw, glist, first, last, rect, font, flags|PF_DONTRENDER, style);

		if (glist[first] == '\n')
			rect->ey -= (font->PixelSize - font->fontDescent)- font->LineSpace;

		if (textboundflag){
			tbound->x2 = MAX(tbound->x2, rect->ex);
			tbound->y2 = MAX(tbound->y2, rect->ey);
		}

		width = (rect->ex - rect->sx) + 1;
		if (width >= rectwidth){
			rect->ex = rect->sx = rect->bx1;
		}else{
			rect->ex = rect->sx = rect->bx1 + l_abs((rectwidth>>1)-((1+width)>>1)-2);  // was '((width)>>1)'
		}

		if (textboundflag){
			tbound->x1 = MIN(tbound->x1, rect->sx);
			tbound->y1 = MIN(tbound->y1, rect->sy);
		}

		next = textRender(frame, hw, glist, first, last, rect, font, flags, style);
		//next = ret;
	}while((next>first) && (next<=last) && next);

	if (glist[last] == '\n'){
		rect->ey += (font->PixelSize - font->fontDescent);
		tbound->y2 = MAX(tbound->y2, rect->ey);
		rect->sy += (font->PixelSize - font->fontDescent);
		tbound->y1 = MIN(tbound->y1, rect->sy);
	}
	return next;	
}

static inline int textWrapRenderHRTL (TFRAME *frame, THWD *hw, const UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, TLPOINTEX *tbound, int style)
{
	int width = 0;
	int next = first;
	int rectwidth = (rect->bx2 - rect->bx1) + 1;
	int textboundflag = (flags&PF_GETTEXTBOUNDS) | (flags&PF_TEXTBOUNDINGBOX) | (flags&PF_INVERTTEXTRECT);
	TLPRINTR metrect;
	TLPRINTR *mrect = &metrect;

	
	if (!frame){
		tbound->x1 = MAXFRAMESIZEW-1;
		tbound->y1 = MAXFRAMESIZEH-1;
	}else{
		tbound->x1 = frame->width-1;
		tbound->y1 = frame->height-1;
	}

	rect->ey = rect->sy;
	rect->sx = rect->bx2 - rect->sx;

	if (textboundflag){
		tbound->x2 = MAX(tbound->x2, rect->ex);
		tbound->y2 = MAX(tbound->y2, rect->ey);
		tbound->x1 = MIN(tbound->x1, rect->sx);
		tbound->y1 = MIN(tbound->y1, rect->sy);
	}

	do{
		l_memcpy(mrect, rect, sizeof(TLPRINTR));
		mrect->ex = mrect->sx = 0;
		first = next;
		textRender(NULL, hw, glist, first, first, mrect, font, flags|PF_DONTRENDER, style);

		if (glist[first] == '\n'){
			if (&glist[first] == glist)
				rect->ey = mrect->ey += font->LineSpace + 2;
			else
				rect->ey = mrect->ey -= (font->PixelSize - font->fontDescent) - font->LineSpace;
				
			rect->sy =  mrect->ey;
			rect->sx = rect->bx2;
		}else{
			width = mrect->ex - mrect->sx;
			if (width >= rectwidth)
				rect->ex = rect->sx = rect->bx2;
			else
				rect->ex = rect->sx -= width;
		}
		next = textRender(frame, hw, glist, first, first, rect, font, flags, style);

		if (textboundflag){
			tbound->x2 = MAX(tbound->x2, rect->ex);
			tbound->y2 = MAX(tbound->y2, rect->ey);
			tbound->x1 = MIN(tbound->x1, rect->sx);
			tbound->y1 = MIN(tbound->y1, rect->sy);
		}
	}while((next>first) && (next<=last) && next);

	if (glist[last] == '\n'){
		rect->ey -= font->LineSpace;
		tbound->y2 -= font->LineSpace;
	}

	if (textboundflag){
		tbound->x2 = MAX(tbound->x2, rect->ex);
		tbound->y2 = MAX(tbound->y2, rect->ey);
		tbound->x1 = MIN(tbound->x1, rect->sx);
		tbound->y1 = MIN(tbound->y1, rect->sy);
	}
	return next;	
}

static inline int textWrapRenderVLTR (TFRAME *frame, THWD *hw, const UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, TLPOINTEX *tbound, int style)
{

	int next = first;
	int textboundflag = (flags&PF_GETTEXTBOUNDS) | (flags&PF_TEXTBOUNDINGBOX) | (flags&PF_INVERTTEXTRECT);

	if (!frame){
		tbound->x1 = MAXFRAMESIZEW-1;
		tbound->y1 = MAXFRAMESIZEH-1;
	}else{
		tbound->x1 = frame->width-1;
		tbound->y1 = frame->height-1;
	}

	rect->ey = rect->sy;
	rect->ex = rect->sx = MAX(rect->bx1, rect->sx);

	if (textboundflag){
		tbound->x1 = MIN(tbound->x1, rect->sx);
		tbound->x2 = MAX(tbound->x2, rect->ex);
		tbound->y1 = MIN(tbound->y1, rect->sy);
		tbound->y2 = MAX(tbound->y2, rect->ey);
	}

	do{
		if (rect->sx >= rect->bx2)
			break;

		if (glist[next] == '\n'){
			if (rect->sx > rect->bx2)
				break;

			rect->sy = rect->ey = rect->by1;
			rect->ex = rect->sx += font->QuadWidth + font->LineSpace;
			first = next++;
		}else{
    		rect->ex = rect->sx = MIN(rect->sx, rect->bx2);
			rect->ey = rect->sy = MAX(rect->sy, 0);
			first = next;
			next = textRender(frame, hw, glist, first, first, rect, font, flags, style);
		}

		if (textboundflag){
			tbound->x1 = MIN(tbound->x1, rect->sx);
			tbound->x2 = MAX(tbound->x2, rect->ex);
			tbound->y1 = MIN(tbound->y1, rect->sy);
			tbound->y2 = MAX(tbound->y2, rect->ey);
		}
		rect->sy = rect->ey + font->CharSpace;
	}while((next>first) && (next<=last));

	if (glist[last] == '\n'){
		rect->ex += font->QuadWidth;
		rect->sx += font->QuadWidth;
	}

	if (textboundflag){
		tbound->x1 = MIN(tbound->x1, rect->sx);
		tbound->x2 = MAX(tbound->x2, rect->ex);
		tbound->y1 = MIN(tbound->y1, rect->sy);
		tbound->y2 = MAX(tbound->y2, rect->ey);
	}

	tbound->y2++;
	return next;	
}

static inline int textWrapRenderVRTL (TFRAME *frame, THWD *hw, const UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, TLPOINTEX *tbound, int style)
{
	int next = first;
	int textboundflag = (flags&PF_GETTEXTBOUNDS) | (flags&PF_TEXTBOUNDINGBOX) | (flags&PF_INVERTTEXTRECT);

	if (!frame){
		tbound->x1 = MAXFRAMESIZEW-1;
		tbound->y1 = MAXFRAMESIZEH-1;
	}else{
		tbound->x1 = frame->width-1;
		tbound->y1 = frame->height-1;
	}

	rect->ey = rect->sy;
	rect->sx = MIN(rect->bx2 - rect->sx - font->QuadWidth, rect->bx2);
//	if ((int)rect->sx < 0) rect->sx = 0;
//	if ((int)rect->sy < 0) rect->sy = 0;
	rect->ex = rect->sx + font->QuadWidth;

	if (textboundflag){
		tbound->x1 = MIN(tbound->x1, rect->sx);
		tbound->x2 = MAX(tbound->x2, rect->ex);
		tbound->y1 = MIN(tbound->y1, rect->sy);
		tbound->y2 = MAX(tbound->y2, rect->ey);
	}

	do{
		if (rect->sx < rect->bx1)
			break;

		if (glist[next] == '\n'){
			if (rect->sx < rect->bx1)
				break;

			rect->sy = rect->ey = rect->by1;
			rect->ex = rect->sx -= font->QuadWidth + font->LineSpace;
			first = next++;
		}else{

    		rect->ex = rect->sx = MAX(rect->sx, 0);
			rect->ey = rect->sy = MAX(rect->sy, 0);
			first = next;
			next = textRender(frame, hw, glist, first, first, rect, font, flags, style);
		}
	
		if (textboundflag){
			tbound->x1 = MIN(tbound->x1, rect->sx);
			tbound->x2 = MAX(tbound->x2, rect->ex);
			tbound->y1 = MIN(tbound->y1, rect->sy);
			tbound->y2 = MAX(tbound->y2, rect->ey);
		}
		rect->sy = rect->ey + font->CharSpace;
	}while((next>first) && (next<=last));

	if (textboundflag){
		tbound->x1 = MIN(tbound->x1, rect->sx);
		tbound->x2 = MAX(tbound->x2, rect->ex);
		tbound->y1 = MIN(tbound->y1, rect->sy);
		tbound->y2 = MAX(tbound->y2, rect->ey);
	}
	tbound->y2++;
	return next;	
}


int textWrapRender (TFRAME *frame, THWD *hw, const UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, int style)
{
	int next = first;
	TLPOINTEX tbound = {0, 0, 0, 0};
	flags |= PF_CLIPDRAW | PF_CLIPWRAP;

    if (flags&PF_MIDDLEJUSTIFY)
		next = textWrapRenderMJ(frame, hw, glist, first, last, rect, font, flags, &tbound, style);
	else if (flags&PF_RIGHTJUSTIFY)
		next = textWrapRenderRJ(frame, hw, glist, first, last, rect, font, flags, &tbound, style);
	else if (flags&PF_VERTICALRTL)
		next = textWrapRenderVRTL(frame, hw, glist, first, last, rect, font, flags, &tbound, style);
	else if (flags&PF_VERTICALLTR)
		next = textWrapRenderVLTR(frame, hw, glist, first, last, rect, font, flags, &tbound, style);
	else if (flags&PF_HORIZONTALRTL)
		next = textWrapRenderHRTL(frame, hw, glist, first, last, rect, font, flags, &tbound, style);
	else	// left justified by default
		next = textWrapRenderLJ(frame, hw, glist, first, last, rect, font, flags, &tbound, style);

	//printf("textWrapRender 1 %i %i\n", tbound.y2, rect->by2);
	if (flags&PF_GETTEXTBOUNDS){
		rect->bx1 = tbound.x1;
		rect->by1 = tbound.y1;
		rect->bx2 = MIN(tbound.x2, rect->bx2);
		rect->by2 = MIN(tbound.y2, rect->by2);
	}

	if (frame){
		if (flags&PF_INVERTTEXTRECT){
			invertArea(frame, tbound.x1, tbound.y1, tbound.x1+tbound.x2, tbound.y2);
		}else if (flags&PF_TEXTBOUNDINGBOX){
			if (frame->bpp == LFRM_BPP_1){
				drawRectangleDotted(frame, tbound.x1, tbound.y1, tbound.x1+tbound.x2, tbound.y2, LSP_XOR);
			}else{
				//int tmp = frame->style;
				//frame->style = LSP_XOR;
				//drawRectangleDotted(frame, tbound.x1, tbound.y1, tbound.x1+tbound.x2, tbound.y2, getRGBMask(frame, LMASK_WHITE));
				drawRectangleDotted(frame, tbound.x1, tbound.y1, tbound.x1+tbound.x2, tbound.y2, getForegroundColour(frame->hw));
				//frame->style = tmp;
			}
		}
	}

	tbound.x2 += tbound.x1;
	return next;
}

static inline int glyphGetTopPixel (TWCHAR *chr)
{
	if (chr->box.top != -1)
		return chr->box.top;
	
	if (chr->gp->pointsTotal){
		chr->box.top = chr->gp->points[0].y;
		return chr->box.top;
	}else{
		return -1;
	}
}

static inline int glyphGetBottomPixel (TWCHAR *chr)
{
	if (chr->box.btm != -1)
		return chr->box.btm;
	
	if (chr->gp->pointsTotal){
		chr->box.btm = chr->gp->points[chr->gp->pointsTotal-1].y;
		return chr->box.btm;
	}else{
		return -1;
	}
}

static inline int glyphGetLeftPixel (TWCHAR *chr)
{
	if (chr->box.left != -1)
		return chr->box.left;

	if (!chr->gp->pointsTotal){
		chr->box.left = -1;
		return chr->box.left;
	}
	
	chr->box.left = 999;
	for (int i = 0; i < chr->gp->pointsTotal; i++){
		if (chr->box.left > chr->gp->points[i].x)
			chr->box.left = chr->gp->points[i].x;
	}

	return chr->box.left;
}

static inline int glyphGetRightPixel (TWCHAR *chr)
{
	if (chr->box.right != -1)
		return chr->box.right;

	
	chr->box.right = -1;
	for (int i = 0; i < chr->gp->pointsTotal; i++){
		if (chr->box.right < chr->gp->points[i].x)
			chr->box.right = chr->gp->points[i].x;
	}

	return chr->box.right;
}

static inline int textRenderGetCharHeight (TWCHAR *chr)
{
	int bottom = glyphGetBottomPixel(chr);
	if (bottom == -1){
		return 7;
	}else{
	 	return (bottom - glyphGetTopPixel(chr)) + 3;
	}
}

static inline int textRenderGetCharWidth (TWCHAR *chr, int *left, int *right)
{
	*left = glyphGetLeftPixel(chr);
	if (*left < 0){
		*left = 0;
		*right = 2;
		
		chr->box.left = 0;
		chr->box.right = 2;
	}else{
		*right = glyphGetRightPixel(chr);
	}

	return MIN(chr->w, (*right-*left)+1);
}

static inline int isWordBreak (const UTF32 ch)
{
	if (/*ch == ' ' ||*/ ch == ',' || ch == '.' || ch == '-' || ch == '|' || ch == '\\' || ch == '\"' || ch == ':' || ch == '/' || ch == '+'/* || ch == 12288*/)
		return 1;
	else
		return 0;
}

static inline int textRenderCheckWordBounds (const UTF32 *glist, int flags, int first, int last, TLPRINTR *rect, TWFONT *font)
{
	TWCHAR *chr = NULL;
	int left=0, right=0;
	int w = 0;
	int status = 0;
	int fixedwflag = flags&PF_FIXEDWIDTH;
	int noautowidthflag = flags&PF_DISABLEAUTOWIDTH;
	int i = first;
	int quadWidth;
	
	if 	(font->CharSpace < 0)
		quadWidth = font->QuadWidth + font->CharSpace;
	else
		quadWidth = font->QuadWidth;

	do{
		if (i<last){
			if (!isWordBreak(glist[i]) && (glist[i] != ' ') && (glist[i] != '\n')){
				if (!fixedwflag){
					if (glist[i] < font->maxCharIdx){
						if ((chr=font->chr[glist[i]])){
							if (!noautowidthflag){
								w += textRenderGetCharWidth(chr, &left, &right) + font->CharSpace + chr->xoffset;
								//printf("2 encoding %X, %i %i %i\n", chr->encoding, w, left, right);
							}else{
								w += chr->w-1+font->CharSpace;
							}
						}
					}		
				}else{
					w += quadWidth;
				}
			}else{
				status = 0xFF;
			}
		}else{
			status = 0xFF;
		}
		i++;
	}while(!status);

	if (w && ((rect->ex+w+1) >= ((rect->bx2 - font->CharSpace)))){
		if (first == (i-1))
			return 0;
		else
			return w;
	}else{
		return 0;
	}
}

static inline int glyphGetTotalPoints (TGLYPHPOINTS *gp)
{
	
	if (gp && gp->pointsTotal)
		return gp->pointsTotal;
	else
		return 0;
}


// returns next char index to render
// returns 0 on error
static inline int textRenderList (TFRAME *to, THWD *hw, const UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, pGlyphRenderFn glyphRenderFunc)
{
	//TWCHAR *chr = NULL;
	TWCHAR *defaultchr = NULL;
	TLPRINTREGION *loc = NULL;
	UTF32 ch = 0;

	int w = 0,h = 0;
	int yoffset = 0;
	int left = 0, right = 0, top, btm;
	//int copymode = style;
	int quadWidth;
	int nodefaultcharflag = flags&PF_NODEFAULTCHAR;
	const int dontrenderflag = flags&PF_DONTRENDER;
	const int cliptexthflag = flags&PF_CLIPTEXTH;
	const int cliptextvflag = flags&PF_CLIPTEXTV;
	const int clipdrawflag = PF_CLIPDRAW-(flags&PF_CLIPDRAW);
	const int wordwrapflag = PF_WORDWRAP-(flags&PF_WORDWRAP);
	const int noescapeflag = flags&PF_NOESCAPE;
	const int invertglyph1 = PF_INVERTGLYPH1-(flags&PF_INVERTGLYPH1);
	const int invertglyph2 = PF_INVERTGLYPH2-(flags&PF_INVERTGLYPH2);
	const int fixedwflag = flags&PF_FIXEDWIDTH;
	const int boundingboxflag = (flags&PF_GLYPHBOUNDINGBOX && to);
	//const int fixedautospace = !fixedwflag && !(flags&PF_DISABLEAUTOWIDTH) && (font->spacing != 'P');
	const int fixedautospace = !(flags&PF_FORCEAUTOWIDTH) && (fixedwflag || flags&PF_DISABLEAUTOWIDTH || !(font->spacing != 'P'));
	
//	printf("fixedautospace %i, '%c' %i\n", fixedautospace, font->spacing, fixedwflag);

	if 	(font->CharSpace < 0)
		quadWidth = font->QuadWidth + font->CharSpace;
	else
		quadWidth = font->QuadWidth;
		
	if (!dontrenderflag){
		if (to->hw->render->copy == NULL){
			mylog("libmylcd: NULL print render function\n");
			return 0;
		}

		loc = to->hw->render->loc;
		loc->to = to;
		loc->flags = flags;
	}

	if (font->DefaultChar < font->maxCharIdx)
		defaultchr = font->chr[font->DefaultChar];
	else
		nodefaultcharflag = 0;

	if (glist[first] == 0x0A){
		rect->ey += (font->PixelSize - font->fontDescent);
		rect->ey = MIN(rect->ey, rect->by2);
	}

	last++;
	while (first < last){
		w = 0;
		h = 0;
		ch = glist[first];

		if (!noescapeflag)
			if (ch == 0x0A){
				return ++first;
			}

		if (!wordwrapflag){
			if (ch == ' ' /*|| ch == 12288*/){ // shouldn't just compute 32
				if ((w=textRenderCheckWordBounds(glist, flags, first+1, last, rect, font))){
					if (w < (rect->bx2 - rect->bx1)+1){
						return ++first;
					}
				}
			}
		}

		if (ch >= font->maxCharIdx){
			if (!nodefaultcharflag)
				ch = font->DefaultChar; //'?';
		}

		if (ch < font->maxCharIdx){
			TWCHAR *chr = font->chr[ch];
			if (chr == NULL && !nodefaultcharflag)
				chr = defaultchr;

			if (chr){
				if (chr->gp && chr->gp->pointsTotal){
					yoffset = MAX(font->PixelSize - chr->h - chr->yoffset-1, 0);
					
					if (fixedautospace){
						h = chr->h;
						w = chr->w-1;
						right = w;
						left = 0;
					}else{
						if (flags&PF_VERTICALRTL)
							h = textRenderGetCharHeight(chr);
						else
							h = chr->h;
						w = textRenderGetCharWidth(chr, &left, &right);
					}
					
					if (!clipdrawflag){
						if (rect->ex+w+chr->xoffset > rect->bx2){
							if (!cliptexthflag)
								w -= (rect->ex+chr->xoffset+w) - rect->bx2;
							else
								return first;
						}
						if (rect->sy+yoffset+h > rect->by2){
							if (!cliptextvflag)
								h -= (rect->sy+yoffset+h) - rect->by2;
							else
								return first;
						}
					}

					if (!dontrenderflag){	// render glyph to destination frame (to)
						loc->glyph = chr;
						loc->dx = rect->ex + chr->xoffset - left;
						loc->dy = rect->sy + yoffset;
						//loc->style = copymode;
						glyphRenderFunc(loc);

						if (!invertglyph1)
							invertArea(to, rect->ex+chr->xoffset, rect->sy+yoffset, rect->ex + chr->box.right, rect->sy + chr->box.btm);
						if (!invertglyph2)
							invertArea(to, rect->ex+chr->xoffset-1, rect->sy+yoffset-1, rect->ex + chr->box.right, rect->sy + chr->box.btm);
					}
					
					if (boundingboxflag){
						/*const int tmp = to->style;
						int c;
						
						if (to->bpp == LFRM_BPP_1){
							c = LSP_XOR;
						}else{
							to->style = LSP_XOR;
							c = getRGBMask(to, LMASK_WHITE);
						}*/

						top = glyphGetTopPixel(chr);
						if (top != -1)
							btm = glyphGetBottomPixel(chr);

						if (top == -1) // empty glyph; a space?
							drawRectangleDotted(to, rect->ex+chr->xoffset, rect->sy+yoffset, rect->ex+w-1, rect->sy+h-1, getForegroundColour(hw));
						else
							drawRectangleDotted(to, rect->ex+chr->xoffset, rect->sy+yoffset+top, rect->ex+chr->xoffset+w, rect->sy+yoffset+btm, getForegroundColour(hw));
						//to->style = tmp;
					}
				}
				
				if (!ch || !glyphGetTotalPoints(chr->gp)){
					if (chr->dwidth > 1)
						w = chr->dwidth-1 + font->spaceSpacing;
					else
						w = (quadWidth>>1) + font->spaceSpacing;
				}
				
				rect->ey = MAX(rect->ey, rect->sy+h+yoffset);

				if (!isCombinedMark(hw, ch)){
					if (fixedwflag)
						rect->ex += quadWidth;
					else
						rect->ex += w + font->CharSpace + chr->xoffset;
					//printf("%i: w %i %i %i\n", first+1, w, font->CharSpace,  chr->xoffset);
					//copymode = style;
				//}else{
					//copymode = LPRT_CPY;
				}
			}
			
			if (!clipdrawflag){
				if ((rect->ex > rect->bx2) || (rect->ey > rect->by2)){
					//if (fixedwflag)
						return ++first;
					//break;
				}
			}
			if (!wordwrapflag){
				if (isWordBreak(ch)){
					if ((w=textRenderCheckWordBounds(glist, flags, first+1, last, rect, font))){
						if (w < (rect->bx2 - rect->bx1)+1){
							rect->ex -= font->CharSpace - 1;	// could be a problem
							return ++first;
						}
					}
				}
			}
		}
		first++;
	}
	rect->ex -= font->CharSpace - 1; // could be a problem
	return first;
}

typedef struct {
	// from print func
	TLPRINTR rect;		// a copy of print functions clip rect
	int flags;

	// from filter
	int colour[2];		// 0 = back, 1 = front
	pGlyphRenderFn renderFunc;
	uint64_t udata;
	uint64_t attributes[8];	// filter settings
}renderfilter;



// returns a filterId
int textFilterAdd (THWD *hw, pGlyphRenderFn renderFunc, uint64_t udata, const int foreCol, const int backCol)
{
	return 1;
}

int textFilterSetAttribute (THWD *hw, const int filterId, const int attribute, const uint64_t value)
{
	return 1;
}

int textFilterGetAttribute (THWD *hw, const int filterId, const int attribute, uint64_t *value)
{
	return 1;
}

// returns a chainId
int textFilterBuildChain (THWD *hw, const int *filterId, const int total)
{
	return 1;
}

// activate and apply filter chain to this font
int textFilterAttach (THWD *hw, const int filterChainId, const int fontId)
{
	return 1;
}

// remove any filter chains from this font
int textFilterDettach (THWD *hw, const int fontId)
{
	return 1;
}

int textRender (TFRAME *to, THWD *hw, const UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, int style)
{
	int ret = textRenderList(to, hw, glist, first, last, rect, font, flags, hw->render->copy);
	return ret;
}

// return 0 if location is valid but not in list
static inline int glyphCheckDupPixel (TGLYPHPOINTS *gp, const int x, const int y)
{
	for (int i = 0; i < gp->pointsTotal; i++){
		if (x == gp->points[i].x && y == gp->points[i].y)
			return 1;
		//else if (x < 0 || y < 0)
		//	return -1;
	}
	return 0;
}

static inline void glyphAddPixel (TGLYPHPOINTS *gp, const int x, const int y, TGLYPHPOINTS *gp_nodup)
{
	if (gp->pointsTotal >= gp->pointsSpace){
		gp->pointsSpace += 128;
		gp->points = l_realloc(gp->points, gp->pointsSpace * sizeof(T2POINT));
	}

#if 0
	if (!glyphCheckDupPixel(gp, x, y)){
#else
	if (!glyphCheckDupPixel(gp, x, y) && !glyphCheckDupPixel(gp_nodup, x, y)){
#endif
		gp->points[gp->pointsTotal].x = x;
		gp->points[gp->pointsTotal++].y = y;
	}
}

static inline void glyphAddPixelData (TGLYPHPOINTS *gp, const int x, const int y, const int data, TGLYPHPOINTS *gp_nodup)
{
	if (gp->pointsTotal >= gp->pointsSpace){
		gp->pointsSpace += 128;
		gp->points = l_realloc(gp->points, gp->pointsSpace * sizeof(T2POINT));

		//printf("gp->points %i\n", gp->pointsSpace);
	}
	
#if 1
	if (!glyphCheckDupPixel(gp, x, y)){
#else
	if (!glyphCheckDupPixel(gp, x, y) && !glyphCheckDupPixel(gp_nodup, x, y)){
#endif
		gp->points[gp->pointsTotal].x = x;
		gp->points[gp->pointsTotal].y = y;
		gp->points[gp->pointsTotal++].data = data;
	}
}

static inline int copyGlyph_Blur1_calc (TWCHAR *glyph, const int radius)
{
	if (!glyph->ep[LTR_BLUR1]){
		glyph->ep[LTR_BLUR1] = (TGLYPHPOINTS*)l_malloc(sizeof(TGLYPHPOINTS));
		if (glyph->ep[LTR_BLUR1]){
			glyph->ep[LTR_BLUR1]->pointsTotal = 0;
			glyph->ep[LTR_BLUR1]->pointsSpace = 32;
			glyph->ep[LTR_BLUR1]->points = l_malloc(32 * sizeof(T2POINT));
		}else{
			return 0;
		}
	}
	
	TGLYPHPOINTS *ep = glyph->ep[LTR_BLUR1];
	TGLYPHPOINTS *gp = glyph->gp;
	
	const int w = glyph->w + (radius*2)+2;
	const int h = glyph->h + (radius*2)+2;
	char *buffer2 = l_calloc(w * h, sizeof(char));
	char *buffer1 = l_calloc(w * h, sizeof(char));
	if (!buffer1) return 0;
	
	const int offset = radius;
	for (int i = 0; i < gp->pointsTotal; i++){
		int x = gp->points[i].x + offset;
		int y = gp->points[i].y + offset;
		buffer1[y*w+x] = 0xFF;
	}
	
	//blurGrey(buffer1, buffer2, w, h, radius);
	//blurGrey(buffer2, buffer1, h, w, radius);
	Kernel *kernel = makeKernel(radius);
	if (kernel){
		convolveAndTranspose8(kernel, buffer1, buffer2, w, h, WRAP_EDGES);
		convolveAndTranspose8(kernel, buffer2, buffer1, h, w, WRAP_EDGES);
		deleteKernel(kernel);
	}
	
	for (int y = 0; y < h; y++){
		for (int x = 0; x < w; x++){
			int pix = buffer1[y*w+x]&0xFF;
			if (pix)
				glyphAddPixelData(ep, x-offset, y-offset, pix, gp);
		}
	}	
	
	l_free(buffer1);
	l_free(buffer2);
	return 1;
}

static inline void copyGlyph_addpixel (char *buffer, const int x, const int y, const int pitch)
{
	buffer[y*pitch+x] = 0xFF;
}

static inline int copyGlyph_Blur2_calc (TWCHAR *glyph, const int radius)
{
	if (!glyph->ep[LTR_BLUR2]){
		glyph->ep[LTR_BLUR2] = (TGLYPHPOINTS*)l_malloc(sizeof(TGLYPHPOINTS));
		if (glyph->ep[LTR_BLUR2]){
			glyph->ep[LTR_BLUR2]->pointsTotal = 0;
			glyph->ep[LTR_BLUR2]->pointsSpace = 16;
			glyph->ep[LTR_BLUR2]->points = l_malloc(16 * sizeof(T2POINT));
		}else{
			return 0;
		}
	}
	
	TGLYPHPOINTS *ep = glyph->ep[LTR_BLUR2];
	TGLYPHPOINTS *gp = glyph->gp;
	
	const int w = glyph->w + (radius*2)+2;
	const int h = glyph->h + (radius*2)+2;
	char *buffer2 = l_calloc(w * h, sizeof(char));
	char *buffer1 = l_calloc(w * h, sizeof(char));
	if (!buffer1) return 0;
	
	const int offset = radius;
	for (int i = 0; i < gp->pointsTotal; i++){
		int x = gp->points[i].x + offset;
		int y = gp->points[i].y + offset;

		copyGlyph_addpixel(buffer1, x-1, y-1, w);/* copyGlyph_addpixel(buffer1, x, y-1, w); copyGlyph_addpixel(buffer1, x+1, y-1, w);*/
		//copyGlyph_addpixel(buffer1, x-1, y,   w);                                         copyGlyph_addpixel(buffer1, x+1, y,   w);
		copyGlyph_addpixel(buffer1, x-1, y+1, w); /*copyGlyph_addpixel(buffer1, x, y+1, w); copyGlyph_addpixel(buffer1, x+1, y+1, w);*/
		
	}
	
	Kernel *kernel = makeKernel(radius);
	if (kernel){
		convolveAndTranspose8(kernel, buffer1, buffer2, w, h, WRAP_EDGES);
		convolveAndTranspose8(kernel, buffer2, buffer1, h, w, WRAP_EDGES);
		deleteKernel(kernel);
	}

	for (int y = 0; y < h; y++){
		for (int x = 0; x < w; x++){
			int pix = buffer1[y*w+x]&0xFF;
			if (pix)
				glyphAddPixelData(ep, x-offset, y-offset, pix, gp);
		}
	}	
	
	l_free(buffer1);
	l_free(buffer2);
	return 1;
}

static inline int copyGlyph_Blur3_calc (TWCHAR *glyph, const int radius)
{
	if (!glyph->ep[LTR_BLUR3]){
		glyph->ep[LTR_BLUR3] = (TGLYPHPOINTS*)l_malloc(sizeof(TGLYPHPOINTS));
		if (glyph->ep[LTR_BLUR3]){
			glyph->ep[LTR_BLUR3]->pointsTotal = 0;
			glyph->ep[LTR_BLUR3]->pointsSpace = 16;
			glyph->ep[LTR_BLUR3]->points = l_malloc(16 * sizeof(T2POINT));
		}else{
			return 0;
		}
	}
	
	TGLYPHPOINTS *ep = glyph->ep[LTR_BLUR3];
	TGLYPHPOINTS *gp = glyph->gp;
	
	const int w = glyph->w + (radius*2)+2;
	const int h = glyph->h + (radius*2)+2;
	char *buffer2 = l_calloc(w * h, sizeof(char));
	char *buffer1 = l_calloc(w * h, sizeof(char));
	if (!buffer1) return 0;
	
	const int offset = radius;
	for (int i = 0; i < gp->pointsTotal; i++){
		int x = gp->points[i].x + offset;
		int y = gp->points[i].y + offset;

		/*copyGlyph_addpixel(buffer1, x-1, y-1, w); copyGlyph_addpixel(buffer1, x, y-1, w);*/ copyGlyph_addpixel(buffer1, x+1, y-1, w);
		//copyGlyph_addpixel(buffer1, x-1, y,   w);                                         copyGlyph_addpixel(buffer1, x+1, y,   w);
		/*copyGlyph_addpixel(buffer1, x-1, y+1, w); copyGlyph_addpixel(buffer1, x, y+1, w);*/ copyGlyph_addpixel(buffer1, x+1, y+1, w);
	}
	
	Kernel *kernel = makeKernel(radius);
	if (kernel){
		convolveAndTranspose8(kernel, buffer1, buffer2, w, h, WRAP_EDGES);
		convolveAndTranspose8(kernel, buffer2, buffer1, h, w, WRAP_EDGES);
		deleteKernel(kernel);
	}

	for (int y = 0; y < h; y++){
		for (int x = 0; x < w; x++){
			int pix = buffer1[y*w+x]&0xFF;
			if (pix)
				glyphAddPixelData(ep, x-offset, y-offset, pix, gp);
		}
	}	
	
	l_free(buffer1);
	l_free(buffer2);
	return 1;
}

static inline int copyGlyph_Blur4_calc (TWCHAR *glyph, const int radius)
{
	if (!glyph->ep[LTR_BLUR4]){
		glyph->ep[LTR_BLUR4] = (TGLYPHPOINTS*)l_malloc(sizeof(TGLYPHPOINTS));
		if (glyph->ep[LTR_BLUR4]){
			glyph->ep[LTR_BLUR4]->pointsTotal = 0;
			glyph->ep[LTR_BLUR4]->pointsSpace = 16;
			glyph->ep[LTR_BLUR4]->points = l_malloc(16 * sizeof(T2POINT));
		}else{
			return 0;
		}
	}
	
	TGLYPHPOINTS *ep = glyph->ep[LTR_BLUR4];
	TGLYPHPOINTS *gp = glyph->gp;
	
	const int w = glyph->w + (radius*2)+2;
	const int h = glyph->h + (radius*2)+2;
	char *buffer2 = l_calloc(w * h, sizeof(char));
	char *buffer1 = l_calloc(w * h, sizeof(char));
	if (!buffer1) return 0;
	
	const int offset = radius;
	for (int i = 0; i < gp->pointsTotal; i++){
		int x = gp->points[i].x + offset;
		int y = gp->points[i].y + offset;

		/*copyGlyph_addpixel(buffer1, x-1, y-1, w);*/ copyGlyph_addpixel(buffer1, x, y-1, w);/* copyGlyph_addpixel(buffer1, x+1, y-1, w);*/
		copyGlyph_addpixel(buffer1, x-1, y,   w);                                         copyGlyph_addpixel(buffer1, x+1, y,   w);
		/*copyGlyph_addpixel(buffer1, x-1, y+1, w);*/ copyGlyph_addpixel(buffer1, x, y+1, w); /*copyGlyph_addpixel(buffer1, x+1, y+1, w);*/
	}
	
	Kernel *kernel = makeKernel(radius);
	if (kernel){
		convolveAndTranspose8(kernel, buffer1, buffer2, w, h, WRAP_EDGES);
		convolveAndTranspose8(kernel, buffer2, buffer1, h, w, WRAP_EDGES);
		deleteKernel(kernel);
	}

	for (int y = 0; y < h; y++){
		for (int x = 0; x < w; x++){
			int pix = buffer1[y*w+x]&0xFF;
			if (pix)
				glyphAddPixelData(ep, x-offset, y-offset, pix, gp);
		}
	}	
	
	l_free(buffer1);
	l_free(buffer2);
	return 1;
}

static inline int copyGlyph_Blur5_calc (TWCHAR *glyph, const int radius)
{
	if (!glyph->ep[LTR_BLUR5]){
		glyph->ep[LTR_BLUR5] = (TGLYPHPOINTS*)l_malloc(sizeof(TGLYPHPOINTS));
		if (glyph->ep[LTR_BLUR5]){
			glyph->ep[LTR_BLUR5]->pointsTotal = 0;
			glyph->ep[LTR_BLUR5]->pointsSpace = 16;
			glyph->ep[LTR_BLUR5]->points = l_malloc(16 * sizeof(T2POINT));
		}else{
			return 0;
		}
	}
	
	TGLYPHPOINTS *ep = glyph->ep[LTR_BLUR5];
	TGLYPHPOINTS *gp = glyph->gp;
	
	const int w = glyph->w + (radius*2)+2;
	const int h = glyph->h + (radius*2)+2;
	char *buffer2 = l_calloc(w * h, sizeof(char));
	char *buffer1 = l_calloc(w * h, sizeof(char));
	if (!buffer1) return 0;
	
	const int offset = radius;
	for (int i = 0; i < gp->pointsTotal; i++){
		int x = gp->points[i].x + offset;
		int y = gp->points[i].y + offset;

		copyGlyph_addpixel(buffer1, x-1, y-1, w);/* copyGlyph_addpixel(buffer1, x, y-1, w);*/ copyGlyph_addpixel(buffer1, x+1, y-1, w);
		//copyGlyph_addpixel(buffer1, x-1, y,   w);                                         copyGlyph_addpixel(buffer1, x+1, y,   w);
		copyGlyph_addpixel(buffer1, x-1, y+1, w); /*copyGlyph_addpixel(buffer1, x, y+1, w);*/ copyGlyph_addpixel(buffer1, x+1, y+1, w);
	}
	
	Kernel *kernel = makeKernel(radius);
	if (kernel){
		convolveAndTranspose8(kernel, buffer1, buffer2, w, h, WRAP_EDGES);
		convolveAndTranspose8(kernel, buffer2, buffer1, h, w, WRAP_EDGES);
		deleteKernel(kernel);
	}

	for (int y = 0; y < h; y++){
		for (int x = 0; x < w; x++){
			int pix = buffer1[y*w+x]&0xFF;
			if (pix)
				glyphAddPixelData(ep, x-offset, y-offset, pix, gp);
		}
	}	
	
	l_free(buffer1);
	l_free(buffer2);
	return 1;
}

static inline int copyGlyph_Blur6_calc (TWCHAR *glyph, const int radius)
{
	if (!glyph->ep[LTR_BLUR6]){
		glyph->ep[LTR_BLUR6] = (TGLYPHPOINTS*)l_malloc(sizeof(TGLYPHPOINTS));
		if (glyph->ep[LTR_BLUR6]){
			glyph->ep[LTR_BLUR6]->pointsTotal = 0;
			glyph->ep[LTR_BLUR6]->pointsSpace = 16;
			glyph->ep[LTR_BLUR6]->points = l_malloc(16 * sizeof(T2POINT));
		}else{
			return 0;
		}
	}
	
	TGLYPHPOINTS *ep = glyph->ep[LTR_BLUR6];
	TGLYPHPOINTS *gp = glyph->gp;
	
	const int w = glyph->w + (radius*2)+2;
	const int h = glyph->h + (radius*2)+2;
	char *buffer2 = l_calloc(w * h, sizeof(char));
	char *buffer1 = l_calloc(w * h, sizeof(char));
	if (!buffer1) return 0;
	
	const int offset = radius;
	for (int i = 0; i < gp->pointsTotal; i++){
		int x = gp->points[i].x + offset;
		int y = gp->points[i].y + offset;

		copyGlyph_addpixel(buffer1, x-1, y-1, w); /*copyGlyph_addpixel(buffer1, x, y-1, w);*/ copyGlyph_addpixel(buffer1, x+1, y-1, w);
		//copyGlyph_addpixel(buffer1, x-1, y,   w);                                         copyGlyph_addpixel(buffer1, x+1, y,   w);
		//copyGlyph_addpixel(buffer1, x-1, y+1, w); copyGlyph_addpixel(buffer1, x, y+1, w); copyGlyph_addpixel(buffer1, x+1, y+1, w);
	}
	
	Kernel *kernel = makeKernel(radius);
	if (kernel){
		convolveAndTranspose8(kernel, buffer1, buffer2, w, h, WRAP_EDGES);
		convolveAndTranspose8(kernel, buffer2, buffer1, h, w, WRAP_EDGES);
		deleteKernel(kernel);
	}

	for (int y = 0; y < h; y++){
		for (int x = 0; x < w; x++){
			int pix = buffer1[y*w+x]&0xFF;
			if (pix)
				glyphAddPixelData(ep, x-offset, y-offset, pix, gp);
		}
	}	
	
	l_free(buffer1);
	l_free(buffer2);
	return 1;
}

static inline int copyGlyph_Blur7_calc (TWCHAR *glyph, const int radius)
{
	if (!glyph->ep[LTR_BLUR7]){
		glyph->ep[LTR_BLUR7] = (TGLYPHPOINTS*)l_malloc(sizeof(TGLYPHPOINTS));
		if (glyph->ep[LTR_BLUR7]){
			glyph->ep[LTR_BLUR7]->pointsTotal = 0;
			glyph->ep[LTR_BLUR7]->pointsSpace = 16;
			glyph->ep[LTR_BLUR7]->points = l_malloc(16 * sizeof(T2POINT));
		}else{
			return 0;
		}
	}
	
	TGLYPHPOINTS *ep = glyph->ep[LTR_BLUR7];
	TGLYPHPOINTS *gp = glyph->gp;
	
	const int w = glyph->w + (radius*2)+2;
	const int h = glyph->h + (radius*2)+2;
	char *buffer2 = l_calloc(w * h, sizeof(char));
	char *buffer1 = l_calloc(w * h, sizeof(char));
	if (!buffer1) return 0;
	
	const int offset = radius;
	for (int i = 0; i < gp->pointsTotal; i++){
		int x = gp->points[i].x + offset;
		int y = gp->points[i].y + offset;

		//copyGlyph_addpixel(buffer1, x-1, y-1, w); copyGlyph_addpixel(buffer1, x, y-1, w); copyGlyph_addpixel(buffer1, x+1, y-1, w);
		//copyGlyph_addpixel(buffer1, x-1, y,   w);                                         copyGlyph_addpixel(buffer1, x+1, y,   w);
		copyGlyph_addpixel(buffer1, x-1, y+1, w); /*copyGlyph_addpixel(buffer1, x, y+1, w);*/ copyGlyph_addpixel(buffer1, x+1, y+1, w);
	}
	
	Kernel *kernel = makeKernel(radius);
	if (kernel){
		convolveAndTranspose8(kernel, buffer1, buffer2, w, h, WRAP_EDGES);
		convolveAndTranspose8(kernel, buffer2, buffer1, h, w, WRAP_EDGES);
		deleteKernel(kernel);
	}

	for (int y = 0; y < h; y++){
		for (int x = 0; x < w; x++){
			int pix = buffer1[y*w+x]&0xFF;
			if (pix)
				glyphAddPixelData(ep, x-offset, y-offset, pix, gp);
		}
	}	
	
	l_free(buffer1);
	l_free(buffer2);
	return 1;
}

static inline int copyGlyph_Blur8_calc (TWCHAR *glyph, const int radius)
{
	if (!glyph->ep[LTR_BLUR8]){
		glyph->ep[LTR_BLUR8] = (TGLYPHPOINTS*)l_malloc(sizeof(TGLYPHPOINTS));
		if (glyph->ep[LTR_BLUR8]){
			glyph->ep[LTR_BLUR8]->pointsTotal = 0;
			glyph->ep[LTR_BLUR8]->pointsSpace = 16;
			glyph->ep[LTR_BLUR8]->points = l_malloc(16 * sizeof(T2POINT));
		}else{
			return 0;
		}
	}
	
	TGLYPHPOINTS *ep = glyph->ep[LTR_BLUR8];
	TGLYPHPOINTS *gp = glyph->gp;
	
	const int w = glyph->w + (radius*2)+2;
	const int h = glyph->h + (radius*2)+2;
	char *buffer2 = l_calloc(w * h, sizeof(char));
	char *buffer1 = l_calloc(w * h, sizeof(char));
	if (!buffer1) return 0;
	
	const int offset = radius;
	for (int i = 0; i < gp->pointsTotal; i++){
		int x = gp->points[i].x + offset;
		int y = gp->points[i].y + offset;

		copyGlyph_addpixel(buffer1, x-1, y-1, w);/* copyGlyph_addpixel(buffer1, x, y-1, w);   copyGlyph_addpixel(buffer1, x+1, y-1, w);*/
		/*copyGlyph_addpixel(buffer1, x-1, y, w);                                             copyGlyph_addpixel(buffer1, x+1, y,   w);*/
		/*copyGlyph_addpixel(buffer1, x-1, y+1, w); copyGlyph_addpixel(buffer1, x, y+1, w);*/ copyGlyph_addpixel(buffer1, x+1, y+1, w);
	}
	
	Kernel *kernel = makeKernel(radius);
	if (kernel){
		convolveAndTranspose8(kernel, buffer1, buffer2, w, h, WRAP_EDGES);
		convolveAndTranspose8(kernel, buffer2, buffer1, h, w, WRAP_EDGES);
		deleteKernel(kernel);
	}

	for (int y = 0; y < h; y++){
		for (int x = 0; x < w; x++){
			int pix = buffer1[y*w+x]&0xFF;
			if (pix)
				glyphAddPixelData(ep, x-offset, y-offset, pix, gp);
		}
	}	
	
	l_free(buffer1);
	l_free(buffer2);
	return 1;
}

static inline void setPixel32_a_NB (const TFRAME *frm, const int x, const int y, const int value)
{
	TCOLOUR4 *dst = (TCOLOUR4*)m_getPixelAddr32(frm, x, y);
	const TCOLOUR4 *src = (TCOLOUR4*)&value;
	const unsigned int alpha = src->u.bgra.a + (unsigned int)(src->u.bgra.a>>7);
	const unsigned int odds2 = (src->u.colour>>8) & 0x00FF00FF;
	const unsigned int odds1 = (dst->u.colour>>8) & 0x00FF00FF;
	const unsigned int evens1 = dst->u.colour & 0x00FF00FF;
	const unsigned int evens2 = src->u.colour & 0x00FF00FF;
	const unsigned int evenRes = ((((evens2-evens1) * alpha)>>8) + evens1) & 0x00FF00FF;
	const unsigned int oddRes = ((odds2-odds1)*alpha + (odds1<<8)) & 0xFF00FF00;

	dst->u.colour = evenRes | oddRes;
}

static inline int check_bounds (const TFRAME *const frm, const int x, const int y)
{
	if (x < 0 || x >= frm->width || y >= frm->height || y < 0)
		return 1;
	else
		return 0;
}

static inline void setPixel32_a (const TFRAME *frm, const int x, const int y, const int value)
{
	if (!check_bounds(frm, x, y))
		setPixel32_a_NB(frm, x, y, value);
}

void copyGlyph_Blur (TLPRINTREGION *loc, const int op)
{

	if (!loc->glyph->ep[op] || (loc->glyph->ep[op] && (!loc->glyph->ep[op]->pointsTotal || loc->glyph->ep[op]->dataState != (loc->attributes[op][LTRA_BLUR_RADIUS]&0xFF)))){
		const int radius = loc->attributes[op][LTRA_BLUR_RADIUS]&0xFF;
		
		switch (op){
		  case LTR_BLUR1: copyGlyph_Blur1_calc(loc->glyph, radius); break;
		  case LTR_BLUR2: copyGlyph_Blur2_calc(loc->glyph, radius); break;
		  case LTR_BLUR3: copyGlyph_Blur3_calc(loc->glyph, radius); break;
		  case LTR_BLUR4: copyGlyph_Blur4_calc(loc->glyph, radius); break;
		  case LTR_BLUR5: copyGlyph_Blur5_calc(loc->glyph, radius); break;
		  case LTR_BLUR6: copyGlyph_Blur6_calc(loc->glyph, radius); break;
		  case LTR_BLUR7: copyGlyph_Blur7_calc(loc->glyph, radius); break;
		  case LTR_BLUR8: copyGlyph_Blur8_calc(loc->glyph, radius); break;
		  default: return;
		}
		loc->glyph->ep[op]->dataState = radius;
	}

	TFRAME *to = loc->to;
	int ink = loc->attributes[op][LTRA_BLUR_COLOUR]&RGB_24_WHITE;

	const int xOffset = loc->attributes[op][LTRA_BLUR_X];
	const int yOffset = loc->attributes[op][LTRA_BLUR_Y];
	float alpha = (float)loc->attributes[op][LTRA_BLUR_ALPHA]/1000.0f;
	alpha *= 1.7f;
	if (alpha > 1.0f) alpha = 1.0f;
	else if (alpha < 0.0f) alpha = 0.0f;

	TGLYPHPOINTS *ep = loc->glyph->ep[op];
	for (int i = 0; i < ep->pointsTotal; i++){
		int x = loc->dx + ep->points[i].x + xOffset;
		int y = loc->dy + ep->points[i].y + yOffset;
		int pix = (int)((float)ep->points[i].data * alpha);
		//l_setPixel(to, x, y, pix<<24 | ink);
		//l_setPixel(to, x, y, pix<<24 | ink);
		setPixel32_a(to, x, y, (pix<<24) | ink);
	}

	const int renderTop = loc->attributes[op][LTRA_BLUR_SETTOP];
	if (renderTop){
		ink = getForegroundColour(to->hw);
		TGLYPHPOINTS *gp = loc->glyph->gp;
		for (int i = 0; i < gp->pointsTotal; i++){
			int x = loc->dx + gp->points[i].x;
			int y = loc->dy + gp->points[i].y;
			//l_setPixel(to, x, y, ink);
			setPixel32_a(to, x, y, ink);
		}
	}	
}

static inline void copyGlyph_Blur1 (TLPRINTREGION *loc)
{
	copyGlyph_Blur(loc, LTR_BLUR1);
}

static inline void copyGlyph_Blur2 (TLPRINTREGION *loc)
{
	copyGlyph_Blur(loc, LTR_BLUR2);
}

static inline void copyGlyph_Blur3 (TLPRINTREGION *loc)
{
	copyGlyph_Blur(loc, LTR_BLUR3);
}

static inline void copyGlyph_Blur4 (TLPRINTREGION *loc)
{
	copyGlyph_Blur(loc, LTR_BLUR4);
}
static inline void copyGlyph_Blur5 (TLPRINTREGION *loc)
{
	copyGlyph_Blur(loc, LTR_BLUR5);
}

static inline void copyGlyph_Blur6 (TLPRINTREGION *loc)
{
	copyGlyph_Blur(loc, LTR_BLUR6);
}

static inline void copyGlyph_Blur7 (TLPRINTREGION *loc)
{
	copyGlyph_Blur(loc, LTR_BLUR7);
}

static inline void copyGlyph_Blur8 (TLPRINTREGION *loc)
{
	copyGlyph_Blur(loc, LTR_BLUR8);
}

void copyGlyph_Shadow (TLPRINTREGION *loc)
{

	const int value = loc->attributes[LTR_SHADOW][0];
 	const int ssize = value & (LTRA_SHADOW_S1|LTRA_SHADOW_S2|LTRA_SHADOW_S3|LTRA_SHADOW_S4|LTRA_SHADOW_S5);
	const int offset = (value>>8)&0xFF;
 	TFRAME *des = loc->to;
	const int ink = getForegroundColour(des->hw);
 	const int inkSh = (LTRA_SHADOW_TR(value)<<24) | LTRA_SHADOW_BKCOL(loc->attributes[LTR_SHADOW][1]);
	TGLYPHPOINTS *gp = loc->glyph->gp;

	for (int i = 0; i < gp->pointsTotal; i++){
		int dx = loc->dx + gp->points[i].x;
		int dy = loc->dy + gp->points[i].y;

		if (value&LTRA_SHADOW_S)
			dy += offset;
		else if (value&LTRA_SHADOW_N)
			dy -= offset;
		
		if (value&LTRA_SHADOW_E)
			dx += offset;
		else if (value&LTRA_SHADOW_W)
			dx -= offset;

		switch (ssize){
	  	  case LTRA_SHADOW_S5: 
			l_setPixel(des, dx, dy-1, inkSh);
	  	  case LTRA_SHADOW_S4: 
			l_setPixel(des, dx-1, dy, inkSh);
	  	  case LTRA_SHADOW_S3: 
			l_setPixel(des, dx, dy+1, inkSh);
	  	  case LTRA_SHADOW_S2: 
			l_setPixel(des, dx+1, dy, inkSh);
	  	  case LTRA_SHADOW_S1: 
			l_setPixel(des, dx, dy, inkSh);
		}
	}

	if ((value&LTRA_SHADOW_W && value&LTRA_SHADOW_E) || (value&LTRA_SHADOW_N && value&LTRA_SHADOW_S)){
		for (int i = 0; i < gp->pointsTotal; i++){
			int dx = loc->dx + gp->points[i].x;
			int dy = loc->dy + gp->points[i].y;
			
			if (value&LTRA_SHADOW_N)
				dy -= offset;
			else if (value&LTRA_SHADOW_S)
				dy += offset;	
			
			if (value&LTRA_SHADOW_W)
				dx -= offset;
			else if (value&LTRA_SHADOW_E)
				dx += offset;

			switch (ssize){
		  	  case LTRA_SHADOW_S5: 
				l_setPixel(des, dx, dy-1, inkSh);
		  	  case LTRA_SHADOW_S4: 
				l_setPixel(des, dx-1, dy, inkSh);
		  	  case LTRA_SHADOW_S3: 
				l_setPixel(des, dx, dy+1, inkSh);
		  	  case LTRA_SHADOW_S2: 
				l_setPixel(des, dx+1, dy, inkSh);
		  	  case LTRA_SHADOW_S1: 
				l_setPixel(des, dx, dy, inkSh);
			}
		}
	}

	for (int i = 0; i < gp->pointsTotal; i++){
		int dx = loc->dx + gp->points[i].x;
		int dy = loc->dy + gp->points[i].y;
		l_setPixel(des, dx, dy, (0xFF<<24)|ink);
	}
}

void copyGlyph_0 (TLPRINTREGION *loc)
{
	const TFRAME *to = loc->to;
	const int ink = getForegroundColour(to->hw);
	TGLYPHPOINTS *gp = loc->glyph->gp;
	T2POINT *points = gp->points;
	
	for (int i = 0; i < gp->pointsTotal; i++){
		int x = loc->dx + points[i].x;
		int y = loc->dy + points[i].y;
		
		if (!checkbounds(to, x, y))
			l_setPixel_NB(to, x, y, ink);
	}
}

static inline int copyGlyph_OL1_calc (TWCHAR *glyph)
{
	if (!glyph->ep[LTR_OUTLINE1]){
		glyph->ep[LTR_OUTLINE1] = (TGLYPHPOINTS*)l_malloc(sizeof(TGLYPHPOINTS));
		if (glyph->ep[LTR_OUTLINE1]){
			glyph->ep[LTR_OUTLINE1]->pointsTotal = 0;
			glyph->ep[LTR_OUTLINE1]->pointsSpace = 16;
			glyph->ep[LTR_OUTLINE1]->points = l_malloc(16 * sizeof(T2POINT));
		}else{
			return 0;
		}
	}
	TGLYPHPOINTS *ep = glyph->ep[LTR_OUTLINE1];
	TGLYPHPOINTS *gp = glyph->gp;
	
	int x,y;
	for (int i = 0; i < gp->pointsTotal; i++){
		x = gp->points[i].x;
		y = gp->points[i].y;
		                              glyphAddPixel(ep, x, y-1, gp);
		glyphAddPixel(ep, x-1, y, gp);                             glyphAddPixel(ep, x+1, y, gp);
		                              glyphAddPixel(ep, x, y+1, gp);
	}
	return 1;
}

void copyGlyph_OL1 (TLPRINTREGION *loc)
{
	const int op = LTR_OUTLINE1;
	if (!loc->glyph->ep[op] || (loc->glyph->ep[op] && !loc->glyph->ep[op]->pointsTotal))
		copyGlyph_OL1_calc(loc->glyph);
		
	int x,y;
	TFRAME *to = loc->to;
	int ink = loc->attributes[op][0];
	
	TGLYPHPOINTS *ep = loc->glyph->ep[op];
	for (int i = 0; i < ep->pointsTotal; i++){
		x = loc->dx + ep->points[i].x + 1;
		y = loc->dy + ep->points[i].y;
		l_setPixel(to, x, y, ink);
		l_setPixel(to, x, y, ink);
	}
	
	ink = getForegroundColour(to->hw);
	TGLYPHPOINTS *gp = loc->glyph->gp;
	for (int i = 0; i < gp->pointsTotal; i++){
		x = loc->dx + gp->points[i].x + 1;
		y = loc->dy + gp->points[i].y;
		l_setPixel(to, x, y, ink);
	}
}

static inline int copyGlyph_OL2_calc (TWCHAR *glyph, const int op)
{
	if (!glyph->ep[op]){
		glyph->ep[op] = (TGLYPHPOINTS*)l_malloc(sizeof(TGLYPHPOINTS));
		if (glyph->ep[op]){
			glyph->ep[op]->pointsTotal = 0;
			glyph->ep[op]->pointsSpace = 16;
			glyph->ep[op]->points = l_malloc(16 * sizeof(T2POINT));
		}else{
			return 0;
		}
	}
	TGLYPHPOINTS *ep = glyph->ep[op];
	TGLYPHPOINTS *gp = glyph->gp;
	
	for (int i = 0; i < gp->pointsTotal; i++){
		int x = gp->points[i].x;
		int y = gp->points[i].y;
		
		glyphAddPixel(ep, x-1, y-1, gp); glyphAddPixel(ep, x, y-1, gp); glyphAddPixel(ep, x+1, y-1, gp);
		glyphAddPixel(ep, x-1, y,   gp);                                glyphAddPixel(ep, x+1, y,   gp);
		glyphAddPixel(ep, x-1, y+1, gp); glyphAddPixel(ep, x, y+1, gp); glyphAddPixel(ep, x+1, y+1, gp);
	}
	return 1;
}

void copyGlyph_OL2 (TLPRINTREGION *loc)
{
	const int op = LTR_OUTLINE2;
	if (!loc->glyph->ep[op] || (loc->glyph->ep[op] && !loc->glyph->ep[op]->pointsTotal))
		copyGlyph_OL2_calc(loc->glyph, op);
		
	int x,y;
	TFRAME *to = loc->to;

	int ink = loc->attributes[op][0];
	TGLYPHPOINTS *ep = loc->glyph->ep[op];
	for (int i = 0; i < ep->pointsTotal; i++){
		x = loc->dx + ep->points[i].x + 1;
		y = loc->dy + ep->points[i].y;
		l_setPixel(to, x, y, ink);
		l_setPixel(to, x, y, ink);
	}
	
	ink = getForegroundColour(to->hw);
	TGLYPHPOINTS *gp = loc->glyph->gp;
	for (int i = 0; i < gp->pointsTotal; i++){
		x = loc->dx + gp->points[i].x + 1;
		y = loc->dy + gp->points[i].y;
		l_setPixel(to, x, y, ink);
	}
}

void copyGlyph_OL3 (TLPRINTREGION *loc)
{
	const int op = LTR_OUTLINE2;
	if (!loc->glyph->ep[op] || (loc->glyph->ep[op] && !loc->glyph->ep[op]->pointsTotal))
		copyGlyph_OL2_calc(loc->glyph, op);

	TFRAME *to = loc->to;
	int ink = loc->attributes[LTR_OUTLINE3][0];
	
	TGLYPHPOINTS *ep = loc->glyph->ep[op];
	for (int i = 0; i < ep->pointsTotal; i++){
		int x = loc->dx + ep->points[i].x + 1;
		int y = loc->dy + ep->points[i].y;
		l_setPixel(to, x, y, ink);
	}
}

static inline int copyGlyph_SM1_calc (TWCHAR *glyph)
{
	if (!glyph->ep[LTR_SMOOTH1]){
		glyph->ep[LTR_SMOOTH1] = (TGLYPHPOINTS*)l_malloc(sizeof(TGLYPHPOINTS));
		if (glyph->ep[LTR_SMOOTH1]){
			glyph->ep[LTR_SMOOTH1]->pointsTotal = 0;
			glyph->ep[LTR_SMOOTH1]->pointsSpace = 16;
			glyph->ep[LTR_SMOOTH1]->points = l_malloc(16 * sizeof(T2POINT));
		}else{
			return 0;
		}
	}
	TGLYPHPOINTS *ep = glyph->ep[LTR_SMOOTH1];
	TGLYPHPOINTS *gp = glyph->gp;
	
	int x,y;
	for (int i = 0; i < gp->pointsTotal; i++){
		x = gp->points[i].x;
		y = gp->points[i].y;
		glyphAddPixel(ep, x+1, y, gp);
		glyphAddPixel(ep, x, y+1, gp);
	}
	return 1;
}

static inline int copyGlyph_SM2_calc (TWCHAR *glyph)
{
	if (!glyph->ep[LTR_SMOOTH2]){
		glyph->ep[LTR_SMOOTH2] = (TGLYPHPOINTS*)l_malloc(sizeof(TGLYPHPOINTS));
		if (glyph->ep[LTR_SMOOTH2]){
			glyph->ep[LTR_SMOOTH2]->pointsTotal = 0;
			glyph->ep[LTR_SMOOTH2]->pointsSpace = 16;
			glyph->ep[LTR_SMOOTH2]->points = l_malloc(16 * sizeof(T2POINT));
		}else{
			return 0;
		}
	}
	TGLYPHPOINTS *ep = glyph->ep[LTR_SMOOTH2];
	TGLYPHPOINTS *gp = glyph->gp;
	
	int x,y;
	for (int i = 0; i < gp->pointsTotal; i++){
		x = gp->points[i].x;
		y = gp->points[i].y;
		glyphAddPixel(ep, x+1, y, gp);		// right
		glyphAddPixel(ep, x, y+1, gp);		// down
		glyphAddPixel(ep, x-1, y, gp);		// left
		glyphAddPixel(ep, x, y-1, gp);		// up
	}
	return 1;
}

void copyGlyph_SM1 (TLPRINTREGION *loc)
{
	const int op = LTR_SMOOTH1;
	if (!loc->glyph->ep[op] || (loc->glyph->ep[op] && !loc->glyph->ep[op]->pointsTotal))
		copyGlyph_SM1_calc(loc->glyph);
		
	int x,y;
	TFRAME *to = loc->to;

	int ink = loc->attributes[op][0];
	TGLYPHPOINTS *ep = loc->glyph->ep[op];
	for (int i = 0; i < ep->pointsTotal; i++){
		x = loc->dx + ep->points[i].x + 1;
		y = loc->dy + ep->points[i].y;
		l_setPixel(to, x, y, ink);
		l_setPixel(to, x, y, ink);
	}
	
	ink = getForegroundColour(to->hw);
	TGLYPHPOINTS *gp = loc->glyph->gp;
	for (int i = 0; i < gp->pointsTotal; i++){
		x = loc->dx + gp->points[i].x + 1;
		y = loc->dy + gp->points[i].y;
		l_setPixel(to, x, y, ink);
	}
}

void copyGlyph_SM2 (TLPRINTREGION *loc)
{
	const int op = LTR_SMOOTH2;
	if (!loc->glyph->ep[op] || (loc->glyph->ep[op] && !loc->glyph->ep[op]->pointsTotal))
		copyGlyph_SM2_calc(loc->glyph);
		
	int x,y;
	TFRAME *to = loc->to;

	int ink = loc->attributes[op][0];
	TGLYPHPOINTS *ep = loc->glyph->ep[op];
	for (int i = 0; i < ep->pointsTotal; i++){
		x = loc->dx + ep->points[i].x + 1;
		y = loc->dy + ep->points[i].y;
		l_setPixel(to, x, y, ink);
		l_setPixel(to, x, y, ink);
	}
	
	ink = getForegroundColour(to->hw);
	TGLYPHPOINTS *gp = loc->glyph->gp;
	for (int i = 0; i < gp->pointsTotal; i++){
		x = loc->dx + gp->points[i].x + 1;
		y = loc->dy + gp->points[i].y;
		l_setPixel(to, x, y, ink);
	}
}

void copyGlyph_SKFSM1 (TLPRINTREGION *loc)
{
	TFRAME *to = loc->to;
	TGLYPHPOINTS *gp = loc->glyph->gp;
	const int ink = getForegroundColour(to->hw);
	const int inksm = 0x40000000 | (ink&0x00FFFFFF);
	const float delta = 1.0f/255.0f * (float)(loc->attributes[LTR_SKEWEDFWSM1][0]&0xFF);
	const float factor = ((loc->glyph->box.btm - loc->glyph->box.top) + 1.0f) *delta;
		
	for (int i = 0; i < gp->pointsTotal; i++){
		int y = loc->dy + gp->points[i].y;
		int x = (loc->dx + gp->points[i].x) - (gp->points[i].y*factor);
				
		l_setPixel(to, x+1, y, inksm);	// right
		l_setPixel(to, x, y+1, inksm);	// down		
		l_setPixel(to, x, y, ink);
	}
}

void copyGlyph_SKFSM2 (TLPRINTREGION *loc)
{
	TFRAME *to = loc->to;
	TGLYPHPOINTS *gp = loc->glyph->gp;
	const int ink = getForegroundColour(to->hw);
	const int inksm = 0x40000000 | (ink&0x00FFFFFF);
	const float delta = 1.0f/255.0f * (float)(loc->attributes[LTR_SKEWEDFWSM2][0]&0xFF);
	const float factor = ((loc->glyph->box.btm - loc->glyph->box.top) + 1.0f) *delta;
		
	for (int i = 0; i < gp->pointsTotal; i++){
		int y = loc->dy + gp->points[i].y;
		int x = (loc->dx + gp->points[i].x) - (gp->points[i].y*factor);
				
		l_setPixel(to, x, y-1, inksm);	// up
		l_setPixel(to, x-1, y, inksm);	// left
		l_setPixel(to, x+1, y, inksm);	// right
		l_setPixel(to, x, y+1, inksm);	// down		
		l_setPixel(to, x, y, ink);
	}
}

void copyGlyph_SKF (TLPRINTREGION *loc)
{
	TFRAME *to = loc->to;
	TGLYPHPOINTS *gp = loc->glyph->gp;
	const int ink = getForegroundColour(to->hw);
	const float delta = 1.0f/255.0f * (float)(loc->attributes[LTR_SKEWEDFW][0]&0xFF);
	const float factor = ((loc->glyph->box.btm - loc->glyph->box.top) + 1.0f) *delta;
		
	for (int i = 0; i < gp->pointsTotal; i++){
		int y = loc->dy + gp->points[i].y;
		int x = (loc->dx + gp->points[i].x) - (gp->points[i].y*factor);
				
		l_setPixel(to, x, y, ink);
	}
}

void copyGlyph_SKB (TLPRINTREGION *loc)
{
	TFRAME *to = loc->to;
	TGLYPHPOINTS *gp = loc->glyph->gp;
	const int ink = getForegroundColour(to->hw);
	const float delta = 1.0f/255.0f * (float)(loc->attributes[LTR_SKEWEDBK][0]&0xFF);
	const float factor = ((loc->glyph->box.btm - loc->glyph->box.top) + 1.0f) *delta;
		
	for (int i = 0; i < gp->pointsTotal; i++){
		int y = loc->dy + gp->points[i].y;
		int x = (loc->dx + gp->points[i].x) + (gp->points[i].y*factor);
				
		l_setPixel(to, x, y, ink);
	}
}

void copyGlyph_SKBSM1 (TLPRINTREGION *loc)
{
	TFRAME *to = loc->to;
	TGLYPHPOINTS *gp = loc->glyph->gp;
	const int ink = getForegroundColour(to->hw);
	const int inksm = 0x40000000 | (ink&0x00FFFFFF);
	const float delta = 1.0f/255.0f * (float)(loc->attributes[LTR_SKEWEDBKSM1][0]&0xFF);
	const float factor = ((loc->glyph->box.btm - loc->glyph->box.top) + 1.0f) *delta;
		
	for (int i = 0; i < gp->pointsTotal; i++){
		int y = loc->dy + gp->points[i].y;
		int x = (loc->dx + gp->points[i].x) + (gp->points[i].y*factor);
				
		l_setPixel(to, x, y, ink);
		l_setPixel(to, x+1, y, inksm);	// right
		l_setPixel(to, x, y+1, inksm);	// down
	}
}

void copyGlyph_SKBSM2 (TLPRINTREGION *loc)
{
	TFRAME *to = loc->to;
	TGLYPHPOINTS *gp = loc->glyph->gp;
	const int ink = getForegroundColour(to->hw);
	const int inksm = 0x40000000 | (ink&0x00FFFFFF);
	const float delta = 1.0f/255.0f * (float)(loc->attributes[LTR_SKEWEDBKSM2][0]&0xFF);
	const float factor = ((loc->glyph->box.btm - loc->glyph->box.top) + 1.0f) *delta;
		
	for (int i = 0; i < gp->pointsTotal; i++){
		int y = loc->dy + gp->points[i].y;
		int x = (loc->dx + gp->points[i].x) + (gp->points[i].y*factor);
				
		l_setPixel(to, x, y-1, inksm);	// up
		l_setPixel(to, x-1, y, inksm);	// left
		l_setPixel(to, x, y, ink);
		l_setPixel(to, x+1, y, inksm);	// right
		l_setPixel(to, x, y+1, inksm);	// down
	}
}

void copyGlyph_FlipH (TLPRINTREGION *loc)
{
	TFRAME *to = loc->to;
	const int ink = getForegroundColour(to->hw);
	TGLYPHPOINTS *gp = loc->glyph->gp;

	for (int i = 0; i < gp->pointsTotal; i++){
		int x = loc->dx + loc->glyph->w - gp->points[i].x;
		int y = loc->dy + gp->points[i].y;
		l_setPixel(to, x, y, ink);
	}
}

void copyGlyph_FlipV (TLPRINTREGION *loc)
{
	TFRAME *to = loc->to;
	const int ink = getForegroundColour(to->hw);
	TGLYPHPOINTS *gp = loc->glyph->gp;

	for (int i = 0; i < gp->pointsTotal; i++){
		int x = loc->dx + gp->points[i].x;
		int y = loc->dy + loc->glyph->h - gp->points[i].y;
		l_setPixel(to, x, y, ink);
	}
}

void copyGlyph_180 (TLPRINTREGION *loc)
{
	TFRAME *to = loc->to;
	const int ink = getForegroundColour(to->hw);
	TGLYPHPOINTS *gp = loc->glyph->gp;

	for (int i = 0; i < gp->pointsTotal; i++){
		int x = loc->dx + loc->glyph->w - gp->points[i].x;
		int y = loc->dy + loc->glyph->h - gp->points[i].y;
		l_setPixel(to, x, y, ink);
	}
}

void renderEffectReset (THWD *hw, const int fontid, const int effectCache)
{
	TWFONT *font = fontIDToFontW(hw, fontid);
	if (!font || !font->built) return;
	if (effectCache >= LTR_TOTAL) return;

	if (font->chr){
		for (int i = font->maxCharIdx; i--;){
			if (font->chr[i]){
				if (font->chr[i]->ep[effectCache])
					font->chr[i]->ep[effectCache]->pointsTotal = 0;
			}
		}
	}
}
	
static inline void *getRenderFunc (TTRENDER *render, const int mode)
{
	static void *renderFunc[LTR_TOTAL];

	if (!renderFunc[LTR_0]){
		assert(LTR_0 == 0);	// sanity check
		
		renderFunc[LTR_0] = copyGlyph_0;
		renderFunc[LTR_180] = copyGlyph_180;
		renderFunc[LTR_FLIPH] = copyGlyph_FlipH;
		renderFunc[LTR_FLIPV] = copyGlyph_FlipV;
		renderFunc[LTR_OUTLINE1] = copyGlyph_OL1;
		renderFunc[LTR_OUTLINE2] = copyGlyph_OL2;
		renderFunc[LTR_OUTLINE3] = copyGlyph_OL3;
		renderFunc[LTR_SMOOTH1] = copyGlyph_SM1;
		renderFunc[LTR_SMOOTH2] = copyGlyph_SM2;
		renderFunc[LTR_SKEWEDFW] = copyGlyph_SKF;
		renderFunc[LTR_SKEWEDFWSM1] = copyGlyph_SKFSM1;
		renderFunc[LTR_SKEWEDFWSM2] = copyGlyph_SKFSM2;
		renderFunc[LTR_SKEWEDBK] = copyGlyph_SKB;
		renderFunc[LTR_SKEWEDBKSM1] = copyGlyph_SKBSM1;
		renderFunc[LTR_SKEWEDBKSM2] = copyGlyph_SKBSM2;
		renderFunc[LTR_SHADOW] = copyGlyph_Shadow;
		renderFunc[LTR_BLUR1] = copyGlyph_Blur1;
		renderFunc[LTR_BLUR2] = copyGlyph_Blur2;
		renderFunc[LTR_BLUR3] = copyGlyph_Blur3;
		renderFunc[LTR_BLUR4] = copyGlyph_Blur4;
		renderFunc[LTR_BLUR5] = copyGlyph_Blur5;
		renderFunc[LTR_BLUR6] = copyGlyph_Blur6;
		renderFunc[LTR_BLUR7] = copyGlyph_Blur7;
		renderFunc[LTR_BLUR8] = copyGlyph_Blur8;
		renderFunc[LTR_USER1] = NULL;
		renderFunc[LTR_USER2] = NULL;
	}

	return renderFunc[mode];
}

int setRenderEffect (THWD *hw, const int mode)
{
	hw->render->copy = getRenderFunc(hw->render, mode);
	return (hw->render->copy != NULL);
}

#else

int textGlueRender (TFRAME *frm, THWD *hw, char *str, TLPRINTR *rect, int fontid, int flags, int style){return 0;}
int textRender (TFRAME *frm, THWD *hw, UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, int style){return 0;}
int textWrapRender (TFRAME *frm, THWD *hw, UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, int style){return 0;}
int setRenderEffect (THWD *hw, const int mode){return 0;}
void renderEffectReset (THWD *hw, const int fontid, const int effectCache){return;}
int cacheCharacterBuffer (THWD *hw, const UTF32 *glist, int total, int fontid){return 0;}
int getTextMetricsList (THWD *hw, const UTF32 *glist, int first, int last, int flags, int fontid, TLPRINTR *rect){return 0;}

#endif

