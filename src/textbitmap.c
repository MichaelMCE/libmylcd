
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

#if (__BUILD_BITMAP_FONT_SUPPORT__)

#include "frame.h"
#include "device.h"
#include "memory.h"
#include "fonts.h"
#include "lstring.h"
#include "textbitmap.h"
#include "draw.h"
#include "pixel.h"
#include "copy.h"
#include "lmath.h"
#include "print.h"
#include "image.h"
#include "api.h"

#include "sync.h"
#include "misc.h"

static int getTop (const TFRAME *const frm);
static int getBottom (const TFRAME *const frm);
static int getLeft (const TFRAME *const frm);
static int getRight(const TFRAME *const frm);




int textBitmapRenderWrap (TFRAME *frm, THWD *hw, const char *string, TLPRINTR *rect, int fontid, int flags, int style)
{
	if (!frm||!string||!rect) return 0;

	int slen = l_strlen(string);
	if (!slen) return 0;

	int c = 0,total = 0;
	int chars = 0,w = 0;
	int rectw, passcount = 1;
	int minx = frm->width;
	int miny = frm->height;
	int maxx = 0, maxy =0 ;
	int textboundflag = (flags&PF_TEXTBOUNDINGBOX) | (flags&PF_INVERTTEXTRECT);
	
	TFONT *font= (TFONT *)fontIDToFont(hw, fontid);
	rect->ey = rect->sy;

	// correct horizontal alignment when using 'PF_USELASTX'
	//if (!(flags&PF_NEWLINE) && (flags&PF_USELASTX))
	//		rect->ey--;

	do{
		total=c;
		if (!passcount)
			rect->sy = rect->ey + font->lineSpace;
		else{
			passcount=0;
			rect->sy = rect->ey;
		}
		
		if (flags&PF_LEFTJUSTIFY){
			// is left justified by default
    	}else if (flags&PF_MIDDLEJUSTIFY){
			rectw = rect->bx2-rect->bx1+1;
			getTextMetrics(hw, string+c, flags, fontid, &w, 0);
			if (w>=rectw)
				rect->sx=rect->bx1;
			else
				rect->sx = rect->bx1 + l_abs((rectw>>1)-(w>>1));
    	}else if (flags&PF_RIGHTJUSTIFY){
			getTextMetrics(hw, string+c, flags, fontid, &w, 0);
			rect->sx = (rect->bx2-rect->bx1)-w;
    	}
    	rect->ex = rect->sx = MAX(rect->sx,0);
		rect->ey = rect->sy = MAX(rect->sy,0);
		chars = textBitmapRender(frm, hw, string+c, rect, fontid, flags|PF_CLIPDRAW, style);
		
		if (textboundflag){
			minx = MIN (minx, rect->sx);
			miny = MIN (miny, rect->sy);
			maxx = MAX (maxx, rect->ex);
			maxy = MAX (maxy, rect->ey);
		}
		if (chars)
			c+=chars;
		else
			break;
	}while ((c<slen-1) && c && (c!=total));

	if (frm){
		if (flags&PF_INVERTTEXTRECT){
			invertArea(frm,minx,miny,maxx-minx,maxy-miny);
		}else if (flags&PF_TEXTBOUNDINGBOX)
			drawRectangleDotted(frm,minx,miny,maxx-minx-1,maxy-miny, style /*LSP_SET*/);
	}
	
	return c;
}

static int textBitmapRenderGetCharWidth (const int c, TFONT *font, const int fixedwflag, int *x1)
{
	if (!fixedwflag){
		*x1 = ((c%font->charsPerRow)*font->charW) + font->xoffset; // x offset within bitmap
		return font->c[c].right - font->c[c].left;			// width of char
	}else{
		*x1 = ((c%font->charsPerRow)*font->charW);
		return font->charW-1;
	}
}


static int getBMPixel (const TFRAME *from, TFRAME *to, const int x, const int y)
{
	if (getPixel_NB(from, x, y))
		return to->hw->render->foreGround;
	else
		return to->hw->render->backGround;
}

static int copyGlyph (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2, int style)
{
	int x, y, xx;
	const int ink = to->hw->render->foreGround;
	y2 = MIN(y2+1,from->height);
	x2 = MIN(x2+1,from->width);

	for (y = y1; y<y2; y++, dy++){
		xx = dx;
		if (style == LCASS_CPY){
			for (x = x1; x<x2; x++, xx++){
				if (getPixel_NB(from, x, y))
					l_setPixel(to, xx, dy, ink);
			}
		}else if (style == LCASS_OR){
			for (x = x1; x<x2; x++, xx++)
				l_setPixel(to, xx, dy, getBMPixel(from, to, x, y) | l_getPixel(to, xx, dy));
					
		}else if (style == LCASS_AND){
			for (x = x1; x<x2; x++, xx++)
				l_setPixel(to, xx, dy, getBMPixel(from, to, x, y) & l_getPixel(to, xx, dy));
						
		}else if (style == LCASS_XOR){
			for (x = x1; x<x2; x++, xx++)
				l_setPixel(to, xx, dy, getBMPixel(from, to, x, y) ^ l_getPixel(to, xx, dy));
						
		}else if (style == LCASS_NOT){
			for (x = x1; x<x2; x++, xx++)
				l_setPixel(to, xx, dy, /*l_getPixel(to,xx+sx,dy+sy) &*/ ~getBMPixel(from, to, x, y));
						
		}else if (style == LCASS_CLEAR){
			for (x = x1; x<x2; x++, xx++)
				l_setPixel(to, xx, dy, to->hw->render->backGround);
		}
	}
	return 1;
}

int textBitmapRender (TFRAME *frm, THWD *hw, const char *string, TLPRINTR *rect, int fontid, int flags, int style)
{
	if (/*!string||!fontid ||*/ (!frm && !(flags&PF_DONTRENDER))) return 0;
	if (!buildBitmapFont(hw, fontid)) return 0;
	
	TFONT *font = fontIDToFontA(hw, fontid);
	if (!font) return 0;
	
	int i,x1,y1,c=0;
	int w=0,h=0;
	int width, chr, pos, status;
	const int slen = l_strlen(string);
	const int dontrenderflag = flags&PF_DONTRENDER;
	const int cliptexthflag = flags&PF_CLIPTEXTH;
	const int cliptextvflag = flags&PF_CLIPTEXTV;
	const int clipdrawflag = flags&PF_CLIPDRAW;
	const int noescapeflag = flags&PF_NOESCAPE;
	const int fixedwflag = flags&PF_FIXEDWIDTH;
	const int boundingboxflag = (flags&PF_GLYPHBOUNDINGBOX) && frm;
	const int wordwrapflag = flags&PF_WORDWRAP;

	for (i=0; i<slen; i++){
		if (!noescapeflag){
			if ((ubyte)*(string+i) == 0x0A)
				return ++i;
		}
		c = MAX((ubyte)*(string+i) - font->choffset, 0);
		font->xoffset = font->c[c].left;
		//f->yoffset = 0;

		//if ((ubyte)*(string+i) != ' '){
			w = textBitmapRenderGetCharWidth(c, font, fixedwflag, &x1);
			h = font->c[c].btm - font->yoffset;					
			y1 = ((c/font->rowsPerImage)*font->charH) + font->yoffset;
						
			if (clipdrawflag){
				if (rect->ex+w > rect->bx2){
					if (cliptexthflag) break;
					w -= (rect->ex+w - rect->bx2);
				}
				if (rect->sy+h > rect->by2){
					if (cliptextvflag) break;
					h -= (rect->sy+h - rect->by2);
				}
			}

			if (!dontrenderflag)
				copyGlyph(font->f, frm, rect->ex, rect->sy, x1, y1, x1+w, y1+h, style);
			if (boundingboxflag)
				drawRectangleDotted(frm, rect->ex, rect->sy+font->c[c].top, w+1, font->c[c].btm-font->c[c].top+1, style/*LSP_SET*/);
		//}

		rect->ey = MAX(rect->ey, rect->sy+h+1);
		
		if ((rect->ex-1 < rect->bx2) && (rect->sy < rect->by2))
			rect->ex+=(font->charSpace+(font->c[c].right - font->c[c].left)+1);
		else
			if (clipdrawflag)
				break;

		if (wordwrapflag){
			if (((ubyte)*(string+i) == ' ') || ((ubyte)*(string+i) == '-')){
				w = 0;
				status = 1;
				pos = i;

				do{
					if (++pos < slen){
						chr = (ubyte)*(string+pos);
						if ((chr != ' ') && (chr != '\n') && (chr != '-') && (chr != 0)){
							width = textBitmapRenderGetCharWidth(MAX(chr-font->choffset,0), font, fixedwflag, &x1);
							w += (font->charSpace+width+1);
						}else
							status = 0;
					}else
						status = 0;
				}while(status);
			}
		}

		if (w && (rect->ex+w > rect->bx2) && (w < (rect->bx2-rect->bx1)))
			return ++i;
	}
	return i;
}


// create a lookup table of each of the characters dimensions
// 1 table per font with 255 individual character dimensions
// need more than 255? use BDF.
int buildBitmapFont (THWD *hw, const int id)
{
	TFONT *font = fontIDToFontA(hw, id);
	if (!font)
		return 0;
	else if (font->built)
		return 1;

	if (font->f)
		deleteFrame(font->f);
	
	if (!(font->f=_newImage(hw, font->file, font->imageType, LFRM_BPP_1))){
		mylogw(L"libmylcd: unable to open font image: '%s'\n", font->file);
		return 0;
	}

	if (font->invert)
		invertFrame(font->f);
	if (font->autowidth || !font->charW)
		font->charW = font->f->width / font->charsPerRow;
	if (font->autoheight || !font->charH)
		font->charH = font->f->height / font->rowsPerImage;

	// extract character dimensions via a temporary surface
	TFRAME *frm = _newFrame(hw, font->charW, font->charH, 1, LFRM_BPP_1);
	if (frm == NULL){
		deleteFrame(font->f);
		font->f = NULL;
		return 0;
	}

	int c,x1,y1;
	// retrieve actual width and height of each glyph
	for (c=0; c < 256-font->choffset; c++){
		x1 = ((c%font->charsPerRow)*font->charW);
		y1 = ((c/font->charsPerRow)*font->charH);
		copyArea_fast8(font->f, frm, 0, 0, x1, y1, x1+font->charW-1, y1+font->charH-1);
		
		font->c[c].top = getTop(frm);
		font->c[c].btm = getBottom(frm);
		font->c[c].left = getLeft(frm);
		font->c[c].right = getRight(frm);
	}
	deleteFrame(frm);
	
	// set the width of a 'space' to be roughly half that of an 'a'
	// seems to work quite well in practice
	font->c[' '-font->choffset].right = font->c['a'-font->choffset].right>>1;
	font->c[' '-font->choffset].btm = font->c['a'-font->choffset].btm;
	font->built = 1;
	return 1;
}


static int getTop (const TFRAME *const frm)
{
	int x,y;
	for (y=0;y<frm->height;y++){
		for (x=frm->width;x--;)
			if (getPixel_NB(frm,x,y)) return y;
	}
	return 1;
}

static int getBottom (const TFRAME *const frm)
{
	int x,y;
	for (y=frm->height;y--;){
		for (x=0;x<frm->width;x++)
			if (getPixel_NB(frm,x,y)) return y;
	}
	return frm->height>>1;
}

static int getLeft (const TFRAME *const frm)
{
	int x,y;
	for (x=0;x<frm->width;x++){
		for (y=frm->height;y--;)
			if (getPixel_NB(frm,x,y)) return x;

	}			
	return 1;
}

static int getRight (const TFRAME *const frm)
{
	int x,y;
	for (x=frm->width-1;x>=0;x--){
		for (y=frm->height;y--;)
			if (getPixel_NB(frm,x,y)) return x;
	}
	return frm->width>>1;
}


#else

int textBitmapRenderWrap (TFRAME *frm, THWD *hw, const char *string, TLPRINTR *rect, int fontid, int flags, int style){return 0;}
int textBitmapRender (TFRAME *frm, THWD *hw, const char *string, TLPRINTR *rect, int fontid, int flags, int style){return 0;}
int buildBitmapFont (THWD *hw, const int id){return 0;}

#endif

