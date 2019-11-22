
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

#if (__BUILD_BMP_SUPPORT__)

#include "memory.h"
#include "frame.h"
#include "fileio.h"
#include "utils.h"
#include "bmp.h"
#include "pixel.h"
#include "copy.h"
#include "image.h"

#include "sync.h"
#include "misc.h"

#define STYLE LSP_SET

static inline int dib2Frame (TFRAME *frame, ubyte *data, TBMP *bmp, const int flags, int ox, int oy);
static inline int pad_bytes (int width);
static inline int dibWidthPadded (int width);

#if (!__BUILD_2432BITONLY_SUPPORT__)
static inline int _32bppDib1bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _32bppDib8bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _32bppDib12bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _32bppDib15bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _32bppDib16bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
#endif
static inline int _32bppDib24bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _32bppDib32bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);

#if (!__BUILD_2432BITONLY_SUPPORT__)
static inline int _24bppDib1bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _24bppDib8bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _24bppDib12bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _24bppDib15bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _24bppDib16bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
#endif
static inline int _24bppDib24bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _24bppDib32bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _24bppDib32AbppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);

#if (!__BUILD_2432BITONLY_SUPPORT__)
static inline int _15bppDib1bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _15bppDib8bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _15bppDib12bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _15bppDib15bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _15bppDib16bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
#endif
static inline int _15bppDib24bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _15bppDib32bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _15bppDib32AbppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);

#if (!__BUILD_2432BITONLY_SUPPORT__)
static inline int _8bppPalDib8bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _8bppPalDib12bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _8bppPalDib15bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _8bppPalDib16bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
#endif
static inline int _8bppPalDib24bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _8bppPalDib32bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _8bppPalDib32AbppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);

#if (!__BUILD_2432BITONLY_SUPPORT__)
static inline int _8bppDib1bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _8bppDib8bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _8bppDib12bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _8bppDib15bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _8bppDib16bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
#endif
static inline int _8bppDib24bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _8bppDib32bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _8bppDib32AbppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);

#if (!__BUILD_2432BITONLY_SUPPORT__)
static inline int _4bppDib1bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _4bppDib8bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _4bppDib12bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _4bppDib16bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _4bppDib15bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
#endif
static inline int _4bppDib24bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _4bppDib32bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _4bppDib32AbppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);

#if (!__BUILD_2432BITONLY_SUPPORT__)
static inline int _1bppDib1bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _1bppDib8bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _1bppDib12bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _1bppDib15bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _1bppDib16bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
#endif
static inline int _1bppDib24bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _1bppDib32bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);
static inline int _1bppDib32AbppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy);



int bmpGetMetrics (const wchar_t *filename, int *width, int *height, int *bpp)
{
    FILE *fp = l_wfopen(filename, L"rb");
   	if (!fp) return 0;
	
	TBMP bmp;
	l_fread(&bmp.bm[0], sizeof(TBMP)-sizeof(bmp.fil), 1,fp);
	l_fclose(fp);
	
	if ((bmp.bm[0] == 'B') && (bmp.bm[1] == 'M')){
		if (width) *width = bmp.Width;
		if (height) *height = bmp.Height;
		if (bpp){
			switch(bmp.bpp){
			case 24: *bpp = LFRM_BPP_24; break;
			case 32: *bpp = LFRM_BPP_32; break;
			case 16: *bpp = LFRM_BPP_16; break;
			case 1: *bpp = LFRM_BPP_1; break;
			case 8: *bpp = LFRM_BPP_8; break;
			default:
				*bpp = LFRM_BPP_32;
			}
		}
		return 1;
	}
	return 0;
}

int loadBmp (TFRAME *frame, const int flags, const wchar_t *filename, int ox, int oy)
{
    FILE *fp = l_wfopen(filename, L"rb");
   	if (!fp)
		return 0;
	
	TBMP bmp;
	l_fread(&bmp.bm[0], sizeof(TBMP)-sizeof(bmp.fil), 1,fp);
	
	if ((bmp.bm[0] != 'B') || (bmp.bm[1] != 'M')){
		l_fclose(fp);
		return 0;
	}
    if (bmp.compression == 1 || bmp.compression == 2){
		l_fclose(fp);
		return 0;
	}
	
    ubyte *dib = l_malloc(bmp.fsize);
    if (!dib){
		l_fclose(fp);
		return 0;
	}
	l_fseek(fp, 0, SEEK_SET); 
    l_fread(dib, sizeof(ubyte),bmp.fsize, fp);
    int ret = dib2Frame(frame, dib, &bmp, flags, ox, oy);
    l_fclose(fp);
	l_free(dib);
   	return ret;
}

static inline int dib2Frame (TFRAME *frame, ubyte *dib, TBMP *bmp, const int flags, const int ox, const int oy)
{
	mylog("libmylcd: dib bpp:%i, %ix%i, frame type:%i, palette:%i compression:%i\n", \
	  bmp->bpp, bmp->Width, bmp->Height, frame->bpp, bmp->ncolours, bmp->compression);

	if (bmp->compression){
		mylog("libmylcd: compressed BMP/DIB's not supported\n");
		return  0;
	}

	if (flags&LOAD_RESIZE)
		_resizeFrame(frame, bmp->Width, bmp->Height, 0);
			
	switch(bmp->bpp){ // src
      case 1 :
#if (!__BUILD_2432BITONLY_SUPPORT__)
      	if (frame->bpp == LFRM_BPP_1) // des
			return _1bppDib1bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_8)
			return _1bppDib8bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_12)
			return _1bppDib12bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_15)
			return _1bppDib15bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_16)
			return _1bppDib16bppFrame(frame, dib, bmp, ox, oy);
		else
#endif
		if (frame->bpp == LFRM_BPP_24)
			return _1bppDib24bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_32)
			return _1bppDib32bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_32A)
			return _1bppDib32AbppFrame(frame, dib, bmp, ox, oy);
		break;

      case 4 :
#if (!__BUILD_2432BITONLY_SUPPORT__)
		if (frame->bpp == LFRM_BPP_1)
			return _4bppDib1bppFrame(frame, dib, bmp, ox, oy);
      	else if (frame->bpp == LFRM_BPP_16 && bmp->ncolours)
			return _4bppDib16bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_8 && bmp->ncolours)
			return _4bppDib8bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_15 && bmp->ncolours)
			return _4bppDib15bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_12 && bmp->ncolours)
			return _4bppDib12bppFrame(frame, dib, bmp, ox, oy);
		else
#endif
		  if (frame->bpp == LFRM_BPP_24 && bmp->ncolours)
			return _4bppDib24bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_32 && bmp->ncolours)
			return _4bppDib32bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_32A && bmp->ncolours)
			return _4bppDib32AbppFrame(frame, dib, bmp, ox, oy);
		break;
						
      case 8 :
      	if (frame->bpp == LFRM_BPP_24 && bmp->ncolours)
			return _8bppPalDib24bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_24 && !bmp->ncolours)
			return _8bppDib24bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_32 && !bmp->ncolours)
			return _8bppDib32bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_32A && !bmp->ncolours)
			return _8bppDib32AbppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_32 && bmp->ncolours)
			return _8bppPalDib32bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_32A && bmp->ncolours)
			return _8bppPalDib32AbppFrame(frame, dib, bmp, ox, oy);
#if (!__BUILD_2432BITONLY_SUPPORT__)
		else if (frame->bpp == LFRM_BPP_8)
			return _8bppPalDib8bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_12 && bmp->ncolours)
			return _8bppPalDib12bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_16 && bmp->ncolours)
			return _8bppPalDib16bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_15 && bmp->ncolours)
			return _8bppPalDib15bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_16 && !bmp->ncolours)
			return _8bppDib16bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_15 && !bmp->ncolours)
			return _8bppDib15bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_12 && !bmp->ncolours)
			return _8bppDib12bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_8 && !bmp->ncolours)
			return _8bppDib8bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_1)
			return _8bppDib1bppFrame(frame, dib, bmp, ox, oy);
#endif
		break;
						
      case 16:
		if (frame->bpp == LFRM_BPP_24)
			return _15bppDib24bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_32)
			return _15bppDib32bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_32A)
			return _15bppDib32AbppFrame(frame, dib, bmp, ox, oy);
#if (!__BUILD_2432BITONLY_SUPPORT__)
      	else if (frame->bpp == LFRM_BPP_1)
			return _15bppDib1bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_8)
			return _15bppDib8bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_12)
			return _15bppDib12bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_15)
			return _15bppDib15bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_16)
			return _15bppDib16bppFrame(frame, dib, bmp, ox, oy);
#endif
		break;
		
      case 24:
		if (frame->bpp == LFRM_BPP_24)
			return _24bppDib24bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_32)
			return _24bppDib32bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_32A)
			return _24bppDib32AbppFrame(frame, dib, bmp, ox, oy);
#if (!__BUILD_2432BITONLY_SUPPORT__)
      	else if (frame->bpp == LFRM_BPP_1)
			return _24bppDib1bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_8)
			return _24bppDib8bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_12)
			return _24bppDib12bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_15)
			return _24bppDib15bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_16)
			return _24bppDib16bppFrame(frame, dib, bmp, ox, oy);
#endif
		break;
						
      case 32:
		if (frame->bpp == LFRM_BPP_24)
			return _32bppDib24bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_32 || frame->bpp == LFRM_BPP_32A)
			return _32bppDib32bppFrame(frame, dib, bmp, ox, oy);
#if (!__BUILD_2432BITONLY_SUPPORT__)
      	else if (frame->bpp == LFRM_BPP_1)
			return _32bppDib1bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_8)
			return _32bppDib8bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_12)
			return _32bppDib12bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_15)
			return _32bppDib15bppFrame(frame, dib, bmp, ox, oy);
		else if (frame->bpp == LFRM_BPP_16)
			return _32bppDib16bppFrame(frame, dib, bmp, ox, oy);
#endif
		break;
    }
    mylog("libmylcd: input format not supported or conversion to format not implemented\n");
    return 0;
}

#if (!__BUILD_2432BITONLY_SUPPORT__)
static inline int _1bppDib1bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{
	
	int x,y,b;
	ubyte *dib;
	int LineWidth = bmp->Width / 8;
	if ((bmp->Width & 7) != 0)
			LineWidth++;
	LineWidth = dibWidthPadded(LineWidth);

	int y2=0;
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=7;x<bmp->Width; x+=8){
			for (b=7;b>=0;b--){
				if (0x01^((*dib>>b)&0x01))
					setPixel_BC(frame,(x-b)+ox,y2+oy,STYLE);
			}
			dib++;
		}
	}
	return 1;
}

static inline int _1bppDib16bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{
	int x,y,b;
	ubyte *dib;
	int LineWidth = bmp->Width / 8;
	if ((bmp->Width & 7) != 0)
			LineWidth++;
	LineWidth = dibWidthPadded(LineWidth);

	int y2=0;
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=7;x<bmp->Width; x+=8){
			for (b=7;b>=0;b--){
				if (0x01^((*dib>>b)&0x01))
					l_setPixel(frame,(x-b)+ox,y2+oy, frame->hw->render->foreGround&0xFFFF);
				else
					l_setPixel(frame,(x-b)+ox,y2+oy, frame->hw->render->backGround&0xFFFF);
			}
			dib++;
		}
	}
	return 1;
}
#endif
static inline int _1bppDib24bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{
	int x,y,b;
	ubyte *dib;
	int LineWidth = bmp->Width / 8;
	if ((bmp->Width & 7) != 0)
			LineWidth++;
	LineWidth = dibWidthPadded(LineWidth);

	int y2=0;
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=7;x<bmp->Width; x+=8){
			for (b=7;b>=0;b--){
				if (0x01^((*dib>>b)&0x01))
					l_setPixel(frame,(x-b)+ox,y2+oy, frame->hw->render->foreGround&0xFFFFFF);
				else
					l_setPixel(frame,(x-b)+ox,y2+oy, frame->hw->render->backGround&0xFFFFFF);
			}
			dib++;
		}
	}
	return 1;
}

static inline int _1bppDib32AbppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{
	int x,y,b;
	ubyte *dib;
	int LineWidth = bmp->Width / 8;
	if ((bmp->Width & 7) != 0)
			LineWidth++;
	LineWidth = dibWidthPadded(LineWidth);

	int y2=0;
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=7;x<bmp->Width; x+=8){
			for (b=7;b>=0;b--){
				if (0x01^((*dib>>b)&0x01))
					l_setPixel(frame,(x-b)+ox,y2+oy, frame->hw->render->foreGround|0xFF000000);
				else
					l_setPixel(frame,(x-b)+ox,y2+oy, frame->hw->render->backGround|0xFF000000);
			}
			dib++;
		}
	}
	return 1;
}

static inline int _1bppDib32bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{
	int x,y,b;
	ubyte *dib;
	int LineWidth = bmp->Width / 8;
	if ((bmp->Width & 7) != 0)
			LineWidth++;
	LineWidth = dibWidthPadded(LineWidth);

	int y2=0;
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=7;x<bmp->Width; x+=8){
			for (b=7;b>=0;b--){
				if (0x01^((*dib>>b)&0x01))
					l_setPixel(frame,(x-b)+ox,y2+oy, frame->hw->render->foreGround&0x00FFFFFF);
				else
					l_setPixel(frame,(x-b)+ox,y2+oy, frame->hw->render->backGround&0x00FFFFFF);
			}
			dib++;
		}
	}
	return 1;
}
#if (!__BUILD_2432BITONLY_SUPPORT__)
static inline int _1bppDib15bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{
	int x,y,b;
	ubyte *dib;
	int LineWidth = bmp->Width / 8;
	if ((bmp->Width & 7) != 0)
			LineWidth++;
	LineWidth = dibWidthPadded(LineWidth);

	int y2=0;
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=7;x<bmp->Width; x+=8){
			for (b=7;b>=0;b--){
				if (0x01^((*dib>>b)&0x01))
					l_setPixel(frame,(x-b)+ox,y2+oy, frame->hw->render->foreGround&0x7FFF);
				else
					l_setPixel(frame,(x-b)+ox,y2+oy, frame->hw->render->backGround&0x7FFF);
			}
			dib++;
		}
	}
	return 1;
}

static inline int _1bppDib8bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{
	int x,y,b;
	ubyte *dib;
	int LineWidth = bmp->Width / 8;
	if ((bmp->Width & 7) != 0)
			LineWidth++;
	LineWidth = dibWidthPadded(LineWidth);

	int y2=0;
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=7;x<bmp->Width; x+=8){
			for (b=7;b>=0;b--){
				if (0x01^((*dib>>b)&0x01))
					l_setPixel(frame,(x-b)+ox,y2+oy, frame->hw->render->foreGround&0xFF);
				else
					l_setPixel(frame,(x-b)+ox,y2+oy, frame->hw->render->backGround&0xFF);
			}
			dib++;
		}
	}
	return 1;
}

static inline int _1bppDib12bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{
	int x,y,b;
	ubyte *dib;
	int LineWidth = bmp->Width / 8;
	if ((bmp->Width & 7) != 0)
			LineWidth++;
	LineWidth = dibWidthPadded(LineWidth);

	int y2=0;
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=7;x<bmp->Width; x+=8){
			for (b=7;b>=0;b--){
				if (0x01^((*dib>>b)&0x01))
					l_setPixel(frame,(x-b)+ox,y2+oy, frame->hw->render->foreGround&0xFFF);
				else
					l_setPixel(frame,(x-b)+ox,y2+oy, frame->hw->render->backGround&0xFFF);
			}
			dib++;
		}
	}
	return 1;
}
#endif
static inline int _4bppDib32AbppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{

	ubyte *dib;
	int x,y;
	int LineWidth = (bmp->Width + 1) / 2;
	LineWidth = dibWidthPadded(LineWidth);
	int *pal = (int *)(data+BMPHEADERSIZE);
	int y2=0;
	
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		
		for (x=0;x<bmp->Width;){
			l_setPixel(frame, x++ +ox, y2+oy, pal[(*dib&0xF0)>>4] | 0xFF000000);
			l_setPixel(frame, x++ +ox, y2+oy, pal[*dib++&0x0F] | 0xFF000000);
		}
	}
	return 1;
}

static inline int _4bppDib32bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{

	ubyte *dib;
	int x,y;
	int LineWidth = (bmp->Width + 1) / 2;
	LineWidth = dibWidthPadded(LineWidth);
	int *pal = (int *)(data+BMPHEADERSIZE);
	int y2=0;
	
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		
		for (x=0;x<bmp->Width;){
			l_setPixel(frame, x++ +ox, y2+oy, pal[(*dib&0xF0)>>4]);
			l_setPixel(frame, x++ +ox, y2+oy, pal[*dib++&0x0F]);
		}
	}
	return 1;
}

static inline int _4bppDib24bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{

	ubyte *dib;
	int x,y;
	int LineWidth = (bmp->Width + 1) / 2;
	LineWidth = dibWidthPadded(LineWidth);
	int *pal = (int *)(data+BMPHEADERSIZE);
	int y2=0;
	
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		
		for (x=0;x<bmp->Width;){
			l_setPixel(frame, x++ +ox, y2+oy, pal[(*dib&0xF0)>>4]);
			l_setPixel(frame, x++ +ox, y2+oy, pal[*dib++&0x0F]);
		}
	}
	return 1;
}
#if (!__BUILD_2432BITONLY_SUPPORT__)
static inline int _4bppDib8bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{

	ubyte *dib;
	int x,y;
	int LineWidth = (bmp->Width + 1) / 2;
	LineWidth = dibWidthPadded(LineWidth);
	int *pal = (int *)(data+BMPHEADERSIZE);
	int y2=0;
	int r,g,b;
	int i;
	
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		
		for (x=0;x<bmp->Width;){
			i = (*dib&0xF0)>>4;
			r = (pal[i]&0xE00000)>>16;
			g = (pal[i]&0x00E000)>>11;
			b = (pal[i]&0x000030)>>6;
			l_setPixel(frame, x++ +ox, y2+oy, r|g|b);

			i = (*dib++&0x0F);
			r = (pal[i]&0xE00000)>>16;
			g = (pal[i]&0x00E000)>>11;
			b = (pal[i]&0x000030)>>6;
			l_setPixel(frame, x++ +ox, y2+oy, r|g|b);
		}
	}
	return 1;
}

static inline int _4bppDib12bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{

	ubyte *dib;
	int x,y;
	int LineWidth = (bmp->Width + 1) / 2;
	LineWidth = dibWidthPadded(LineWidth);
	int *pal = (int *)(data+BMPHEADERSIZE);
	int y2=0;
	int r,g,b;
	int i;
	
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		
		for (x=0;x<bmp->Width;){
			i = (*dib&0xF0)>>4;
			r = (pal[i]&0xF00000)>>12;
			g = (pal[i]&0x00F000)>>8;
			b = (pal[i]&0x0000F0)>>4;
			l_setPixel(frame, x++ +ox, y2+oy, r|g|b);

			i = (*dib++&0x0F);
			r = (pal[i]&0xF00000)>>12;
			g = (pal[i]&0x00F000)>>8;
			b = (pal[i]&0x0000F0)>>4;
			l_setPixel(frame, x++ +ox, y2+oy, r|g|b);
		}
	}
	return 1;
}

static inline int _4bppDib15bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{

	ubyte *dib;
	int x,y;
	int LineWidth = (bmp->Width + 1) / 2;
	LineWidth = dibWidthPadded(LineWidth);
	int *pal = (int *)(data+BMPHEADERSIZE);
	int y2=0;
	int r,g,b;
	int i;
	
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		
		for (x=0;x<bmp->Width;){
			i = (*dib&0xF0)>>4;
			r = (pal[i]&0xF80000)>>9;
			g = (pal[i]&0x00F800)>>6;
			b = (pal[i]&0x0000F8)>>3;
			l_setPixel(frame, x++ +ox, y2+oy, r|g|b);

			i = (*dib++&0x0F);
			r = (pal[i]&0xF80000)>>9;
			g = (pal[i]&0x00F800)>>6;
			b = (pal[i]&0x0000F8)>>3;
			l_setPixel(frame, x++ +ox, y2+oy, r|g|b);
		}
	}
	return 1;
}

static inline int _4bppDib16bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{

	ubyte *dib;
	int x,y;
	int LineWidth = (bmp->Width + 1) / 2;
	LineWidth = dibWidthPadded(LineWidth);
	int *pal = (int *)(data+BMPHEADERSIZE);
	int y2=0;
	int r,g,b;
	int i;
	
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		
		for (x=0;x<bmp->Width;){
			i = (*dib&0xF0)>>4;
			r = (pal[i]&0xF80000)>>8;
			g = (pal[i]&0x00FC00)>>5;
			b = (pal[i]&0x0000F8)>>3;
			l_setPixel(frame, x++ +ox, y2+oy, r|g|b);

			i = (*dib++&0x0F);
			r = (pal[i]&0xF80000)>>8;
			g = (pal[i]&0x00FC00)>>5;
			b = (pal[i]&0x0000F8)>>3;
			l_setPixel(frame, x++ +ox, y2+oy, r|g|b);
		}
	}
	return 1;
}

static inline int _4bppDib1bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{

	ubyte *dib;
	int x,y;
	int LineWidth = ( bmp->Width + 1) / 2;
	LineWidth = dibWidthPadded(LineWidth);

	int y2=0;
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		
		for (x=0;x<bmp->Width;){
			if ((*dib&0x0F)<<4 < DIBTRESHOLD)
				setPixel_BC(frame,x+ox,y2+oy,STYLE);
			x++;
			if (((*dib>>4)&0x0F)<<4 < DIBTRESHOLD)
				setPixel_BC(frame,x+ox,y2+oy,STYLE);
			x++;
			dib++;
		}
	}
	return 1;
}

static inline int _8bppPalDib8bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{
		
	ubyte *dib;
	int x,y;
	const int LineWidth = dibWidthPadded(bmp->Width);
	int y2=0;
	int *pal = (int *)(data+BMPHEADERSIZE);

	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=0;x<bmp->Width;x++){
			int r = (pal[*dib]&0xE00000)>>16;
			int g = (pal[*dib]&0x00E000)>>11;
			int b = (pal[*dib]&0x0000C0)>>6;
			l_setPixel(frame, x+ox, y2+oy, r|g|b);
			dib++;
		}
	}
	return 1;
}
#endif
static inline int _8bppDib32AbppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{
	
	ubyte *dib;
	int x,y;
	const int LineWidth = dibWidthPadded(bmp->Width);
	int y2=0;
	
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=0;x<bmp->Width;x++){
			l_setPixel(frame, x+ox, y2+oy, (0xFF<<24) | (*dib<<16) | (*dib<<8) | *dib);
			dib++;
		}
	}
	return 1;
}

static inline int _8bppDib32bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{
	
	ubyte *dib;
	int x,y;
	const int LineWidth = dibWidthPadded(bmp->Width);
	int y2=0;
	
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=0;x<bmp->Width;x++){
			l_setPixel(frame, x+ox, y2+oy, (*dib<<16) | (*dib<<8) | *dib);
			dib++;
		}
	}
	return 1;
}

static inline int _8bppDib24bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{
	
	ubyte *dib;
	int x,y;
	const int LineWidth = dibWidthPadded(bmp->Width);
	int y2=0;
	
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=0;x<bmp->Width;x++){
			l_setPixel(frame, x+ox, y2+oy, (*dib<<16) | (*dib<<8) | *dib);
			dib++;
		}
	}
	return 1;
}
#if (!__BUILD_2432BITONLY_SUPPORT__)
static inline int _8bppDib8bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{
	
	ubyte *dib;
	int x,y;
	const int LineWidth = dibWidthPadded(bmp->Width);
	int y2=0;
	
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=0;x<bmp->Width;x++)
			l_setPixel(frame, x+ox, y2+oy, *dib++);
	}
	return 1;
}

static inline int _8bppDib12bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{
	
	ubyte *dib;
	int x,y,col;
	const int LineWidth = dibWidthPadded(bmp->Width);
	int y2=0;
	
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=0;x<bmp->Width;x++){
			col = *dib>>4;
			l_setPixel(frame, x+ox, y2+oy, (col<<8) | (col<<4) | col);
			dib++;
		}
	}
	return 1;
}

static inline int _8bppDib15bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{
	
	ubyte *dib;
	int x,y,col;
	const int LineWidth = dibWidthPadded(bmp->Width);
	int y2=0;
	
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=0;x<bmp->Width;x++){
			col = *dib>>3;
			l_setPixel(frame, x+ox, y2+oy, (col<<10) | (col<<5) | col);
			dib++;
		}
	}
	return 1;
}

static inline int _8bppDib16bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{
	
	ubyte *dib;
	int x,y,col;
	const int LineWidth = dibWidthPadded(bmp->Width);
	int y2=0;
	
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=0;x<bmp->Width;x++){
			col = *dib>>2;
			l_setPixel(frame, x+ox, y2+oy, ((col>>1)<<11) | (col<<5) | (col>>1));
			dib++;
		}
	}
	return 1;
}

static inline int _8bppPalDib15bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{
	
	ubyte *dib;
	int x,y;
	const int LineWidth = dibWidthPadded(bmp->Width);
	int *pal = (int *)(data+BMPHEADERSIZE);
	int y2=0;
	
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=0;x<bmp->Width;x++){
			int r = (pal[*dib]&0xF80000)>>9;
			int g = (pal[*dib]&0x00F800)>>6;
			int b = (pal[*dib]&0x0000F8)>>3;
			l_setPixel(frame, x+ox, y2+oy, r|g|b);
			dib++;
		}
	}
	return 1;
}

static inline int _8bppPalDib16bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{
	
	ubyte *dib;
	int x,y;
	const int LineWidth = dibWidthPadded(bmp->Width);
	int *pal = (int *)(data+BMPHEADERSIZE);
	int y2=0;
	
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=0;x<bmp->Width;x++){
			int r = (pal[*dib]&0xF80000)>>8;
			int g = (pal[*dib]&0x00FC00)>>5;
			int b = (pal[*dib]&0x0000F8)>>3;
			l_setPixel(frame, x+ox, y2+oy, r|g|b);
			dib++;
		}
	}
	return 1;
}

static inline int _8bppPalDib12bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{
	
	ubyte *dib;
	int x,y;
	const int LineWidth = dibWidthPadded(bmp->Width);
	int *pal = (int *)(data+BMPHEADERSIZE);
	int y2=0;
	
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=0;x<bmp->Width;x++){
			int r = (pal[*dib]&0xF00000)>>12;
			int g = (pal[*dib]&0x00F000)>>8;
			int b = (pal[*dib]&0x0000F0)>>4;
			l_setPixel(frame, x+ox, y2+oy, r|g|b);
			dib++;
		}
	}
	return 1;
}
#endif
static inline int _8bppPalDib24bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{


	ubyte *dib;
	int x,y;
	const int LineWidth = dibWidthPadded(bmp->Width);
	int *pal = (int *)(data+BMPHEADERSIZE);
	int y2=0;
	
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=0;x<bmp->Width;x++){
			l_setPixel(frame, x+ox, y2+oy, pal[*dib]);
			dib++;
		}
	}
	return 1;
}

static inline int _8bppPalDib32AbppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{
	
	ubyte *dib;
	int x,y;
	const int LineWidth = dibWidthPadded(bmp->Width);
	int *pal = (int *)(data+BMPHEADERSIZE);
	int y2=0;
	
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=0;x<bmp->Width;x++){
			l_setPixel(frame, x+ox, y2+oy, 0xFF000000 | pal[*dib]);
			dib++;
		}
	}
	return 1;
}

static inline int _8bppPalDib32bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{
	
	ubyte *dib;
	int x,y;
	const int LineWidth = dibWidthPadded(bmp->Width);
	int *pal = (int *)(data+BMPHEADERSIZE);
	int y2=0;
	
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=0;x<bmp->Width;x++){
			l_setPixel(frame, x+ox, y2+oy, pal[*dib]);
			dib++;
		}
	}
	return 1;
}
#if (!__BUILD_2432BITONLY_SUPPORT__)
static inline int _8bppDib1bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{
	
	ubyte *dib;
	int x,y;
	const int LineWidth = dibWidthPadded(bmp->Width);

	int y2=0;
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=0;x<bmp->Width;x++){
			if (*(dib++) < DIBTRESHOLD)
				setPixel_BC(frame,x+ox,y2+oy,STYLE);
		}
	}
	return 1;
}

static inline int _15bppDib1bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{

	unsigned short usdib;
	ubyte *dib;
	int x,y,y2=0;
	const int LineWidth = dibWidthPadded(bmp->Width<<1);
	
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=0;x<bmp->Width;x++){
			usdib = *(unsigned short*)dib;
			if ((((usdib&0x1F)<<3) < DIBTRESHOLD) || ((((usdib>>5)&0x1F)<<3) < DIBTRESHOLD) || ((((usdib>>10)&0x1F)<<3) < DIBTRESHOLD))
				setPixel_BC(frame,x+ox,y2+oy,STYLE);
			dib += 2;
		}
	}
	return 1;
}

static inline int _15bppDib15bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{

	ubyte *dib;
	int x,y, y2 = 0;
	const int LineWidth = dibWidthPadded(bmp->Width<<1);

	for (y=bmp->Height; y--; y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		
		for (x = 0; x<bmp->Width; x++){
			l_setPixel(frame, x+ox, y2+oy, *(uint16_t*)dib);
			dib += 2;
		}
	}
	return 1;
}

static inline int _15bppDib8bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{

	ubyte *dib;
	int c,x,y,r,g,b, y2 = 0;
	const int LineWidth = dibWidthPadded(bmp->Width<<1);

	for (y=bmp->Height; y--; y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		
		for (x = 0; x<bmp->Width; x++){
			c = *(uint16_t*)dib;
			r = (c&0x7000)>>7;	// 3
			g = (c&0x0380)>>5;	// 3
			b = (c&0x0018)>>3;	// 2
			l_setPixel(frame, x+ox, y2+oy, r|g|b);
			dib += 2;
		}
	}
	return 1;
}

static inline int _15bppDib12bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{

	ubyte *dib;
	int c,x,y,r,g,b, y2 = 0;
	const int LineWidth = dibWidthPadded(bmp->Width<<1);

	for (y=bmp->Height; y--; y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		
		for (x = 0; x<bmp->Width; x++){
			c = *(uint16_t*)dib;
			r = (c&0x7800)>>3;	// 4
			g = (c&0x03C0)>>2;	// 4
			b = (c&0x001E)>>1;	// 4
			l_setPixel(frame, x+ox, y2+oy, r|g|b);
			dib += 2;
		}
	}
	return 1;
}
#endif
static inline int _15bppDib32AbppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{

	ubyte *dib;
	int c,x,y,r,g,b, y2 = 0;
	const int LineWidth = dibWidthPadded(bmp->Width<1);

	for (y=bmp->Height; y--; y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		
		for (x = 0; x<bmp->Width; x++){
			c = *(uint16_t*)dib;
			r = (c&0x7C00)<<9;		// red			
			g = (c&0x03E0)<<6;		// green
			b = (c&0x001F)<<3;		// blue
			l_setPixel(frame, x+ox, y2+oy, 0xFF000000|r|g|b);
			dib += 2;
		}
	}
	return 1;
}

static inline int _15bppDib32bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{

	ubyte *dib;
	int c,x,y,r,g,b, y2 = 0;
	const int LineWidth = dibWidthPadded(bmp->Width<1);

	for (y=bmp->Height; y--; y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		
		for (x = 0; x<bmp->Width; x++){
			c = *(uint16_t*)dib;
			r = (c&0x7C00)<<9;		// red			
			g = (c&0x03E0)<<6;		// green
			b = (c&0x001F)<<3;		// blue
			l_setPixel(frame, x+ox, y2+oy, r|g|b);
			dib += 2;
		}
	}
	return 1;
}

static inline int _15bppDib24bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{

	ubyte *dib;
	int c,x,y,r,g,b, y2 = 0;
	const int LineWidth = dibWidthPadded(bmp->Width<<1);

	for (y=bmp->Height; y--; y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		
		for (x = 0; x<bmp->Width; x++){
			c = *(uint16_t*)dib;
			r = (c&0x7C00)<<9;		// red			
			g = (c&0x03E0)<<6;		// green
			b = (c&0x001F)<<3;		// blue
			l_setPixel(frame, x+ox, y2+oy, r|g|b);
			dib += 2;
		}
	}
	return 1;
}
#if (!__BUILD_2432BITONLY_SUPPORT__)
static inline int _15bppDib16bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{

	ubyte *dib;
	int x,y, y2 = 0;
	const int LineWidth = dibWidthPadded(bmp->Width<<1);

	for (y=bmp->Height; y--; y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		
		for (x = 0; x<bmp->Width; x++){
			l_setPixel(frame, x+ox, y2+oy, *(uint16_t*)dib);
			dib += 2;
		}
	}
	return 1;
}
#endif
static inline int _24bppDib24bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{

	ubyte *dib = data;
	int x,y;
	const int LineWidth = dibWidthPadded(bmp->Width*3);
	int y2=0;

	for (y=bmp->Height; y--; y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=0; x<bmp->Width; x++){
			l_setPixel(frame, x+ox, y2+oy, (dib[2]<<16) | (dib[1]<<8) | dib[0]);
			dib += 3;
		}
	}
	return 1;
}

static inline int _24bppDib32AbppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{
	ubyte *dib = data;
	int x,y;
	const int LineWidth = dibWidthPadded(bmp->Width*3);
	int y2=0;

	for (y=bmp->Height; y--; y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=0; x<bmp->Width; x++){
			l_setPixel(frame, x+ox, y2+oy, 0xFF000000 | (dib[2]<<16) | (dib[1]<<8) | dib[0]);
			dib += 3;
		}
	}
	return 1;
}

static inline int _24bppDib32bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{
	ubyte *dib = data;
	int x,y;
	const int LineWidth = dibWidthPadded(bmp->Width*3);
	int y2=0;

	for (y=bmp->Height; y--; y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=0; x<bmp->Width; x++){
			l_setPixel(frame, x+ox, y2+oy, (dib[2]<<16) | (dib[1]<<8) | dib[0]);
			dib += 3;
		}
	}
	return 1;
}
#if (!__BUILD_2432BITONLY_SUPPORT__)
static inline int _24bppDib8bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{

	ubyte *dib = data;
	int x,y,r,g,b;
	const int LineWidth = dibWidthPadded(bmp->Width*3);

	int y2=0;
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=0; x<bmp->Width; x++){
			b = ((*dib++)&0xC0)>>6;
			g = ((*dib++)&0xE0)>>3;
			r = (*dib++)&0xE0;
			l_setPixel(frame, x+ox, y2+oy, r|g|b);
		}
	}
	return 1;
}

static inline int _24bppDib15bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{

	ubyte *dib = data;
	int x,y,r,g,b;
	const int LineWidth = dibWidthPadded(bmp->Width*3);

	int y2=0;
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=0; x<bmp->Width; x++){
			b = *dib++ >> 3;
			g = *dib++ >> 3;
			r = *dib++ >> 3;
			l_setPixel(frame, x+ox, y2+oy, (r<<10)|(g<<5)|b);
		}
	}
	return 1;
}

static inline int _24bppDib16bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{

	ubyte *dib;
	int x,y,r,g,b;
	const int LineWidth = dibWidthPadded(bmp->Width*3);

	int y2=0;
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=0; x<bmp->Width; x++){
			b = *dib++ >> 3;
			g = *dib++ >> 2;
			r = *dib++ >> 3;
			l_setPixel(frame, x+ox, y2+oy, (r<<11)|(g<<5)|b);
		}
	}
	return 1;
}

static inline int _32bppDib8bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{

	uint32_t *dib;
	int x,y,r,g,b;
	const int LineWidth = dibWidthPadded(bmp->Width*3);

	int y2=0;
	for (y=bmp->Height;y--;y2++){
		dib = (uint32_t*)(data+bmp->dataOffset+(y*LineWidth));
		for (x=0; x<bmp->Width; x++){
			r = (*dib&0xE00000)>>24;	// 3
			g = (*dib&0x00E000)>>11;	// 3
			b = (*dib&0x0000C0)>>6;		// 2
			l_setPixel(frame, x+ox, y2+oy, r|g|b);
			dib++;
		}
	}
	return 1;
}

static inline int _32bppDib12bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{

	uint32_t *dib;
	int x,y,r,g,b;
	const int LineWidth = dibWidthPadded(bmp->Width*3);

	int y2=0;
	for (y=bmp->Height;y--;y2++){
		dib = (uint32_t*)(data+bmp->dataOffset+(y*LineWidth));
		for (x=0; x<bmp->Width; x++){
			r = (*dib&0xF00000)>>12;	// 4
			g = (*dib&0x00F000)>>8;		// 4
			b = (*dib&0x0000F0)>>4;		// 4
			l_setPixel(frame, x+ox, y2+oy, r|g|b);
			dib++;
		}
	}
	return 1;
}

static inline int _32bppDib15bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{

	ubyte *dib;
	int x,y,r,g,b;
	const int LineWidth = dibWidthPadded(bmp->Width*3);

	int y2=0;
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=0; x<bmp->Width; x++){
			b = *dib++ >> 3;
			g = *dib++ >> 3;
			r = *dib++ >> 3;
			l_setPixel(frame, x+ox, y2+oy, (r<<10)|(g<<5)|b);
			dib++;
		}
	}
	return 1;
}
#endif
static inline int _32bppDib24bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{

	uint32_t *dib;
	int x,y;
	const int LineWidth = dibWidthPadded(bmp->Width*3);

	int y2=0;
	for (y=bmp->Height;y--;y2++){
		dib = (uint32_t*)(data+bmp->dataOffset+(y*LineWidth));
		for (x=0; x<bmp->Width; x++){
			l_setPixel(frame, x+ox, y2+oy, (*dib)&0x00FFFFFF);
			dib++;
		}
	}
	return 1;
}

static inline int _32bppDib32bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{
	uint32_t *dib;
	int x,y;
	const int LineWidth = dibWidthPadded(bmp->Width*3);

	int y2=0;
	for (y=bmp->Height; y--; y2++){
		dib = (uint32_t*)(data+bmp->dataOffset+(y*LineWidth));
		for (x=0; x<bmp->Width; x++){
			l_setPixel(frame, x+ox, y2+oy, *dib);
			dib++;
		}
	}
	return 1;
}
#if (!__BUILD_2432BITONLY_SUPPORT__)
static inline int _32bppDib16bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{

	ubyte *dib;
	int x,y,r,g,b;
	const int LineWidth = dibWidthPadded(bmp->Width*3);

	int y2=0;
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=0; x<bmp->Width; x++){
			b = *dib++ >> 3;
			g = *dib++ >> 2;
			r = *dib++ >> 3;
			l_setPixel(frame, x+ox, y2+oy, (r<<11)|(g<<5)|b);
			dib++;
		}
	}
	return 1;
}

static inline int _24bppDib12bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{

	ubyte *dib;
	int x,y,r,g,b;
	const int LineWidth = dibWidthPadded(bmp->Width*3);

	int y2=0;
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=0; x<bmp->Width; x++){
			b = *dib++ >> 4;
			g = *dib++ >> 4;
			r = *dib++ >> 4;
			l_setPixel(frame, x+ox, y2+oy, (r<<8)|(g<<4)|b);
		}
	}
	return 1;
}

static inline int _24bppDib1bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{

	ubyte *dib = data;
	int x,y;
	const int LineWidth = dibWidthPadded(bmp->Width*3);
		
	int y2=0;
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=0;x<bmp->Width;x++){
			if ((*dib < DIBTRESHOLD) || (*(dib+1) < DIBTRESHOLD) || (*(dib+2) < DIBTRESHOLD))
				setPixel_BC(frame,x+ox,y2+oy,STYLE);

			dib+=3;
		}
	}
	return 1;
}


static inline int _32bppDib1bppFrame (TFRAME *frame, ubyte *data, TBMP *bmp, int ox, int oy)
{

	ubyte *dib;
	int x,y;
	const int LineWidth = dibWidthPadded(bmp->Width<<2);
	
	int y2=0;
	for (y=bmp->Height;y--;y2++){
		dib = data+bmp->dataOffset+(y*LineWidth);
		for (x=0;x<bmp->Width;x++){
			if ((*dib < DIBTRESHOLD) || (*(dib+1) < DIBTRESHOLD) || (*(dib+2) < DIBTRESHOLD) || (*(dib+3) < DIBTRESHOLD))
				setPixel_BC(frame,x+ox,y2+oy,STYLE);

			dib+=4;
		}
	}
	return 1;
}
#endif
static inline int _getPixel24 (TFRAME *frame, int x, int y)
{
#if (!__BUILD_2432BITONLY_SUPPORT__)
	int c = 0;
#endif
	
	switch (frame->bpp){
#if (!__BUILD_2432BITONLY_SUPPORT__)
	  case LFRM_BPP_1: 
	  	//c = 240-(getPixel_NB(frame, x, y)*140);
	  	
	  	if (!getPixel_NB(frame, x, y)){
	  		if (frame->hw->render)
	  			c = frame->hw->render->backGround*255;
	  		else
	  			c = 0x00;
	  	}else{
	  		if (frame->hw->render)
	  			c = frame->hw->render->foreGround*255;
	  		else
	  			c = 0xFF;
	  	}
	  	return c<<16|c<<8|c;

	  case LFRM_BPP_12:
	  	c = l_getPixel_NB(frame, x, y);
	  	return ((c&0xF00)<<12)|((c&0x0F0)<<8)|((c&0x00F)<<4);
	  	
	  case LFRM_BPP_8:
	  	c = l_getPixel_NB(frame, x, y);
	  	return (c&0xE0)<<16|(c&0x1C)<<11|(c&0x03)<<6;

	  case LFRM_BPP_15: 
		c = l_getPixel_NB(frame, x, y);
	  	return (c&0x7C00)<<9|(c&0x3E0)<<6|(c&0x1F)<<3;
  	
  	  case LFRM_BPP_16: 
		c = l_getPixel_NB(frame, x, y);
	  	return (c&0xF800)<<8|(c&0x7E0)<<5|(c&0x1F)<<3;
#endif
  	  case LFRM_BPP_24:
	  case LFRM_BPP_32: 
	  case LFRM_BPP_32A:
	  	return l_getPixel_NB(frame, x, y)&0xFFFFFF;
	 }
	 
	 return 0;
}

int saveBmp (TFRAME *frame, const wchar_t *filename, int width, int height)
{
	if (!frame || !filename)
		return 0;

	const double yscale = frame->height / (double)height;
	const double xscale = frame->width / (double)width;
	const int rowwidth = pad_bytes(width);
	const unsigned int dataOffset = BMPHEADERSIZE;
	int trowpos = sizeof(TBMP);
	int pos, pixel;
	double x,y;

	FILE *fp = l_wfopen(filename, L"wb");
	if (!fp) return 0;

	ubyte *bmpa = (ubyte *)l_malloc(sizeof(TBMP)+(rowwidth * height));
	if (bmpa == NULL){
		l_fclose(fp);
		return 0;
	}

	if (yscale == 1.00 && xscale == 1.00){
		for (y = frame->height-1; y >= 0; y--){
			pos = trowpos;
			for (x = 0; x < frame->width; x++){
				pixel = _getPixel24(frame, x, y);
				*(bmpa+pos++) = (pixel&0x0000FF);		// blue
				*(bmpa+pos++) = (pixel&0x00FF00)>>8;	// green
				*(bmpa+pos++) = (pixel&0xFF0000)>>16;	// red
			}
			trowpos += rowwidth;
		}		
	}else{
		TFRAME *framevflip = _newFrame(frame->hw, frame->width, frame->height, 1, frame->bpp);
		if (framevflip == NULL){
			l_free(bmpa);
			l_fclose(fp);
			return 0;		
		}
		flipFrame(frame, framevflip, FF_VERTICAL);

		for (y=0; y<(double)frame->height; y+=yscale){
			pos = trowpos;
			for (x=0; x<(double)frame->width; x+=xscale){
				pixel = _getPixel24(framevflip, x, y);
				*(bmpa+pos++) = (pixel&0x0000FF);		// blue
				*(bmpa+pos++) = (pixel&0x00FF00)>>8;	// green
				*(bmpa+pos++) = (pixel&0xFF0000)>>16;	// red
			}
			trowpos += rowwidth;
		}
		deleteFrame(framevflip);
	}
	
	
	TBMP *dib = (TBMP*)bmpa;
	l_memset(dib, 0, sizeof(TBMP));

	dib->bm[0] = 'B';
	dib->bm[1] = 'M';
	dib->fsize = dataOffset + trowpos;
	dib->temp = 0;
	dib->dataOffset = dataOffset;
	dib->HInfo = 40;
	dib->Width = width;
	dib->Height = height;
	dib->Planes = 1;
	dib->bpp = 24;
	dib->compression = 0;

	l_fseek(fp,0, SEEK_SET); 
    l_fwrite(&dib->bm[0], sizeof(ubyte), trowpos+/*sizeof(TBMP)*/BMPHEADERSIZE, fp);
    l_fclose(fp);
    l_free(bmpa);
    return 1;
}


static inline int pad_bytes (int width)
{
	return (width * 3 + 3) & ~3;
}

static inline int dibWidthPadded (int width)
{
	if (width&3)
		return ((width>>2)<<2) + 4;
	else 
		return width;
}

#else

int loadBmp (TFRAME *frame, const wchar_t* filename, int style){return 0;}
int saveBmp (TFRAME *frame, const wchar_t* filename, int width, int height){return 0;}

int bmpGetMetrics (const wchar_t *filename, int *width, int *height, int *bpp){return 0;}

#endif
