
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
#include "memory.h"
#include "apilock.h"
#include "frame.h"
#include "pixel.h"
#include "copy.h"
#include "convert.h"
#include <string.h>

#include "sync.h"
#include "misc.h"

#ifdef USE_MMX
#include "mmx.h"
#include "mmx_rgb.h"
#endif





unsigned int frame1BPPToRGB (TFRAME *frame, void *des, int des_bpp, const int clrLow, const int clrHigh)
{
	int ret = 0;
#if (!__BUILD_2432BITONLY_SUPPORT__)
	if (frame->bpp == LFRM_BPP_1){
		switch(des_bpp){
		  case LFRM_BPP_1: ret = frame1To1BPP(frame, des); break;
		  case LFRM_BPP_8: ret = frame1To8BPP(frame, des, clrLow, clrHigh); break;
		  case LFRM_BPP_12: ret = frame1ToRGB444(frame, des, clrLow, clrHigh); break;
		  case LFRM_BPP_15: ret = frame1ToRGB555(frame, des, clrLow, clrHigh); break;
		  case LFRM_BPP_16: ret = frame1ToRGB565(frame, des, clrLow, clrHigh); break;
		  case LFRM_BPP_24: ret = frame1ToRGB888(frame, des, clrLow, clrHigh); break;
		  case LFRM_BPP_32:
		  case LFRM_BPP_32A: ret = frame1ToARGB8888(frame, des, clrLow, clrHigh); break;
		  default: mylog("lFrame1BPPToRGB(): invalid bpp option: %i\n", des_bpp);
		}
	}
#endif
	return ret;
}

int convertFrame (TFRAME *src, TFRAME *des)
{
	if (src->bpp == des->bpp){
		if (src->width == des->width && src->height == des->height && src->frameSize == des->frameSize){
			l_memcpy(des->pixels, src->pixels, src->frameSize);
			return 1;
		}
	}
	return convertFrameArea(src, des, 0, 0, 0, 0, MIN(src->width-1, des->width-1), MIN(src->height-1, des->height-1));
}

int convertFrameArea (TFRAME *src, TFRAME *des, int dx, int dy, int x1, int y1, int x2, int y2)
{

	if (x1 < 0) x1 = 0;
	else if (x1 > src->width-1)
		return 0;
		
	if (x2 < 0) x2 = 0;
	else if (x2 > src->width-1)
		x2 = src->width-1;
			
	if (y1 < 0) y1 = 0;
	else if (y1 > src->height-1)
		return 0;

	if (y2 < 0) y2 = 0;
	else if (y2 > src->height-1)
		y2 = src->height-1;

#if (!__BUILD_2432BITONLY_SUPPORT__)
	if (src->bpp == LFRM_BPP_1 && des->bpp == LFRM_BPP_1){
		copyArea_fast8(src, des, dx, dy, x1, y1, x2, y2);
		return 1;
	}
#endif
	y2++;
	x2++;

	if (src->bpp == des->bpp && src->bpp != LFRM_BPP_1){
		int n;

		if (src->bpp == LFRM_BPP_32 || src->bpp == LFRM_BPP_32A)
			n = (x2-x1)<<2;
		else if (src->bpp == LFRM_BPP_24)
			n = (x2-x1)*3;
#if (!__BUILD_2432BITONLY_SUPPORT__)
		else if (src->bpp == LFRM_BPP_16)
			n = (x2-x1)<<1;
		else if (src->bpp == LFRM_BPP_8)
			n = (x2-x1);
		else if (src->bpp == LFRM_BPP_12 || src->bpp == LFRM_BPP_15)
			n = (x2-x1)<<1;
#endif
		else
			return 0;

		for (int y = y1; y<y2; y++, dy++)
			l_memcpy(l_getPixelAddress(des, dx, dy), l_getPixelAddress(src, x1, y), n);
	}else{
		const int tmpStyle = des->style;
		des->style = LSP_SET;
		
		for (int y = y1; y < y2; y++,dy++){
			int xx = dx;
			for (int x = x1; x < x2; x++, xx++)
				l_setPixel(des, xx, dy, l_getPixel(src, x, y));
		}
		des->style = tmpStyle;
	}
	return 1;
}
#if (!__BUILD_2432BITONLY_SUPPORT__)
unsigned int frame1To1BPP (TFRAME *frame, void *restrict des)
{
	ubyte *output = (ubyte* restrict)des;
	int x,y,bx;
	ubyte b;

	for (y=0; y<frame->height ;y++){
		for (x=0; x<frame->width ;x+=8){
			bx=x;
			for (b=0;b<8;b++){
				if (m_getPixel1_NB(frame,bx++,y))
					*output |= 1<<b;
			}
			output++;
		}
	}
	return (uintptr_t)output-(uintptr_t)des;
}

unsigned int frame1To8BPP (TFRAME *frame, void *restrict des, const ubyte clrLow, const ubyte clrHigh)
{
	ubyte *output = (ubyte *)des;
	int x,y;
	int row;

	for (y=0; y<frame->height ;y++){
		row = y*frame->pitch;

		for (x=0; x<frame->width ;x++){
			if (!m_getPixel1_NBr(frame, x, row))
				*(output++) = clrLow;
			else 
				*(output++) = clrHigh;
		}
	}
	return (uintptr_t)output-(uintptr_t)des;
}

unsigned int frame1ToRGB555 (TFRAME *frame, void *restrict des, const int clrLow, const int clrHigh)
{
	unsigned short *output = (unsigned short*restrict  )des;
	unsigned short cLow = (unsigned short)(clrLow&0x7FFF);
	unsigned short cHigh = (unsigned short)(clrHigh&0x7FFF);
	int x,y;

	for (y=0; y<frame->height ;y++){
		for (x=0; x<frame->width ;x++){
			if (!m_getPixel1_NB(frame,x,y))
				*(output++)=cLow;
			else 
				*(output++)=cHigh;
		}
	}

	return (uintptr_t)output-(uintptr_t)des;
}

unsigned int frame1ToRGB565 (TFRAME *frame, void *restrict des, const int clrLow, const int clrHigh)
{
	unsigned short *output = (unsigned short*restrict )des;
	unsigned short cLow = (unsigned short)(clrLow&0xFFFF);
	unsigned short cHigh = (unsigned short)(clrHigh&0xFFFF);
	int x,y;

	for (y=0; y<frame->height ;y++){
		for (x=0; x<frame->width ;x++){
			if (!m_getPixel1_NB(frame,x,y))
				*(output++)=cLow;
			else 
				*(output++)=cHigh;
		}
	}

	return (uintptr_t)output-(uintptr_t)des;
}

unsigned int frame1ToRGB444 (TFRAME *frame, void *restrict des, const int clrLow, const int clrHigh)
{
	unsigned short *output = (unsigned short*restrict )des;
	unsigned short cLow = (unsigned short)(clrLow&0xFFF);
	unsigned short cHigh = (unsigned short)(clrHigh&0xFFF);
	int x,y;

	for (y=0; y<frame->height ;y++){
		for (x=0; x<frame->width ;x++){
			if (!m_getPixel1_NB(frame,x,y))
				*(output++)=cLow;
			else 
				*(output++)=cHigh;
		}
	}

	return (uintptr_t)output-(uintptr_t)des;
}

unsigned int frame1ToRGB888 (TFRAME *frame, void *restrict des, const int clrLow, const int clrHigh)
{
	int *output = (int*restrict )des;
	int x,y;
	
	for (y=0; y<frame->height ;y++){
		for (x=0; x<frame->width ;x++){
			if (!m_getPixel1_NB(frame,x,y))
				*(output) = clrLow&0xFFFFFF;
			else
				*(output) = clrHigh&0xFFFFFF;
			output = (int*)((ubyte*)output + 3);
		}
	}
	return (uintptr_t)output-(uintptr_t)des;
}

unsigned int frame1ToARGB8888 (TFRAME *frame, void *restrict des, const int clrLow, const int clrHigh)
{
	int *output = (int*restrict )des;
	int x,y;

	for (y=0; y<frame->height ;y++){
		for (x=0; x<frame->width ;x++){
			if (!m_getPixel1_NB(frame,x,y))
				*(output++)=clrLow;
			else
				*(output++)=clrHigh;
		}
	}
	return (uintptr_t)output-(uintptr_t)des;
}
#endif // #if (!__BUILD_2432BITONLY_SUPPORT__)

#if 0
void buffer32ToFrame32 (unsigned int * restrict buffer, const int width, const int height, const int pitch, TFRAME *frm)
{


#ifdef USE_MMX
	if (0 && !(width&7)){
		printf("MMX width %i %i\n", width, width&7);
		rgb_32_to_32_mmx(buffer, width, height, frm->pixels, pitch, frm->pitch);
		
		emms();
		
		return;
	}
#endif

//	printf("STD width %i %i\n", width, width&7);

	uint8_t *des = (uint8_t*)m_getPixelAddr32(frm, 0, 0);
	uint8_t *src = (uint8_t*)buffer;

	int tpixels = frm->width * frm->height;
	while(tpixels--){
		*des++ = src[2];
		*des++ = src[1];
		*des++ = src[0];
		*des++ = src[3];
		src += 4;
	}
}
#endif


static void frame32ToBuffer24 (TFRAME *frm, unsigned int * restrict buffer)
{
#ifdef USE_MMX
	if (!(frm->width&7)){
		rgb_32_to_24_mmx(frm->pixels, frm->width, frm->height, buffer, frm->pitch, frm->width*3);
		return;
	}
#endif

	uint8_t *src = (uint8_t*)m_getPixelAddr32(frm, 0, 0);
	uint8_t *des = (uint8_t*)buffer;

	int tpixels = frm->width * frm->height;
	while(tpixels--){
		*des++ = src[2];
		*des++ = src[1];
		*des++ = src[0];
		src += 4;
	}
}

#if (!__BUILD_2432BITONLY_SUPPORT__)
/* fix and test me!! */
static void frame15ToBuffer24 (TFRAME *frm, unsigned int * restrict buffer)
{
#ifdef USE_MMX
	if (!(frm->width&7)){
		rgb_15_to_24_mmx(frm->pixels, frm->width, frm->height, buffer, frm->pitch, frm->width*3);
		return;
	}
#endif

	int x,y,r,g,b,c;

	for (y=0; y< frm->height; y++){
		for (x=0; x< frm->width; x++){
			c = m_getPixel15_NB(frm, x, y); 
			r = (c&0x7C00)<<9;		// red
			g = (c&0x03E0)<<6;		// green
			b = (c&0x001F)<<3;		// blue
			*(buffer+x) = (r|g|b)&0x00FFFFFF;
		}
		buffer += frm->width;
	}
}

static void frame1ToBuffer24 (TFRAME *frm, unsigned int * restrict buffer)
{
	const int high = (0xFFFFFF*frm->hw->render->foreGround)&0xFFFFFF;
	const int low = (0xFFFFFF*frm->hw->render->backGround)&0xFFFFFF;
	frame1ToRGB888(frm, buffer, low, high);
}

static void frame1ToBuffer32 (TFRAME *frm, unsigned int * restrict buffer)
{
	const int high = (0x00FFFFFF*frm->hw->render->foreGround)&0x00FFFFFF;
	const int low = (0x00FFFFFF*frm->hw->render->backGround)&0x00FFFFFF;
	frame1ToARGB8888(frm, buffer, low, high);
}

static void frame12ToBuffer24 (TFRAME *frm, unsigned int * restrict buffer)
{
	int x,y, c;

	for (y=0; y< frm->height; y++){
		for (x=0; x< frm->width; x++){
			c = m_getPixel12_NB(frm, x, y); 
			*(buffer+x) = ((c&0xF00)<<12|(c&0x0F0)<<8|(c&0x00F)<<4)&0x00FFFFFF;
		}
		buffer += frm->width;
	}
}

static void frame12ToBuffer32 (TFRAME *frm, unsigned int * restrict buffer)
{
	int x,y, c;

	for (y=0; y< frm->height; y++){
		for (x=0; x< frm->width; x++){
			c = m_getPixel12_NB(frm, x, y); 
			*(buffer+x) = (c&0xF00)<<12|(c&0x0F0)<<8|(c&0x00F)<<4;
		}
		buffer += frm->width;
	}
}

static void frame8ToBuffer24 (TFRAME *frm, unsigned int * restrict buffer)
{
	int x,y,c;

	for (y=0; y< frm->height; y++){
		for (x=0; x< frm->width; x++){
			c = m_getPixel8_NB(frm, x, y); 
			*(buffer+x) = (((c&0xE0)<<16)|((c&0x1C)<<11)|((c&0x03)<<6))&0x00FFFFFF;
		}
		buffer += frm->width;
	}
}

static void frame8ToBuffer16 (TFRAME *frm, unsigned int * restrict buffer)
{
	int x,y,c;
	uint16_t * restrict width = (uint16_t*)buffer;
	const unsigned int pitch = calcPitch(frm->width, LFRM_BPP_16);

	for (y=0; y< frm->height; y++){
		for (x=0; x< frm->width; x++){
			c = m_getPixel8_NB(frm, x, y); 
			*(width+x) = ((c&0xE0)<<8)|((c&0x1C)<<6)|((c&0x03)<<3);
		}
		width = (uint16_t*)((uint8_t*)width+pitch);
	}
}

static void frame8ToBuffer15 (TFRAME *frm, unsigned int * restrict buffer)
{
	int x,y,c;
	uint16_t * restrict width = (uint16_t*)buffer;
	const unsigned int pitch = calcPitch(frm->width, LFRM_BPP_15);

	for (y=0; y< frm->height; y++){
		for (x=0; x< frm->width; x++){
			c = m_getPixel8_NB(frm, x, y); 
			*(width+x) = ((c&0xE0)<<7)|((c&0x1C)<<5)|((c&0x03)<<3);
		}
		width = (uint16_t*)((uint8_t*)width+pitch);
	}
}

static void frame8ToBuffer12 (TFRAME *frm, unsigned int * restrict buffer)
{
	int x,y,c;
	uint16_t * restrict width = (uint16_t*)buffer;
	const unsigned int pitch = calcPitch(frm->width, LFRM_BPP_12);

	for (y=0; y< frm->height; y++){
		for (x=0; x< frm->width; x++){
			c = m_getPixel8_NB(frm, x, y); 
			*(width+x) = ((c&0xE0)<<4)|((c&0x1C)<<2)|((c&0x03)<<2);
		}
		width = (uint16_t*)((uint8_t*)width+pitch);
	}
}

static void frame8ToBuffer32 (TFRAME *frm, unsigned int * restrict buffer)
{
	int x,y,c;

	for (y=0; y< frm->height; y++){
		for (x=0; x< frm->width; x++){
			c = m_getPixel8_NB(frm, x, y); 
			*(buffer+x) = ((c&0xE0)<<16)|((c&0x1C)<<11)|((c&0x03)<<6);
		}
		buffer += frm->width;
	}
}

static void frame1ToBuffer16 (TFRAME *frm, unsigned int * restrict buffer)
{
	const int high = (0xFFFF*frm->hw->render->foreGround)&0xFFFF;
	const int low = (0xFFFF*frm->hw->render->backGround)&0xFFFF;
	frame1ToRGB565(frm, buffer, low, high);
}

static void frame1ToBuffer15 (TFRAME *frm, unsigned int * restrict buffer)
{
	const int high = (0x7FFF*frm->hw->render->foreGround)&0x7FFF;
	const int low = (0x7FFF*frm->hw->render->backGround)&0x7FFF;
	frame1ToRGB555(frm, buffer, low, high);
}

static void frame1ToBuffer12 (TFRAME *frm, unsigned int * restrict buffer)
{
	const int high = (0xFFF*frm->hw->render->foreGround)&0xFFF;
	const int low = (0xFFF*frm->hw->render->backGround)&0xFFF;
	frame1ToRGB444(frm, buffer, low, high);
}

static void frame15ToBuffer32 (TFRAME *frm, unsigned int * restrict buffer)
{
#ifdef USE_MMX
	if (!(frm->width&7)){
		rgb_15_to_32_mmx(frm->pixels, frm->width, frm->height, buffer, frm->pitch, frm->width*4);
		return;
	}
#endif

	int x,y;
	uint16_t *p;

	for (y=0; y < frm->height; y++){
		p = (uint16_t*)m_getPixelAddr16(frm, 0, y);
		for (x=0; x < frm->width; x++,p++)
			*(buffer+x) = (*p&0x7C00)<<9|(*p&0x03E0)<<6|(*p&0x001F)<<3;
		buffer += frm->width;
	}
} 

static void frame16ToBuffer24 (TFRAME *frm, unsigned int * restrict buffer)
{
#ifdef USE_MMX
	if (!(frm->width&7)){
		rgb_16_to_24_mmx(frm->pixels, frm->width, frm->height, buffer, frm->pitch, frm->width*3);
		return;
	}
#endif

	int x,y;
	uint16_t *p;

	for (y=0; y < frm->height; y++){
		p = (uint16_t*)m_getPixelAddr16(frm, 0, y);
		for (x=0; x < frm->width; x++,p++)
			*(buffer+x) = ((*p&0xF800)<<8|(*p&0x07E0)<<5|(*p&0x001F)<<3)&0x00FFFFFF;
		buffer += frm->width;
	}
}

static void frame16ToBuffer32 (TFRAME *frm, unsigned int * restrict buffer)
{
#ifdef USE_MMX
	if (!(frm->width&7)){
		rgb_16_to_32_mmx(frm->pixels, frm->width, frm->height, buffer, frm->pitch, frm->width*4);
		return;
	}
#endif

	int x,y;
	uint16_t *p;

	for (y=0; y < frm->height; y++){
		p = (uint16_t*)m_getPixelAddr16(frm, 0, y);
		for (x=0; x < frm->width; x++,p++)
			*(buffer+x) = (*p&0xF800)<<8|(*p&0x07E0)<<5|(*p&0x001F)<<3;
		buffer += frm->width;
	}
}
#endif // #if (!__BUILD_2432BITONLY_SUPPORT__)

#if 0
void buffer24ToFrame32A (unsigned int * restrict buffer, const int width, const int height, const int pitch, TFRAME *frm)
{
#ifdef USE_MMX
	if (0&&!(frm->width&7)){
		rgb_24_to_32_mmx(buffer, width, height, frm->pixels, pitch, frm->pitch);
		emms();
		//printf("buffer24ToFrame32 MMX\n");
		return;
	}
#endif


	//printf("buffer24ToFrame32 STD\n");
	
	uint8_t *des = (uint8_t*)m_getPixelAddr32(frm, 0, 0);
	uint8_t *src = (uint8_t*)buffer;

	int tpixels = frm->width * frm->height;
	while(tpixels--){
		*des++ = src[2];
		*des++ = src[1];
		*des++ = src[0];
		*des++ = 0xFF;
		src += 3;
	}
}
#endif

static void frame24ToBuffer32 (TFRAME *frm, unsigned int * restrict buffer)
{
#ifdef USE_MMX
	if (!(frm->width&7)){
		rgb_24_to_32_mmx(frm->pixels, frm->width, frm->height, buffer, frm->pitch, frm->width*4);
		return;
	}
#endif

	int x,y;
	unsigned int * restrict width;
	ubyte *src = frm->pixels;
	
	for (y=0; y< frm->height; y++){
		width = buffer + (y * frm->width);
		for (x=0; x< frm->width; x++){
			*(width+x) = src[0] | src[1]<<8 | src[2]<<16;
			src += 3;
		}
	}
}

void ABGRToARGB (unsigned int *buffer, size_t totalPixels)
{
	int i = totalPixels;
	int r,b;
	while(i--){
		b = (buffer[i]&0xFF0000)>>16;
		r = (buffer[i]&0x0000FF)<<16;
		buffer[i] = r|b|(buffer[i]&0x00FF00);
	}
}

static void frame24ToBuffer16 (TFRAME *frm, unsigned int * restrict buffer)
{
#ifdef USE_MMX
	if (!(frm->width&7)){
		rgb_24_to_16_mmx(frm->pixels, frm->width, frm->height, buffer, frm->pitch, frm->width*2);
		return;
	}
#endif

	unsigned int x,y,c,r,g,b;
	uint16_t * restrict width = (uint16_t*)buffer;
	const unsigned int pitch = calcPitch(frm->width, LFRM_BPP_16);
	
	for (y=0; y < frm->height; y++){
		for (x=0; x < frm->width; x++){
			c = m_getPixel24_NB(frm, x, y); 
			r = (c&0xF80000)>>8 ;	// 5
			g = (c&0x00FC00)>>5;	// 6
			b = (c&0x0000F8)>>3;	// 5
			*(width+x) = (r|g|b);
		}
		width = (uint16_t*)((uint8_t*)width+pitch);
	}
}

static void frame32ToBuffer16 (TFRAME *frm, unsigned int * restrict buffer)
{
#ifdef USE_MMX
	if (!(frm->width&7)){
		rgb_32_to_16_mmx(frm->pixels, frm->width, frm->height, buffer, frm->pitch, frm->width*2);
		return;
	}
#endif


	int x,y;
	uint16_t * restrict out = (uint16_t*)buffer;
	TRGBA * restrict in = (TRGBA*)frm->pixels;
	TRGBA * restrict p;
	const int h = frm->height;
	const int w = frm->width;
	const int cpitch = calcPitch(w, LFRM_BPP_16);
	const int fpitch = frm->pitch>>2;
	
	for (y = 0; y < h; y++){
		p = in+(y*fpitch);
		for (x = 0; x < w; x++)
			*(out+x) = (p[x].r&0xF8)>>3|(p[x].g&0xFC)<<3|(p[x].b&0xF8)<<8;

		out = (uint16_t*)((uint8_t*)out+cpitch);
	}
}

#if (!__BUILD_2432BITONLY_SUPPORT__)
static void frame24ToBuffer15 (TFRAME *frame, unsigned int * restrict buffer)
{
#ifdef USE_MMX
	if (!(frame->width&7)){
		rgb_24_to_15_mmx(frame->pixels, frame->width, frame->height, buffer, frame->pitch, frame->width*2);
		return;
	}
#endif

	int x,y,r,g,b,c;
	uint16_t * restrict out = (uint16_t*)buffer;
	const unsigned int pitch = calcPitch(frame->width, LFRM_BPP_15);

	for (y=0; y < frame->height; y++){
		for (x=0; x < frame->width; x++){
			c = m_getPixel24_NB(frame, x, y); 
			r = (c&0xF80000)>>9;	// 5
			g = (c&0x00F800)>>6;	// 5
			b = (c&0x0000F8)>>3;	// 5
			*(out+x) = r|g|b;
		}
		out = (uint16_t*)((uint8_t*)out+pitch);
	}
}

static void frame16ToBuffer15 (TFRAME *frm, unsigned int * restrict buffer)
{
#ifdef USE_MMX
	if (!(frm->width&7)){
		rgb_16_to_15_mmx(frm->pixels, frm->width, frm->height, buffer, frm->pitch, frm->width*2);
		return;
	}
#endif

	int x,y,r,g,b,c;
	uint16_t * restrict width = (uint16_t*)buffer;
	const unsigned int pitch = calcPitch(frm->width, LFRM_BPP_15);

	for (y=0; y < frm->height; y++){
		for (x=0; x < frm->width; x++){
			c = m_getPixel16_NB(frm, x, y); 
			r = (c&0xF800)>>1;	// 5
			g = (c&0x0F80)>>1;	// 5
			b = (c&0x00F8);		// 5
			*(width+x) = r|g|b;
		}
		width = (uint16_t*)((uint8_t*)width+pitch);
	}
}

static void frame32ToBuffer15 (TFRAME *frm, unsigned int * restrict buffer)
{
#ifdef USE_MMX
	if (!(frm->width&7)){
		rgb_32_to_15_mmx(frm->pixels, frm->width, frm->height, buffer, frm->pitch, frm->width*2);
		return;
	}
#endif
	frame24ToBuffer15(frm, buffer);

}

static void frame15ToBuffer16 (TFRAME *frm, unsigned int * restrict buffer)
{
#ifdef USE_MMX
	if (!(frm->width&7)){
		rgb_15_to_16_mmx(frm->pixels, frm->width, frm->height, buffer, frm->pitch, frm->width*2);
		return;
	}
#endif
	int x,y,r,g,b,c;
	uint16_t * restrict width = (uint16_t*)buffer;
	const unsigned int pitch = calcPitch(frm->width, LFRM_BPP_16);

	for (y=0; y< frm->height; y++){
		for (x=0; x< frm->width; x++){
			c = m_getPixel15_NB(frm, x, y); 
			r = (c&0x7C00)<<1;	// red			
			g = (c&0x03E0)<<1;	// green
			b = (c&0x001F);		// blue
			*(width+x) = r|g|b;
		}
		width = (uint16_t*)((uint8_t*)width+pitch);
	}
}

static void frame12ToBuffer16 (TFRAME *frm, unsigned int * restrict buffer)
{
	int x,y, c;
	uint16_t * restrict width = (uint16_t*)buffer;
	const unsigned int pitch = calcPitch(frm->width, LFRM_BPP_16);

	for (y=0; y < frm->height; y++){
		for (x=0; x < frm->width; x++){
			c = m_getPixel12_NB(frm, x, y); 
			*(width+x) = (c&0xF00)<<4|(c&0x0F0)<<3|(c&0x00F)<<1;
		}
		width = (uint16_t*)((uint8_t*)width+pitch);
	}
}

static void frame24ToBuffer12 (TFRAME *frame, unsigned int * restrict buffer)
{
	int x,y,r,g,b,c;
	uint16_t * restrict width = (uint16_t*)buffer;
	const unsigned int pitch = calcPitch(frame->width, LFRM_BPP_12);

	for (y=0; y < frame->height; y++){
		for (x=0; x < frame->width; x++){
			c = m_getPixel24_NB(frame, x, y); 
			r = (c&0xF00000)>>12;	// 4
			g = (c&0x00F000)>>8;	// 4
			b = (c&0x0000F0)>>4;	// 4
			*(width+x) = r|g|b;
		}
		width = (uint16_t*)((uint8_t*)width+pitch);
	}
}

static void frame32ToBuffer12 (TFRAME *frm, unsigned int * restrict buffer)
{
	frame24ToBuffer12(frm, buffer);
}

static void frame24ToBuffer8 (TFRAME *frm, unsigned int * restrict buffer)
{
	int x,y,r,g,b,c;
	uint8_t * restrict width;

	for (y=0; y < frm->height; y++){
		width = (uint8_t*)buffer + (y * frm->width);
		for (x=0; x < frm->width; x++){
			c = m_getPixel24_NB(frm, x, y); 
			r = (c&0xE00000)>>24;	// 3
			g = (c&0x00E000)>>11;	// 3
			b = (c&0x0000C0)>>6;	// 2
			*(width+x) = r|g|b;
		}
	}
}

static void frame32ToBuffer8 (TFRAME *frm, unsigned int * restrict buffer)
{
	frame24ToBuffer8(frm, buffer);
}

static void frame16ToBuffer8 (TFRAME *frm, unsigned int * restrict buffer)
{
	int x,y,r,g,b,c;
	uint8_t * restrict width;

	for (y=0; y < frm->height; y++){
		width = (uint8_t*)buffer + (y * frm->width);
		for (x=0; x < frm->width; x++){
			c = m_getPixel16_NB(frm, x, y); 
			r = (c&0xE000)>>8;	// 3
			g = (c&0x0E00)>>6;	// 3
			b = (c&0x00C0)>>4;	// 2
			*(width+x) = r|g|b;
		}
	}
}

static void frame15ToBuffer8 (TFRAME *frm, unsigned int * restrict buffer)
{
	int x,y,r,g,b,c;
	uint8_t * restrict width;

	for (y=0; y < frm->height; y++){
		width = (uint8_t*)buffer + (y * frm->width);
		for (x=0; x < frm->width; x++){
			c = m_getPixel15_NB(frm, x, y); 
			r = (c&0x7000)>>7;	// 3
			g = (c&0x0380)>>5;	// 3
			b = (c&0x0018)>>3;	// 2
			*(width+x) = r|g|b;
		}
	}
}

static void frame12ToBuffer8 (TFRAME *frm, unsigned int * restrict buffer)
{
	int x,y,r,g,b,c;
	uint8_t * restrict width;

	for (y=0; y < frm->height; y++){
		width = (uint8_t*)buffer + (y * frm->width);
		for (x=0; x < frm->width; x++){
			c = m_getPixel12_NB(frm, x, y); 
			r = (c&0xE00)>>4;	// 3
			g = (c&0x0E0)>>3;	// 3
			b = (c&0x00C)>>2;	// 2
			*(width+x) = r|g|b;
		}
	}
}

static void frame1ToBuffer8 (TFRAME *frm, unsigned int * restrict buffer)
{
	const int high = (0xFF*frm->hw->render->foreGround)&0xFF;
	const int low = (0xFF*frm->hw->render->backGround)&0xFF;
	frame1To8BPP(frm, buffer, low, high);
}

static void frame16ToBuffer12 (TFRAME *frame, unsigned int * restrict buffer)
{
	int x,y,r,g,b,c;
	uint16_t * restrict width = (uint16_t*)buffer;
	const unsigned int pitch = calcPitch(frame->width, LFRM_BPP_12);

	for (y=0; y < frame->height; y++){
		for (x=0; x < frame->width; x++){
			c = m_getPixel16_NB(frame, x, y); 
			r = (c&0xF000)>>8;	// 4
			g = (c&0x0780)>>3;	// 4
			b = (c&0x001F)>>1;	// 4
			*(width+x) = r|g|b;
		}
		width = (uint16_t*)((uint8_t*)width+pitch);
	}
}

static void frame15ToBuffer12 (TFRAME *frame, unsigned int * restrict buffer)
{
	int x,y,r,g,b,c;
	uint16_t * restrict width = (uint16_t*)buffer;
	const unsigned int pitch = calcPitch(frame->width, LFRM_BPP_12);

	for (y=0; y < frame->height; y++){
		for (x=0; x < frame->width; x++){
			c = m_getPixel15_NB(frame, x, y); 
			r = (c&0x7800)>>3;	// 4
			g = (c&0x03C0)>>2;	// 4
			b = (c&0x001E)>>1;	// 4
			*(width+x) = r|g|b;
		}
		width = (uint16_t*)((uint8_t*)width+pitch);
	}
}

static void frame12ToBuffer15 (TFRAME *frame, unsigned int * restrict buffer)
{
	int x,y, c;
	uint16_t * restrict width = (uint16_t*)buffer;
	const unsigned int pitch = calcPitch(frame->width, LFRM_BPP_15);

	for (y=0; y < frame->height; y++){
		for (x=0; x < frame->width; x++){
			c = m_getPixel12_NB(frame, x, y); 
			*(width+x) = (c&0xF00)<<3|(c&0x0F0)<<2|(c&0x00F)<<1;
		}
		width = (uint16_t*)((uint8_t*)width+pitch);
	}
}
#endif // #if (!__BUILD_2432BITONLY_SUPPORT__)
static void frame32ToBuffer32 (TFRAME *frm, unsigned int * restrict buffer)
{
	l_memcpy(buffer, l_getPixelAddress(frm, 0, 0), frm->frameSize);
}

static void frame24ToBuffer24 (TFRAME *frm, unsigned int * restrict buffer)
{
	l_memcpy(buffer, l_getPixelAddress(frm, 0, 0), frm->frameSize);
}
#if (!__BUILD_2432BITONLY_SUPPORT__)
static void frame16ToBuffer16 (TFRAME *frm, unsigned int * restrict buffer)
{
	l_memcpy(buffer, l_getPixelAddress(frm, 0, 0), frm->frameSize);
}

static void frame15ToBuffer15 (TFRAME *frm, unsigned int * restrict buffer)
{
	l_memcpy(buffer, l_getPixelAddress(frm, 0, 0), frm->frameSize);
}

static void frame12ToBuffer12 (TFRAME *frm, unsigned int * restrict buffer)
{
	l_memcpy(buffer, l_getPixelAddress(frm, 0, 0), frm->frameSize);
}

static void frame8ToBuffer8 (TFRAME *frm, unsigned int * restrict buffer)
{
	l_memcpy(buffer, l_getPixelAddress(frm, 0, 0), frm->frameSize);
}

static void frame1ToBuffer1 (TFRAME *frm, unsigned int * restrict buffer)
{
	l_memcpy(buffer, l_getPixelAddress(frm, 0, 0), frm->frameSize);
}
#endif

pConverterFn getConverter (const int src_bpp, const int des_bpp)
{
	if (des_bpp == LFRM_BPP_32 || des_bpp == LFRM_BPP_32A){
		if (src_bpp == LFRM_BPP_32 || src_bpp == LFRM_BPP_32A){
			return frame32ToBuffer32;
		}else if (src_bpp == LFRM_BPP_24){
			return frame24ToBuffer32;
#if (!__BUILD_2432BITONLY_SUPPORT__)
		}else if (src_bpp == LFRM_BPP_16){
			return frame16ToBuffer32;
		}else if (src_bpp == LFRM_BPP_15){
			return frame15ToBuffer32;
		}else if (src_bpp == LFRM_BPP_12){
			return frame12ToBuffer32;
		}else if (src_bpp == LFRM_BPP_8){
			return frame8ToBuffer32;
		}else if (src_bpp == LFRM_BPP_1){
			return frame1ToBuffer32;
#endif
		}
	}else if (des_bpp == LFRM_BPP_24){
		if (src_bpp == LFRM_BPP_32 || src_bpp == LFRM_BPP_32A){
			return frame32ToBuffer24;
		}else if (src_bpp == LFRM_BPP_24){
			return frame24ToBuffer24;
#if (!__BUILD_2432BITONLY_SUPPORT__)
		}else if (src_bpp == LFRM_BPP_16){
			return frame16ToBuffer24;
		}else if (src_bpp == LFRM_BPP_15){
			return frame15ToBuffer24;
		}else if (src_bpp == LFRM_BPP_12){
			return frame12ToBuffer24;
		}else if (src_bpp == LFRM_BPP_8){
			return frame8ToBuffer24;
		}else if (src_bpp == LFRM_BPP_1){
			return frame1ToBuffer24;
#endif
		}
	}else if (des_bpp == LFRM_BPP_16){
		if (src_bpp == LFRM_BPP_32 || src_bpp == LFRM_BPP_32A){
			return frame32ToBuffer16;
		}else if (src_bpp == LFRM_BPP_24){
			return frame24ToBuffer16;
#if (!__BUILD_2432BITONLY_SUPPORT__)
		}else if (src_bpp == LFRM_BPP_16){
			return frame16ToBuffer16;
		}else if (src_bpp == LFRM_BPP_15){
			return frame15ToBuffer16;
		}else if (src_bpp == LFRM_BPP_12){
			return frame12ToBuffer16;
		}else if (src_bpp == LFRM_BPP_8){
			return frame8ToBuffer16;
		}else if (src_bpp == LFRM_BPP_1){
			return frame1ToBuffer16;
#endif
		}
#if (!__BUILD_2432BITONLY_SUPPORT__)
	}else if (des_bpp == LFRM_BPP_15){
		if (src_bpp == LFRM_BPP_32 || src_bpp == LFRM_BPP_32A){
			return frame32ToBuffer15;
		}else if (src_bpp == LFRM_BPP_24){
			return frame24ToBuffer15;
		}else if (src_bpp == LFRM_BPP_16){
			return frame16ToBuffer15;
		}else if (src_bpp == LFRM_BPP_15){
			return frame15ToBuffer15;
		}else if (src_bpp == LFRM_BPP_12){
			return frame12ToBuffer15;
		}else if (src_bpp == LFRM_BPP_8){
			return frame8ToBuffer15;
		}else if (src_bpp == LFRM_BPP_1){
			return frame1ToBuffer15;
		}
	}else if (des_bpp == LFRM_BPP_12){
		if (src_bpp == LFRM_BPP_32 || src_bpp == LFRM_BPP_32A){
			return frame32ToBuffer12;
		}else if (src_bpp == LFRM_BPP_24){
			return frame24ToBuffer12;
		}else if (src_bpp == LFRM_BPP_16){
			return frame16ToBuffer12;
		}else if (src_bpp == LFRM_BPP_15){
			return frame15ToBuffer12;
		}else if (src_bpp == LFRM_BPP_12){
			return frame12ToBuffer12;
		}else if (src_bpp == LFRM_BPP_8){
			return frame8ToBuffer12;
		}else if (src_bpp == LFRM_BPP_1){
			return frame1ToBuffer12;
		}
	}else if (des_bpp == LFRM_BPP_8){
		if (src_bpp == LFRM_BPP_32 || src_bpp == LFRM_BPP_32A){
			return frame32ToBuffer8;
		}else if (src_bpp == LFRM_BPP_24){
			return frame24ToBuffer8;
		}else if (src_bpp == LFRM_BPP_16){
			return frame16ToBuffer8;
		}else if (src_bpp == LFRM_BPP_15){
			return frame15ToBuffer8;
		}else if (src_bpp == LFRM_BPP_12){
			return frame12ToBuffer8;
		}else if (src_bpp == LFRM_BPP_8){
			return frame8ToBuffer8;
		}else if (src_bpp == LFRM_BPP_1){
			return frame1ToBuffer8;
		}
	}else if (des_bpp == LFRM_BPP_1){
		if (src_bpp == LFRM_BPP_1){
			return frame1ToBuffer1;
		}
#endif
	}
	mylog("libmylcd: invalid colour converter requested. s:%i d:%i\n", src_bpp, des_bpp);
	return NULL;
}

