
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


#include <math.h>
#include "mylcd.h"
#include "memory.h"
#include "pixel.h"
#include "misc.h"
#include "copy.h"
#include "mmx.h"

static int flipFrameV (TFRAME *src, TFRAME *des);
static int flipFrameH (TFRAME *src, TFRAME *des);
static int flipFrameHV (TFRAME *src, TFRAME *des);

static int copyAreaExScaleHV (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2, int scaleh, int scalev, int style);
static int copyAreaExNoScale (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2, int style);
int copyArea_glyph_gp (TLPRINTREGION *loc);


static inline int getPixel32a_BL (TFRAME *frame, float x, float y)
{

	#define PSEUDO_FLOOR( V ) ((V) >= 0 ? (int)(V) : (int)((V) - 1))
	
	const int width = frame->width;
	const int height = frame->height;
	const int pitch = frame->pitch;
	const int spp = 4;
		
	//if (y < 0) y = 0;
	//if (x < 0) x = 0;

	float x1 = PSEUDO_FLOOR(x);
	float x2 = x1+1.0;
	
	if (x2 >= width) {
		x = x2 = (float)width-1.0;
		x1 = x2 - 1;
	}
	const int wx1 = (int)((x2 - x)*256.0);
	const int wx2 = 256-wx1;
	float y1 = PSEUDO_FLOOR(y);
	float y2 = y1+1.0;
	
	if (y2 >= height) {
		y = y2 = (float)height-1.0;
		y1 = y2 - 1;
	}
	const int wy1 = (int)(256*(y2 - y));
	const int wy2 = 256 - wy1;
	const int wx1y1 = wx1*wy1;
	const int wx2y1 = wx2*wy1;
	const int wx1y2 = wx1*wy2;
	const int wx2y2 = wx2*wy2;

	const unsigned char *px1y1 = &frame->pixels[pitch * (int)y1 + spp * (int)x1];
	const unsigned char *px2y1 = px1y1 + spp;
	const unsigned char *px1y2 = px1y1 + pitch;
	const unsigned char *px2y2 = px1y1 + pitch+spp;

	const TCOLOUR4 *restrict cx1y1 = (TCOLOUR4*)px1y1;
	const TCOLOUR4 *restrict cx2y1 = (TCOLOUR4*)px2y1;
	const TCOLOUR4 *restrict cx1y2 = (TCOLOUR4*)px1y2;
	const TCOLOUR4 *restrict cx2y2 = (TCOLOUR4*)px2y2;
	
	TCOLOUR4 c;
	c.u.bgra.r = (wx1y1 * cx1y1->u.bgra.r + wx2y1 * cx2y1->u.bgra.r + wx1y2 * cx1y2->u.bgra.r + wx2y2 * cx2y2->u.bgra.r + 32768) / 65536;
	c.u.bgra.g = (wx1y1 * cx1y1->u.bgra.g + wx2y1 * cx2y1->u.bgra.g + wx1y2 * cx1y2->u.bgra.g + wx2y2 * cx2y2->u.bgra.g + 32768) / 65536;
	c.u.bgra.b = (wx1y1 * cx1y1->u.bgra.b + wx2y1 * cx2y1->u.bgra.b + wx1y2 * cx1y2->u.bgra.b + wx2y2 * cx2y2->u.bgra.b + 32768) / 65536;
	c.u.bgra.a = (wx1y1 * cx1y1->u.bgra.a + wx2y1 * cx2y1->u.bgra.a + wx1y2 * cx1y2->u.bgra.a + wx2y2 * cx2y2->u.bgra.a + 32768) / 65536;
	return c.u.colour;
}

static int moveAreaLeft (TFRAME *frm, int x1, int y1, int x2, int y2, int pixels, const int mode)
{

	int p=0,x,y,l;
	y2 = MIN(y2+1,frm->height);
	x2 = MIN(x2,frm->width);
	
	for (l=0;l<pixels;l++){
		for (y=y1;y<y2;y++){
			if (mode == LMOV_LOOP)
				p = l_getPixel_NB(frm, x1,y);

			for (x=x1;x<x2;x++)
				l_setPixel(frm, x,y, l_getPixel_NB(frm,x+1,y));

			if (mode == LMOV_CLEAR)
				l_setPixel(frm, x2,y,LSP_CLEAR);
				
			else if (mode == LMOV_SET)
				l_setPixel(frm, x2,y,LSP_SET);
				
			else if (mode == LMOV_LOOP)
				l_setPixel(frm,x2,y,p);
				
			//else if (mode == LMOV_BIN)
			//{}
		}
	}
	return 1;
}

static int moveAreaRight (TFRAME *frm, int x1, int y1, int x2, int y2, int pixels, const int mode)
{
	
	int p=0,x,y,l;
	y2 = MIN(y2+1,frm->height);
	x2 = MIN(x2,frm->width);
	
	for (l=0;l<pixels;l++){
		for (y=y1;y<y2;y++){
			if (mode==LMOV_LOOP)
				p = l_getPixel_NB(frm,x2,y);

			for (x=x2;x>x1;x--)
				l_setPixel(frm,x,y, l_getPixel_NB(frm,x-1,y));

			if (mode == LMOV_CLEAR)
				l_setPixel(frm,x1,y,LSP_CLEAR);
			else if (mode == LMOV_SET)
				l_setPixel(frm,x1,y,LSP_SET);
			else if (mode == LMOV_LOOP)
				l_setPixel(frm,x1,y,p);
			//else if (mode == LMOV_BIN)
			//{}
		}
	}
	return 1;
}

static int moveAreaUp (TFRAME *frm, int x1, int y1, int x2, int y2, int pixels, const int mode)
{
	int p=0,x,y,l;
	y2 = MIN(y2,frm->height);
	x2 = MIN(x2+1,frm->width);
	
	for (l=0;l<pixels;l++){
		for (x=x1;x<x2;x++){
			
			if (mode==LMOV_LOOP)
				p = l_getPixel_NB(frm, x, y1);

			for (y=y1;y<y2;y++)
				l_setPixel(frm, x, y, 0xFF000000|l_getPixel_NB(frm, x, y+1));

			if (mode == LMOV_CLEAR)
				l_setPixel(frm, x, y2, LSP_CLEAR);
				
			else if (mode == LMOV_SET)
				l_setPixel(frm, x, y2, LSP_SET);
				
			else if (mode == LMOV_LOOP)
				l_setPixel(frm, x, y2, p);
				
				
			//else if (mode == LMOV_BIN)
			//{}
		}
	}

	return 1;
}

static int moveAreaDown (TFRAME *frm, int x1, int y1, int x2, int y2, int pixels, const int mode)
{
	int p=0,x,y,l;
	y2 = MIN(y2,frm->height);
	x2 = MIN(x2+1,frm->width);

	for (l=0;l<pixels;l++){
		for (x=x1;x<x2;x++){
			if (mode==LMOV_LOOP)
				p = l_getPixel_NB(frm,x,y2);

			for (y=y2;y>y1;y--)
				l_setPixel(frm,x,y,l_getPixel_NB(frm,x,y-1));

			if (mode == LMOV_CLEAR)
				l_setPixel(frm,x,y1,LSP_CLEAR);
			else if (mode == LMOV_SET)
				l_setPixel(frm,x,y1,LSP_SET);
			else if (mode == LMOV_LOOP)
				l_setPixel(frm,x,y1,p);
			//else if (mode == LMOV_BIN)
			//{}
		}
	}
	return 1;
}

int moveArea (TFRAME *frm, int x1, int y1, int x2, int y2, int tpixels, const int mode, const int dir)
{
	int ret = 0;
	switch (dir){
	  case LMOV_LEFT:
	  	ret = moveAreaLeft(frm, x1, y1, x2, y2, tpixels, mode);
	  	break;
	  case LMOV_RIGHT:
	  	ret = moveAreaRight(frm, x1, y1, x2, y2, tpixels, mode);
	  	break;
	  case LMOV_UP:
	  	ret = moveAreaUp(frm, x1, y1, x2, y2, tpixels, mode);
	  	break;
	  case LMOV_DOWN:
	  	ret = moveAreaDown(frm, x1, y1, x2, y2, tpixels, mode);
	  	break;
	}
	return ret;
}

int copyAreaScaled (TFRAME *from, TFRAME *to, const int src_x, const int src_y, const int src_width, const int src_height, const int dest_x, const int dest_y, const int dest_width, const int dest_height, const int style)
{
	const float scalex = (float)dest_width / (float)src_width;
	const float scaley = (float)dest_height / (float)src_height;
	float x2,y2;
	int x,y;

	if (style == LCASS_CPY && from->bpp == LFRM_BPP_32A){
		for (y=dest_y; y<dest_y+dest_height; y++){
			y2 = (float)src_y+(float)(y-dest_y)/scaley;
			for (x=dest_x; x<dest_x+dest_width; x++){
				x2 = (float)src_x+(float)(x-dest_x)/scalex;
				l_setPixel(to, x, y, getPixel32a_BL(from, x2, y2));
			}
		}
	}else if (style == LCASS_CPY && to->bpp == LFRM_BPP_32A){
		for (y=dest_y; y<dest_y+dest_height; y++){
			y2 = (float)src_y+(float)(y-dest_y)/scaley;
			for (x=dest_x; x<dest_x+dest_width; x++){
				x2 = (float)src_x+(float)(x-dest_x)/scalex;
				l_setPixel(to, x, y, RGB_32A_ALPHA | l_getPixel(from,(int)x2,(int)y2));
			}
		}
	}else if (style == LCASS_CPY){
		for (y=dest_y; y<dest_y+dest_height; y++){
			y2 = (float)src_y+(float)(y-dest_y)/scaley;
			for (x=dest_x; x<dest_x+dest_width; x++){
				x2 = (float)src_x+(float)(x-dest_x)/scalex;
				l_setPixel(to, x, y, l_getPixel(from,(int)x2,(int)y2));
			}
		}
	}else if (style == LCASS_OR){
		for (y=dest_y; y<dest_y+dest_height; y++){
			y2 = (float)src_y+(float)(y-dest_y)/scaley;
			for (x=dest_x; x<dest_x+dest_width; x++){
				x2 = (float)src_x+(float)(x-dest_x)/scalex;
				l_setPixel(to, x, y, l_getPixel(to,x,y) | l_getPixel(from,(int)x2,(int)y2));
			}
		}
	}else if (style == LCASS_AND){
		for (y=dest_y; y<dest_y+dest_height; y++){
			y2 = (float)src_y+(float)(y-dest_y)/scaley;
			for (x=dest_x; x<dest_x+dest_width; x++){
				x2 = (float)src_x+(float)(x-dest_x)/scalex;
				l_setPixel(to, x, y, l_getPixel(to,x,y) & l_getPixel(from,(int)x2,(int)y2));
			}
		}		
	}else if (style == LCASS_XOR){
		for (y=dest_y; y<dest_y+dest_height; y++){
			y2 = (float)src_y+(float)(y-dest_y)/scaley;
			for (x=dest_x; x<dest_x+dest_width; x++){
				x2 = (float)src_x+(float)(x-dest_x)/scalex;
				l_setPixel(to, x, y, l_getPixel(to,x,y) ^ l_getPixel(from,(int)x2,(int)y2));
			}
		}
	}else if (style == LCASS_NXOR){
		for (y=dest_y; y<dest_y+dest_height; y++){
			y2 = (float)src_y+(float)(y-dest_y)/scaley;
			for (x=dest_x; x<dest_x+dest_width; x++){
				x2 = (float)src_x+(float)(x-dest_x)/scalex;
				l_setPixel(to, x, y, 1 ^ (l_getPixel(to,x,y) | l_getPixel(from,(int)x2,(int)y2)));
			}
		}
	}else if (style == LCASS_NOT){
		for (y=dest_y; y<dest_y+dest_height; y++){
			y2 = (float)src_y+(float)(y-dest_y)/scaley;
			for (x=dest_x; x<dest_x+dest_width; x++){
				x2 = (float)src_x+(float)(x-dest_x)/scalex;
				l_setPixel(to, x, y, ~l_getPixel(from,(int)x2,(int)y2));
			}
		}
	}else if (style == LCASS_CLEAR){
		for (y=dest_y; y<dest_y+dest_height; y++){
			y2 = (float)src_y+(float)(y-dest_y)/scaley;
			for (x=dest_x; x<dest_x+dest_width; x++){
				x2 = (float)src_x+(float)(x-dest_x)/scalex;
				l_setPixel(to, x, y, to->hw->render->backGround);
			}
		}
	}else{
		return 0;
	}
	return 1;
}

int copyAreaA (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2, float alpha)
{
	const float a1 = alpha;
	const float a2 = 1.0 - alpha;
	float r1, r2;
	float g1, g2;
	float b1, b2;
	int x, y, xx = dx;
	x2++; y2++;
		
	for (y=y1; y<y2; y++,dy++){
		xx=dx;
		for (x=x1; x<x2; x++,xx++){
			l_getPixelf(from, x, y, &r1, &g1, &b1);	// src
			l_getPixelf(to, xx, dy, &r2, &g2, &b2);	// des
			l_setPixelf(to, xx, dy, (r1 * a1) + (r2 * a2), (g1 * a1) + (g2 * a2), (b1 * a1) + (b2 * a2));
		}
	}
	return 1;
}

int copyFrame (TFRAME *from, TFRAME *to)
{
	return copyAreaEx(from,to,0,0,0,0,MIN(from->width-1,to->width-1),MIN(from->height-1,to->height-1),1,1,LCASS_CPY);
}

#ifdef HAVE_3DNOW


#define DUFFS_LOOP8(pixel_copy_increment, width)	\
{	int n = (width+7)>>3;							\
	switch (width&7) {								\
	case 0: do {pixel_copy_increment;				\
	case 7:		pixel_copy_increment;				\
	case 6:		pixel_copy_increment;				\
	case 5:		pixel_copy_increment;				\
	case 4:		pixel_copy_increment;				\
	case 3:		pixel_copy_increment;				\
	case 2:		pixel_copy_increment;				\
	case 1:		pixel_copy_increment;				\
		} while ( --n > 0 );						\
	}												\
}

#define DUFFS_LOOP4(pixel_copy_increment, width)	\
{   int n = (width+3)>>2;							\
	 switch (width&3) {								\
	case 0: do {pixel_copy_increment;				\
	case 3:		pixel_copy_increment;				\
	case 2:		pixel_copy_increment;				\
	case 1:		pixel_copy_increment;				\
		} while ( --n > 0 );						\
	}												\
}

static inline int BlitRGBtoRGBPixelAlphaMMX3DNOW4 (TFRAME *src, TFRAME *des, int s_x1, int s_y1, int s_x2, int s_y2, int d_x, int d_y)
{
	if (d_x < 0){ 
		s_x1 += abs(d_x);
		d_x = 0;
	}
	if (s_x1 < 0) s_x1 = 0;
	int width = (s_x2-s_x1)+1;
	if (d_x+width >= des->width-1) width = des->width - d_x;
	if (width&3) return -1;
	
	if (d_y < 0){
		s_y1 += abs(d_y);
		d_y = 0;
	}
	if (s_y1 < 0) s_y1 = 0;
	int height = (s_y2-s_y1)+1;
	if (d_y+height >= des->height-1) height = des->height - d_y;
	if (height < 0) return -2;

	const uint32_t spitch = src->pitch>>2;
	const uint32_t dpitch = des->pitch>>2;	
	uint32_t *_srcp = (uint32_t*)lGetPixelAddress(src, s_x1, s_y1);
	uint32_t *_dstp = (uint32_t*)lGetPixelAddress(des, d_x, d_y);
	uint32_t *srcp = _srcp;
	uint32_t *dstp = _dstp;
	uint32_t amask = RGB_32A_ALPHA;
	uint32_t Ashift = 24;
	
	__asm volatile (
	/* make mm6 all zeros. */
	"pxor       %%mm6, %%mm6\n"
	
	/* Make a mask to preserve the alpha. */
	"movd      %0, %%mm7\n"           /* 0000F000 -> mm7 */
	"punpcklbw %%mm7, %%mm7\n"        /* FF000000 -> mm7 */
	"pcmpeqb   %%mm4, %%mm4\n"        /* FFFFFFFF -> mm4 */
	"movq      %%mm4, %%mm3\n"        /* FFFFFFFF -> mm3 (for later) */
	"pxor      %%mm4, %%mm7\n"        /* 00FFFFFF -> mm7 (mult mask) */

	/* form channel masks */
	"movq      %%mm7, %%mm4\n"        /* 00FFFFFF -> mm4 */
	"packsswb  %%mm6, %%mm4\n"        /* 00000FFF -> mm4 (channel mask) */
	"packsswb  %%mm6, %%mm3\n"        /* 0000FFFF -> mm3 */
	"pxor      %%mm4, %%mm3\n"        /* 0000F000 -> mm3 (~channel mask) */
	
	/* get alpha channel shift */
	"movd      %1, %%mm5\n" /* Ashift -> mm5 */

	  : /* nothing */ : "rm" (amask), "rm" ((uint32_t) Ashift) );

	while (height--) {
		srcp = _srcp;
		dstp = _dstp;
		
	    DUFFS_LOOP4({
		uint32_t alpha;

		alpha = *srcp & amask;
		/* FIXME: Here we special-case opaque alpha since the
		   compositioning used (>>8 instead of /255) doesn't handle
		   it correctly. Also special-case alpha=0 for speed?
		   Benchmark this! */
		   
		__asm__ __volatile__ ("prefetchnta 224(%0)\n" :: "r" (srcp) : "memory");
		
		if (!alpha){
		    /* do nothing */
#if 1
		}else if (alpha == amask){
			/* opaque alpha -- copy RGB, keep alpha */
		    /* using MMX here to free up regular registers for other things */
			    __asm volatile (
		    "movd      (%0),  %%mm0\n" /* src(ARGB) -> mm0 (0000ARGB)*/
		    "movd      (%1),  %%mm1\n" /* dst(ARGB) -> mm1 (0000ARGB)*/
#if 1  /* keep src alpha */
		    "movd       %2,   %%mm1\n"	 
#else  /* keep dst alpha */
		    "pand      %%mm4, %%mm0\n" /* src & chanmask -> mm0 */
		    "pand      %%mm3, %%mm1\n" /* dst & ~chanmask -> mm2 */
#endif
		    "por       %%mm0, %%mm1\n" /* src | dst -> mm1 */
		    "movd      %%mm1, (%1) \n" /* mm1 -> dst */
		    
		     : : "r" (srcp), "r" (dstp), "r"(alpha));
#endif
		}else{

			    __asm volatile(

		    /* load in the source, and dst. */
		    "movd      (%0), %%mm0\n"		    /* mm0(s) = 0 0 0 0 | As Rs Gs Bs */
		    "movd      (%1), %%mm1\n"		    /* mm1(d) = 0 0 0 0 | Ad Rd Gd Bd */

		    /* Move the src alpha into mm2 */

			/* if supporting pshufw */
#if 1
		    "pshufw     $0x55, %%mm0, %%mm2\n"  /* mm2 = 0 As 0 As |  0 As  0  As */
		    "psrlw     $8, %%mm2\n" 
		    
#else
		    "movd       %2,    %%mm2\n"
		    "psrld      %%mm5, %%mm2\n"                /* mm2 = 0 0 0 0 | 0  0  0  As */
		    "punpcklwd	%%mm2, %%mm2\n"	            /* mm2 = 0 0 0 0 |  0 As  0  As */
		    "punpckldq	%%mm2, %%mm2\n"             /* mm2 = 0 As 0 As |  0 As  0  As */
		   	"pand       %%mm7, %%mm2\n"              /* to preserve dest alpha */
#endif
		    /* move the colors into words. */
		    "punpcklbw %%mm6, %%mm0\n"		    /* mm0 = 0 As 0 Rs | 0 Gs 0 Bs */
		    "punpcklbw %%mm6, %%mm1\n"              /* mm0 = 0 Ad 0 Rd | 0 Gd 0 Bd */

		    /* src - dst */
		    "psubw    %%mm1, %%mm0\n"		    /* mm0 = As-Ad Rs-Rd | Gs-Gd  Bs-Bd */

		    /* A * (src-dst) */
		    "pmullw    %%mm2, %%mm0\n"		    /* mm0 = 0*As-d As*Rs-d | As*Gs-d  As*Bs-d */
		    "psrlw     $8,    %%mm0\n"		    /* mm0 = 0>>8 Rc>>8 | Gc>>8  Bc>>8 */
		    "paddb     %%mm1, %%mm0\n"		    /* mm0 = 0+Ad Rc+Rd | Gc+Gd  Bc+Bd */

		    "packuswb  %%mm0, %%mm0\n"              /* mm0 =             | Ac Rc Gc Bc */
		    
		    "movd      %%mm0, (%1)\n"               /* result in mm0 */

		     : : "r" (srcp), "r" (dstp), "r" (alpha) );

		}
		++srcp;
		++dstp;
		}, width);
			
	    _srcp += spitch;
	    _dstp += dpitch;
	    
		__asm volatile ("prefetch 128(%0)\n" :: "r" (dstp) : "memory");
	}

	__asm__ __volatile__ ("sfence":::"memory");
    __asm__ __volatile__ ("femms":::"memory");
    return 1;
}


static inline int BlitRGBtoRGBPixelAlphaMMX3DNOW8 (TFRAME *src, TFRAME *des, int s_x1, int s_y1, int s_x2, int s_y2, int d_x, int d_y)
{
	if (d_x < 0){ 
		s_x1 += abs(d_x);
		d_x = 0;
	}
	if (s_x1 < 0) s_x1 = 0;
	int width = (s_x2-s_x1)+1;
	if (d_x+width >= des->width-1) width = des->width - d_x;
	if (width&7) return -1;
	
	if (d_y < 0){
		s_y1 += abs(d_y);
		d_y = 0;
	}
	if (s_y1 < 0) s_y1 = 0;
	int height = (s_y2-s_y1)+1;
	if (d_y+height >= des->height-1) height = des->height - d_y;
	if (height < 0) return -2;

	const uint32_t spitch = src->pitch>>2;
	const uint32_t dpitch = des->pitch>>2;	
	uint32_t *_srcp = (uint32_t*)lGetPixelAddress(src, s_x1, s_y1);
	uint32_t *_dstp = (uint32_t*)lGetPixelAddress(des, d_x, d_y);
	uint32_t *srcp = _srcp;
	uint32_t *dstp = _dstp;
	uint32_t amask = RGB_32A_ALPHA;
	uint32_t Ashift = 24;
	
	__asm volatile (
	/* make mm6 all zeros. */
	"pxor       %%mm6, %%mm6\n"
	
	/* Make a mask to preserve the alpha. */
	"movd      %0, %%mm7\n"           /* 0000F000 -> mm7 */
	"punpcklbw %%mm7, %%mm7\n"        /* FF000000 -> mm7 */
	"pcmpeqb   %%mm4, %%mm4\n"        /* FFFFFFFF -> mm4 */
	"movq      %%mm4, %%mm3\n"        /* FFFFFFFF -> mm3 (for later) */
	"pxor      %%mm4, %%mm7\n"        /* 00FFFFFF -> mm7 (mult mask) */

	/* form channel masks */
	"movq      %%mm7, %%mm4\n"        /* 00FFFFFF -> mm4 */
	"packsswb  %%mm6, %%mm4\n"        /* 00000FFF -> mm4 (channel mask) */
	"packsswb  %%mm6, %%mm3\n"        /* 0000FFFF -> mm3 */
	"pxor      %%mm4, %%mm3\n"        /* 0000F000 -> mm3 (~channel mask) */
	
	/* get alpha channel shift */
	"movd      %1, %%mm5\n" /* Ashift -> mm5 */
	  : /* nothing */ : "rm" (amask), "rm" ((uint32_t) Ashift) );

	while (height--) {
		srcp = _srcp;
		dstp = _dstp;
		
	    DUFFS_LOOP8({
		uint32_t alpha;

		alpha = *srcp & amask;
		/* FIXME: Here we special-case opaque alpha since the
		   compositioning used (>>8 instead of /255) doesn't handle
		   it correctly. Also special-case alpha=0 for speed?
		   Benchmark this! */
		   
		  __asm volatile ("prefetchnta 160(%0)\n" :: "r" (srcp) : "memory");
		   
		if (!alpha){
		    /* do nothing */
#if 1
		}else if (alpha == amask){
			/* opaque alpha -- copy RGB, keep alpha */
		    /* using MMX here to free up regular registers for other things */
			    __asm volatile (
		    "movd      (%0),  %%mm0\n" /* src(ARGB) -> mm0 (0000ARGB)*/
		    "movd      (%1),  %%mm1\n" /* dst(ARGB) -> mm1 (0000ARGB)*/
#if 1  /* keep src alpha */
		    "movd       %2,   %%mm1\n"	 
#else  /* keep dst alpha */
		    "pand      %%mm4, %%mm0\n" /* src & chanmask -> mm0 */
		    "pand      %%mm3, %%mm1\n" /* dst & ~chanmask -> mm2 */
#endif
		    "por       %%mm0, %%mm1\n" /* src | dst -> mm1 */
		    "movd      %%mm1, (%1) \n" /* mm1 -> dst */
		    
		     : : "r" (srcp), "r" (dstp), "r"(alpha));
#endif
		}else{

			    __asm volatile(

		    /* load in the source, and dst. */
		    "movd      (%0), %%mm0\n"		    /* mm0(s) = 0 0 0 0 | As Rs Gs Bs */
		    "movd      (%1), %%mm1\n"		    /* mm1(d) = 0 0 0 0 | Ad Rd Gd Bd */

		    /* Move the src alpha into mm2 */

			/* if supporting pshufw */
#if 1
		    "pshufw     $0x55, %%mm0, %%mm2\n"  /* mm2 = 0 As 0 As |  0 As  0  As */
		    "psrlw     $8, %%mm2\n" 
		    
#else
		    "movd       %2,    %%mm2\n"
		    "psrld      %%mm5, %%mm2\n"                /* mm2 = 0 0 0 0 | 0  0  0  As */
		    "punpcklwd	%%mm2, %%mm2\n"	            /* mm2 = 0 0 0 0 |  0 As  0  As */
		    "punpckldq	%%mm2, %%mm2\n"             /* mm2 = 0 As 0 As |  0 As  0  As */
		   	"pand       %%mm7, %%mm2\n"              /* to preserve dest alpha */
#endif
		    /* move the colors into words. */
		    "punpcklbw %%mm6, %%mm0\n"		    /* mm0 = 0 As 0 Rs | 0 Gs 0 Bs */
		    "punpcklbw %%mm6, %%mm1\n"              /* mm0 = 0 Ad 0 Rd | 0 Gd 0 Bd */

		    /* src - dst */
		    "psubw    %%mm1, %%mm0\n"		    /* mm0 = As-Ad Rs-Rd | Gs-Gd  Bs-Bd */

		    /* A * (src-dst) */
		    "pmullw    %%mm2, %%mm0\n"		    /* mm0 = 0*As-d As*Rs-d | As*Gs-d  As*Bs-d */
		    "psrlw     $8,    %%mm0\n"		    /* mm0 = 0>>8 Rc>>8 | Gc>>8  Bc>>8 */
		    "paddb     %%mm1, %%mm0\n"		    /* mm0 = 0+Ad Rc+Rd | Gc+Gd  Bc+Bd */

		    "packuswb  %%mm0, %%mm0\n"              /* mm0 =             | Ac Rc Gc Bc */
		    
		    "movd      %%mm0, (%1)\n"               /* result in mm0 */

		     : : "r" (srcp), "r" (dstp), "r" (alpha) );

		}
		++srcp;
		++dstp;
		}, width);
	
	    _srcp += spitch;
	    _dstp += dpitch;

		__asm volatile ("prefetch 256(%0)\n" :: "r" (dstp) : "memory");
	}

	__asm__ __volatile__ ("sfence":::"memory");
    __asm__ __volatile__ ("femms":::"memory");
    return 1;
}
#endif

int copyAreaEx (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2, int scaleh, int scalev, int style)
{
#ifdef HAVE_3DNOW

	if (((from->bpp == LFRM_BPP_32A && (to->bpp == LFRM_BPP_32A || to->bpp == LFRM_BPP_32)) ||
		 (from->bpp == LFRM_BPP_32  && (to->bpp == LFRM_BPP_32A/* || to->bpp == LFRM_BPP_32*/))) &&
		(scaleh == 1 && scalev == 1 && style == LCASS_CPY)
	){
		const int ret = BlitRGBtoRGBPixelAlphaMMX3DNOW8(from, to, x1, y1, x2, y2, dx, dy);
		if (ret > 0){
			return 1;
		}else if (ret == -1){
			if (BlitRGBtoRGBPixelAlphaMMX3DNOW4(from, to, x1, y1, x2, y2, dx, dy) > 0)
				return 1;
		}else if (ret == -2){
			return 0;
		}
	}
#endif

#if 1
	if (from->bpp != LFRM_BPP_32A && to->bpp != LFRM_BPP_32A){
		if (style == LCASS_CPY && to->frameSize == from->frameSize && to->bpp == from->bpp){
			if (from->height == to->height && from->width == to->width && scaleh == 1 && scalev == 1){
				if (!dx && !dy && !x1 && !y1 && x2 == from->width-1 && y2 == from->height-1){
					void *top = to->pixels;
					void *fromp = from->pixels;
					size_t framesize = from->frameSize;
					l_memcpy(top, fromp, framesize);
					return 1;
				}
			}
		}
	}
#endif	
	if (scalev == 1 && scaleh == 1)
		return copyAreaExNoScale(from, to, dx, dy, x1, y1, x2, y2, style);
	else
		return copyAreaExScaleHV(from, to, dx, dy, x1, y1, x2, y2, scaleh, scalev, style);
}

static int copyAreaExNoScale (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2, const int style)
{
	int x, y, xx, pf;
	y2 = MIN(y2+1, from->height);
	x2 = MIN(x2+1, from->width);

	if (style == LCASS_CPY){
		for (y=y1; y<y2; y++, dy++){
			xx = dx;
			for (x=x1; x<x2; x++, xx++){
				pf = /*0xFF000000|*/l_getPixel_NB(from,x,y);
				l_setPixel(to,xx,dy, pf);
			}
		}
	}else if (style == LCASS_OR){
		for (y=y1; y<y2; y++, dy++){
			xx = dx;
			for (x=x1; x<x2; x++, xx++){
				pf = l_getPixel_NB(from,x,y);
				l_setPixel(to,xx,dy, l_getPixel(to,xx,dy) | pf);
			}
		}
	}else if (style == LCASS_AND){
		for (y=y1; y<y2; y++, dy++){
			xx = dx;
			for (x=x1; x<x2; x++, xx++){
				pf = l_getPixel_NB(from,x,y);
				l_setPixel(to,xx,dy, l_getPixel(to,xx,dy) & pf);
			}
		}
	}else if (style == LCASS_XOR){
		for (y=y1; y<y2; y++, dy++){
			xx = dx;
			for (x=x1; x<x2; x++, xx++){
				pf = l_getPixel_NB(from,x,y);
				l_setPixel(to,xx,dy, l_getPixel(to,xx,dy) ^ pf);
			}
		}
	}else if (style == LCASS_NXOR){
		for (y=y1; y<y2; y++, dy++){
			xx = dx;
			for (x=x1; x<x2; x++, xx++){
				pf = l_getPixel_NB(from,x,y);
				l_setPixel(to,xx,dy, ~(l_getPixel(to,xx,dy) | pf));
			}
		}
	}else if (style == LCASS_NOT){
		for (y=y1; y<y2; y++, dy++){
			xx = dx;
			for (x=x1; x<x2; x++, xx++){
				pf = l_getPixel_NB(from,x,y);
				l_setPixel(to,xx,dy, ~l_getPixel(to,xx,dy));
			}
		}
	}else if (style == LCASS_CLEAR){
		for (y=y1; y<y2; y++, dy++){
			xx = dx;
			for (x=x1; x<x2; x++, xx++){
				l_setPixel(to,xx,dy,0);
			}
		}
	}
	return 1;
}

static int copyAreaExScaleHV (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2, int scaleh, int scalev, int style)
{
	int x,y;
	int xx;
	int sx=0,sy=0;
	int pf;
	y2 = MIN(y2+1, from->height);
	x2 = MIN(x2+1, from->width);

	for (y=y1; y<y2; y++, dy += scalev){
		xx=dx;

		if (style == LCASS_CPY){
			for (x=x1; x<x2; x++, xx += scaleh){
				pf = l_getPixel_NB(from,x,y);
				for (sy=0; sy<scalev; sy++){
					for (sx=0; sx<scaleh; sx++)
						l_setPixel(to,xx+sx,dy+sy, pf);
				}
			}
		}else if (style == LCASS_OR){
			for (x=x1; x<x2; x++, xx += scaleh){
				pf = l_getPixel_NB(from,x,y);
				for (sy=0; sy<scalev; sy++){
					for (sx=0; sx<scaleh; sx++)
						l_setPixel(to,xx+sx,dy+sy, l_getPixel(to,xx+sx,dy+sy) | pf);
				}
			}
		}else if (style == LCASS_AND){
			for (x=x1; x<x2; x++, xx += scaleh){
				pf = l_getPixel_NB(from,x,y);
				for (sy=0; sy<scalev; sy++){
					for (sx=0; sx<scaleh; sx++)
						l_setPixel(to,xx+sx,dy+sy, l_getPixel(to,xx+sx,dy+sy) & pf);
				}
			}
		}else if (style == LCASS_XOR){
			for (x=x1; x<x2; x++, xx += scaleh){
				pf = l_getPixel_NB(from,x,y);
				for (sy=0; sy<scalev; sy++){
					for (sx=0; sx<scaleh; sx++)
						l_setPixel(to,xx+sx,dy+sy, l_getPixel(to,xx+sx,dy+sy) ^ pf);
				}
			}
		}else if (style == LCASS_NXOR){
			for (x=x1; x<x2; x++, xx += scaleh){
				pf = l_getPixel_NB(from,x,y);
				for (sy=0; sy<scalev; sy++){
					for (sx=0; sx<scaleh; sx++)
						l_setPixel(to,xx+sx,dy+sy, ~(l_getPixel(to,xx+sx,dy+sy) | pf));
				}
			}
		}else if (style == LCASS_NOT){
			for (x=x1; x<x2; x++, xx += scaleh){
				pf = l_getPixel_NB(from,x,y);
				for (sy=0; sy<scalev; sy++){
					for (sx=0; sx<scaleh; sx++)
						l_setPixel(to,xx+sx,dy+sy, ~l_getPixel(to,xx+sx,dy+sy));
				}
			}
		}else if (style == LCASS_CLEAR){
			for (x=x1; x<x2; x++, xx += scaleh){
				for (sy=0; sy<scalev; sy++){
					for (sx=0; sx<scaleh; sx++)
						l_setPixel(to,xx+sx,dy+sy,0);
				}
			}
		}
	}
	return 1;
}

int copyArea (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2)
{
	return copyAreaEx(from, to, dx, dy, x1, y1, x2, y2, 1, 1, LCASS_CPY);
}

int flipFrame (TFRAME *src, TFRAME *des, const int flag)
{
	if (flag&FF_VERTANDHORIZ)
		return flipFrameHV(src,des);
	else if (flag&FF_VERTICAL)
		return flipFrameV(src,des);
	else if (flag&FF_HORIZONTAL)
		return flipFrameH(src,des);
	else
		return 0;
}

static int flipFrameV (TFRAME *src, TFRAME *des)
{

	int x,y;
	int y2=src->height-1;
		
	for (y=0;y<src->height;y++){
		for (x=0;x<src->width;x++)
			l_setPixel(des,x,y2,l_getPixel_NB(src,x,y));
		y2--;
	}
	return 1;
}

static int flipFrameH (TFRAME *src, TFRAME *des)
{

	int x,y;
	int x2=src->width-1;
	
	for (x=0;x<src->width;x++){
		for (y=0;y<src->height;y++)
			l_setPixel(des,x2,y,l_getPixel_NB(src,x,y));
		x2--;
	}
	return 1;
}

static int flipFrameHV (TFRAME *src, TFRAME *des)
{

	int x,y;
	int x2=des->width-1;
	int y2=des->height-1;
	
	for (x=0;x<src->width;x++){
		y2=des->height-1;
		for (y=0;y<src->height;y++){
			l_setPixel(des,x2,y2,l_getPixel_NB(src,x,y));
			y2--;
		}
		x2--;
	}
	return 1;
}
