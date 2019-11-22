	
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

#if (__BUILD_FONTS_SUPPORT__ && (__BUILD_BITMAP_FONT_SUPPORT__ || __BUILD_BDF_FONT_SUPPORT__))

#include "memory.h"
#include "utils.h"
#include "device.h"
#include "frame.h"
#include "lstring.h"
#include "pixel.h"
#include "fonts.h"
#include "cmap.h"
#include "bdf.h"
#include "print.h"
#include "chardecode.h"
#include "api.h"
#include "sync.h"
#include "misc.h"



#if (__BUILD_BITMAP_FONT_SUPPORT__)


static TFONT *rootFontA (THWD *hw)
{
	return hw->fonts->bmp;
}

static inline int isFontIdValid (THWD *hw, int id)
{
	TFONT *f = rootFontA(hw);
	if (!f) return 0;

	do{
		if (f){
			if (f->id == id)
				return id;
		}else{
			return 0;
		}
		f = f->next;
	}while(f);
	return 0;
}

static inline int getFontTotalA (THWD *hw)
{
	TFONT *f = rootFontA(hw);
	if (!f) return 0;
	
	int total = 0;
	f = f->next;
	
	while (f){
		total++;
		f = f->next;
	}
	return total;
}

TENUMFONT *enumFontsBeginA (THWD *hw)
{
	TENUMFONT *enf = l_calloc(1, sizeof(TENUMFONT));
	if (enf){
		enf->fi = 0;
		enf->wfont = NULL;
		enf->font = rootFontA(hw);
		enf->font = enf->font->next;
		enf->id = enf->font->id;
		enf->total = getFontTotalA(hw);
		enf->hw = hw;
	}
	return enf;
}

int enumFontsNextA (TENUMFONT *enf)
{
	if (!enf) return 0;
	if (enf->font->next){
		enf->fi++;
		enf->wfont = NULL;
		enf->font = enf->font->next; 
		enf->id = enf->font->id;
		enf->total = getFontTotalA(enf->hw);
		enf->hw = enf->font->hw;
		const int ret = enf->id;
		return ret;
	}else{
		return 0;
	}
}

void enumFontsDeleteA (TENUMFONT *enf)
{
	l_free(enf);
}

static inline int flushFontA (THWD *hw, int id)
{
	TFONT *font = fontIDToFontA(hw, id);
	if (font){
		if (font->f){
			deleteFrame(font->f);
			font->f = NULL;
		}
		font->built = 0;
		return 1;
	}
	return 0;
}

int unregisterFontA (THWD *hw, int id)
{
	if (flushFontA(hw, id)){
		TFONT *font = fontIDToFontA(hw, id);
		if (!font){
			font->built = 0;
			font->id = 0;
			return 1;
		}
	}
	return 0;
}

static inline int addFontToListA (THWD *hw, TFONT *font)
{
	TFONT *f = rootFontA(hw);
	if (!f)
		return 0;

	if (!f->next){
		f->next = font;
	}else{
		font->next = f->next;
		f->next = font;
	}
	return 1;
}

int registerFontA (THWD *hw, const wchar_t *filename, TFONT *f)
{
	if (!f) return 0;
	TFONT *font = l_calloc(1, sizeof(TFONT));
	if (font == NULL) return 0;

	font->built = 0;
	font->id = f->id;
	font->imageType = f->imageType;
	font->charW = f->charW;
	font->charH = f->charH;
	font->charsPerRow = f->charsPerRow;
	font->rowsPerImage = f->rowsPerImage;
	font->xoffset = f->xoffset;
	font->yoffset = f->yoffset;
	font->choffset = f->choffset;
	font->charSpace = f->charSpace;
	font->lineSpace = f->lineSpace;
	font->invert = f->invert;
	font->autoheight = f->autoheight;
	font->autowidth = f->autoheight;
	font->f = NULL;
	font->next = NULL;
	font->hw = hw;
	l_wcsncpy(font->file, filename, MIN(MaxPath*sizeof(wchar_t), l_wcslen(filename)));

	font->file[MaxPath-1] = 0;

	if (addFontToListA(hw, font)){
		return 1;
	}else{
		l_free(font);
		return 0;
	}
}

static inline int setFontSpaceA (THWD *hw, int fontid, int pixels)
{
	TFONT *font = fontIDToFontA(hw, fontid);
	if (!font){
		return -1;
	}else{
		int oldspace = font->charSpace;
		font->charSpace = pixels;
		return oldspace;
	}
}

static inline int getFontSpaceA (THWD *hw, int fontid)
{
	TFONT *font = fontIDToFontA(hw, fontid);
	if (!font)
		return -1;
	else
		return font->charSpace;
}

static inline int deleteFontsA (THWD *hw, TFONT *f)
{
	if (!f) return 0;
	TFONT *p = NULL;
	
	// free last font first (LIFO - last reg'd font is first in list)
	do{
		p = f;
		f = f->next;
		if (!f->next){
			unregisterFontA(hw, f->id);
			l_free(f);
			p->next = NULL;
			if (p == rootFontA(hw)) break;
			f = rootFontA(hw);
		}
	}while(f);

	return 1;
}


TFONT *fontIDToFontA (THWD *hw, int id)
{
	if (!isFontIdValid(hw, id)) return 0;	
	TFONT *f = rootFontA(hw);
	if (!f) return 0;
	
	do{
		if (f){
			if (f->id == id)
				return f;
		}else{
			return NULL;
		}

		f = f->next;
	} while(f);
	return NULL;
}


#else


#define registerFontsA intstub
#define flushFontA intstub
#define setFontSpaceA intstub
#define getFontSpaceA intstub
#define deleteFontsA voidstub
#define rootFontA ptrstub
#define enumFontsBeginA ptrstub
#define enumFontsNextA intstub
#define enumFontsDeleteA voidstub
#define registerFontA intstub
#define unregisterFontA intstub
#define fontIDToFontA ptrstub

#endif



#if 1

static inline int qsortdec (const void *_a, const void *_b)
{
	if (!_a || !_b) return 0;
	const UTF32 a = *(UTF32*)_a;
	const UTF32 b = *(UTF32*)_b;

	if (a > b)
		return -1;
	else if (a < b)
		return 1;
	else
		return 0;
}

static inline void sortDescending (UTF32 *glist, const int gtotal)
{
	qsort(glist, gtotal, sizeof(UTF32), qsortdec);
}

#else

static inline void swapu32 (UTF32 *a, UTF32 *b)
{
	UTF32 tmp = *a;
	*a = *b;
	*b = tmp;
}

static inline void sortDescending (UTF32 *glist, int gtotal)
{
	if (!gtotal) return;
	int i = --gtotal;
	
	while(i--){
		for (int j = 0; j < gtotal; j++){
			if (glist[j] < glist[j+1])
				swapu32(&glist[j], &glist[j+1]);
		}
	}
}
#endif

static inline int isCharBuilt (UTF32 ch, TWFONT *font)
{
	if (ch && font){
		if (ch < font->maxCharIdx){
			if (font->chr)
				if (font->chr[ch])
					return 1;
		}
	}
	return 0;
}

static inline int isCharUnsupported (UTF32 ch, TWFONT *font)
{
	//if (ch){
		if (font->uscl.uslist){
			for (int i = font->uscl.total; i--; ){
				if (font->uscl.uslist[i] == ch)
					return 1;
			}
		}
	//}
	return 0;
}

static inline int updateUnsupportedCharList (UTF32 *glist, int gtotal, TWFONT *font)
{
	for (int i = gtotal; i--; ){
		if (!isCharBuilt(*glist, font)){
			if (!isCharUnsupported(*glist, font)){
				if (!font->uscl.uslist){
					font->uscl.total = 0;
					font->uscl.uslist = (UTF32 *)l_malloc(sizeof(UTF32)*2);
				}else{
					font->uscl.uslist = (UTF32 *)l_realloc(font->uscl.uslist, sizeof(UTF32)*(font->uscl.total+2));
				}
			
				if (font->uscl.uslist == NULL)
					return 0;
				font->uscl.uslist[font->uscl.total++] = *glist;
			//	if (*glist != 13 && *glist != 10)
			//		mylog("unsupported char: %i, total: %i, fontID: %i\n", *glist, font->uscl.total, font->FontID);
			}
		}
		glist++;
	}
	return font->uscl.total;
}

static inline TWFONT *rootFontW (THWD *hw)
{
	return hw->fonts->bdf;
}

static inline int getFontTotalW (THWD *hw)
{
	TWFONT *f = rootFontW(hw);
	if (!f) return 0;
	
	int total=0;
	
	while ((f = f->next))
		total++;

	return total;
}


// FILO - first in last out.
static int addFontToListW (THWD *hw, TWFONT *font)
{
	TWFONT *f = rootFontW(hw);
	if (!f) return 0;

	if (!f->next){
		f->next = font;
	}else{
		font->next = f->next;
		f->next = font;
	}
	return 1;
}

int fontFlush (TWFONT *font)
{
	if (!font->built){
		if (font->chr){
			l_free(font->chr);
			font->chr = NULL;
		}
		return 1;
	}

	int flushed = 1;

	if (font->chr){
		for (int i = 0; i < font->maxCharIdx; i++){
			if (font->chr[i]){
				flushed++;
				
				if (font->chr[i]->gp->pointsSpace){
					l_free(font->chr[i]->gp->points);
					l_free(font->chr[i]->gp);
				}
				
				for (int j = 0; j < LTR_TOTAL; j++){
					if (font->chr[i]->ep[j]){
						if (font->chr[i]->ep[j]->pointsSpace)
							l_free(font->chr[i]->ep[j]->points);
						l_free(font->chr[i]->ep[j]);
						//font->chr[i]->ep[i] = NULL;
					}
				}
				
				l_free(font->chr[i]);
			}
		}

		l_free(font->chr);
		font->chr = NULL;
		font->maxCharIdx = 0;
		font->CharsBuilt = 0;
		font->flags = 0;
		if (font->uscl.total){
			l_free(font->uscl.uslist);
			font->uscl.uslist = NULL;
			font->uscl.total = 0;
		}
	}

	font->built = 0;
	return flushed;
}

static inline INLINE int fontFlushAll (THWD *hw)
{
	TWFONT *f = rootFontW(hw);
	if (!f) return 0;
	
	int ct = 0;
	do{
		if (f->FontID){
			//ct += f->built;
			ct += fontFlush(f);
		}
	}while((f=f->next));
	
	return ct;
}

static inline int flushFontW (THWD *hw, const int fontid)
{
	if (fontid == -1){
		return fontFlushAll(hw);
	}else {
		TWFONT *font = fontIDToFontW(hw, fontid);
		if (!font) return 0;
		return fontFlush(font);
	}
}

static inline wchar_t *formatFontPath (THWD *hw, const wchar_t *file, wchar_t *path)
{
	//snwprintf(path, MaxPath, L"%s%s", getFontPath(hw), file);
	path[0] = 0;
	wcsncat(path, getFontPath(hw), MaxPath);
	wcsncat(path, file, MaxPath);
	return path;
}

static inline void regwidefont(THWD *hw, wchar_t *file, int fontid, int charSpace)
{ 
	wchar_t path[MaxPath];
	TWFONT regwfont;
	regwfont.FontID = fontid;
	regwfont.CharSpace = charSpace;
	regwfont.LineSpace = lLineSpaceHeight;
	registerFontW(hw, formatFontPath(hw, file, path), &regwfont);
}

static inline int deleteFontsW (THWD *hw, TWFONT *f)
{
	if (!f) return 0;
	TWFONT *p = NULL;
	
	do{
		p=f;
		f=f->next;
		if (!f->next){
			unregisterFontW(hw, f->FontID);
			l_free(f);
			p->next = NULL;
			if (p == rootFontW(hw)) break;
			f = rootFontW(hw);
		}
	}while(f);

	return 1;
}

void deleteFonts (THWD *hw)
{
	deleteFontsA(hw, rootFontA(hw));
	deleteFontsW(hw, rootFontW(hw));
}

int cacheCharactersAll (THWD *hw, int fontid)
{
	TWFONT *font = fontIDToFontW(hw, fontid);
	if (font == NULL) return -1;
	font->built = 0;
	buildBDFFont(font);
	return font->CharsBuilt;
}

int cacheCharacters (THWD *hw, const char *str, int fontid)
{
	TWFONT *font = fontIDToFontW(hw, fontid);
	if (font == NULL) return -1;
	
	int ctotal = 0;
	int total = countCharacters(hw, str);
	if (total){
		
		UTF32 *glist = (UTF32*)l_calloc(total+1, sizeof(UTF32));
		if (glist){
			if ((total=createCharacterList(hw, str, glist, total))){
				if ((total=stripCharacterList(hw, glist, total, font))){
					ctotal=cacheGlyphs(glist, total, font);
				}
			}
			l_free(glist);
		}
	}
	return ctotal;
}

int stripCharacterList (THWD *hw, UTF32 *glist, const int gtotal, TWFONT *font)
{
	UTF32 *pglist = glist;
	int total = 0;
	int i = gtotal;

	while(i--){
		if (!isCharBuilt(*pglist,font)){
			if (!isCharUnsupported(*pglist,font)){
				total++;
			}else{
				*pglist = 0;
			}
		}else{
			*pglist = 0;
		}
		pglist++;
	}
	sortDescending(glist, gtotal);

	return total;
}

int cacheCharacterList (THWD *hw, UTF32 *glist, int gtotal, int fontid)
{
	TWFONT *font = fontIDToFontW(hw, fontid);
	if (!font) return -1;
	
	int ret = 0;
	int total;

	if ((total=stripCharacterList(hw, glist, gtotal, font)))
		ret = cacheGlyphs(glist, total, font);
	return ret;
}

int cacheCharacterRange (THWD *hw, UTF32 min, UTF32 max, int fontid)
{
	TWFONT *font = fontIDToFontW(hw, fontid);
	if (!font || (max < min)) return -1;
	
	int ctotal = 0;
	UTF32 mini = min;
	int total = max-min+1;

	if (total){
		UTF32 *glist = (UTF32 *)l_calloc(total+1, sizeof(UTF32));
		if (glist){

			int i = total;
			while(i--)
				glist[i] = mini++;

			if ((total=stripCharacterList(hw, glist, total, font)))
				ctotal = cacheGlyphs(glist, total, font);

			l_free(glist);
		}
	}
	return ctotal;
}


// return 0 if TGA, 1 if BDF
static inline int fontType (int fontid)
{
	return (fontid < lBaseFontW) ? 0: 1;
}

static inline int setFontSpaceW (THWD *hw, int fontid, int pixels)
{
	TWFONT *font = fontIDToFontW(hw, fontid);
	if (!font){
		return -1;
	}else{
		int oldspace = font->CharSpace;
		font->CharSpace = pixels;
		return oldspace;
	}
}

static inline int getFontSpaceW (THWD *hw, int fontid)
{
	TWFONT *font = fontIDToFontW(hw, fontid);
	if (!font)
		return -1;
	else
		return font->CharSpace;
}

int getFontCharacterSpacing (THWD *hw, int fontid)
{
	if (fontType(fontid))
		return getFontSpaceW(hw, fontid);
	else
		return getFontSpaceA(hw, fontid);
}

int setFontCharacterSpacing (THWD *hw, int fontid, int pixels)
{
	if (fontType(fontid))
		return setFontSpaceW(hw, fontid, pixels);
	else
		return setFontSpaceA(hw, fontid, pixels);
}

int setFontZeroWidthSpacing (THWD *hw, int fontid, int pixels)
{
	TWFONT *font = fontIDToFontW(hw, fontid);
	if (!font){
		return -1;
	}else{
		int old = font->spaceSpacing;
		font->spaceSpacing = pixels;
		return old;
	}
}

int getFontLineSpacing (THWD *hw, int fontid)
{
	int ret = 0;
	if (fontType(fontid)){
		TWFONT *wfont = fontIDToFontW(hw, fontid);
		if (wfont)
			ret = wfont->LineSpace;
	}else{
		TFONT *font = fontIDToFontA(hw, fontid);
		if (font)
			ret = font->lineSpace;
	}
	return ret;
}

int setFontLineSpacing (THWD *hw, int fontid, int pixels)
{
	int ret = 0;
	if (fontType(fontid)){
		TWFONT *font = fontIDToFontW(hw, fontid);
		if (font){
			ret = font->LineSpace;
			font->LineSpace = pixels;
		}
	}else{
		TFONT *font = fontIDToFontA(hw, fontid);
		if (font){
			ret = font->lineSpace;
			font->lineSpace = pixels;
		}
	}
	return ret;
}

static inline TWCHAR *getGlyphW (THWD *hw, const char *c, UTF32 ch, int fontid)
{
	if (c && !ch)
		decodeCharacterCode(hw, (ubyte*)c, &ch);

	if ((int)ch == (int)-1){
		TWFONT *font = fontIDToFontW(hw, fontid);
		
		ch = font->DefaultChar;
		UTF32 glist[2] = {font->DefaultChar, font->DefaultChar};
		/*int total =*/ cacheGlyphs(glist, 2, font);
		
		//printf("getGlyphW %i %i\n", font->DefaultChar, total);
	}
	

	//if (ch){
		TWFONT *font = buildBDFGlyph(hw, ch, fontid);
		if (font){
			if (/*ch &&*/ (ch < font->maxCharIdx)){
				if (font->chr){
					if (font->chr[ch])
						return font->chr[ch];
				}
			}
		}
	//}
	return NULL;
}

// fetch a single glyph from font
TWCHAR *getGlyph (THWD *hw, const char *c, UTF32 ch, int fontid)
{
	TWCHAR *glyph = NULL;
	
	if (fontType(fontid)){
		glyph = getGlyphW(hw, c, ch, fontid);
		//if (glyph){
			//if (glyph->f == NULL)
				//glyph = NULL;
		//}
	}
	return glyph;
}

// fetch pre-loaded glyphs from font
int getCachedGlyphs (THWD *hw, UTF32 *glist, int gtotal, int fontid)
{
	TWFONT *font = fontIDToFontW(hw, fontid);
	if (font == NULL) return -1;

	int total = 0;
	for (UTF32 ch = 0; ch < font->maxCharIdx; ch++){
		if (font->chr[ch]){
			if (total < gtotal){
				*glist++ = ch;
				total++;
			}else{
				total = -total;
			}
		}
	}
	return total;
}

// merge font id2 on to font id1
// id1 has glyph priority
int mergeFontW (THWD *hw, int fontid1, int fontid2)
{
	int ret = 0;
	TWFONT *font1 = fontIDToFontW(hw, fontid1);
	if (font1){
		TWFONT *font2 = fontIDToFontW(hw, fontid2);
		if (font2){
			if (buildBDFFont(font1))
				ret = BDFReadFile(font1, font2->file);
		}
	}
	return ret;
}

void *fontIDToFont (THWD *hw, int fontid)
{
	if (fontType(fontid))
		return (TWFONT*)fontIDToFontW(hw, fontid);
	else
		return (TFONT*)fontIDToFontA(hw, fontid);
}

int registerFontW (THWD *hw, const wchar_t *filename, TWFONT *f)
{
	if (fontIDToFontW(hw, f->FontID)){
		mylog("libmylcd: registerFontW(): ID in use\n");
		return 0;
	}
	if (!fontType(f->FontID)){
		mylog("libmylcd: registerFontW(): ID out of range\n");
		return 0;
	}
	TWFONT *font = (TWFONT *)l_calloc(1, sizeof(TWFONT));
	if (font == NULL){
		return 0;
	}

	font->built = 0;
	font->FontID = f->FontID;
	font->CharSpace = f->CharSpace;
	font->LineSpace = f->LineSpace;
	font->CharsBuilt = 0;
	font->chr = NULL;
	font->next = NULL;
	font->uscl.total = 0;
	font->uscl.uslist = NULL;
	font->coTotal = 1;
	font->hw = hw;
	
	font->filter.enabled = 0;
	font->filter.chainId = 0;

	font->chrOffset = (TCHAROFFSET*)l_calloc(font->coTotal+1, sizeof(TCHAROFFSET));
	if (font->chrOffset == NULL){
		l_free(font);
		return 0;
	}
	l_wcsncpy(font->file, filename, MIN(MaxPath*sizeof(wchar_t), l_wcslen(filename)));

	if (addFontToListW(hw, font)){
		return 1;
	}else{
		l_free(font);
		return 0;
	}
}

int flushFont (THWD *hw, int fontid)
{
	if (fontType(fontid) || fontid == -1)
		return flushFontW(hw, fontid);
	else
		return flushFontA(hw, fontid);
}

int unregisterFontW (THWD *hw, int fontid)
{
	if (flushFontW(hw, fontid)){
		TWFONT *font = fontIDToFontW(hw, fontid);
		if (font){
			font->built = 0;
			font->FontID = 0;
			font->flags = 0;
			font->coTotal = 0;
			l_free(font->chrOffset);
			font->chrOffset = NULL;
			return 1;
		}
	}
	return 0;
}

TENUMFONT *enumFontsBeginW (THWD *hw)
{
	TENUMFONT *enf = l_calloc(1, sizeof(TENUMFONT));
	if (enf){
		enf->fi = 0;
		enf->font = NULL;
		enf->wfont = rootFontW(hw);
		enf->wfont = enf->wfont->next;
		enf->id = enf->wfont->FontID;
		enf->total = getFontTotalW(hw);
		enf->hw = hw;
	}
	return enf;
}

int enumFontsNextW (TENUMFONT *enf)
{
	if (!enf) return 0;
	if (enf->wfont->next){
		enf->fi++;
		enf->font = NULL;
		enf->wfont = enf->wfont->next; 
		enf->id = enf->wfont->FontID;
		enf->total = getFontTotalW(enf->wfont->hw);
		enf->hw = enf->wfont->hw;
		const int ret = enf->id;
		return ret;
	}else{
		return 0;
	}
}

void enumFontsDeleteW (TENUMFONT *enf)
{
	l_free(enf);
}

wchar_t *getFontPath (THWD *hw)
{
	return hw->paths->font;
}

TWFONT *fontIDToFontW (THWD *hw, int fontid)
{
	if (!fontid) return NULL;
	TWFONT *f = rootFontW(hw);
	if (!f) return NULL;
	
	do{
		if (f->FontID == fontid)
			return f;
		else
			f = f->next;
	}while(f);
	return NULL;
}

int cacheGlyphs (UTF32 *glist, int gtotal, TWFONT *font)
{
	if (BDFOpen(font)){
		int total = BDFLoadGlyphs(glist, gtotal, font);
		
		// ensure default is always loaded
	 	// todo: check if font actually contains a default char
		if (!(font->flags&WFONT_DEFAULTCHARBUILT)){
			UTF32 defaultchar[2] = {font->DefaultChar, 0};
			BDFLoadGlyphs(defaultchar, 1, font);
		}

		BDFClose(font);
		updateUnsupportedCharList(glist, gtotal, font);
		font->built = 1;
		return total;
	}else{
		return 0;
	}
}

TWFONT *buildBDFGlyphs (THWD *hw, const char *str, int fontid)
{
	int ret = cacheCharacters(hw, str, fontid);
	if (ret != -1){
		TWFONT *font = fontIDToFontW(hw, fontid);
		if (font){
			font->built = 1;
			return font;
		}
	}
	return NULL;
}


TWFONT *buildBDFGlyph (THWD *hw, UTF32 ch, int fontid)
{
	if (cacheCharacterRange(hw, ch, ch, fontid) != -1){
		TWFONT *font = fontIDToFontW(hw, fontid);
		if (font){
			font->built = 1;
			return font;
		}
	}
	return NULL;
}


TWFONT *buildBDFFont (TWFONT *font)
{
	if (!font) return NULL;
	if (font->built > 0) return font;

	font->CharsBuilt = 0;
	font->built = (BDFReadFile(font, font->file) > 0);
	if (font->built > 0)
		return font;
	else
		return NULL;
}

static inline int registerFontsW (THWD *hw)
{
	regwidefont(hw, L"bdf/wenquanyi_9pt.bdf", LFTW_WENQUANYI9PT, 2);	// proportional

#if (__BUILD_BDF_FONT_SUPPORT__)

	regwidefont(hw, L"bdf/wenquanyi_12pt.bdf", LFTW_WENQUANYI12PT, 2);	// proportional
	regwidefont(hw, L"bdf/wenquanyi_36pt.bdf", LFTW_WENQUANYI36PT, 2);	// proportional
	regwidefont(hw, L"bdf/wqy-unibit.bdf", LFTW_WQYUNIBIT, 2);			// proportional
	regwidefont(hw, L"bdf/b34.bdf", LFTW_B34, 2);						// proportional
	regwidefont(hw, L"bdf/b28.bdf", LFTW_B28, 2);						// proportional
	regwidefont(hw, L"bdf/b26.bdf", LFTW_B26, 2);
	regwidefont(hw, L"bdf/b24.bdf", LFTW_B24, 2);						// B10-24 are fixed width
	regwidefont(hw, L"bdf/b16_b.bdf", LFTW_B16B, 2);
	regwidefont(hw, L"bdf/b16.bdf", LFTW_B16, 1);
	regwidefont(hw, L"bdf/b14.bdf", LFTW_B14, 1);
	regwidefont(hw, L"bdf/b12.bdf", LFTW_B12, 1);
	regwidefont(hw, L"bdf/b10.bdf", LFTW_B10, 1);
	regwidefont(hw, L"bdf/monau16.bdf", LFTW_MONAU16, 1);
	regwidefont(hw, L"bdf/monau14.bdf", LFTW_MONAU14, 1);
	regwidefont(hw, L"bdf/mona8x16a.bdf", LFTW_MONA8X16A, 1);
	regwidefont(hw, L"bdf/mona7x14a.bdf", LFTW_MONA7X14A, 1);
	regwidefont(hw, L"bdf/18x18ko.bdf", LFTW_18x18KO, 1);
	regwidefont(hw, L"bdf/nimbus14.bdf", LFTW_NIMBUS14, 1);
	regwidefont(hw, L"bdf/unicode.bdf", LFTW_UNICODE, 1);
	regwidefont(hw, L"bdf/unicode36.bdf", LFTW_UNICODE36, 1);
	regwidefont(hw, L"bdf/unicode72.bdf", LFTW_UNICODE72, 2);
	regwidefont(hw, L"bdf/snap.bdf", LFTW_SNAP, 1);					// proportional
	regwidefont(hw, L"bdf/rought18.bdf", LFTW_ROUGHT18, 2);			// proportional
	regwidefont(hw, L"bdf/cu12.bdf", LFTW_CU12, 1);
	regwidefont(hw, L"bdf/cu-alt12.bdf", LFTW_CUALT12, 1);
	regwidefont(hw, L"bdf/4x6.bdf", LFTW_4x6, 1);
	regwidefont(hw, L"bdf/5x7.bdf", LFTW_5x7, 1);
	regwidefont(hw, L"bdf/5x8.bdf", LFTW_5x8, 1);
	regwidefont(hw, L"bdf/6x8-cp437.bdf", LFTW_6x8_CP437, 1);
	regwidefont(hw, L"bdf/6x9.bdf", LFTW_6x9, 1);
	regwidefont(hw, L"bdf/6x10.bdf", LFTW_6x10, 1);
	regwidefont(hw, L"bdf/6x12.bdf", LFTW_6x12, 1);
	regwidefont(hw, L"bdf/clr6x12.bdf", LFTW_CLR6X12, 1);
	regwidefont(hw, L"bdf/6x13.bdf", LFTW_6x13, 1);
	regwidefont(hw, L"bdf/6x13b.bdf", LFTW_6x13B, 2);
	regwidefont(hw, L"bdf/6x13O.bdf", LFTW_6x13O, 1);
	regwidefont(hw, L"bdf/7x13.bdf", LFTW_7x13, 1);
	regwidefont(hw, L"bdf/7x13b.bdf", LFTW_7x13B, 1);
	regwidefont(hw, L"bdf/7x13O.bdf", LFTW_7x13O, 1);
	regwidefont(hw, L"bdf/7x14.bdf", LFTW_7x14, 1);
	regwidefont(hw, L"bdf/7x14b.bdf", LFTW_7x14B, 1);
	regwidefont(hw, L"bdf/8x13.bdf", LFTW_8x13, 1);
	regwidefont(hw, L"bdf/9x18.bdf", LFTW_9x18, 1);
	regwidefont(hw, L"bdf/9x18b.bdf", LFTW_9x18B, 1);
	regwidefont(hw, L"bdf/10x20.bdf", LFTW_10x20, 1);
	regwidefont(hw, L"bdf/13x26.bdf", LFTW_13x26, 1);
	regwidefont(hw, L"bdf/helvb18.bdf", LFTW_HELVB18, 2);
	regwidefont(hw, L"bdf/comicsans20.bdf", LFTW_COMICSANS20, 2);
	regwidefont(hw, L"bdf/helvr08.bdf", LFTW_HELVR08, 2);
	regwidefont(hw, L"bdf/helvr10.bdf", LFTW_HELVR10, 2);
	regwidefont(hw, L"bdf/helvr12.bdf", LFTW_HELVR12, 1);
	regwidefont(hw, L"bdf/helvr14.bdf", LFTW_HELVR14, 2);
	regwidefont(hw, L"bdf/helvr18.bdf", LFTW_HELVR18, 2);
	regwidefont(hw, L"bdf/helvr24.bdf", LFTW_HELVR24, 2);
	regwidefont(hw, L"bdf/courr08.bdf", LFTW_COURR08, 2);
	regwidefont(hw, L"bdf/courr10.bdf", LFTW_COURR10, 1);
	regwidefont(hw, L"bdf/courr12.bdf", LFTW_COURR12, 1);
	regwidefont(hw, L"bdf/courr14.bdf", LFTW_COURR14, 1);
	regwidefont(hw, L"bdf/courr18.bdf", LFTW_COURR18, 1);
	regwidefont(hw, L"bdf/courr24.bdf", LFTW_COURR24, 1);
	regwidefont(hw, L"bdf/herir08.bdf", LFTW_HERIR08, 1);
	regwidefont(hw, L"bdf/herir10.bdf", LFTW_HERIR10, 2);
	regwidefont(hw, L"bdf/herir12.bdf", LFTW_HERIR12, 1);
	regwidefont(hw, L"bdf/herir14.bdf", LFTW_HERIR14, 1);
	regwidefont(hw, L"bdf/herir18.bdf", LFTW_HERIR18, 2);
	regwidefont(hw, L"bdf/herir24.bdf", LFTW_HERIR24, 2);
	regwidefont(hw, L"bdf/lutrs08.bdf", LFTW_LUTRS08, 2);
	regwidefont(hw, L"bdf/lutrs10.bdf", LFTW_LUTRS10, 1);
	regwidefont(hw, L"bdf/lutrs12.bdf", LFTW_LUTRS12, 1);
	regwidefont(hw, L"bdf/lutrs14.bdf", LFTW_LUTRS14, 1);
	regwidefont(hw, L"bdf/lutrs18.bdf", LFTW_LUTRS18, 2);
	regwidefont(hw, L"bdf/lutrs19.bdf", LFTW_LUTRS19, 2);
	regwidefont(hw, L"bdf/lutrs24.bdf", LFTW_LUTRS24, 1);
	regwidefont(hw, L"bdf/etl14.bdf", LFTW_ETL14, 1);
	regwidefont(hw, L"bdf/etl16.bdf", LFTW_ETL16, 1);
	regwidefont(hw, L"bdf/etl24.bdf", LFTW_ETL24, 1);
	regwidefont(hw, L"bdf/xsymb1_12.bdf", LFTW_XSYMB1_12, 2);
	regwidefont(hw, L"bdf/xsymb0_12.bdf", LFTW_XSYMB0_12, 2);
	regwidefont(hw, L"bdf/ncenr10.bdf", LFTW_NCENR10, 2);
	regwidefont(hw, L"bdf/timr10.bdf", LFTW_TIMR10, 2);
	regwidefont(hw, L"bdf/utrg_10.bdf", LFTW_UTRG__10, 2);
	regwidefont(hw, L"bdf/koi5x8.bdf", LFTW_KOI5X8, 1);
	regwidefont(hw, L"bdf/koi7x14.bdf", LFTW_KOI7X14, 1);
	regwidefont(hw, L"bdf/koi9x18.bdf", LFTW_KOI9X18, 1);	
	regwidefont(hw, L"bdf/koi10x20.bdf", LFTW_KOI10X20, 1);		
	regwidefont(hw, L"bdf/koi12x24b.bdf", LFTW_KOI12X24B, 2);
	regwidefont(hw, L"bdf/proof9x16.bdf", LFTW_PROOF9X16, 2);
	regwidefont(hw, L"bdf/screen8x16.bdf", LFTW_SCREEN8X16, 1);
	regwidefont(hw, L"bdf/trajan132.bdf", LFTW_TRAJAN132, 10);
	regwidefont(hw, L"bdf/levenimmt132.bdf", LFTW_LEVENIMMT132, 10);
	regwidefont(hw, L"bdf/actionman132.bdf", LFTW_ACTIONMAN132, 10);
	regwidefont(hw, L"bdf/racer96.bdf", LFTW_RACER96, 6);
	regwidefont(hw, L"bdf/racer102.bdf", LFTW_RACER102, 7);
	regwidefont(hw, L"bdf/racer132.bdf", LFTW_RACER132, 10);
	regwidefont(hw, L"bdf/198five132.bdf", LFTW_198FIVE132, 18);
	regwidefont(hw, L"bdf/198five162.bdf", LFTW_198FIVE162, 22);
	regwidefont(hw, L"bdf/76London34.bdf", LFTW_76LONDON34, 3);
	regwidefont(hw, L"bdf/76London38.bdf", LFTW_76LONDON38, 3);
	regwidefont(hw, L"bdf/76London150.bdf", LFTW_76LONDON150, 8);
	regwidefont(hw, L"bdf/notoSerif36.bdf", LFTW_NOTOSANS36, 1);
	regwidefont(hw, L"bdf/namco91.bdf", LFTW_NAMCO91, 5);

#endif
	return 1;
}


#if (__BUILD_BITMAP_FONT_SUPPORT__ && __BUILD_TGA_SUPPORT__)

static inline void regbitmapfont (THWD *hw, wchar_t *file, int fontid, int charSpace)
{ 
	wchar_t path[MaxPath];
	TFONT f;
	
	f.autoheight = 1;
	f.autowidth = 1;
	f.choffset = 0;
	f.charsPerRow = 16;
	f.rowsPerImage = 16;
	f.xoffset = 0;
	f.yoffset = 0;
	f.invert = 0;
	f.charW = 0;
	f.charH = 0;
	f.id = fontid;
	f.imageType = IMG_TGA;
	f.charSpace = charSpace;
	f.lineSpace = lLineSpaceHeight;
	registerFontA(hw, formatFontPath(hw, file, path), &f);
}

static int registerFontsA (THWD *hw)
{

	wchar_t path[MaxPath];

	TFONT f;
	f.lineSpace = lLineSpaceHeight;
	f.imageType = IMG_TGA;
	f.invert = 0;
	f.autoheight = 0;
	f.autowidth = 1;
	f.rowsPerImage = 16;

	// font 0
	f.charsPerRow = 16;
	f.charW =16;
	f.charH =16;
	f.xoffset =0;
	f.yoffset =2;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_SMALLFONTS7X7;
	registerFontA(hw, formatFontPath(hw, L"tga/smallfonts7x7.tga", path), &f);

	// font 1
	f.charsPerRow = 224;
	f.rowsPerImage = 224;
	f.autowidth = 0;
	f.charW =6;
	f.charH =13;
	f.xoffset =0;
	f.yoffset =2;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_LONG6X13;
	registerFontA(hw, formatFontPath(hw, L"tga/long6x13.tga", path), &f);

	// font 2
	f.charsPerRow = 16;
	f.rowsPerImage = 16;
	f.autowidth = 1;
	f.charW =16;
	f.charH =16;
	f.xoffset =0;
	f.yoffset =3;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_COMICSANSMS7X8;
	registerFontA(hw, formatFontPath(hw, L"tga/comicsansms7x8.tga", path), &f);

	// font 3
	f.charW =16;
	f.charH =16;
	f.xoffset =0;
	f.yoffset =2;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_COURIERNEWCE8;
	registerFontA(hw, formatFontPath(hw, L"tga/couriernewce8.tga", path), &f);

	// font 4
	f.charW =16;
	f.charH =16;
	f.xoffset =0;
	f.yoffset =0;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_F1;
	registerFontA(hw, formatFontPath(hw, L"tga/f1.tga", path), &f);			
	
	// font 5
	f.charW =16;
	f.charH =16;
	f.xoffset =0;
	f.yoffset =3;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_COMICSANSMS8X9;
	registerFontA(hw, formatFontPath(hw, L"tga/comicsansms8x9.tga", path), &f);
	
	// font 6
	f.charW =16;
	f.charH =16;
	f.xoffset =0;
	f.yoffset =1;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_GUNGSUHCHE8X8;
	registerFontA(hw, formatFontPath(hw, L"tga/gungsuhche8x8.tga", path), &f);
		
	// font 7
	f.charW =16;
	f.charH =16;
	f.xoffset =4;
	f.yoffset =0;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_F3;
	registerFontA(hw, formatFontPath(hw, L"tga/f3.tga", path), &f);

	// font 8
	f.charW =16;
	f.charH =16;
	f.xoffset =0;
	f.yoffset =2;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_GEORGIA9;
	registerFontA(hw, formatFontPath(hw, L"tga/georgia9.tga", path), &f);	

	// font 9
	f.charW =16;
	f.charH =16;
	f.xoffset =0;
	f.yoffset =3;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_SIMPSONS8;
	registerFontA(hw, formatFontPath(hw, L"tga/simpsons8.tga", path), &f);

	// font 10
	f.charW =24;
	f.charH =24;
	f.xoffset =0;
	f.yoffset =1;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_XFILES12;
	registerFontA(hw, formatFontPath(hw, L"tga/xfiles12.tga", path), &f);	
	
	// font 11
	f.charW =24;
	f.charH =24;
	f.xoffset =0;
	f.yoffset =7;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_GOTHAMNIGHTS;
	registerFontA(hw, formatFontPath(hw, L"tga/gothamnights.tga", path), &f);
			
	// font 12
	f.charW =16;
	f.charH =16;
	f.xoffset =0;
	f.yoffset =0;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_BLACKADDERII;
	registerFontA(hw, formatFontPath(hw, L"tga/blackadderii.tga", path), &f);

	// font 13
	f.charW =16;
	f.charH =16;
	f.xoffset =0;
	f.yoffset =3;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_ARIAL;
	registerFontA(hw, formatFontPath(hw, L"tga/arial.tga", path), &f);
	
	// font 14
	f.charW =16;
	f.charH =16;
	f.xoffset =0;
	f.yoffset =2;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_COURIER10;
	registerFontA(hw, formatFontPath(hw, L"tga/courier10.tga", path), &f);
			
	// font 15
	f.charW =16;
	f.charH =16;
	f.xoffset =0;
	f.yoffset =0;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_F0;
	registerFontA(hw, formatFontPath(hw, L"tga/f0.tga", path), &f);
	
	// font 16
	f.charW =16;
	f.charH =16;
	f.xoffset =0;
	f.yoffset =3;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_COMICSANSMS15X13;
	registerFontA(hw, formatFontPath(hw, L"tga/comicsansms15x13.tga", path), &f);
	
	// font 17
	f.charW =16;
	f.charH =16;
	f.xoffset =0;
	f.yoffset =2;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_NTR9;
	registerFontA(hw, formatFontPath(hw, L"tga/ntr9.tga", path), &f);
	
	// font 18
	f.charW =16;
	f.charH =16;
	f.xoffset =4;
	f.yoffset =2;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_FONT0;
	registerFontA(hw, formatFontPath(hw, L"tga/font0.tga", path), &f);
	
	// font 19
	f.charW =16;
	f.charH =16;
	f.xoffset =0;
	f.yoffset =3;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_ARIALBOLD;
	registerFontA(hw, formatFontPath(hw, L"tga/arialbold.tga", path), &f);
	
	// font 20
	f.charW =16;
	f.charH =16;
	f.xoffset =0;
	f.yoffset =3;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_COURIER14;
	registerFontA(hw, formatFontPath(hw, L"tga/courier14.tga", path), &f);
		
	// font 21
	f.charW =16;
	f.charH =16;
	f.xoffset =0;
	f.yoffset =4;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_ARIALBLACK9X10;
	registerFontA(hw, formatFontPath(hw, L"tga/arialblack9x10.tga", path), &f);
		
	// font 22
	f.charW =16;
	f.charH =16;
	f.xoffset =0;
	f.yoffset =3;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_ARIALITALIC;
	registerFontA(hw, formatFontPath(hw, L"tga/arialitalic.tga", path), &f);
	
	// font 23
	f.charW =16;
	f.charH =16;
	f.xoffset =0;
	f.yoffset =1;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_BASICFONT;
	registerFontA(hw, formatFontPath(hw, L"tga/basicfont.tga", path), &f);
	
	// font 24
	f.charW =16;
	f.charH =16;
	f.xoffset =1;
	f.yoffset = 0;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_F2;
	registerFontA(hw, formatFontPath(hw, L"tga/f2.tga", path), &f);	

	// font 25
	f.charW =16;
	f.charH =32;
	f.xoffset =2;
	f.yoffset =6;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_FONT1;
	registerFontA(hw, formatFontPath(hw, L"tga/font1.tga", path), &f);
	
	// font 26
	f.charW =16;
	f.charH =16;
	f.xoffset =0;
	f.yoffset =0;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_QUAKEA;
	registerFontA(hw, formatFontPath(hw, L"tga/quakea.tga", path), &f);

	// font 27
	f.charW =16;
	f.charH =16;
	f.xoffset =0;
	f.yoffset =2;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_QUAKEB;
	registerFontA(hw, formatFontPath(hw, L"tga/quakeb.tga", path), &f);
	
	// font 28
	f.charW =24;
	f.charH =24;
	f.xoffset =0;
	f.yoffset =1;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_DOTUMCHE24X24;
	registerFontA(hw, formatFontPath(hw, L"tga/dotumche24x24.tga", path), &f);
		
	// font 29
	f.charW =24;
	f.charH =24;
	f.xoffset =0;
	f.yoffset =5;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_SIMPSONS14;
	registerFontA(hw, formatFontPath(hw, L"tga/simpsons14.tga", path), &f);

	// font 30
	f.charW =24;
	f.charH =24;
	f.xoffset =0;
	f.yoffset =1;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_LANSBURY;
	registerFontA(hw, formatFontPath(hw, L"tga/lansbury.tga", path), &f);

	// font 31
	f.charW =24;
	f.charH =24;
	f.xoffset =0;
	f.yoffset =5;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_SFGRANDEZZA;
	registerFontA(hw, formatFontPath(hw, L"tga/sfgrandezza.tga", path), &f);

	// font 32
	f.charW =24;
	f.charH =24;
	f.xoffset =0;
	f.yoffset =0;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_JAPANESE_KANJI;
	registerFontA(hw, formatFontPath(hw, L"tga/japanese_kanji.tga", path), &f);

	// font 32
	f.charW =24;
	f.charH =24;
	f.xoffset =0;
	f.yoffset =1;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_DRAGONFLY;
	registerFontA(hw, formatFontPath(hw, L"tga/dragonfly.tga", path), &f);

	// font 34
	f.charW =24;
	f.charH =24;
	f.xoffset =0;
	f.yoffset =2;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_RODDY;
	registerFontA(hw, formatFontPath(hw, L"tga/roddy.tga", path), &f);

	// font 35
	f.charW =24;
	f.charH =24;
	f.xoffset =0;
	f.yoffset =5;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_MISSCLUDE;
	registerFontA(hw, formatFontPath(hw, L"tga/missclude.tga", path), &f);

	// font 36
	f.charW =32;
	f.charH =32;
	f.xoffset =0;
	f.yoffset =0;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_VIT;
	registerFontA(hw, formatFontPath(hw, L"tga/vit.tga", path), &f);

	// font 37
	f.charW =32;
	f.charH =32;
	f.xoffset =0;
	f.yoffset =0;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_VITBOLD;
	registerFontA(hw, formatFontPath(hw, L"tga/vitbold.tga", path), &f);

	// font 38
	f.charW =28;
	f.charH =28;
	f.xoffset =0;
	f.yoffset =4;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_BLOCKUP;
	registerFontA(hw, formatFontPath(hw, L"tga/blockup.tga", path), &f);

	// font 39
	f.charW =24;
	f.charH =24;
	f.xoffset =0;
	f.yoffset =0;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_LIQUIS;
	registerFontA(hw, formatFontPath(hw, L"tga/liquis.tga", path), &f);

	// font 40
	f.charW =24;
	f.charH =24;
	f.xoffset =0;
	f.yoffset =4;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_CHINATOWN;
	registerFontA(hw, formatFontPath(hw, L"tga/chinatown.tga", path), &f);

	// font 41
	f.charW =32;
	f.charH =32;
	f.xoffset =0;
	f.yoffset =3;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_CHOWFUN;
	registerFontA(hw, formatFontPath(hw, L"tga/chowfun.tga", path), &f);	

	// font 42
	f.charW =24;
	f.charH =24;
	f.xoffset =0;
	f.yoffset =4;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_WONTON;
	registerFontA(hw, formatFontPath(hw, L"tga/wonton.tga", path), &f);	
	
	// font 43
	f.charW =32;
	f.charH =32;
	f.xoffset =0;
	f.yoffset =6;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_SANTAMONL;
	registerFontA(hw, formatFontPath(hw, L"tga/santamonl.tga", path), &f);	

	// font 44
	f.charW =32;
	f.charH =32;
	f.xoffset =0;
	f.yoffset =7;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_SANTAMON;
	registerFontA(hw, formatFontPath(hw, L"tga/santamon.tga", path), &f);

	// font 45
	f.charW =28;
	f.charH =28;
	f.xoffset =0;
	f.yoffset =5;
	f.choffset =32;
	f.charSpace =0;
	f.id = LFT_MARATHILEKHANI;
	registerFontA(hw, formatFontPath(hw, L"tga/marathi-lekhani.tga", path), &f);

	// font 46
	f.charW =24;
	f.charH =24;
	f.xoffset =0;
	f.yoffset =2;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_KORV;
	registerFontA(hw, formatFontPath(hw, L"tga/korv.tga", path), &f);	
	
	// font 47
	f.charW =32;
	f.charH =32;
	f.xoffset =0;
	f.yoffset =3;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_XFILES20;
	registerFontA(hw, formatFontPath(hw, L"tga/xfiles20.tga", path), &f);	

	// font 48
	f.charW =38;
	f.charH =40;
	f.xoffset =0;
	f.yoffset =8;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_CHARMING;
	registerFontA(hw, formatFontPath(hw, L"tga/charming.tga", path), &f);

	// font 49
	f.charW =40;
	f.charH =40;
	f.xoffset =0;
	f.yoffset =7;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_GEORGIA24;
	registerFontA(hw, formatFontPath(hw, L"tga/georgia24.tga", path), &f);	

	// font 50
	f.charW =40;
	f.charH =40;
	f.xoffset =0;
	f.yoffset =6;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_GEORGIA24I;
	registerFontA(hw, formatFontPath(hw, L"tga/georgia24i.tga", path), &f);

	// font 51
	f.charW =24;
	f.charH =24;
	f.xoffset =0;
	f.yoffset =1;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_LUCKY;
	registerFontA(hw, formatFontPath(hw, L"tga/lucky.tga", path), &f);
		
	// font 52
	f.charW =32;
	f.charH =32;
	f.xoffset =0;
	f.yoffset =4;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_TRUMANIA;
	registerFontA(hw, formatFontPath(hw, L"tga/trumania.tga", path), &f);
	
	// font 53
	f.charW =40;
	f.charH =40;
	f.xoffset =0;
	f.yoffset =6;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_ANGLICANTEXT;
	registerFontA(hw, formatFontPath(hw, L"tga/anglicantext.tga", path), &f);
		
	// font 54
	f.charW =48;
	f.charH =48;
	f.xoffset =0;
	f.yoffset =9;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_COMICSANSMS48X48;
	registerFontA(hw, formatFontPath(hw, L"tga/comicsansms48x48.tga", path), &f);
	
	// font 55
	f.charW =40;
	f.charH =50;
	f.xoffset =0;
	f.yoffset =1;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_GUNGSUHCHE24X36;
	registerFontA(hw, formatFontPath(hw, L"tga/gungsuhche24x36.tga", path), &f);

	// font 56
	f.charW =44;
	f.charH =44;
	f.xoffset =0;
	f.yoffset =1;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_ARAKAWA;
	registerFontA(hw, formatFontPath(hw, L"tga/arakawa.tga", path), &f);	
	
	// font 57
	f.charW =32;
	f.charH =44;
	f.xoffset =0;
	f.yoffset =0;
	f.choffset =32;
	f.charSpace = 2;
	f.id = LFT_TETRICIDE;
	registerFontA(hw, formatFontPath(hw, L"tga/tetricide.tga", path), &f);
		
	// font 58
	f.charW =56;
	f.charH =56;
	f.xoffset =0;
	f.yoffset =9;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_COURIERNEWCE56;
	registerFontA(hw, formatFontPath(hw, L"tga/couriernewce56.tga", path), &f);

	// font 59
	f.charW =32;
	f.charH =32;
	f.xoffset =0;
	f.yoffset =4;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_INTERDIMENSIONAL12;
	registerFontA(hw, formatFontPath(hw, L"tga/interdimensional12.tga", path), &f);

	// font 60
	f.charW =32;
	f.charH =32;
	f.xoffset =0;
	f.yoffset =5;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_INTERDIMENSIONAL16;
	registerFontA(hw, formatFontPath(hw, L"tga/interdimensional16.tga", path), &f);

	// font 61
	f.charW =40;
	f.charH =40;
	f.xoffset =0;
	f.yoffset =6;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_INTERDIMENSIONAL18;
	registerFontA(hw, formatFontPath(hw, L"tga/interdimensional18.tga", path), &f);
	
	// font 62
	f.charW =60;
	f.charH =50;
	f.xoffset =0;
	f.yoffset =11;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_INTERDIMENSIONAL;
	registerFontA(hw, formatFontPath(hw, L"tga/interdimensional.tga", path), &f);

	// font 63
	f.charW =24;
	f.charH =24;
	f.xoffset =0;
	f.yoffset =4;
	f.choffset =32;
	f.charSpace =1;
	f.id = LFT_PORSCHE911;
	registerFontA(hw, formatFontPath(hw, L"tga/911porsche.tga", path), &f);

	// font 64
	f.charW =20;
	f.charH =20;
	f.xoffset =0;
	f.yoffset =0;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_LDW10;
	registerFontA(hw, formatFontPath(hw, L"tga/ldw10.tga", path), &f);
	
	// font 65
	f.charW =24;
	f.charH =24;
	f.xoffset =0;
	f.yoffset =0;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_LDW16;
	registerFontA(hw, formatFontPath(hw, L"tga/ldw16.tga", path), &f);	
	
	// font 66
	f.charW =32;
	f.charH =32;
	f.xoffset =0;
	f.yoffset =0;
	f.choffset =0;
	f.charSpace =1;
	f.id = LFT_LDW20;
	registerFontA(hw, formatFontPath(hw, L"tga/ldw20.tga", path), &f);

	regbitmapfont(hw, L"tga/icons_16.tga", LFT_ICONS_16, 1);
	regbitmapfont(hw, L"tga/icons_22.tga", LFT_ICONS_22, 1);
	regbitmapfont(hw, L"tga/icons_32.tga", LFT_ICONS_32, 1);
	regbitmapfont(hw, L"tga/icons2_16.tga", LFT_ICONS2_16, 1);
	regbitmapfont(hw, L"tga/icons2_22.tga", LFT_ICONS2_22, 1);
	regbitmapfont(hw, L"tga/icons2_32.tga", LFT_ICONS2_32, 1);
	regbitmapfont(hw, L"tga/asiandings_20.tga", LFT_ASIANDINGS_20, 1);
	regbitmapfont(hw, L"tga/asiandings_24.tga", LFT_ASIANDINGS_24, 1);
	regbitmapfont(hw, L"tga/asiandings_32.tga", LFT_ASIANDINGS_32, 1);	
	regbitmapfont(hw, L"tga/smallfonts_5.tga", LFT_SMALLFONTS_5, 1);
	regbitmapfont(hw, L"tga/smallfonts_6.tga", LFT_SMALLFONTS_6, 1);
	regbitmapfont(hw, L"tga/smallfonts_7.tga", LFT_SMALLFONTS_7, 1);
	regbitmapfont(hw, L"tga/fixedsys_8.tga", LFT_FIXEDSYS_8, 1);
	regbitmapfont(hw, L"tga/system_8.tga", LFT_SYSTEM_8, 1);
	regbitmapfont(hw, L"tga/system_16.tga", LFT_SYSTEM_16, 1);
	regbitmapfont(hw, L"tga/04b2_10.tga", LFT_04B2_10, 1);
	regbitmapfont(hw, L"tga/04b2_16.tga", LFT_04B2_16, 2);
	regbitmapfont(hw, L"tga/04b2_20.tga", LFT_04B2_20, 2);
	regbitmapfont(hw, L"tga/script_24.tga", LFT_SCRIPT_24, 0);
	regbitmapfont(hw, L"tga/script_24b.tga", LFT_SCRIPT_24B, 0);
	regbitmapfont(hw, L"tga/script_32.tga", LFT_SCRIPT_32, 0);
	regbitmapfont(hw, L"tga/roman_32.tga", LFT_ROMAN_32, 1);
	regbitmapfont(hw, L"tga/akbar_16.tga", LFT_AKBAR_16, 1);
	regbitmapfont(hw, L"tga/akbar_18.tga", LFT_AKBAR_18, 1);
	regbitmapfont(hw, L"tga/akbar_20.tga", LFT_AKBAR_20, 1);
	regbitmapfont(hw, L"tga/akbar_24.tga", LFT_AKBAR_24, 1);
	regbitmapfont(hw, L"tga/akbar_32.tga", LFT_AKBAR_32, 1);
	regbitmapfont(hw, L"tga/hanshand_16.tga", LFT_HANSHAND_16, 0);
	regbitmapfont(hw, L"tga/hanshand_18.tga", LFT_HANSHAND_18, 0);
	regbitmapfont(hw, L"tga/hanshand_20.tga", LFT_HANSHAND_20, 0);
	regbitmapfont(hw, L"tga/hanshand_24.tga", LFT_HANSHAND_24, 0);
	regbitmapfont(hw, L"tga/hanshand_32.tga", LFT_HANSHAND_32, 0);
	regbitmapfont(hw, L"tga/modem_16.tga", LFT_MODEM_16, 1);
	regbitmapfont(hw, L"tga/modem_18.tga", LFT_MODEM_18, 1);
	regbitmapfont(hw, L"tga/modem_20.tga", LFT_MODEM_20, 1);
	regbitmapfont(hw, L"tga/modem_24.tga", LFT_MODEM_24, 1);
	regbitmapfont(hw, L"tga/modem_32.tga", LFT_MODEM_32, 1);
	regbitmapfont(hw, L"tga/modem_32b.tga", LFT_MODEM_32B, 1);
	regbitmapfont(hw, L"tga/arial_10.tga", LFT_ARIAL_10, 1);
	regbitmapfont(hw, L"tga/arial_10b.tga", LFT_ARIAL_10B, 1);
	regbitmapfont(hw, L"tga/arial_12.tga", LFT_ARIAL_12, 1);
	regbitmapfont(hw, L"tga/arial_12b.tga", LFT_ARIAL_12B, 1);
	regbitmapfont(hw, L"tga/arial_16.tga", LFT_ARIAL_16, 1);
	regbitmapfont(hw, L"tga/arial_18.tga", LFT_ARIAL_18, 1);
	regbitmapfont(hw, L"tga/arial_20.tga", LFT_ARIAL_20, 1);
	regbitmapfont(hw, L"tga/arial_24.tga", LFT_ARIAL_24, 1);
	regbitmapfont(hw, L"tga/arial_32.tga", LFT_ARIAL_32, 1);
	regbitmapfont(hw, L"tga/batang_16.tga", LFT_BATANG_16, 1);
	regbitmapfont(hw, L"tga/batang_18.tga", LFT_BATANG_18, 1);
	regbitmapfont(hw, L"tga/batang_20.tga", LFT_BATANG_20, 1);
	regbitmapfont(hw, L"tga/batang_24.tga", LFT_BATANG_24, 1);
	regbitmapfont(hw, L"tga/batang_32.tga", LFT_BATANG_32, 1);
	regbitmapfont(hw, L"tga/batangche_16.tga", LFT_BATANGCHE_16, 1);
	regbitmapfont(hw, L"tga/batangche_18.tga", LFT_BATANGCHE_18, 1);
	regbitmapfont(hw, L"tga/batangche_20.tga", LFT_BATANGCHE_20, 1);
	regbitmapfont(hw, L"tga/batangche_24.tga", LFT_BATANGCHE_24, 1);
	regbitmapfont(hw, L"tga/batangche_32.tga", LFT_BATANGCHE_32, 1);
	regbitmapfont(hw, L"tga/comicsans_16.tga", LFT_COMICSANS_16, 1);
	regbitmapfont(hw, L"tga/comicsans_18.tga", LFT_COMICSANS_18, 1);
	regbitmapfont(hw, L"tga/comicsans_20.tga", LFT_COMICSANS_20, 1);
	regbitmapfont(hw, L"tga/comicsans_24.tga", LFT_COMICSANS_24, 1);
	regbitmapfont(hw, L"tga/comicsans_32.tga", LFT_COMICSANS_32, 1);
	regbitmapfont(hw, L"tga/couriernew_16.tga", LFT_COURIERNEW_16, 1);
	regbitmapfont(hw, L"tga/couriernew_16b.tga", LFT_COURIERNEW_16B, 1);
	regbitmapfont(hw, L"tga/couriernew_18.tga", LFT_COURIERNEW_18, 1);
	regbitmapfont(hw, L"tga/couriernew_18b.tga", LFT_COURIERNEW_18B, 1);
	regbitmapfont(hw, L"tga/couriernew_20.tga", LFT_COURIERNEW_20, 1);
	regbitmapfont(hw, L"tga/couriernew_20b.tga", LFT_COURIERNEW_20B, 1);
	regbitmapfont(hw, L"tga/couriernew_24.tga", LFT_COURIERNEW_24, 1);
	regbitmapfont(hw, L"tga/couriernew_24b.tga", LFT_COURIERNEW_24B, 1);
	regbitmapfont(hw, L"tga/couriernew_32.tga", LFT_COURIERNEW_32, 1);
	regbitmapfont(hw, L"tga/couriernew_32b.tga", LFT_COURIERNEW_32B, 1);
	regbitmapfont(hw, L"tga/dotum_16.tga", LFT_DOTUM_16, 1);
	regbitmapfont(hw, L"tga/dotum_18.tga", LFT_DOTUM_18, 1);
	regbitmapfont(hw, L"tga/dotum_20.tga", LFT_DOTUM_20, 1);
	regbitmapfont(hw, L"tga/dotum_24.tga", LFT_DOTUM_24, 1);
	regbitmapfont(hw, L"tga/dotum_32.tga", LFT_DOTUM_32, 1);
	regbitmapfont(hw, L"tga/dotumche_16.tga", LFT_DOTUMCHE_16, 1);
	regbitmapfont(hw, L"tga/dotumche_18.tga", LFT_DOTUMCHE_18, 1);
	regbitmapfont(hw, L"tga/dotumche_20.tga", LFT_DOTUMCHE_20, 1);
	regbitmapfont(hw, L"tga/dotumche_24.tga", LFT_DOTUMCHE_24, 1);
	regbitmapfont(hw, L"tga/dotumche_32.tga", LFT_DOTUMCHE_32, 2);
	regbitmapfont(hw, L"tga/georgia_16.tga", LFT_GEORGIA_16, 1);
	regbitmapfont(hw, L"tga/georgia_18.tga", LFT_GEORGIA_18, 1);
	regbitmapfont(hw, L"tga/georgia_20.tga", LFT_GEORGIA_20, 1);
	regbitmapfont(hw, L"tga/georgia_24.tga", LFT_GEORGIA_24, 1);
	regbitmapfont(hw, L"tga/georgia_32.tga", LFT_GEORGIA_32, 1);
	regbitmapfont(hw, L"tga/gulim_16.tga", LFT_GULIM_16, 1);
	regbitmapfont(hw, L"tga/gulim_18.tga", LFT_GULIM_18, 1);
	regbitmapfont(hw, L"tga/gulim_20.tga", LFT_GULIM_20, 1);
	regbitmapfont(hw, L"tga/gulim_24.tga", LFT_GULIM_24, 1);
	regbitmapfont(hw, L"tga/gulim_32.tga", LFT_GULIM_32, 1);
	regbitmapfont(hw, L"tga/gungsuh_16.tga", LFT_GUNGSUH_16, 1);
	regbitmapfont(hw, L"tga/gungsuh_18.tga", LFT_GUNGSUH_18, 1);
	regbitmapfont(hw, L"tga/gungsuh_20.tga", LFT_GUNGSUH_20, 1);
	regbitmapfont(hw, L"tga/gungsuh_24.tga", LFT_GUNGSUH_24, 1);
	regbitmapfont(hw, L"tga/gungsuh_32.tga", LFT_GUNGSUH_32, 1);
	regbitmapfont(hw, L"tga/gungsuhche_16.tga", LFT_GUNGSUHCHE_16, 1);
	regbitmapfont(hw, L"tga/gungsuhche_18.tga", LFT_GUNGSUHCHE_18, 1);
	regbitmapfont(hw, L"tga/gungsuhche_20.tga", LFT_GUNGSUHCHE_20, 1);
	regbitmapfont(hw, L"tga/gungsuhche_24.tga", LFT_GUNGSUHCHE_24, 1);
	regbitmapfont(hw, L"tga/gungsuhche_32.tga", LFT_GUNGSUHCHE_32, 1);
	regbitmapfont(hw, L"tga/msgothic_16.tga", LFT_MSGOTHIC_16, 1);
	regbitmapfont(hw, L"tga/msgothic_18.tga", LFT_MSGOTHIC_18, 1);
	regbitmapfont(hw, L"tga/msgothic_20.tga", LFT_MSGOTHIC_20, 1);
	regbitmapfont(hw, L"tga/msgothic_24.tga", LFT_MSGOTHIC_24, 1);
	regbitmapfont(hw, L"tga/msgothic_32.tga", LFT_MSGOTHIC_32, 1);
	regbitmapfont(hw, L"tga/shui_16.tga", LFT_SHUI_16, 1);
	regbitmapfont(hw, L"tga/shui_18.tga", LFT_SHUI_18, 1);
	regbitmapfont(hw, L"tga/shui_20.tga", LFT_SHUI_20, 1);
	regbitmapfont(hw, L"tga/shui_24.tga", LFT_SHUI_24, 2);
	regbitmapfont(hw, L"tga/shui_32.tga", LFT_SHUI_32, 3);
	regbitmapfont(hw, L"tga/impact_14.tga", LFT_IMPACT_14, 1);
	regbitmapfont(hw, L"tga/impact_16.tga", LFT_IMPACT_16, 1);
	regbitmapfont(hw, L"tga/impact_18.tga", LFT_IMPACT_18, 1);
	regbitmapfont(hw, L"tga/impact_20.tga", LFT_IMPACT_20, 1);
	regbitmapfont(hw, L"tga/impact_24.tga", LFT_IMPACT_24, 1);
	regbitmapfont(hw, L"tga/impact_32.tga", LFT_IMPACT_32, 1);
	regbitmapfont(hw, L"tga/linotype_10.tga", LFT_LINOTYPE_10, 1);
	regbitmapfont(hw, L"tga/linotype_12.tga", LFT_LINOTYPE_12, 1);
	regbitmapfont(hw, L"tga/linotype_14.tga", LFT_LINOTYPE_14, 1);
	regbitmapfont(hw, L"tga/linotype_16.tga", LFT_LINOTYPE_16, 1);
	regbitmapfont(hw, L"tga/linotype_18.tga", LFT_LINOTYPE_18, 1);
	regbitmapfont(hw, L"tga/linotype_20.tga", LFT_LINOTYPE_20, 1);
	regbitmapfont(hw, L"tga/linotype_24.tga", LFT_LINOTYPE_24, 1);
	regbitmapfont(hw, L"tga/linotype_32.tga", LFT_LINOTYPE_32, 1);
	regbitmapfont(hw, L"tga/linotype_8.tga", LFT_LINOTYPE_8, 1);
	regbitmapfont(hw, L"tga/lucidaconsole_10.tga", LFT_LUCIDACONSOLE_10, 1);
	regbitmapfont(hw, L"tga/lucidaconsole_12.tga", LFT_LUCIDACONSOLE_12, 1);
	regbitmapfont(hw, L"tga/lucidaconsole_14.tga", LFT_LUCIDACONSOLE_14, 1);
	regbitmapfont(hw, L"tga/lucidaconsole_16.tga", LFT_LUCIDACONSOLE_16, 1);
	regbitmapfont(hw, L"tga/lucidaconsole_18.tga", LFT_LUCIDACONSOLE_18, 1);
	regbitmapfont(hw, L"tga/lucidaconsole_20.tga", LFT_LUCIDACONSOLE_20, 1);
	regbitmapfont(hw, L"tga/lucidaconsole_24.tga", LFT_LUCIDACONSOLE_24, 1);
	regbitmapfont(hw, L"tga/lucidaconsole_32.tga", LFT_LUCIDACONSOLE_32, 1);
	regbitmapfont(hw, L"tga/lucidasans_10.tga", LFT_LUCIDASANS_10, 1);
	regbitmapfont(hw, L"tga/lucidasans_12.tga", LFT_LUCIDASANS_12, 1);
	regbitmapfont(hw, L"tga/lucidasans_14.tga", LFT_LUCIDASANS_14, 1);
	regbitmapfont(hw, L"tga/lucidasans_16.tga", LFT_LUCIDASANS_16, 1);
	regbitmapfont(hw, L"tga/lucidasans_18.tga", LFT_LUCIDASANS_18, 1);
	regbitmapfont(hw, L"tga/lucidasans_20.tga", LFT_LUCIDASANS_20, 1);
	regbitmapfont(hw, L"tga/lucidasans_24.tga", LFT_LUCIDASANS_24, 1);
	regbitmapfont(hw, L"tga/lucidasans_32.tga", LFT_LUCIDASANS_32, 1);
	regbitmapfont(hw, L"tga/symbol_10.tga", LFT_SYMBOL_10, 1);
	regbitmapfont(hw, L"tga/symbol_12.tga", LFT_SYMBOL_12, 1);
	regbitmapfont(hw, L"tga/symbol_14.tga", LFT_SYMBOL_14, 1);
	regbitmapfont(hw, L"tga/symbol_16.tga", LFT_SYMBOL_16, 1);
	regbitmapfont(hw, L"tga/symbol_18.tga", LFT_SYMBOL_18, 1);
	regbitmapfont(hw, L"tga/symbol_20.tga", LFT_SYMBOL_20, 1);
	regbitmapfont(hw, L"tga/symbol_24.tga", LFT_SYMBOL_24, 1);
	regbitmapfont(hw, L"tga/symbol_32.tga", LFT_SYMBOL_32, 1);
	regbitmapfont(hw, L"tga/tahoma_8.tga", LFT_TAHOMA_8, 1);
	regbitmapfont(hw, L"tga/tahoma_10.tga", LFT_TAHOMA_10, 1);
	regbitmapfont(hw, L"tga/tahoma_12.tga", LFT_TAHOMA_12, 1);
	regbitmapfont(hw, L"tga/tahoma_14.tga", LFT_TAHOMA_14, 1);
	regbitmapfont(hw, L"tga/tahoma_16.tga", LFT_TAHOMA_16, 1);
	regbitmapfont(hw, L"tga/tahoma_18.tga", LFT_TAHOMA_18, 1);
	regbitmapfont(hw, L"tga/tahoma_20.tga", LFT_TAHOMA_20, 1);
	regbitmapfont(hw, L"tga/tahoma_24.tga", LFT_TAHOMA_24, 1);
	regbitmapfont(hw, L"tga/tahoma_32.tga", LFT_TAHOMA_32, 1);
	regbitmapfont(hw, L"tga/terminal_9.tga", LFT_TERMINAL_9, 1);
	regbitmapfont(hw, L"tga/terminal_8.tga", LFT_TERMINAL_8, 1);
	regbitmapfont(hw, L"tga/terminal_12.tga", LFT_TERMINAL_12, 1);
	regbitmapfont(hw, L"tga/terminal_14.tga", LFT_TERMINAL_14, 1);
	regbitmapfont(hw, L"tga/terminal_18.tga", LFT_TERMINAL_18, 1);
	regbitmapfont(hw, L"tga/terminal_24.tga", LFT_TERMINAL_24, 1);
	regbitmapfont(hw, L"tga/terminal_32.tga", LFT_TERMINAL_32, 1);
	regbitmapfont(hw, L"tga/timesroman_8.tga", LFT_TIMESROMAN_8, 1);
	regbitmapfont(hw, L"tga/timesroman_9.tga", LFT_TIMESROMAN_9, 1);
	regbitmapfont(hw, L"tga/timesroman_10.tga", LFT_TIMESROMAN_10, 1);
	regbitmapfont(hw, L"tga/timesroman_11.tga", LFT_TIMESROMAN_11, 1);
	regbitmapfont(hw, L"tga/timesroman_12.tga", LFT_TIMESROMAN_12, 1);
	regbitmapfont(hw, L"tga/timesroman_14.tga", LFT_TIMESROMAN_14, 1);
	regbitmapfont(hw, L"tga/timesroman_16.tga", LFT_TIMESROMAN_16, 1);
	regbitmapfont(hw, L"tga/timesroman_18.tga", LFT_TIMESROMAN_18, 1);
	regbitmapfont(hw, L"tga/timesroman_20.tga", LFT_TIMESROMAN_20, 1);
	regbitmapfont(hw, L"tga/timesroman_24.tga", LFT_TIMESROMAN_24, 1);
	regbitmapfont(hw, L"tga/timesroman_32.tga", LFT_TIMESROMAN_32, 1);
	regbitmapfont(hw, L"tga/trebuchet_8.tga", LFT_TREBUCHET_8, 1);
	regbitmapfont(hw, L"tga/trebuchet_10.tga", LFT_TREBUCHET_10, 1);
	regbitmapfont(hw, L"tga/trebuchet_12.tga", LFT_TREBUCHET_12, 1);
	regbitmapfont(hw, L"tga/trebuchet_14.tga", LFT_TREBUCHET_14, 1);
	regbitmapfont(hw, L"tga/trebuchet_16.tga", LFT_TREBUCHET_16, 1);
	regbitmapfont(hw, L"tga/trebuchet_18.tga", LFT_TREBUCHET_18, 1);
	regbitmapfont(hw, L"tga/trebuchet_20.tga", LFT_TREBUCHET_20, 1);
	regbitmapfont(hw, L"tga/trebuchet_24.tga", LFT_TREBUCHET_24, 1);
	regbitmapfont(hw, L"tga/trebuchet_32.tga", LFT_TREBUCHET_32, 1);

	return getFontTotalA(hw);
}

#endif	// build tga


void registerInternalFonts (THWD *hw, const wchar_t *fontPath)
{
	if (fontPath)
		hw->paths->font = l_wcsdup(fontPath);
	else
		hw->paths->font = l_wcsdup(lFontPath);

	hw->fonts->bdf->hw = hw;
	hw->fonts->bmp->hw = hw;
	registerFontsW(hw);
	registerFontsA(hw);
}


#else


int initFonts (){return 1;}
void closeFonts(){return;}
int cacheGlyphs (UTF32 *glist, int gtotal, TWFONT *font){return 0;}
TWFONT *buildBDFFont (TWFONT *font){return NULL;}
TWFONT *buildBDFGlyph (THWD *hw, UTF32 ch, int fontid){return NULL;}
TWFONT *fontIDToFontW (THWD *hw, int fontid){return NULL;}
TWFONT *buildBDFGlyphs (THWD *hw, char *str, int fontid){return NULL;}
TFONT *fontIDToFontA (THWD *hw, int id){return NULL;}
int deleteFonts (THWD *hw){return 0;}
void registerInternalFonts (THWD *hw, const wchar_t *fontPath){return;}



#endif

