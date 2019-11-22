
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




#ifndef _API_H_
#define _API_H_


void *ptrstub (const void *stub, ...);
int intstub (const void *stub, ...);
void voidstub (const void *stub, ...);


#if (!__BUILD_DRAW_SUPPORT__)
#define drawImage intstub
#define drawLineDotted intstub
#define drawRectangleDottedFilled intstub
#define drawRectangleFilled intstub
#define drawRectangleDotted intstub
#define drawRectangle intstub
#define invertFrame intstub
#define invertArea intstub
#define drawCircle intstub
#define drawPBar intstub
#define drawMask intstub
#define drawMaskA intstub 
#define drawMaskAEx intstub 
#define drawCircleFilled intstub
#define drawEllipse intstub
#define drawEnclosedArc intstub
#define drawArc intstub
#define drawLine intstub
#define drawTriangle intstub
#define drawTriangleFilled intstub
#define drawPolyLineTo intstub
#define drawPolyLineDottedTo intstub
#define drawPolyLine intstub
#define drawPolyLineEx intstub
#define floodFill intstub
#define edgeFill intstub
#define blur intstub
#define blurArea intstub
#define blurImage intstub
#define convolve2DSeparable intstub
#define convolve2D intstub
#endif


#if (!__BUILD_CHRDECODE_SUPPORT__)
#define enumLanguageTables intstub
#define encodingAliasToID intstub
#define setCharacterEncoding intstub
#define getCharEncoding intstub
#define htmlCharRefEnable voidstub
#define htmlCharRefDisable voidstub
#define combinedCharEnable voidstub
#define combinedCharDisable voidstub
#define createCharacterList intstub
#define decodeCharacterCode intstub
#define decodeCharacterBuffer intstub
#define countCharacters intstub
#endif

#if (!__BUILD_SCROLL_SUPPORT__)
#define newScroll ptrstub
#define scrCopyToClient intstub
#define deleteScroll voidstub
#define updateScroll intstub
#endif


#if (!__BUILD_PRINT_SUPPORT__)
#define getCharMetrics intstub
#define getFontMetrics intstub
#define getTextMetrics intstub
#define _print intstub
#define _printEx intstub
#define _printf intstub
#define _printList intstub
#define newString ptrstub
#define newStringEx ptrstub
#define newStringList ptrstub
#define newStringListEx ptrstub
#endif


#if (!__BUILD_ROTATE_SUPPORT__)
#define rotate intstub
#define rotateFrameEx intstub
#define rotateFrameR90 intstub
#define rotateFrameL90 intstub
#define rotateX(a,b,c,d,e) voidstub((void*)&a,b,c,d,e)
#define rotateY(a,b,c,d,e) voidstub((void*)&a,b,c,d,e)
#define rotateZ(a,b,c,d,e) voidstub((void*)&a,b,c,d,e)
#define point3DTo2D(a,b,c,d,e,g,h,i,j) voidstub((void*)&a,b,c,d,e,g,h,i,j)
#endif

#if (!__BUILD_BDF_FONT_SUPPORT__)
#define getTextMetricsList intstub
#define cacheCharacterBuffer intstub
#define setRenderEffect intstub
#define renderEffectReset voidstub

#endif

#if (!__BUILD_BITMAP_FONT_SUPPORT__)
#define enumFontsBeginA ptrstub
#define enumFontsNextA intstub
#define enumFontsDeleteA voidstub
#define registerFontA intstub
#define unregisterFontA intstub
#define fontIDToFontA ptrstub
#endif

#if (!__BUILD_FONTS_SUPPORT__ || (!__BUILD_BITMAP_FONT_SUPPORT__ && !__BUILD_BDF_FONT_SUPPORT__))
#define cacheCharactersAll intstub
#define cacheCharacters intstub
#define cacheCharacterList intstub
#define cacheCharacterRange intstub
#define stripCharacterList intstub
#define getFontCharacterSpacing intstub
#define setFontCharacterSpacing intstub
#define setFontLineSpacing intstub
#define getFontLineSpacing intstub
#define setFontZeroWidthSpacing intstub
#define getCachedGlyphs intstub
#define mergeFontW intstub
#define registerFontW intstub
#define flushFont intstub
#define unregisterFontW intstub
#define enumFontsNextW intstub

#define enumFontsDeleteW ptrstub
#define enumFontsBeginW ptrstub
#define fontIDToFont ptrstub
#define getGlyph ptrstub
#define deleteFonts voidstub
#endif

#endif


/*
MYLCD_EXPORT void * my_Malloc (size_t size, const char *func, const int line);
MYLCD_EXPORT void * my_Calloc (size_t nelem, size_t elsize, const char *func, const int line);
MYLCD_EXPORT void * my_Realloc (void *ptr, size_t size, const char *func, const int line);
MYLCD_EXPORT void my_Free (void *ptr, const char *func, const int line);
MYLCD_EXPORT char * my_Strdup (const char *str, const char *func, const int line);
MYLCD_EXPORT wchar_t * my_Wcsdup (const wchar_t *str, const char *func, const int line);
MYLCD_EXPORT void * my_Memcpy (void *s1, const void *s2, size_t n);
*/
