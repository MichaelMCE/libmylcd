
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

#if (__BUILD_CHRDECODE_SUPPORT__)

#include "apilock.h"
#include "fonts.h"
#include "lstring.h"
#include "memory.h"
#include "utils.h"
#include "utf.h"
#include "html.h"
#include "chardecode.h"
#include "combmarks.h"
#include "cmap.h"
#include "lmath.h"



static inline int _isCharRef (THWD *hw, const char *str, UTF32 *wc);



static inline void setCharEncoding (THWD *hw, const int id)
{
	hw->cmap->mapCode = id;
}

static inline int _getCharEncoding (THWD *hw)
{
	return hw->cmap->mapCode;
}

int getCharEncoding (THWD *hw)
{
	return _getCharEncoding(hw);
}

void resetCharDecodeFlags (THWD *hw)
{
	hw->cmap->active = NULL;
	setCharEncoding(hw, CMT_DEFAULT);
	combinedCharEnable(hw);
	htmlCharRefEnable(hw);
}

int decodeCharacterCode (THWD *hw, const ubyte *buffer, UTF32 *ch)
{
	
	*ch = 0;
	const int enc = _getCharEncoding(hw);

	switch (enc){
	  //case CMT_JISX0213:
	  //case CMT_JISX0208:
	  case CMT_ISO_2022_JP_EUC_SJIS:
	  case CMT_ISO2022_JP:
	  case CMT_ISO2022_KR:
	  //case CMT_EUC_CN:
	  //case CMT_EUC_TW:
	  //case CMT_EUC_KR:
	  //case CMT_JISX0201:
	  case CMT_HZ_GB2312:
		return decodeMultiChar(hw, buffer, ch, enc);

	  case CMT_UTF16:
	  case CMT_UTF8:
	  default:{
	  	int chars;
		if (!(chars=_isCharRef(hw, (char*)buffer, ch)))
			chars = decodeMultiChar(hw, buffer, ch, enc);
		return chars;
	  }
	}
}

void htmlCharRefEnable (THWD *hw)
{
	hw->flags.charRef = 0;
}

void htmlCharRefDisable (THWD *hw)
{
	hw->flags.charRef = 1;
}

void combinedCharEnable (THWD *hw)
{
	hw->flags.charCombine = 0;
}

void combinedCharDisable (THWD *hw)
{
	hw->flags.charCombine = 1;
}

static inline UTF32 _remapChar (THWD *hw, const UTF32 enc)
{
	if (_getCharEncoding(hw) == CMT_NONE){
		return enc;
	}else{
		if (hw->cmap->active){
			if (enc&0xFFFF0000 /*> 65535*/){
				for (unsigned int i = 0; i < hw->cmap->active->ltotal; i++){
					if (hw->cmap->active->ltable[i].enc == enc)
						return hw->cmap->active->ltable[i].uni;
				}
				return 0;
			}else{
				return hw->cmap->active->table[enc];
			}
		}else{
			return enc;
		}
	}
}

UTF32 remapChar (THWD *hw, const UTF32 enc)
{
	return _remapChar(hw, enc);
}

// converts localized code point to unicode code point
// returns number of bytes read from buffer
int decodeMultiChar (THWD *hw, const ubyte *buffer, UTF32 *wc, const int cmap)
{
	if (!buffer) return 1;

	if (cmap == CMT_UTF8){
		return UTF8ToUTF32(buffer, wc);
	}else if (cmap == CMT_UTF16){
		return UTF16ToUTF32(buffer, wc);
	}else if (cmap == CMT_ASCII){
		*wc = (UTF32)*buffer;
		return sizeof(ubyte);
	}else if ((cmap == CMT_JISX0213) || (cmap == CMT_ISO_2022_JP_EUC_SJIS)){
		return JISX0213ToUTF32(buffer, wc);
	}else if (cmap == CMT_JISX0208){
		return EUCJPToUTF32(buffer, wc);
	}else if (cmap == CMT_ISO2022_JP){
		return ISO2022JPToUTF32(hw, buffer, wc);
	}else if (cmap == CMT_GB18030){
		return GB18030ToUTF32(hw, buffer, wc);
	}else if (cmap == CMT_BIG5){
		return BIG5ToUTF32(hw, buffer, wc);
	}else if (cmap == CMT_GBK){
		return GBKToUTF32(hw, buffer, wc);
	}else if (cmap == CMT_ISO2022_KR){
		return ISO2022KRToUTF32(hw, buffer, wc);
	}else if (cmap == CMT_EUC_KR){
		return EUCKRToUTF32(hw, buffer, wc);
	}else if (cmap == CMT_EUC_TW){
		return EUCTWToUTF32(hw, buffer, wc);
	}else if (cmap == CMT_EUC_CN){
		return EUCCNToUTF32(hw, buffer, wc);
	}else if (cmap == CMT_HZ_GB2312){	
		return HZToUTF32(buffer, wc);
	}else if (cmap == CMT_UTF16BE){
		return UTF16BEToUTF32(buffer, wc);
	}else if (cmap == CMT_UTF16LE){
		return UTF16LEToUTF32(buffer, wc);
	}else if (cmap == CMT_TIS620){
		return TIS620ToUTF32(buffer, wc);
	}else if (cmap == CMT_UTF32){
		*wc = *(UTF32 *)buffer;
		return sizeof(UTF32);
	}else{
		//CPxxxx, ISO8859_xx, JISX0201 and other single byte encodings
		*wc = _remapChar(hw, *buffer);
		return sizeof(ubyte);
	}
}

static inline int isCharRefW (THWD *hw, const UTF16 *str, UTF32 *wc)
{
#if (__BUILD_NUM_ENTITYREF_SUPPORT__ || __BUILD_CHR_ENTITYREF_SUPPORT__)

	if (!hw->flags.charRef){
		if (*(str++) == L'&'){
#if (__BUILD_NUM_ENTITYREF_SUPPORT__)
			if (*str == L'#'){
				int chars = 0;
				if ((chars=l_wcatoi2(++str, (int*)wc))){
					if (*(str+chars) == L';')
						return (chars+3);
				}
				return 0;
			}
#endif
#if (__BUILD_CHR_ENTITYREF_SUPPORT__)
			for (int i = 0; wentities[i].len; i++){
				if (str[0] == wentities[i].entity[0]){
					if (!l_wcsncmp(str, wentities[i].entity, wentities[i].len)){
						*wc = wentities[i].encoding;
						return wentities[i].len+1;
					}
				}
			}
#endif
		}
	}
#endif
	return 0;
}

// misnamed
static inline int isCharRefA (THWD *hw, const char *str, UTF32 *wc)
{

#if (__BUILD_NUM_ENTITYREF_SUPPORT__ || __BUILD_CHR_ENTITYREF_SUPPORT__)

	if (!hw->flags.charRef){
		if (*(str++) == '&'){
#if (__BUILD_NUM_ENTITYREF_SUPPORT__)
			if (*str == '#'){
				int chars = 0;
				if ((chars=l_atoi2(++str, (int*)wc))){
					if (*(str+chars) == ';'){
						return chars+3;
					}
				}
				return 0;
			}
#endif
#if (__BUILD_CHR_ENTITYREF_SUPPORT__)
			for (int i = 0; entities[i].len; i++){
				if (str[0] == entities[i].entity[0]){
					if (!l_strncmp(str, entities[i].entity, entities[i].len)){
						*wc = entities[i].encoding;
						return entities[i].len+1;
					}
				}
			}
#endif
		}
	}
#endif
	return 0;
}

/*	determine if string is a character reference,
	if so, return its unicode value in wc

	returns bytes read from str if str points to a char ref
	returns 0 if not a char ref

converts a character reference to a unicode code point
returns bytes read if string is an entity or reference, or 0 if not */
static inline int _isCharRef (THWD *hw, const char *str, UTF32 *wc)
{
	if (!str) return 1;
	
	const int enc = _getCharEncoding(hw);
	if (enc == CMT_UTF16 || enc == CMT_UTF16BE || enc == CMT_UTF16LE)
		return isCharRefW(hw, (UTF16*)str, wc);
	else
		return isCharRefA(hw, str, wc);
}

int isCharRef (THWD *hw, const char *str, UTF32 *wc)
{
	return _isCharRef(hw, str, wc);
}

static inline int isCharInList (UTF32 ch, UTF32 *glist, int gtotal)
{
	int ct = 0;
	while(gtotal--){
		ct++;
		if (*(glist++) == ch)
			return 1;
	}
	return 0;
}

int createCharacterList (THWD *hw, const char *str, UTF32 *glist, int total)
{
	ubyte *c = (ubyte*)str;
	UTF32 ch = 1;
	int tchars = 0;
	int ctotal = 0;

	while (ch && (ctotal < total)){
		tchars = decodeCharacterCode(hw, c, &ch);
		if (tchars > 0){
			if (ch){
				if (ch == 0x09)
					ch = lTabSpaceChar;
				else if (ch != 0x0D)
					if (!isCharInList(ch,glist,ctotal))
						glist[ctotal++] = ch;
			}
			c += tchars;
		}else{
			break;
		}
	}
	return ctotal;
}

static inline void swapu32 (UTF32 *a, UTF32 *b)
{
	UTF32 tmp = *a;
	*a = *b;
	*b = tmp;
}

static inline int decodeCharacterBuffer32 (THWD *hw, const UTF32 *buffer, UTF32 *glist, int total)
{

	UTF32 *c = (UTF32*)buffer;
	int ctotal = 0;

	while (*c && (ctotal < total)){
		if (*c == 0x09){
			int i = lTabSpaceWidth;
			while (i--)
				glist[ctotal++] = lTabSpaceChar;

		}else if (*c != 0x0D){
			glist[ctotal] = *c;
			if (isCombinedMark(hw, glist[ctotal]))
				swapu32(&glist[ctotal-1], &glist[ctotal]);

			ctotal++;
		}
		c++;
	}
	return ctotal;
}

static inline int decodeCharacterBuffer16 (THWD *hw, const UTF16 *buffer, UTF32 *glist, int total, int (*func) (const ubyte *, UTF32 *))
{
	
	UTF16 *c = (UTF16*)buffer;
	int tchars = 0;
	int ctotal = 0;
	UTF32 wc = 0;

	int (*utf16Toutf32) (const ubyte *, UTF32 *) = func;

	while (*c && (ctotal < total)){
		if (!(tchars=_isCharRef(hw, (char*)c, &wc)))
			tchars = utf16Toutf32((ubyte*)c, &wc);
		if (tchars > 0){
			if (wc){
				if (wc == 0x09){
					int i = lTabSpaceWidth;
					while (i--)
						glist[ctotal++] = lTabSpaceChar;

				}else if (wc != 0x0D){
					glist[ctotal] = wc;
					if (isCombinedMark(hw, glist[ctotal]))
						swapu32(&glist[ctotal-1], &glist[ctotal]);

					ctotal++;
				}
			}
			c += tchars;
		}else if (tchars < 0){
			wc = 1;
			c += l_abs(tchars);
		}else{
			break;
		}
	}
	return ctotal;
}

static inline int decodeCharacterBuffer8 (THWD *hw, const UTF8 *buffer, UTF32 *glist, int total)
{

	ubyte *c = (ubyte*)buffer;
	int tchars = 0;
	int ctotal = 0;
	UTF32 wc = 1;

	while (*c && (ctotal < total)){
		tchars = decodeCharacterCode(hw, c, &wc);
		if (tchars > 0){
			if (wc){
				if (wc == 0x09){
					int i = lTabSpaceWidth;
					while (i--)
						glist[ctotal++] = lTabSpaceChar;

				}else if (wc != 0x0D){
					glist[ctotal] = wc;
					if (isCombinedMark(hw, glist[ctotal]))
						swapu32(&glist[ctotal-1], &glist[ctotal]);

					ctotal++;
				}
			}
			c += tchars;
		}else if (tchars < 0){
			wc = 1;
			c += l_abs(tchars);
		}else{
			break;
		}
	}
	return ctotal;
}

static inline int decodeCharacterBufferJP (THWD *hw, const UTF8 *buffer, UTF32 *glist, int total)
{
	int jtotal = 0;

	ubyte *sjis = (ubyte *)l_calloc(sizeof(ubyte),(4 + l_strlen((char*)buffer))<<1);
	if (sjis != NULL){
		if (JaToSJIS(buffer, sjis))
			jtotal = decodeCharacterBuffer8(hw, (UTF8 *)sjis, glist, total);
		l_free(sjis);
	}
	return jtotal;
}


int decodeCharacterBuffer (THWD *hw, const char *buffer, UTF32 *glist, int total)
{
	const int enc = _getCharEncoding(hw);
	if (enc == CMT_ISO_2022_JP_EUC_SJIS)
		return decodeCharacterBufferJP(hw, (UTF8*)buffer, glist, total);
	else if (enc == CMT_UTF16)
		return decodeCharacterBuffer16(hw, (UTF16*)buffer, glist, total, UTF16ToUTF32);
	else if (enc == CMT_UTF16BE)
		return decodeCharacterBuffer16(hw, (UTF16*)buffer, glist, total, UTF16BEToUTF32);
	else if (enc == CMT_UTF16LE)
		return decodeCharacterBuffer16(hw, (UTF16*)buffer, glist, total, UTF16LEToUTF32);
	else if (enc == CMT_UTF32)
		return decodeCharacterBuffer32(hw, (UTF32*)buffer, glist, total);
	else
		return decodeCharacterBuffer8(hw, (UTF8*)buffer, glist, total);
}

static inline int countCharacters32 (THWD *hw, const UTF32 *buffer)
{
	UTF32 *c = (UTF32*)buffer;
	int ctotal = 0;
	
	while (*c){
		if (*c == 0x09)
			ctotal += lTabSpaceWidth;
		else if (*c != 0x0D)
			ctotal++;

		c++;
	}
	return ctotal;
}

static inline int countCharacters16 (THWD *hw, const UTF16 *buffer, int (*func) (const ubyte *, UTF32 *))
{
	UTF16 *c = (UTF16*)buffer;
	int ctotal = 0;
	int tchars = 0;
	UTF32 ch = 1;
	int (*utf16Toutf32) (const ubyte *, UTF32 *) = func;
	
	while (*c){
		if (!(tchars=_isCharRef(hw, (char*)c, &ch)))
			tchars = utf16Toutf32((ubyte*)c, &ch);
		if (tchars > 0){
			if (ch){
				if (ch == 0x09)
					ctotal += lTabSpaceWidth;
				else if (ch != 0x0D)
					ctotal++;
			}
			c += tchars;
		}else if (tchars<0)
			c += l_abs(tchars);
		else{
			break;
		}
	}
	return ctotal;
}

static inline int countCharacters8 (THWD *hw, const UTF8 *buffer)
{

	ubyte *c = (ubyte*)buffer;
	int ctotal = 0;
	int tchars = 0;
	UTF32 ch = 1;
	
	while(*c){
		tchars = decodeCharacterCode(hw, c, &ch);
		if (tchars > 0){
			if (ch){
				if (ch == 0x09)
					ctotal += lTabSpaceWidth;
				else if (ch != 0x0D)
					ctotal++;
			}
			c += tchars;
		}else if (tchars < 0)
			c += l_abs(tchars);
		else{
			break;
		}
	}
	return ctotal;
}

static inline int countCharactersJP (THWD *hw, const UTF8 *buffer)
{
	int jtotal = 0;

	// we can't determine exact buffer length required,
	// but we do know it can not be more than strlen() * 2
	ubyte *sjis = (ubyte *)l_calloc(sizeof(ubyte), (4+ l_strlen((char*)buffer)) * 2);
	if (sjis != NULL){
		if (JaToSJIS(buffer, sjis))
			jtotal = countCharacters8(hw, (UTF8 *)sjis);
		l_free(sjis);
	}
	return jtotal;
}

int countCharacters (THWD *hw, const char *buffer)
{
	const int enc = _getCharEncoding(hw);
	if (enc == CMT_UTF8)
		return countCharacters8(hw, (UTF8*)buffer);
	else if (enc == CMT_UTF16)
		return countCharacters16(hw, (UTF16*)buffer, UTF16ToUTF32);
	else if (enc == CMT_UTF16LE)
		return countCharacters16(hw, (UTF16*)buffer, UTF16LEToUTF32);
	else if (enc == CMT_UTF16BE)
		return countCharacters16(hw, (UTF16*)buffer, UTF16BEToUTF32);
	else if (enc == CMT_UTF32)
		return countCharacters32(hw, (UTF32*)buffer);
	else if (enc == CMT_ISO_2022_JP_EUC_SJIS)
		return countCharactersJP(hw, (UTF8*)buffer);
	else
		return countCharacters8(hw, (UTF8*)buffer);
}

// language encoding needs to be redesigned
int setCharacterEncodingTable (THWD *hw, const int id)
{

	if (id == CMT_UTF8 || id == CMT_UTF16 || id == CMT_UTF32 || id == CMT_JISX0213 || 
		id == CMT_ASCII || id == CMT_TIS620 || id == CMT_HZ_GB2312 || id == CMT_AUTO_JP ||
		id == CMT_UTF16BE || id == CMT_UTF16LE || id == CMT_JISX0208 || id == CMT_EUC_CN
#if (__BUILD_INTERNAL_BIG5__)
		  || id == CMT_BIG5
#endif
	){

		hw->cmap->active = NULL;
		int old = _getCharEncoding(hw);
		setCharEncoding(hw, id);
		return old;
	}

	TMAPTABLE *cmt = CMapIDToTable(hw, id);
	if (cmt && (id != CMT_NONE)){
		if (!cmt->built){
			if (!buildCMapTable(hw, id))
				return 0;
		}
		
		hw->cmap->active = cmt;
		int oldCode = _getCharEncoding(hw);
		setCharEncoding(hw, id);
		return oldCode;
	}else{
		hw->cmap->active = NULL;
		setCharEncoding(hw, CMT_NONE);
		return 0;
	}
}


#else

int setCharacterEncodingTable (THWD *hw, int id){return 1;}
void resetCharDecodeFlags (THWD *hw){return;}

#endif
