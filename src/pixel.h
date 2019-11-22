
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

#ifndef _PIXEL_H_
#define _PIXEL_H_

#include "rgbmasks.h"


typedef struct{
	ubyte r;
	ubyte g;
	ubyte b;
}__attribute__ ((packed))TRGB;		// 888

typedef struct{
	ubyte r;
	ubyte g;
	ubyte b;
	ubyte a;
}__attribute__ ((packed))TRGBA;		// 8888

typedef struct{
	ubyte b;
	ubyte g;
	ubyte r;
	ubyte a;
}__attribute__ ((packed))TBGRA;		// 8888

typedef struct {
	union {
		TBGRA bgra;
		int colour;
	}u;
}TCOLOUR4;

typedef struct{
	union{
		TRGB rgb;
		int colour;
	}u;
}TINT4TO3;

#define clipFloat(x)				\
	if ((x) > 1.0) (x) = 1.0;		\
	else if ((x) < 0.0) (x) = 0.0


#define l_setPixel(f, x, y, c)			f->pops->set(f, x, y, c)
#define l_setPixel_NB(f, x, y, c)		f->pops->set_NB(f, x, y, c)
#define l_setPixelf(f, x, y, r, g, b)	f->pops->setf(f, x, y, r, g, b)
#define l_getPixel(f, x, y)				f->pops->get(f, x, y)
#define l_getPixel_NB(f, x, y)			f->pops->get_NB(f, x, y)
#define l_getPixelf(f, x, y, r, g, b)	f->pops->getf(f, x, y, r, g, b)
#define l_getPixelAddress(f, x, y)		f->pops->getAddr(f, x, y)
#define l_getPixel_NBr(f, x, row)		f->pops->get_NBr(f, x, row)
#define l_setPixel_NBr(f, x, row, c)	f->pops->set_NBr(f, x, row, c)

#if (!__BUILD_2432BITONLY_SUPPORT__)
#define m_getPixelAddr1(f,x,y)	getPixel_addr_NB(f, x, y)
#define m_getPixelAddr8(f,x,y)	(f->pixels+((y*f->pitch)+x))
#define m_getPixelAddr12(f,x,y)	(f->pixels+((y*f->pitch)+(x<<1)))
#define m_getPixelAddr15(f,x,y)	(f->pixels+((y*f->pitch)+(x<<1)))
#define m_getPixelAddr16(f,x,y)	(f->pixels+((y*f->pitch)+(x<<1)))
#endif
#define m_getPixelAddr24(f,x,y)	(f->pixels+((y*f->pitch)+(x*3)))
#define m_getPixelAddr32(f,x,y)	(f->pixels+((y*f->pitch)+(x<<2)))

#if (!__BUILD_2432BITONLY_SUPPORT__)
#define m_getPixel1_NB(f,x,y)	((uint32_t)getPixel_NB(f,x,y))
#define m_getPixel1_NBr(f,x,r)	((uint32_t)getPixel_NBr(f,x,r))
#define m_getPixel8_NB(f,x,y)	((uint32_t)*(uint8_t*)m_getPixelAddr8(f,x,y)/*&0xFF*/)
#define m_getPixel8_NBr(f,x,r)	((uint32_t)*(uint8_t*)(f->pixels+(r+x))/*&0xFF*/)
#define m_getPixel12_NB(f,x,y) 	((uint32_t)*(uint16_t*)m_getPixelAddr12(f,x,y))
#define m_getPixel12_NBr(f,x,r)	((uint32_t)*(uint16_t*)(f->pixels+(r+(x<<1))))
#define m_getPixel15_NB(f,x,y)	((uint32_t)*(uint16_t*)m_getPixelAddr15(f,x,y)/*&0x7FFF*/)
#define m_getPixel15_NBr(f,x,r)	((uint32_t)*(uint16_t*)(f->pixels+(r+(x<<1)))/*&0x7FFF*/)
#define m_getPixel16_NB(f,x,y)	((uint32_t)*(uint16_t*)m_getPixelAddr16(f,x,y))
#define m_getPixel16_NBr(f,x,r)	((uint32_t)*(uint16_t*)(f->pixels+(r+(x<<1))))
#endif
#define m_getPixel24_NB(f,x,y)	(*((uint32_t*)m_getPixelAddr24(f,x,y))&0xFFFFFF)
#define m_getPixel24_NBr(f,x,r)	(*(uint32_t*)(f->pixels+(r+(x*3)))&0xFFFFFF)
#define m_getPixel32_NB(f,x,y)	(*(uint32_t*)m_getPixelAddr32(f,x,y))
#define m_getPixel32_NBr(f,x,r)	(*(uint32_t*)(f->pixels+(r+(x<<2))))


void closePixel (THWD *hw);
int initPixel (THWD *hw);
int getRGBMask (const TFRAME *frame, const int colourMaskIdx);
int getImageBoundingBox (TFRAME *frame, TLPOINTEX *p);


// check x,y location resides within frame
// returns 0 if in bounds, 1 if not
static INLINE int checkbounds (const TFRAME *const frm, const int x, const int y)
{
	if (x < 0 || x >= frm->width || y >= frm->height || y < 0)
		return 1;
	else
		return 0;
}


// return pixel address
static INLINE void *getPixel_addr_NB (const TFRAME *const frm, const int x, const int y)
{
	return frm->pixels+((y*frm->pitch)+(x>>3));
}


// return pixel without bound checking
static INLINE ubyte getPixel_NBr (const TFRAME *const frm, const int x, const int row)
{
	return *(frm->pixels+(row+(x>>3)))>>(x&7)&0x01;
}


// return pixel without bound checking (No Bounds)
static INLINE ubyte getPixel_NB (const TFRAME *const frm, const int x, const int y)
{
	return *(frm->pixels+((y*frm->pitch)+(x>>3))) >>(x&7)&0x01;
}


// Set pixel
static INLINE void setPixel_NBSr (const TFRAME *frm, const int x, const int row)
{
	*(frm->pixels+(row+(x>>3))) |= (1<<(x&7));
}

// Set pixel
static INLINE void setPixel_NBS (const TFRAME *frm, const int x, const int y)
{
	*(frm->pixels+((y*frm->pitch)+(x>>3))) |= (1<<(x&7));
}

// Clear pixel
static INLINE void setPixel_NBCr (const TFRAME *frm, const int x, const int row)
{
	*(frm->pixels+(row+(x>>3))) &= ~(1<<(x&7));
}

// Clear pixel
static INLINE void setPixel_NBC (const TFRAME *frm, const int x, const int y)
{
	*(frm->pixels+((y*frm->pitch)+(x>>3))) &= ~(1<<(x&7));
}

// Xor (invert) pixel
static INLINE void setPixel_NBXr (const TFRAME *frm, const int x, const int row)
{
	*(frm->pixels+(row+(x>>3))) ^= (1<<(x&7));
}


// Xor (invert) pixel
static INLINE void setPixel_NBX (const TFRAME *frm, const int x, const int y)
{
	*(frm->pixels+((y*frm->pitch)+(x>>3))) ^= (1<<(x&7));
}


// return pixel value with bound checking
static INLINE ubyte getPixel_BC (const TFRAME *const frm, const int x, const int y)
{
	if (!checkbounds(frm,x,y))
		return getPixel_NB(frm,x,y);
	else
		return 0;
}


// set pixel without bound checking
static INLINE void setPixel_NB (const TFRAME *frm, const int x, const int y, const int style)
{
	if (style == LSP_SET)
		setPixel_NBS(frm,x,y);
	else if (style == LSP_CLEAR)
		setPixel_NBC(frm,x,y);
	else if (style == LSP_XOR)
		setPixel_NBX(frm,x,y);

}

// set pixel without bound checking
static INLINE void setPixel_NBr (const TFRAME *frm, const int x, const int row, const int style)
{
	if (style == LSP_SET)
		setPixel_NBSr(frm, x, row);
	else if (style == LSP_CLEAR)
		setPixel_NBCr(frm, x, row);
	else if (style == LSP_XOR)
		setPixel_NBXr(frm, x, row);

}

// set pixel with bound checking
static INLINE void setPixel_BC (const TFRAME *frm, const int x, const int y, const int style)
{
	if (!checkbounds(frm,x,y))
		setPixel_NB(frm,x,y,style);
}


// return row containing first pixel
// return -1 if none
static INLINE int getTopPixel (const TFRAME *const frm)
{
	int x,y;
	for (y=0;y<frm->height;y++){
		for (x=frm->width;x--;){
			if (getPixel_NB(frm,x,y))
				return y;
		}
	}
	return -1;
}

// return row containing last pixel, ie, first pixel from the bottom up
// return -1 if none
static INLINE int getBottomPixel (const TFRAME *const frm)
{
	int x,y;
	for (y=frm->height;y--;){
		for (x=frm->width;x--;){
			if (getPixel_NB(frm,x,y))
				return y;
		}
	}
			
	return -1;
}

// return column of containing first pixel from left in
// return -1 if none
static INLINE int getLeftPixel (const TFRAME *const frm)
{
	int x,y;
	for (x=0;x<frm->width;x++){
		for (y=frm->height;y--;){
			if (getPixel_NB(frm,x,y))
				return x;
		}
	}
	return -1;
}

// return column of containing first pixel from right
// return -1 if none
static INLINE int getRightPixel (const TFRAME *const frm)
{
	int x,y;
	for (x=frm->width;x--;){
		for (y=frm->height;y--;){
			if (getPixel_NB(frm,x,y))
				return x;
		}
	}
	return -1;
}

#if (!__BUILD_2432BITONLY_SUPPORT__)
static INLINE void setPixel1 (const TFRAME *frm, const int x, const int y, const int value)
{
	setPixel_BC(frm, x, y, value&0x07);
}


static INLINE int getPixel1 (const TFRAME *frm, const int x, const int y)
{
	if (!checkbounds(frm, x, y))
		return m_getPixel1_NB(frm, x, y);
	else
		return 0;
}

static INLINE int getPixel8 (const TFRAME *frm, const int x, const int y)
{
	if (!checkbounds(frm, x, y))
		return (uint32_t)*(uint8_t*)(frm->pixels+((y*frm->pitch)+x))/*&0xFF*/;
	else
		return 0;
}

static INLINE int getPixel12 (const TFRAME *frm, const int x, const int y)
{
	if (!checkbounds(frm, x, y))
		return m_getPixel12_NB(frm, x, y);
	else
		return 0;
}

static INLINE int getPixel15 (const TFRAME *frm, const int x, const int y)
{
	if (!checkbounds(frm, x, y))
		return m_getPixel15_NB(frm, x, y);
	else
		return 0;
}

static INLINE int getPixel16 (const TFRAME *frm, const int x, const int y)
{
	if (!checkbounds(frm, x, y))
		return m_getPixel16_NB(frm, x, y);
	else
		return 0;
}
#endif
static INLINE int getPixel24 (const TFRAME *frm, const int x, const int y)
{
	if (!checkbounds(frm, x, y))
		return m_getPixel24_NB(frm, x, y);
	else
		return 0;
}

static INLINE int getPixel32 (const TFRAME *frm, const int x, const int y)
{
	if (!checkbounds(frm, x, y))
		return m_getPixel32_NB(frm, x, y);
	else
		return 0;
}
#if (!__BUILD_2432BITONLY_SUPPORT__)
static INLINE int getPixel1_NB (const TFRAME *frm, const int x, const int y)
{
	return m_getPixel1_NB(frm, x, y);
}

static INLINE int getPixel8_NB (const TFRAME *frm, const int x, const int y)
{
	return m_getPixel8_NB(frm, x, y);
}

static INLINE int getPixel12_NB (const TFRAME *frm, const int x, const int y)
{
	return m_getPixel12_NB(frm, x, y);
}

static INLINE int getPixel15_NB (const TFRAME *frm, const int x, const int y)
{
	return m_getPixel15_NB(frm, x, y);
}

static INLINE int getPixel16_NB (const TFRAME *frm, const int x, const int y)
{
	return m_getPixel16_NB(frm, x, y);
}
#endif
static INLINE int getPixel24_NB (const TFRAME *frm, const int x, const int y)
{
	return m_getPixel24_NB(frm, x, y);
}

static INLINE int getPixel32_NB (const TFRAME *frm, const int x, const int y)
{
	return m_getPixel32_NB(frm, x, y);
}
#if (!__BUILD_2432BITONLY_SUPPORT__)
static INLINE int getPixel1_NBr (const TFRAME *frm, const int x, const int row)
{
	return m_getPixel1_NBr(frm, x, row);
}

static INLINE int getPixel8_NBr (const TFRAME *frm, const int x, const int row)
{
	return m_getPixel8_NBr(frm, x, row);
}

static INLINE int getPixel12_NBr (const TFRAME *frm, const int x, const int row)
{
	return m_getPixel12_NBr(frm, x, row)/*&0xFFF*/;
}

static INLINE int getPixel15_NBr (const TFRAME *frm, const int x, const int row)
{
	return m_getPixel15_NBr(frm, x, row);
}

static INLINE int getPixel16_NBr (const TFRAME *frm, const int x, const int row)
{
	//return (uint32_t)*(uint16_t*)(frm->pixels+(row+(x<<1)))/*&0xFFFF*/;
	return m_getPixel16_NBr(frm, x, row);
}
#endif
static INLINE int getPixel24_NBr (const TFRAME *frm, const int x, const int row)
{
	return m_getPixel24_NBr(frm, x, row);
}

static INLINE int getPixel32_NBr (const TFRAME *frm, const int x, const int row)
{
	return m_getPixel32_NBr(frm, x, row);
}

#if (!__BUILD_2432BITONLY_SUPPORT__)
static INLINE void setPixel1_NBr (const TFRAME *frm, const int x, const int row, const int value)
{
	setPixel_NBr(frm, x, row, value&0x07);
}

static INLINE void setPixel8_NBr (const TFRAME *frm, const int x, const int row, const int value)
{
	if (frm->style == LSP_SET)
		*(uint8_t*)(frm->pixels+(row+x)) = value;
	else if (frm->style == LSP_XOR)
		*(uint8_t*)(frm->pixels+(row+x)) ^= value;
	else if (frm->style == LSP_OR)
		*(uint8_t*)(frm->pixels+(row+x)) |= value;
	else if (frm->style == LSP_AND)
		*(uint8_t*)(frm->pixels+(row+x)) &= value;
	else /*if (frm->style == LSP_CLEAR)*/
		*(uint8_t*)(frm->pixels+(row+x)) = 0;
}

static INLINE void setPixel12_NBr (const TFRAME *frm, const int x, const int row, const int value)
{
	if (frm->style == LSP_SET)
		*(uint16_t*)(frm->pixels+(row+(x<<1))) = value/*&0xFFF*/;
	else if (frm->style == LSP_XOR)
		*(uint16_t*)(frm->pixels+(row+(x<<1))) ^= value/*&0xFFF*/;
	else if (frm->style == LSP_OR)
		*(uint16_t*)(frm->pixels+(row+(x<<1))) |= value/*&0xFFF*/;
	else if (frm->style == LSP_AND)
		*(uint16_t*)(frm->pixels+(row+(x<<1))) &= value/*&0xFFF*/;
	else /*if (frm->style == LSP_CLEAR)*/
		*(uint16_t*)(frm->pixels+(row+(x<<1))) = 0;
}

static INLINE void setPixel15_NBr (const TFRAME *frm, const int x, const int row, const int value)
{
	if (frm->style == LSP_SET)
		*(uint16_t*)(frm->pixels+(row+(x<<1))) = value&0x7FFF;
	else if (frm->style == LSP_XOR)
		*(uint16_t*)(frm->pixels+(row+(x<<1))) ^= value&0x7FFF;
	else if (frm->style == LSP_OR)
		*(uint16_t*)(frm->pixels+(row+(x<<1))) |= value&0x7FFF;
	else if (frm->style == LSP_AND)
		*(uint16_t*)(frm->pixels+(row+(x<<1))) &= value&0x7FFF;
	else /*if (frm->style == LSP_CLEAR)*/
		*(uint16_t*)(frm->pixels+(row+(x<<1))) = 0;
}

static INLINE void setPixel16_NBr (const TFRAME *frm, const int x, const int row, const int value)
{
	if (frm->style == LSP_SET)
		*(uint16_t*)(frm->pixels+(row+(x<<1))) = value;
	else if (frm->style == LSP_XOR)
		*(uint16_t*)(frm->pixels+(row+(x<<1))) ^= value;
	else if (frm->style == LSP_OR)
		*(uint16_t*)(frm->pixels+(row+(x<<1))) |= value;
	else if (frm->style == LSP_AND)
		*(uint16_t*)(frm->pixels+(row+(x<<1))) &= value;
	else /*if (frm->style == LSP_CLEAR)*/
		*(uint16_t*)(frm->pixels+(row+(x<<1))) = 0;
}
#endif

static INLINE void setPixel24_NBr (const TFRAME *frm, const int x, const int row, const int value)
{
	
	if (frm->style == LSP_SET){
		TRGB *addr = (TRGB*)(frm->pixels+(row+(x*3)));
		*addr = *(TRGB*)&value;
		
	}else if (frm->style == LSP_XOR){
		TINT4TO3 int3;
		int3.u.colour = value;
		ubyte *addr = (uint8_t*)(frm->pixels+(row+(x*3)));
		*addr++ ^= int3.u.rgb.r;
		*addr++ ^= int3.u.rgb.g;
		*addr   ^= int3.u.rgb.b;
		
	}else if (frm->style == LSP_OR){
		TINT4TO3 int3;
		int3.u.colour = value;
		ubyte *addr = (uint8_t*)(frm->pixels+(row+(x*3)));
		*addr++ |= int3.u.rgb.r;
		*addr++ |= int3.u.rgb.g;
		*addr   |= int3.u.rgb.b;
		
	}else if (frm->style == LSP_AND){
		TINT4TO3 int3;
		ubyte *addr = (uint8_t*)(frm->pixels+(row+(x*3)));
		int3.u.colour = value;
		*addr++ &= int3.u.rgb.r;
		*addr++ &= int3.u.rgb.g;
		*addr   &= int3.u.rgb.b;
	}else /*if (frm->style == LSP_CLEAR)*/{
		int pix = 0;
		TRGB *addr = (TRGB*)(frm->pixels+(row+(x*3)));
		*addr = *(TRGB*)&pix;
	}
}

static INLINE void setPixel32_NBr (const TFRAME *frm, const int x, const int row, const int value)
{
	if (frm->style == LSP_SET)
		*(uint32_t*)(frm->pixels+(row+(x<<2))) = value;
	else if (frm->style == LSP_XOR)
		*(uint32_t*)(frm->pixels+(row+(x<<2))) ^= value;
	else if (frm->style == LSP_OR)
		*(uint32_t*)(frm->pixels+(row+(x<<2))) |= value;
	else if (frm->style == LSP_AND)
		*(uint32_t*)(frm->pixels+(row+(x<<2))) &= value;
	else /*if (frm->style == LSP_CLEAR)*/
		*(uint32_t*)(frm->pixels+(row+(x<<2))) = 0;
}

#if (!__BUILD_2432BITONLY_SUPPORT__)
static INLINE void setPixel8 (const TFRAME *frm, const int x, const int y, const int value)
{
	if (!checkbounds(frm, x, y)){
		if (frm->style == LSP_SET)
			*(uint8_t*)(frm->pixels+((y*frm->pitch)+x)) = value;
		else if (frm->style == LSP_XOR)
			*(uint8_t*)(frm->pixels+((y*frm->pitch)+x)) ^= value;
		else if (frm->style == LSP_OR)
			*(uint8_t*)(frm->pixels+((y*frm->pitch)+x)) |= value;
		else if (frm->style == LSP_AND)
			*(uint8_t*)(frm->pixels+((y*frm->pitch)+x)) &= value;
		else /*if (frm->style == LSP_CLEAR)*/
			*(uint8_t*)(frm->pixels+((y*frm->pitch)+x)) = 0;
	}
}

static INLINE void setPixel8_NB (const TFRAME *frm, const int x, const int y, const int value)
{
	if (frm->style == LSP_SET)
		*(uint8_t*)(frm->pixels+((y*frm->pitch)+x)) = value;
	else if (frm->style == LSP_XOR)
		*(uint8_t*)(frm->pixels+((y*frm->pitch)+x)) ^= value;
	else if (frm->style == LSP_OR)
		*(uint8_t*)(frm->pixels+((y*frm->pitch)+x)) |= value;
	else if (frm->style == LSP_AND)
		*(uint8_t*)(frm->pixels+((y*frm->pitch)+x)) &= value;
	else /*if (frm->style == LSP_CLEAR)*/
		*(uint8_t*)(frm->pixels+((y*frm->pitch)+x)) = 0;

}

static INLINE void setPixel12_NB (const TFRAME *frm, const int x, const int y, const int value)
{
	if (frm->style == LSP_SET)
		*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) = value/*&0xFFF*/;
	else if (frm->style == LSP_XOR)
		*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) ^= value/*&0xFFF*/;
	else if (frm->style == LSP_OR)
		*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) |= value/*&0xFFF*/;
	else if (frm->style == LSP_AND)
		*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) &= value/*&0xFFF*/;
	else /*if (frm->style == LSP_CLEAR)*/
		*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) = 0;
}

static INLINE void setPixel12 (const TFRAME *frm, const int x, const int y, const int value)
{
	if (!checkbounds(frm, x, y)){
		if (frm->style == LSP_SET)
			*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) = value/*&0xFFF*/;
		else if (frm->style == LSP_XOR)
			*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) ^= value/*&0xFFF*/;
		else if (frm->style == LSP_OR)
			*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) |= value/*&0xFFF*/;
		else if (frm->style == LSP_AND)
			*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) &= value/*&0xFFF*/;
		else // /*if (frm->style == LSP_CLEAR)*/
			*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) = 0;
			
	}
}

static INLINE void setPixel15_NB (const TFRAME *frm, const int x, const int y, const int value)
{
	if (frm->style == LSP_SET)
		*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) = value&0x7FFF;
	else if (frm->style == LSP_XOR)
		*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) ^= value&0x7FFF;
	else if (frm->style == LSP_OR)
		*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) |= value&0x7FFF;
	else if (frm->style == LSP_AND)
		*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) &= value&0x7FFF;
	else /*if (frm->style == LSP_CLEAR)*/
		*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) = 0;
}

static INLINE void setPixel15 (const TFRAME *frm, const int x, const int y, const int value)
{
	if (!checkbounds(frm, x, y)){
		if (frm->style == LSP_SET)
			*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) = value&0x7FFF;
		else if (frm->style == LSP_XOR)
			*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) ^= value&0x7FFF;
		else if (frm->style == LSP_OR)
			*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) |= value&0x7FFF;
		else if (frm->style == LSP_AND)
			*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) &= value&0x7FFF;
		else /*if (frm->style == LSP_CLEAR)*/
			*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) = 0;
	}
}

static INLINE void setPixel16 (const TFRAME *frm, const int x, const int y, const int value)
{
	if (!checkbounds(frm, x, y)){
		if (frm->style == LSP_SET)
			*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) = value;
		else if (frm->style == LSP_XOR)
			*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) ^= value;
		else if (frm->style == LSP_OR)
			*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) |= value;
		else if (frm->style == LSP_AND)
			*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) &= value;
		else /*if (frm->style == LSP_CLEAR)*/
			*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) = 0;
	}
}

static INLINE void setPixel16_NB (const TFRAME *frm, const int x, const int y, const int value)
{
	if (frm->style == LSP_SET)
		*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) = value;
	else if (frm->style == LSP_XOR)
		*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) ^= value;
	else if (frm->style == LSP_OR)
		*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) |= value;
	else if (frm->style == LSP_AND)
		*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) &= value;
	else /*if (frm->style == LSP_CLEAR)*/
		*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) = 0;
}
#endif
static INLINE void setPixel24 (const TFRAME *frm, const int x, const int y, const int value)
{
	if (!checkbounds(frm, x, y)){
		if (frm->style == LSP_SET){
			TRGB *addr = (TRGB*)(frm->pixels+((y*frm->pitch)+(x*3)));
			*addr = *(TRGB*)&value;
			
		}else if (frm->style == LSP_XOR){
			TINT4TO3 int3;	
			int3.u.colour = value;
			uint8_t *addr = (uint8_t*)(frm->pixels+((y*frm->pitch)+(x*3)));
			*addr++ ^= int3.u.rgb.r;
			*addr++ ^= int3.u.rgb.g;
			*addr   ^= int3.u.rgb.b;
			
		}else if (frm->style == LSP_OR){
			TINT4TO3 int3;	
			int3.u.colour = value;
			uint8_t *addr = (uint8_t*)(frm->pixels+((y*frm->pitch)+(x*3)));
			*addr++ |= int3.u.rgb.r;
			*addr++ |= int3.u.rgb.g;
			*addr   |= int3.u.rgb.b;
			
		}else if (frm->style == LSP_AND){
			TINT4TO3 int3;	
			uint8_t *addr = (uint8_t*)(frm->pixels+((y*frm->pitch)+(x*3)));
			int3.u.colour = value;
			*addr++ &= int3.u.rgb.r;
			*addr++ &= int3.u.rgb.g;
			*addr   &= int3.u.rgb.b;
			
		}else /*if (frm->style == LSP_CLEAR)*/{
			int pix = 0;
			TRGB *addr = (TRGB*)(frm->pixels+((y*frm->pitch)+(x*3)));
			*addr = *(TRGB*)&pix;
		}
	}
}

static INLINE void setPixel24_NB (const TFRAME *frm, const int x, const int y, const int value)
{
	if (frm->style == LSP_SET){
		TRGB *addr = (TRGB*)(frm->pixels+((y*frm->pitch)+(x*3)));
		*addr = *(TRGB*)&value;

	}else if (frm->style == LSP_OR){
		TINT4TO3 int3;
		int3.u.colour = value;
		ubyte *addr = (uint8_t*)(frm->pixels+((y*frm->pitch)+(x*3)));
		*addr++ |= int3.u.rgb.r;
		*addr++ |= int3.u.rgb.g;
		*addr   |= int3.u.rgb.b;
		
	}else if (frm->style == LSP_XOR){
		TINT4TO3 int3;
		int3.u.colour = value;
		ubyte *addr = (uint8_t*)(frm->pixels+((y*frm->pitch)+(x*3)));
		*addr++ ^= int3.u.rgb.r;
		*addr++ ^= int3.u.rgb.g;
		*addr   ^= int3.u.rgb.b;
		
	}else if (frm->style == LSP_AND){
		TINT4TO3 int3;
		ubyte *addr = (uint8_t*)(frm->pixels+((y*frm->pitch)+(x*3)));
		int3.u.colour = value;
		*addr++ &= int3.u.rgb.r;
		*addr++ &= int3.u.rgb.g;
		*addr   &= int3.u.rgb.b;
		
	}else /*if (frm->style == LSP_CLEAR)*/{
		int pix = 0;
		TRGB *addr = (TRGB*)(frm->pixels+((y*frm->pitch)+(x*3)));
		*addr = *(TRGB*)&pix;
	}
}

static INLINE void setPixel32 (const TFRAME *frm, const int x, const int y, const int value)
{
	if (!checkbounds(frm, x, y)){
		if (frm->style == LSP_SET)
			*(uint32_t*)(frm->pixels+((y*frm->pitch)+(x<<2))) = value;
		else if (frm->style == LSP_XOR)
			*(uint32_t*)(frm->pixels+((y*frm->pitch)+(x<<2))) ^= value;
		else if (frm->style == LSP_OR)
			*(uint32_t*)(frm->pixels+((y*frm->pitch)+(x<<2))) |= value;
		else if (frm->style == LSP_AND)
			*(uint32_t*)(frm->pixels+((y*frm->pitch)+(x<<2))) &= value;
		else /*if (frm->style == LSP_CLEAR)*/
			*(uint32_t*)(frm->pixels+((y*frm->pitch)+(x<<2))) = 0;
	}
}

static INLINE void setPixel32_NB (const TFRAME *frm, const int x, const int y, const int value)
{
	if (frm->style == LSP_SET)
		*(uint32_t*)(frm->pixels+((y*frm->pitch)+(x<<2))) = value;
	else if (frm->style == LSP_XOR)
		*(uint32_t*)(frm->pixels+((y*frm->pitch)+(x<<2))) ^= value;
	else if (frm->style == LSP_OR)
		*(uint32_t*)(frm->pixels+((y*frm->pitch)+(x<<2))) |= value;
	else if (frm->style == LSP_AND)
		*(uint32_t*)(frm->pixels+((y*frm->pitch)+(x<<2))) &= value;
	else /*if (frm->style == LSP_CLEAR)*/
		*(uint32_t*)(frm->pixels+((y*frm->pitch)+(x<<2))) = 0;
}
#if (!__BUILD_2432BITONLY_SUPPORT__)
static INLINE void *getPixelAddr1 (const TFRAME *frm, const int x, const int y)
{
	return m_getPixelAddr1(frm,x,y);
}

static INLINE void *getPixelAddr8 (const TFRAME *frm, const int x, const int y)
{
	return m_getPixelAddr8(frm,x,y);
}

static INLINE void *getPixelAddr12 (const TFRAME *frm, const int x, const int y)
{
	return m_getPixelAddr12(frm,x,y);
}

static INLINE void *getPixelAddr15 (const TFRAME *frm, const int x, const int y)
{
	return m_getPixelAddr15(frm,x,y);
}

static INLINE void *getPixelAddr16 (const TFRAME *frm, const int x, const int y)
{
	return m_getPixelAddr16(frm,x,y);
}
#endif
static INLINE void *getPixelAddr24 (const TFRAME *frm, const int x, const int y)
{
	return m_getPixelAddr24(frm,x,y);
}

static INLINE void *getPixelAddr32 (const TFRAME *frm, const int x, const int y)
{
	return m_getPixelAddr32(frm,x,y);
}

static INLINE void *getPixelAddr32a (const TFRAME *frm, const int x, const int y)
{
	return m_getPixelAddr32(frm,x,y);
}
#if (!__BUILD_2432BITONLY_SUPPORT__)
static INLINE void setPixel1f (const TFRAME *frm, const int x, const int y, float r, float g, float b)
{
	clipFloat(r);
	const int r2 = (int)(r*255.0);
	clipFloat(g);
	const int g2 = (int)(g*255.0);
	clipFloat(b);
	const int b2 = (int)(b*255.0);
	return setPixel1(frm, x, y, (r2&0x80) | (g2&0x80) | (b2&0x80));
}

static INLINE void setPixel8f (const TFRAME *frm, const int x, const int y, float r, float g, float b)
{
	clipFloat(r);
	const int r2 = (int)(r*255.0);
	clipFloat(g);
	const int g2 = (int)(g*255.0);
	clipFloat(b);
	const int b2 = (int)(b*255.0);
	return setPixel8(frm, x, y, (r2&0xE0) | ((g2&0xE0)>>3) | ((b2&0xC0)>>6));
}

static INLINE void setPixel12f (const TFRAME *frm, const int x, const int y, float r, float g, float b)
{
	clipFloat(r);
	const int r2 = (int)(r*255.0);
	clipFloat(g);
	const int g2 = (int)(g*255.0);
	clipFloat(b);
	const int b2 = (int)(b*255.0);
	return setPixel12(frm, x, y, ((r2&0xF0)<<4) | (g2&0xF0) | ((b2&0xF0)>>4));
}

static INLINE void setPixel15f (const TFRAME *frm, const int x, const int y, float r, float g, float b)
{
	clipFloat(r);
	const int r2 = (int)(r*255.0);
	clipFloat(g);
	const int g2 = (int)(g*255.0);
	clipFloat(b);
	const int b2 = (int)(b*255.0);
	return setPixel15(frm, x, y, ((r2&0xF8)<<7) | ((g2&0xF8)<<2) | ((b2&0xF8)>>3));
}

static INLINE void setPixel16f (const TFRAME *frm, const int x, const int y, float r, float g, float b)
{
	clipFloat(r);
	const int r2 = (int)(r*255.0);
	clipFloat(g);
	const int g2 = (int)(g*255.0);
	clipFloat(b);
	const int b2 = (int)(b*255.0);
	return setPixel16(frm, x, y, ((r2&0xF8)<<8) | ((g2&0xFC)<<3) | ((b2&0xF8)>>3));
}
#endif
static INLINE void setPixel24f (const TFRAME *frm, const int x, const int y, float r, float g, float b)
{
	clipFloat(r);
	const int r2 = (int)(r*255.0);
	clipFloat(g);
	const int g2 = (int)(g*255.0);
	clipFloat(b);
	const int b2 = (int)(b*255.0);
	return setPixel24(frm, x, y, (r2<<16)|(g2<<8)|b2);
}

static INLINE void setPixel32f (const TFRAME *frm, const int x, const int y, float r, float g, float b)
{
	clipFloat(r);
	const int r2 = (int)(r*255.0);
	clipFloat(g);
	const int g2 = (int)(g*255.0);
	clipFloat(b);
	const int b2 = (int)(b*255.0);
	return setPixel32(frm, x, y, (r2<<16)|(g2<<8)|b2);
}
#if (!__BUILD_2432BITONLY_SUPPORT__)
static INLINE int getPixel1f (const TFRAME *frm, const int x, const int y, float *r, float *g, float *b)
{
	if (!checkbounds(frm, x, y)){
		*r = *g = *b = (float)getPixel_NB(frm,x,y);
		return 1;
	}else{
		return 0;
	}
}

static INLINE int getPixel8f (const TFRAME *frm, const int x, const int y, float *r, float *g, float *b)
{
	if (!checkbounds(frm, x, y)){
		const int pixel = (int)*(uint8_t*)(frm->pixels+((y*frm->pitch)+x))/*&0xFF*/;
		*r =  (pixel&0xE0)/255.0;
		*g = ((pixel&0x1C)<<3)/255.0;
		*b = ((pixel&0x03)<<6)/255.0;
		return 1;
	}else{
		return 0;
	}
}

static INLINE int getPixel12f (const TFRAME *frm, const int x, const int y, float *r, float *g, float *b)
{
	if (!checkbounds(frm, x, y)){
		const int pixel = m_getPixel12_NB(frm, x, y);
		*r = ((pixel&0xF00)>>4)/255.0;
		*g = ((pixel&0x0F0)/255.0);
		*b = ((pixel&0x00F)<<4)/255.0;
		return 1;
	}else{
		return 0;
	}
}

static INLINE int getPixel15f (const TFRAME *frm, const int x, const int y, float *r, float *g, float *b)
{
	if (!checkbounds(frm, x, y)){
		const int pixel = m_getPixel15_NB(frm, x, y);
		*r = ((pixel&0x7C00)>>7)/255.0;
		*g = ((pixel&0x03E0)>>2)/255.0;
		*b = ((pixel&0x001F)<<3)/255.0;
		return 1;
	}else{
		return 0;
	}
}

static INLINE int getPixel16f (const TFRAME *frm, const int x, const int y, float *r, float *g, float *b)
{
	if (!checkbounds(frm, x, y)){
		const int pixel = m_getPixel16_NB(frm,x,y);
		*r = ((pixel&0xF800)>>8)/255.0;
		*g = ((pixel&0x07E0)>>3)/255.0;
		*b = ((pixel&0x001F)<<3)/255.0;
		return 1;
	}else{
		return 0;
	}
}
#endif
static INLINE int getPixel24f (const TFRAME *frm, const int x, const int y, float *r, float *g, float *b)
{
	if (!checkbounds(frm, x, y)){
		const int pixel = m_getPixel24_NB(frm, x, y);
		*r = ((pixel&0xFF0000)>>16)/255.0;
		*g = ((pixel&0x00FF00)>>8)/255.0;
		*b =  (pixel&0x0000FF)/255.0;
		return 1;
	}else{
		return 0;
	}
}

static INLINE int getPixel32f (const TFRAME *frm, const int x, const int y, float *r, float *g, float *b)
{
	if (!checkbounds(frm, x, y)){
		const int pixel = m_getPixel32_NB(frm, x, y);
		*r = ((pixel&0x00FF0000)>>16)/255.0;
		*g = ((pixel&0x0000FF00)>>8)/255.0;
		*b =  (pixel&0x000000FF)/255.0;
		return 1;
	}else{
		return 0;
	}
}

#if (!__BUILD_2432BITONLY_SUPPORT__)
static INLINE void setPixel1_NB (const TFRAME *frm, const int x, const int y, const int value)
{
	setPixel_NB(frm, x, y, value&0x07);
}
#endif


#if 1
static INLINE void setPixel32a_NB (const TFRAME *frm, const int x, const int y, const int value)
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


#if 1
	dst->u.colour = evenRes + oddRes;
#else
	switch(frm->style){
	  case LSP_SET: dst->u.colour = (evenRes | oddRes); return;
	  case LSP_OR: dst->u.colour |= (evenRes | oddRes); return;
	  case LSP_XOR: dst->u.colour ^= (evenRes | oddRes); return;
	  case LSP_AND: dst->u.colour &= (evenRes | oddRes); return;
	  case LSP_CLEAR: dst->u.colour = value; return;
	};
#endif

}


static INLINE void setPixel32a_NBr (const TFRAME *frm, const int x, const int row, const int value)
{
	TCOLOUR4 *dst = (TCOLOUR4*)(frm->pixels+(row+(x<<2)));
	const TCOLOUR4 *src = (TCOLOUR4*)&value;
	const unsigned int alpha = src->u.bgra.a + (unsigned int)(src->u.bgra.a>>7);
	const unsigned int odds2 = (src->u.colour>>8)&0xFF00FF;
	const unsigned int odds1 = (dst->u.colour>>8)&0xFF00FF;
	const unsigned int evens1 = dst->u.colour&0xFF00FF;
	const unsigned int evens2 = src->u.colour&0xFF00FF;
	const unsigned int evenRes = ((((evens2-evens1)*alpha)>>8) + evens1)& 0xFF00FF;
	const unsigned int oddRes = ((odds2-odds1)*alpha + (odds1<<8)) & 0xFF00FF00;

#if 1
	dst->u.colour = (evenRes + oddRes);
#else
	switch(frm->style){
	  case LSP_SET: dst->u.colour = (evenRes | oddRes); return;
	  case LSP_OR: dst->u.colour |= (evenRes | oddRes); return;
	  case LSP_XOR: dst->u.colour ^= (evenRes | oddRes); return;
	  case LSP_AND: dst->u.colour &= (evenRes | oddRes); return;
	  case LSP_CLEAR: dst->u.colour = value; return;
	};
#endif
}

#else

static INLINE unsigned int alphablend (const unsigned int des, const unsigned int src)
{
	const unsigned int alpha = (src&0xFF000000)>>24;
	const unsigned int odds2 = (src>>8) & 0xFF00FF;
	const unsigned int odds1 = (des>>8) & 0xFF00FF;
	const unsigned int evens1 = des & 0xFF00FF;
	const unsigned int evens2 = src & 0xFF00FF;
	const unsigned int evenRes = ((((evens2-evens1)*alpha)>>8) + evens1)& 0xFF00FF;
	const unsigned int oddRes = ((odds2-odds1)*alpha + (odds1<<8)) &0xFF00FF00;
	return (unsigned int)(evenRes + oddRes);
}


static INLINE void setPixel32a_NB (const TFRAME *frm, const int x, const int y, const int value)
{
	unsigned int *paddr = (unsigned int*)m_getPixelAddr32(frm, x, y);
#if 1
	*paddr = alphablend(*paddr, value);
#else
	switch(frm->style){
	  case LSP_SET: *paddr = alphablend(*paddr, value); break;
	  case LSP_XOR: *paddr ^= alphablend(*paddr, value); break;
	  case LSP_OR: *paddr |= alphablend(*paddr, value); break;
	  case LSP_AND: *paddr &= alphablend(*paddr, value); break;
	  case LSP_CLEAR: *paddr = value; 
	};
#endif
}

static INLINE void setPixel32a_NBr (const TFRAME *frm, const int x, const int row, const int value)
{
	unsigned int *paddr = (unsigned int*)(frm->pixels+(row+(x<<2)));
#if 1
	*paddr = alphablend(*paddr, value);
#else
	switch(frm->style){
	  case LSP_SET: *paddr = alphablend(*paddr, value); break;
	  case LSP_XOR: *paddr ^= alphablend(*paddr, value); break;
	  case LSP_OR: *paddr |= alphablend(*paddr, value); break;
	  case LSP_AND: *paddr &= alphablend(*paddr, value); break;
	  case LSP_CLEAR: *paddr = value; 
	};
#endif
}
#endif

static INLINE void setPixel32a (const TFRAME *frm, const int x, const int y, const int value)
{
	if (!checkbounds(frm, x, y))
		setPixel32a_NB(frm, x, y, value);
}

static INLINE void assignPixelPrimitives (TPIXELPRIMITVES *pixel, int bpp)
{
#if (!__BUILD_2432BITONLY_SUPPORT__)
	if (bpp == LFRM_BPP_1){
		pixel->set = setPixel1;
		pixel->get = getPixel1;
		pixel->get_NB = getPixel1_NB;
		pixel->set_NB = setPixel1_NB;
		pixel->get_NBr = getPixel1_NBr;
		pixel->set_NBr = setPixel1_NBr;
		pixel->setf = setPixel1f;
		pixel->getf = getPixel1f;
		pixel->getAddr = getPixelAddr1;
		
	}else if (bpp == LFRM_BPP_8){
		pixel->set = setPixel8;
		pixel->get = getPixel8;
		pixel->get_NB = getPixel8_NB;
		pixel->set_NB = setPixel8_NB;
		pixel->get_NBr = getPixel8_NBr;
		pixel->set_NBr = setPixel8_NBr;
		pixel->setf = setPixel8f;
		pixel->getf = getPixel8f;
		pixel->getAddr = getPixelAddr8;
		
	}else if (bpp == LFRM_BPP_12){
		pixel->set = setPixel12;
		pixel->get = getPixel12;
		pixel->get_NB = getPixel12_NB;
		pixel->set_NB = setPixel12_NB;
		pixel->get_NBr = getPixel12_NBr;
		pixel->set_NBr = setPixel12_NBr;
		pixel->setf = setPixel12f;
		pixel->getf = getPixel12f;
		pixel->getAddr = getPixelAddr12;
		
	}else if (bpp == LFRM_BPP_15){
		pixel->set = setPixel15;
		pixel->get = getPixel15;
		pixel->get_NB = getPixel15_NB;
		pixel->set_NB = setPixel15_NB;
		pixel->get_NBr = getPixel15_NBr;
		pixel->set_NBr = setPixel15_NBr;
		pixel->setf = setPixel15f;
		pixel->getf = getPixel15f;
		pixel->getAddr = getPixelAddr15;
		
	}else if (bpp == LFRM_BPP_16){
		pixel->set = setPixel16;
		pixel->get = getPixel16;
		pixel->get_NB = getPixel16_NB;
		pixel->set_NB = setPixel16_NB;
		pixel->get_NBr = getPixel16_NBr;
		pixel->set_NBr = setPixel16_NBr;
		pixel->setf = setPixel16f;
		pixel->getf = getPixel16f;
		pixel->getAddr = getPixelAddr16;
	}else
#endif
	  if (bpp == LFRM_BPP_32A){
		pixel->set = setPixel32a;
		pixel->set_NB = setPixel32a_NB;
		pixel->set_NBr = setPixel32a_NBr;
		pixel->setf = setPixel32f;
		
		pixel->get = getPixel32;
		pixel->get_NB = getPixel32_NB;
		pixel->get_NBr = getPixel32_NBr;		
		pixel->getf = getPixel32f;
		pixel->getAddr = getPixelAddr32;
	}else if (bpp == LFRM_BPP_32){
		pixel->set = setPixel32;
		pixel->get = getPixel32;
		pixel->get_NB = getPixel32_NB;
		pixel->set_NB = setPixel32_NB;
		pixel->get_NBr = getPixel32_NBr;
		pixel->set_NBr = setPixel32_NBr;
		pixel->setf = setPixel32f;
		pixel->getf = getPixel32f;
		pixel->getAddr = getPixelAddr32;
	}else if (bpp == LFRM_BPP_24){
		pixel->set = setPixel24;
		pixel->get = getPixel24;
		pixel->get_NB = getPixel24_NB;
		pixel->set_NB = setPixel24_NB;
		pixel->get_NBr = getPixel24_NBr;
		pixel->set_NBr = setPixel24_NBr;
		pixel->setf = setPixel24f;
		pixel->getf = getPixel24f;
		pixel->getAddr = getPixelAddr24;		
	}
}

#endif 

