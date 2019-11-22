
// libmylcd - http://mylcd.sourceforge.net/
// An LCD framebuffer library
// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2010  Michael McElligott
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

#if (__BUILD_DRAW_SUPPORT__)


#include "memory.h"
#include "utils.h"
#include "pixel.h"
#include "lmath.h"
#include "copy.h"
#include "draw.h"


static inline void circlePts (TFRAME *frm, int xc, int yc, int x, int y, int colour);
static inline int floodFill_op (TFILL *fill, int x, int y, const int newColor, const int oldColor);
static inline int floodfill_init (TFILL *fill, TFRAME *frame);
static inline void floodfill_cleanup (TFILL *fill);
static inline int edgefill_op (TFRAME *frame, TFILL *stack, int ox, int oy, int newColour, int edgeColour);
static inline int edgefill_init (TFILL *stack, TFRAME *frame);
static inline void edgefill_cleanup (TFILL *stack);
static inline void bindCoordinates (TFRAME *frame, int *x1, int *y1, int *x2, int *y2);
static inline void horLine (TFRAME *frame, int y, int x1, int x2, int colour);
static inline int clipLine (TFRAME *frame, int x1, int y1, int x2, int y2, int *x3, int *y3, int *x4, int *y4);



static inline void swapint (int *a, int *b) 
{ 
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

int floodFill (TFRAME *frame, const int x, const int y, const int colour)
{
	int ret = 0;
	TFILL *fill = l_malloc(sizeof(TFILL));
	if (fill != NULL){
		if (floodfill_init(fill, frame)){
			ret = floodFill_op(fill, x, y, colour, l_getPixel(frame, x, y));
			floodfill_cleanup(fill);
		}
		l_free(fill);
	}
	return ret;
}

int edgeFill (TFRAME *frame, const int x, const int y, const int fillColour, const int edgeColour)
{
	int ret = 0;
	TFILL *stack = l_malloc(sizeof(TFILL));
	if (stack != NULL){
		if (edgefill_init(stack, frame)){
			ret = edgefill_op(frame, stack, x, y, fillColour, edgeColour);
			edgefill_cleanup(stack);
		}
		l_free(stack);
	}

	return ret;
}

int drawPolyLineEx (TFRAME *frm, TLPOINTEX *pt, int n, const int colour)
{
	while(n--)
		drawLine(frm, pt[n].x1, pt[n].y1, pt[n].x2, pt[n].y2, colour);
	return n;
}

int drawPolyLineDottedTo (TFRAME *frm, T2POINT *pt, const int tPoints, const int colour)
{
	for (int ct = 1; ct < tPoints; ct++)
		drawLineDotted(frm, pt[ct-1].x, pt[ct-1].y, pt[ct].x, pt[ct].y, colour);
	return 1;
}

int drawPolyLine (TFRAME *frm, T2POINT *pt, int tPoints, const int colour)
{
	if (!frm || !pt) return 0;
	
	for (int ct = 0; ct < tPoints-1; ct += 2)
		drawLine(frm, pt[ct].x, pt[ct].y, pt[ct+1].x, pt[ct+1].y, colour);
	return 1;
}

int drawPolyLineTo (TFRAME *frm, TPOINTXY *pt, int tPoints, const int colour)
{
	tPoints--;
	for (int ct = 0; ct < tPoints; ct++)
		drawLine(frm, pt[ct].x, pt[ct].y, pt[ct+1].x, pt[ct+1].y, colour);
	return 1;
}

int drawCircleFilled (TFRAME *frame, int xc, int yc, int radius, const int colour)
{	
	if (xc + radius < 0 || xc - radius >= frame->width || yc + radius < 0 || yc - radius >= frame->height)
		return 0;
  	
	int y = radius;
	int p = 3 - (radius << 1);
	int a, b, c, d, e, f, g, h, x = 0;
	int pb = yc + radius + 1, pd = yc + radius + 1;
  
	while (x <= y){
		a = xc + x; b = yc + y;
		c = xc - x; d = yc - y;
		e = xc + y; f = yc + x;
		g = xc - y; h = yc - x;
		if (b != pb)
			horLine(frame, b, a, c, colour);
		if (d != pd)
			horLine(frame, d, a, c, colour);
		if (f != b)
			horLine(frame, f, e, g, colour);
		if (h != d && h != f)
			horLine(frame, h, e, g, colour);
		pb = b; pd = d;
		if (p < 0)
			p += (x++ << 2) + 6;
		else
			p += ((x++ - y--) << 2) + 10;
	}
	return 1;
}

int drawTriangle (TFRAME *frame, int x1, int y1, int x2, int y2, int x3, int y3, int colour)
{
	drawLine(frame, x1, y1, x2, y2, colour);
	drawLine(frame, x2, y2, x3, y3, colour);
	return drawLine(frame, x1, y1, x3, y3, colour);
}

static inline void swapd (double *a, double *b) 
{ 
	double tmp = *a;
	*a = *b;
	*b = tmp;
}

int drawTriangleFilled (TFRAME *frame, const int x0, const int y0, const int x1, const int y1, const int x2, const int y2, const int colour)
{
	double XA, XB;
  	double XA1 = 0.0, XB1 = 0.0, XC1 = 0.0;
  	double XA2, XB2;
  	double XAd, XBd; 
  	double HALF;
  	
	int t = y0;
	int b = y0;
	int CAS = 0;
	
	if (y1 < t){
		t = y1;
		CAS = 1;
	}
	if (y1 > b)
		b = y1;
		
	if (y2 < t){
		t = y2;
		CAS = 2;
	}
	if (y2 > b)
		b = y2;
   	
	if (CAS == 0){
		XA = x0;
		XB = x0;
		XA1 = (x1-x0)/(double)(y1-y0);
		XB1 = (x2-x0)/(double)(y2-y0);
		XC1 = (x2-x1)/(double)(y2-y1);
		
		if (y1<y2){
			HALF = y1;
      		XA2 = XC1;
      		XB2 = XB1;
    	}else{
    		HALF = y2;
      		XA2 = XA1;
      		XB2 = XC1;
    	}
		if (y0 == y1)
			XA = x1;
		if (y0 == y2)
			XB = x2;
  	}else if (CAS == 1){
    	XA = x1;
    	XB = x1;
    	XA1 = (x2-x1)/(double)(y2-y1);
    	XB1 = (x0-x1)/(double)(y0-y1);
    	XC1 = (x0-x2)/(double)(y0-y2);
    	
    	if ( y2 < y0){
    		HALF = y2;
      		XA2 = XC1;
      		XB2 = XB1;
    	}else{
    		HALF = y0;
      		XA2 = XA1;
      		XB2 = XC1;
    	} 
    	if (y1 == y2)
    		XA = x2;
    	if (y1 == y0)
			XB = x0;
	}else if (CAS == 2){
		XA = x2;
		XB = x2;
    	XA1 = (x0-x2)/(double)(y0-y2);
    	XB1 = (x1-x2)/(double)(y1-y2);
    	XC1 = (x1-x0)/(double)(y1-y0);
    	if (y0<y1){
    		HALF = y0;
      		XA2 = XC1;
      		XB2 = XB1;
    	}else{
    		HALF = y1;
      		XA2 = XA1;
      		XB2 = XC1;
    	}
    	if (y2 == y0)
    		XA = x0;
    	if (y2 == y1)
    		XB = x1;
	}
  
	if (XA1 > XB1){
		swapd(&XA, &XB);
		swapd(&XA1, &XB1);
		swapd(&XA2, &XB2);
	}
  
	for (int y = t; y < HALF; y++){
		XAd = XA;
		XBd = XB;
		for (int x = XAd; x <= XBd; x++)
			l_setPixel(frame, x, y, colour);
		XA += XA1;
		XB += XB1;	
	}

	for (int y = HALF; y <= b; y++){
		XAd = XA;
		XBd = XB;
		for (int x = XAd; x <= XBd; x++)
			l_setPixel(frame, x, y, colour);
		XA += XA2;
		XB += XB2;	
	}
	return 1;
}

int drawRectangleDottedFilled (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour)
{
	if (!frm) return 0;

	bindCoordinates(frm, &x1, &y1, &x2, &y2);
	
	if (x2 < x1) swapint(&x2,&x1);
	if (y2 < y1) swapint(&y2,&y1);

	int swp = 0;
	for (int y = y1; y < y2+1; y++){
		swp = y&1;
		for (int x = x1; x < x2+1; x++){
			if (swp)
				l_setPixel_NB(frm, x, y, colour);
			swp ^= 1;
		}
	}
	return 1;
}

int drawRectangleFilled (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour)
{
	bindCoordinates(frm, &x1, &y1, &x2, &y2);
	
	if (x2 < x1) swapint(&x2,&x1);
	if (y2 < y1) swapint(&y2,&y1);

	if (frm->bpp == LFRM_BPP_32A){
		for (int y = y1; y <= y2; y++){
			for (int x = x1; x <= x2; x++)
				setPixel32a_NB(frm, x, y, colour);
		}
	}else if (frm->bpp == LFRM_BPP_32){
		for (int y = y1; y <= y2; y++){
			for (int x = x1; x <= x2; x++)
				setPixel32_NB(frm, x, y, colour);
		}
	}else{
		for (int y = y1; y <= y2; y++){
			for (int x = x1; x <= x2; x++)
				l_setPixel_NB(frm, x, y, colour);
		}
	}
	
	return 1;
}

int drawRectangleDotted (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour)
{
	if (!frm) return 0;

	//bindCoordinates(frm, &x1, &y1, &x2, &y2);
	
	if (x2 < x1) swapint(&x2,&x1);
	if (y2 < y1) swapint(&y2,&y1);
	
	for (int x = x1; x < x2; x+=2){
		l_setPixel(frm, x, y1, colour);
		l_setPixel(frm, x, y2, colour);
	}
	for (int y = y1+1; y < y2; y+=2){
		l_setPixel(frm, x1, y, colour);
		l_setPixel(frm, x2, y, colour);
	}
	return 1;
}

int drawRectangle (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour)
{
	// top
	drawLine(frm, x1, y1, x2, y1, colour);
	// bottom
	drawLine(frm, x1, y2, x2, y2, colour);
	// left
	drawLine(frm, x1, y1+1, x1, y2-1, colour);
	// right
	return drawLine(frm, x2, y1+1, x2, y2-1, colour);
}

int invertArea (TFRAME *frm, const int x1, const int y1, const int x2, const int y2)
{
	int ret;
	const int tmp = frm->style;
	frm->style = LSP_XOR;
#if (!__BUILD_2432BITONLY_SUPPORT__)
	if (frm->bpp == LFRM_BPP_1)
		ret = drawRectangleFilled(frm, x1, y1, x2, y2, frm->style);
	else
#endif
		ret = drawRectangleFilled(frm, x1, y1, x2, y2, getRGBMask(frm, LMASK_WHITE));
	frm->style = tmp;
	return ret;
}

int invertFrame (TFRAME *frm)
{
	return invertArea(frm, 0, 0, frm->width-1, frm->height-1);
}

int drawLineDotted (TFRAME *frm, const int x1, const int y1, const int x2, const int y2, const int colour)
{
	if (!frm) return 0;

	//bindCoordinates(frm, &x1, &y1, &x2, &y2);
	
    int dx = x2-x1;
    int dy = y2-y1;

    if (dx || dy){
        if (l_abs(dx) >= l_abs(dy)){
            float y = y1+0.5;
            float dly = (float)dy/(float)dx;
            
            if (dx > 0){
                for (int xx = x1; xx<=x2; xx += 2){
                    l_setPixel(frm,xx,(int)y,colour);
                    y += dly;
                }
            }else{
                for (int xx = x1; xx>=x2; xx -= 2){
                    l_setPixel(frm,xx,(int)y,colour);
                    y -= dly;
                }
			}
        }else{
           	float x = x1+0.5;
           	float dlx = (float)dx/(float)dy;

            if (dy > 0){
   	            for (int yy = y1; yy<=y2; yy += 2){
       	            l_setPixel(frm,(int)x,yy,colour);
           	        x += dlx;
               	}
			}else{
                for (int yy = y1; yy >= y2; yy -= 2){
   	                l_setPixel(frm,(int)x,yy,colour);
       	            x -= dlx;
           	    }
			}
        }
    }else if (!(dx&dy)){
    	l_setPixel(frm,x1,y1,colour);
    }

    return 1;
}

#if 1

static inline int drawLine32 (TFRAME *frame, int x0, int y0, int x1, int y1, const int colour)
{
	if (!clipLine(frame, x0, y0, x1, y1, &x0, &y0, &x1, &y1))
		return 1;
		
	int dy = y1 - y0;
	y0 *= frame->width;
	int *pixels = (int*)frame->pixels;
    pixels[x0+y0] = colour;
    y1 *= frame->width;
	int dx = x1 - x0;
	int stepx, stepy;
        		
	if (dy < 0) { dy = -dy;  stepy = -frame->width; } else { stepy = frame->width; }
	if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
    dy <<= 1;
    dx <<= 1;

                
        if (dx > dy) {
            int fraction = dy - (dx >> 1);
            while (x0 != x1) {
                if (fraction >= 0) {
                    y0 += stepy;
                    fraction -= dx;
                }
                x0 += stepx;
                fraction += dy;
                pixels[x0+y0] = colour;
            }
        } else {
            int fraction = dx - (dy >> 1);
            while (y0 != y1) {
                if (fraction >= 0) {
                    x0 += stepx;
                    fraction -= dy;
                }
                y0 += stepy;
                fraction += dx;
                pixels[x0+y0] = colour;
            }
        }
	return 1;
}

#if (!__BUILD_2432BITONLY_SUPPORT__)
static inline int drawLine16 (TFRAME *frame, int x0, int y0, int x1, int y1, const uint16_t colour)
{
	if (!clipLine(frame, x0, y0, x1, y1, &x0, &y0, &x1, &y1))
		return 1;
		
	int dy = y1 - y0;
	int dx = x1 - x0;
	int stepx, stepy;
	uint16_t *pixels = (uint16_t*)frame->pixels;
	
	if (dy < 0) { dy = -dy;  stepy = -frame->width; } else { stepy = frame->width; }
	if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
	dy <<= 1;
	dx <<= 1;
	
	y0 *= frame->width;
	y1 *= frame->width;
	pixels[x0+y0] = colour;
	
	if (dx > dy) {
	    int fraction = dy - (dx >> 1);
	    while (x0 != x1) {
	        if (fraction >= 0) {
	            y0 += stepy;
	            fraction -= dx;
	        }
	        x0 += stepx;
	        fraction += dy;
	        pixels[x0+y0] = colour;
	    }
	} else {
	    int fraction = dx - (dy >> 1);
	    while (y0 != y1) {
	        if (fraction >= 0) {
	            x0 += stepx;
	            fraction -= dy;
	        }
	        y0 += stepy;
	        fraction += dx;
	        pixels[x0+y0] = colour;
	    }
	}
	return 1;
}

static inline int drawLine8 (TFRAME *frame, int x0, int y0, int x1, int y1, const char colour)
{
	if (!clipLine(frame, x0, y0, x1, y1, &x0, &y0, &x1, &y1))
		return 1;
		
        int dy = y1 - y0;
        int dx = x1 - x0;
        int stepx, stepy;
		char *pixels = (char*)frame->pixels;
		
        if (dy < 0) { dy = -dy;  stepy = -frame->width; } else { stepy = frame->width; }
        if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
        dy <<= 1;
        dx <<= 1;

        y0 *= frame->width;
        y1 *= frame->width;
        pixels[x0+y0] = colour;
        if (dx > dy) {
            int fraction = dy - (dx >> 1);
            while (x0 != x1) {
                if (fraction >= 0) {
                    y0 += stepy;
                    fraction -= dx;
                }
                x0 += stepx;
                fraction += dy;
                pixels[x0+y0] = colour;
            }
        } else {
            int fraction = dx - (dy >> 1);
            while (y0 != y1) {
                if (fraction >= 0) {
                    x0 += stepx;
                    fraction -= dy;
                }
                y0 += stepy;
                fraction += dx;
                pixels[x0+y0] = colour;
            }
        }
	return 1;
}
#endif

static inline int drawLineFast (TFRAME *frame, int x, int y, int x2, int y2, const int colour)
{

	if (!clipLine(frame, x, y, x2, y2, &x, &y, &x2, &y2))
		return 1;

   	int yLonger = 0;
	int shortLen = y2-y;
	int longLen = x2-x;
	
	if (abs(shortLen) > abs(longLen)){
		swapint(&shortLen, &longLen);
		yLonger = 1;
	}
	int decInc;
	
	if (longLen == 0)
		decInc = 0;
	else
		decInc = (shortLen << 16) / longLen;

	if (frame->bpp == LFRM_BPP_32A){
		if (yLonger) {
			if (longLen>0) {
				longLen+=y;
				for (int j=0x8000+(x<<16);y<=longLen;++y) {
					setPixel32a_NB(frame,j >> 16,y, colour);
					j+=decInc;
				}
				return 1;
			}
			longLen+=y;
			for (int j=0x8000+(x<<16);y>=longLen;--y) {
				setPixel32a_NB(frame,j >> 16,y, colour);
				j-=decInc;
			}
			return 1;
		}

		if (longLen>0) {
			longLen+=x;
			for (int j=0x8000+(y<<16);x<=longLen;++x){
				setPixel32a_NB(frame,x,j >> 16, colour);
				j+=decInc;
			}
			return 1;
		}
		longLen+=x;
		for (int j=0x8000+(y<<16);x>=longLen;--x){
			setPixel32a_NB(frame,x,j >> 16, colour);
			j-=decInc;
		}		
	}else if (frame->bpp == LFRM_BPP_32){
		if (yLonger) {
			if (longLen>0) {
				longLen+=y;
				for (int j=0x8000+(x<<16);y<=longLen;++y) {
					setPixel32_NB(frame,j >> 16,y, colour);
					j+=decInc;
				}
				return 1;
			}
			longLen+=y;
			for (int j=0x8000+(x<<16);y>=longLen;--y) {
				setPixel32_NB(frame,j >> 16,y, colour);
				j-=decInc;
			}
			return 1;
		}

		if (longLen>0) {
			longLen+=x;
			for (int j=0x8000+(y<<16);x<=longLen;++x){
				setPixel32_NB(frame,x,j >> 16, colour);
				j+=decInc;
			}
			return 1;
		}
		longLen+=x;
		for (int j=0x8000+(y<<16);x>=longLen;--x){
			setPixel32_NB(frame,x,j >> 16, colour);
			j-=decInc;
		}
	}else{
		if (yLonger) {
			if (longLen>0) {
				longLen+=y;
				for (int j=0x8000+(x<<16);y<=longLen;++y) {
					l_setPixel_NB(frame,j >> 16,y, colour);
					j+=decInc;
				}
				return 1;
			}
			longLen+=y;
			for (int j=0x8000+(x<<16);y>=longLen;--y) {
				l_setPixel_NB(frame,j >> 16,y, colour);
				j-=decInc;
			}
			return 1;
		}

		if (longLen>0) {
			longLen+=x;
			for (int j=0x8000+(y<<16);x<=longLen;++x){
				l_setPixel_NB(frame,x,j >> 16, colour);
				j+=decInc;
			}
			return 1;
		}
		longLen+=x;
		for (int j=0x8000+(y<<16);x>=longLen;--x){
			l_setPixel_NB(frame,x,j >> 16, colour);
			j-=decInc;
		}
	}
	return 1;
}

int drawLine (TFRAME *frame, int x1, int y1, int x2, int y2, const int colour)
{
	if (frame->style == LSP_SET){
		if (((colour>>24)&0xFF) == 0xFF && (frame->bpp == LFRM_BPP_32A || frame->bpp == LFRM_BPP_32))
			return drawLine32(frame, x1, y1, x2, y2, colour);
		else if (frame->bpp == LFRM_BPP_32)
			return drawLine32(frame, x1, y1, x2, y2, colour);
#if (!__BUILD_2432BITONLY_SUPPORT__)
		else if (frame->bpp == LFRM_BPP_16 || frame->bpp == LFRM_BPP_15 || frame->bpp == LFRM_BPP_12)
			return drawLine16(frame, x1, y1, x2, y2, colour&0xFFFF);
		else if (frame->bpp == LFRM_BPP_8)
			return drawLine8(frame, x1, y1, x2, y2, colour&0xFF);
#endif
	}
	return drawLineFast(frame, x1, y1, x2, y2, colour);
}

#else

static inline int drawLine32 (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour)
{
	int x3 = 0, y3 = 0, x4 = 0, y4 = 0;
	if (!clipLine(frm, x1, y1, x2, y2, &x3, &y3, &x4, &y4))
		return 1;

	x1 = x3;
	y1 = y3;
	x2 = x4;
	y2 = y4;
	
    const int dx = x2-x1;
    const int dy = y2-y1;

    if (dx || dy){
        if (l_abs(dx) >= l_abs(dy)){
            float y = y1+0.5f;
            float dly = (float)dy/(float)dx;
            
            if (dx > 0){
                for (int xx = x1; xx<=x2; xx++){
                    setPixel32_NB(frm,xx,(int)y,colour);
                    y += dly;
                }
            }else{
                for (int xx = x1; xx>=x2; xx--){
                    setPixel32_NB(frm,xx,(int)y,colour);
                    y -= dly;
                }
			}
        }else{
           	float x = x1+0.5f;
           	float dlx = (float)dx/(float)dy;

            if (dy > 0){
   	            for (int yy = y1; yy<=y2; yy++){
       	            setPixel32_NB(frm,(int)x,yy,colour);
           	        x += dlx;
               	}
			}else{
                for (int yy = y1; yy >= y2; yy--){
   	                setPixel32_NB(frm,(int)x,yy,colour);
       	            x -= dlx;
           	    }
			}
        }
    }else if (!(dx&dy)){
    	setPixel32_NB(frm,x1,y1,colour);
    }

    return 1;
}

static inline int drawLine32a (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour)
{
	int x3 = 0, y3 = 0, x4 = 0, y4 = 0;
	if (!clipLine(frm, x1, y1, x2, y2, &x3, &y3, &x4, &y4))
		return 1;

	x1 = x3;
	y1 = y3;
	x2 = x4;
	y2 = y4;
	
    const int dx = x2-x1;
    const int dy = y2-y1;

    if (dx || dy){
        if (l_abs(dx) >= l_abs(dy)){
            float y = y1+0.5f;
            float dly = dy/(float)dx;
            
            if (dx > 0){
                for (int xx = x1; xx<=x2; xx++){
                    setPixel32a_NB(frm,xx,(int)y,colour);
                    y += dly;
                }
            }else{
                for (int xx = x1; xx>=x2; xx--){
                    setPixel32a_NB(frm,xx,(int)y,colour);
                    y -= dly;
                }
			}
        }else{
           	float x = x1+0.5f;
           	float dlx = dx/(float)dy;

            if (dy > 0){
   	            for (int yy = y1; yy<=y2; yy++){
       	            setPixel32a_NB(frm,(int)x,yy,colour);
           	        x += dlx;
               	}
			}else{
                for (int yy = y1; yy >= y2; yy--){
   	                setPixel32a_NB(frm,(int)x,yy,colour);
       	            x -= dlx;
           	    }
			}
        }
    }else if (!(dx&dy)){
    	setPixel32a_NB(frm,x1,y1,colour);
    }

    return 1;
}

static inline int drawLineStd (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour)
{
	int x3 = 0, y3 = 0, x4 = 0, y4 = 0;
	if (!clipLine(frm, x1, y1, x2, y2, &x3, &y3, &x4, &y4))
		return 1;

	x1 = x3;
	y1 = y3;
	x2 = x4;
	y2 = y4;
	
    const int dx = x2-x1;
    const int dy = y2-y1;

    if (dx || dy){
        if (l_abs(dx) >= l_abs(dy)){
            float y = y1+0.5f;
            float dly = dy/(float)dx;
            
            if (dx > 0){
                for (int xx = x1; xx<=x2; xx++){
                    l_setPixel_NB(frm,xx,(int)y,colour);
                    y += dly;
                }
            }else{
                for (int xx = x1; xx>=x2; xx--){
                    l_setPixel_NB(frm,xx,(int)y,colour);
                    y -= dly;
                }
			}
        }else{
           	float x = x1+0.5f;
           	float dlx = dx/(float)dy;

            if (dy > 0){
   	            for (int yy = y1; yy<=y2; yy++){
       	            l_setPixel_NB(frm,(int)x,yy,colour);
           	        x += dlx;
               	}
			}else{
                for (int yy = y1; yy >= y2; yy--){
   	                l_setPixel_NB(frm,(int)x,yy,colour);
       	            x -= dlx;
           	    }
			}
        }
    }else if (!(dx&dy)){
    	l_setPixel_NB(frm,x1,y1,colour);
    }

    return 1;
}

int drawLine (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour)
{
	
	if ((colour>>24)&0xFF == 0xFF && (frm->bpp == LFRM_BPP_32A || frm->bpp == LFRM_BPP_32))
		return drawLine32(frm, x1, y1, x2, y2, colour);
	else if (frm->bpp == LFRM_BPP_32A)
		return drawLine32a(frm, x1, y1, x2, y2, colour);
	else if (frm->bpp == LFRM_BPP_32)
		return drawLine32(frm, x1, y1, x2, y2, colour);
	else
		return drawLineStd(frm, x1, y1, x2, y2, colour);
}

#endif

int drawEnclosedArc (TFRAME *frm, const int x, const int y, const int r1, const int r2, double a1, double a2, const int colour)
{
	if (!frm) return 0;

	int myx, myy, lastx = 0, lasty = 0;

	a1  = -a1 + 90.0;
	a2 = a2 + 90.0;
	
	a1 = 360.0 - a1;
	a2 = 360.0 - a2;
	const int N = (int)l_fabs(a2-a1)+8.0;
	const double a = a1*2.0*M_PI/360.0;
	const double da = (a2-a1)*(2.0*M_PI/360.0)/(N-1);
	
	for (int loop = 0; loop < N; loop++){
		myx = x+(int)(r1*l_cos(a+loop*da));
		myy = y+(int)(r2*l_sin(a+loop*da));
		if (loop)
			drawLine(frm, lastx, lasty, myx, myy, colour);
		if (loop == N-1 || !loop)
			drawLine(frm, x, y, myx, myy ,colour);
		lastx = myx;
		lasty = myy;
	}
	return 1;
}

int drawArc (TFRAME *frm, const int x, const int y, const int r1, const int r2, double a1, double a2, const int colour)
{
	a1 = 360.0 - (-a1 + 90.0);
	a2 = 360.0 - ( a2 + 90.0);

	const int N = l_fabs(a2-a1)+8.0;
	const double a = a1*2.0*M_PI/360.0;
	const double da = (a2-a1)*(2.0*M_PI/360.0)/(double)(N-1.0);
	
	double lastx = x + (r1*l_cosf(a/*+0.0*da*/));
	double lasty = y + (r2*l_sinf(a/*+0.0*da*/));

	for (double loop = 1.0; loop < N; loop += 1.0){
		double myx = x + (r1*l_cos(a+loop*da));
		double myy = y + (r2*l_sin(a+loop*da));
		//if (loop)
			drawLine(frm, lastx, lasty, myx, myy, colour);
			
		lastx = myx;
		lasty = myy;
	}
	
	return 1;
}

int drawEllipse (TFRAME *frm, const int x, const int y, const int r1, const int r2, const int colour)
{
	drawArc(frm, x, y, r1, r2, 0.0, 90.0, colour);
	return drawArc(frm, x, y, r1, r2, 90.0, 360.0, colour);
}

int drawMaskA (const TFRAME *src, const TFRAME *mask, TFRAME *des, const int Xoffset, const int Yoffset, const int srcX1, const int srcY1, const int srcX2, const int srcY2, const float alpha)
{
	if (!src || !mask || !des)
		return 0;

	const float afactor = 1.0/255.0;
	float a1, a2;
	float r1, r2;
	float g1, g2;
	float b1, b2;

	for (int y = srcY1; y<srcY2; y++){
		for (int x = srcX1; x<srcX2; x++){
			a1 = alpha * (float)((l_getPixel_NB(mask, x, y)&0xFF) * afactor);
			a2 = 1.0 - a1;
			l_getPixelf(src, x, y, &r1, &g1, &b1);			// src
			l_getPixelf(des, x+Xoffset, y+Yoffset, &r2, &g2, &b2);	// des
			l_setPixelf(des, x+Xoffset, y+Yoffset, (r1 * a1) + (r2 * a2), (g1 * a1) + (g2 * a2), (b1 * a1) + (b2 * a2));
		}
	}
	return 1;
}

int drawMask (TFRAME *src, TFRAME *mask, TFRAME *des, int maskXOffset, int maskYOffset, int mode)
{
	if (!src || !mask || !des)
		return 0;

	int w = MIN(src->width, des->width);
	int h = MIN(src->height, des->height);

	if (mode==LMASK_AND){
		for (int y = maskYOffset; y < h; y++){
			for (int x = maskXOffset; x < w; x++)
				l_setPixel(des,x,y,l_getPixel(src,x,y) & l_getPixel(mask,x-maskXOffset,y-maskYOffset));
		}
	}else if (mode==LMASK_OR){
		for (int y = 0; y < h; y++){
			for (int x = 0; x < w; x++)
				l_setPixel(des,x,y,l_getPixel(src,x,y) | l_getPixel(mask,x-maskXOffset,y-maskYOffset));
		}
	}else if (mode==LMASK_XOR && (des->bpp == LFRM_BPP_32A || mask->bpp == LFRM_BPP_32A)){
		for (int y = 0; y < h; y++){
			for (int x = 0; x < w; x++)
				l_setPixel(des,x,y,l_getPixel(src,x,y) ^ (l_getPixel(mask,x-maskXOffset,y-maskYOffset)&0xFFFFFF));
		}
	}else if (mode==LMASK_XOR){
		for (int y = 0; y < h; y++){
			for (int x = 0; x < w; x++)
				l_setPixel(des,x,y,l_getPixel(src,x,y) ^ l_getPixel(mask,x-maskXOffset,y-maskYOffset));
		}
	}else if (mode==LMASK_CPYSRC){
		for (int y = 0; y < h; y++){
			for (int x = 0; x < w; x++)
				l_setPixel(des,x,y,l_getPixel(src,x,y));
		}
	}else if (mode==LMASK_CPYMASK){
		for (int y = maskYOffset; y < h; y++){
			for (int x = maskXOffset; x < w; x++)
				l_setPixel(des,x,y,l_getPixel(mask,x-maskXOffset,y-maskYOffset));
		}
	}else if (mode==LMASK_CLEAR){
		copyFrame(src, des);
		for (int y = maskYOffset; y < h; y++){
			for (int x = maskXOffset; x < w; x++){
				if (l_getPixel(mask,x-maskXOffset,y-maskYOffset))
					l_setPixel(des,x,y,LSP_CLEAR);
			}
		}
	}else{
		return 0;
	}
	return 1;
}

int drawCircle (TFRAME *frm, const int xc, const int yc, const int radius, const int colour)
{
	if (!frm) return 0;

	double x = 0.0; 
	double y = radius;
	double p = 1.0-radius;

	circlePts(frm, xc, yc, x, y, colour);
	while (x < y){
		x += 1.0;
		if (p < 0.0){
			p += 2.0*x+1.0;
		}else{
			y -= 1.0;
			p += 2.0*x+1.0-2.0*y;
		}
		circlePts(frm, xc, yc, x, y, colour);
	}
	return 1;
}

static inline int floodfill_init (TFILL *fill, TFRAME *frame)
{
	fill->size = frame->width * frame->height;
	fill->stack = l_malloc(fill->size * sizeof(T2POINT));
	if (fill->stack != NULL){
		fill->width = frame->width;
		fill->height= frame->height;
		fill->frame = frame;
		fill->position = 0;
		return 1;
	}else{
		return 0;
	}
}

static inline void floodfill_cleanup (TFILL *fill)
{
	if (fill->stack != NULL)
		l_free(fill->stack);
	fill->stack = NULL;
	fill->position = 0;
}

static inline int floodfill_pop (TFILL *fill, int *x, int *y) 
{ 
    if (fill->position > 0){ 
        *x = fill->stack[fill->position].x;
        *y = fill->stack[fill->position].y;
        fill->position--; 
        return 1; 
    }else{ 
        return 0; 
    }    
}    

static inline int floodfill_push (TFILL *fill, const int x, const int y) 
{ 
    if (fill->position < fill->size - 1){ 
        fill->position++; 
        fill->stack[fill->position].x = x;
        fill->stack[fill->position].y = y;
        return 1; 
	}else{ 
        return 0; 
    }    
}     

static inline void emptyStack (TFILL *fill) 
{ 
    int x = 0, y = 0; 
    while(floodfill_pop(fill, &x, &y)); 
}

static inline int floodFill_op (TFILL *fill, int x, int y, const int newColor, const int oldColor)
{
    if (oldColor == newColor) return 0;
   
    emptyStack(fill);
    int y1, spanLeft, spanRight;
    
    if (!floodfill_push(fill, x, y))
    	return 0;
    
    while (floodfill_pop(fill, &x, &y)){    
        y1 = y;
        while (y1 >= 0 && l_getPixel(fill->frame, x, y1) == oldColor)
        	y1--;
        y1++;
        spanLeft = spanRight = 0;
        
        while (y1 < fill->height && l_getPixel(fill->frame, x, y1) == oldColor){
            l_setPixel(fill->frame, x, y1, newColor);
            
            if (!spanLeft && x > 0 && l_getPixel(fill->frame, x - 1, y1) == oldColor){
                if (!floodfill_push(fill, x - 1, y1)) return 0;
                spanLeft = 1;
            }else if (spanLeft && x > 0 && l_getPixel(fill->frame, x - 1, y1) != oldColor){
                spanLeft = 0;
            }
            
            if (!spanRight && x < fill->frame->width - 1 && l_getPixel(fill->frame, x + 1, y1) == oldColor){
                if (!floodfill_push(fill, x + 1, y1)) return 0;
                spanRight = 1;
            }else if (spanRight && x < fill->frame->width - 1 && l_getPixel(fill->frame, x + 1, y1) != oldColor){
                spanRight = 0;
            } 
            y1++;
        }
    }
    return 1;
}


static inline void circlePts (TFRAME *frm, int xc, int yc, int x, int y, const int colour)
{
	l_setPixel(frm, xc+y, yc-x, colour);
	l_setPixel(frm, xc-y, yc-x, colour);
	l_setPixel(frm, xc+y, yc+x, colour);
	l_setPixel(frm, xc-y, yc+x, colour);
	l_setPixel(frm, xc+x, yc+y, colour);
	l_setPixel(frm, xc-x, yc+y, colour);
	l_setPixel(frm, xc+x, yc-y, colour);
	l_setPixel(frm, xc-x, yc-y, colour);
}

// bind coordinate to frame
static inline void bindCoordinates (TFRAME *frame, int *x1, int *y1, int *x2, int *y2)
{
	if (*x1 > frame->width-1)
		*x1 = frame->width-1;
	else if (*x1 < 0)
		*x1 = 0;
		
	if (*x2 > frame->width-1)
		*x2 = frame->width-1;
	else if (*x2 < 0)
		*x2 = 0;

	if (*y1 > frame->height-1)
		*y1 = frame->height-1;
	else if (*y1 < 0)
		*y1 = 0;
		
	if (*y2 > frame->height-1)
		*y2 = frame->height-1;
	else if (*y2 < 0)
		*y2 = 0;
}

static inline int findRegion (TFRAME *frame, int x, int y)
{
	int code=0;
	
	if (y >= frame->height)
		code |= 1; //top
	else if( y < 0)
		code |= 2; //bottom
		
	if (x >= frame->width)
		code |= 4; //right
	else if ( x < 0)
		code |= 8; //left

  return(code);
}

static inline void horLine (TFRAME *frame, int y, int x1, int x2, int colour)
{
	drawLine(frame, x1, y, x2, y, colour);
}

static inline int clipLine (TFRAME *frame, int x1, int y1, int x2, int y2, int *x3, int *y3, int *x4, int *y4)
{
  
  int accept = 0, done = 0;
  int code1 = findRegion(frame, x1, y1); //the region outcodes for the endpoints
  int code2 = findRegion(frame, x2, y2);
  
  const int h = frame->height;
  const int w = frame->width;
  
  do{  //In theory, this can never end up in an infinite loop, it'll always come in one of the trivial cases eventually
    if (!(code1 | code2)){
    	accept = done = 1;  //accept because both endpoints are in screen or on the border, trivial accept
    }else if (code1 & code2){
    	done = 1; //the line isn't visible on screen, trivial reject
    }else{  //if no trivial reject or accept, continue the loop

      int x, y;
	  int codeout = code1 ? code1 : code2;
      if (codeout&1){			//top
        x = x1 + (x2 - x1) * (h - y1) / (y2 - y1);
        y = h - 1;
      }else if (codeout & 2){	//bottom
        x = x1 + (x2 - x1) * -y1 / (y2 - y1);
        y = 0;
      }else if (codeout & 4){	//right
        y = y1 + (y2 - y1) * (w - x1) / (x2 - x1);
        x = w - 1;
      }else{					//left
        y = y1 + (y2 - y1) * -x1 / (x2 - x1);
        x = 0;
      }
      
      if (codeout == code1){ //first endpoint was clipped
        x1 = x; y1 = y;
        code1 = findRegion(frame, x1, y1);
      }else{ //second endpoint was clipped
        x2 = x; y2 = y;
        code2 = findRegion(frame, x2, y2);
      }
    }
  }
  while(done == 0);

  if (accept){
    *x3 = x1;
    *x4 = x2;
    *y3 = y1;
    *y4 = y2;
    return 1;
  }else{
   // *x3 = *x4 = *y3 = *y4 = 0;
    return 0;
  }
}

static inline int edgefill_init (TFILL *stack, TFRAME *frame)
{
	stack->size = frame->width * frame->height;
	stack->stack = l_calloc(sizeof(T2POINT), stack->size);
	if (stack->stack){
		stack->position = 0;
		stack->width = frame->width;
		stack->height = frame->height;
		stack->frame = frame; /*unused*/
		return 1;
	}else{
		return 0;
	}
}

static inline void edgefill_cleanup (TFILL *stack)
{
	if (stack->stack)
		l_free(stack->stack);
	stack->stack = NULL;
	stack->size = 0;
	stack->position = 0;
}

#if 0
static inline int edgefill_search (TFILL *stack, int x, int y)
{
	for (int i = 0; i < stack->position; i++){
		if (stack->stack[i].x == x && stack->stack[i].y == y)
			return i+1;
	}
	return 0;
}
#endif

static inline int edgefill_push (TFILL *stack, const int x, const int y)
{
	if (stack->position >= stack->size-1){
		printf("edgefill_op: out of stack %i %i\n", x, y);
		return 0;
	}

	if (x < 0 || x >= stack->width || y < 0 || y >= stack->height)
		return 0;

//	if (!edgefill_search(stack, x, y)){
		stack->stack[stack->position].x = x;
		stack->stack[stack->position].y = y;
		return ++stack->position;
//	}else{
//		return stack->position;
//	}
}

static inline int edgefill_pop (TFILL *stack, int *x, int *y)
{
	if (!stack->position)
		return 0;
	
	*x = stack->stack[--stack->position].x;
	*y = stack->stack[stack->position].y;
	return stack->position+1;
}

static inline int edgefill_op (TFRAME *frame, TFILL *stack, const int ox, const int oy, int newColour, int edgeColour)
{
	if ((l_getPixel(frame, ox, oy)) == edgeColour)
		return 1;
	
	int x = 0 , y = 0;
	int colour;
	
	edgefill_push(stack, ox, oy);
	while(edgefill_pop(stack, &x, &y)){
		l_setPixel(frame, x, y, newColour);
		
		//check neighbour to the right
		colour = l_getPixel(frame, x+1, y);
		if (colour != newColour && colour != edgeColour){
			if (!edgefill_push(stack, x+1, y))
				return 0;
		}
			
		//check neighbour to the left
		colour = l_getPixel(frame, x-1, y);
		if (colour != newColour && colour != edgeColour){
			if (!edgefill_push(stack, x-1, y))
				return 0;
		}
		
		//check neighbour above
		colour = l_getPixel(frame, x, y+1);
		if (colour != newColour && colour != edgeColour){
			if (!edgefill_push(stack, x, y+1))
				return 0;
		}

		//check neighbour below
		colour = l_getPixel(frame, x, y-1);
		if (colour != newColour && colour != edgeColour){
			if (!edgefill_push(stack, x, y-1))
				return 0;
		}
	}
	return 1;
}

static inline int blurHuhtanen (TFRAME *des, int c1, int r1, int c2, int r2, const int iterations)
{
	if (c1 < 0) c1 = 0;
	if (c2 > des->width-1) c2 = des->width-1;
	if ((c2 - c1)+1 < 1) return 0;
	if (r1 < 0) r1 = 0;
	if (r2 > des->height-1) r2 = des->height-1;
	if ((r2 - r1)+1 < 1) return 0;
	
	int rgba[4] = {0,0,0,0};
	unsigned char *p;
	const int bpl = des->pitch;
	
	for (int loop = 0; loop < iterations; loop++){
		for (int col = c1; col < c2; col++){
			p = lGetPixelAddress(des, col, r1);
			for (int i = 0; i < 4; i++)
				rgba[i] = p[i] << 4;

			for (int j = r1; j < r2; j++){
				p += bpl;
				for (int i = 0; i < 4; i++)
					p[i] = (rgba[i] += (((p[i] << 4) - rgba[i])) >> 1) >> 4;
			}
		}
		for (int row = r1; row <= r2; row++){
			p = lGetPixelAddress(des, c1, row);
			for (int i = 0; i < 4; i++)
				rgba[i] = p[i] << 4;

			for (int j = c1; j < c2; j++){
				p += 4;
				for (int i = 0; i < 4; i++)
					p[i] = (rgba[i] += (((p[i] << 4) - rgba[i])) >> 1) >> 4;
			}
		}
		for (int col = c1; col <= c2; col++){
			p = lGetPixelAddress(des, col, r2);
			for (int i = 0; i < 4; i++)
				rgba[i] = p[i] << 4;

			for (int j = r1; j < r2; j++){
				p -= bpl;
				for (int i = 0; i < 4; i++)
					p[i] = (rgba[i] += (((p[i] << 4) - rgba[i])) >> 1) >> 4;
			}
		}
		for (int row = r1; row <= r2; row++){
			p = lGetPixelAddress(des, c2, row);
			for (int i = 0; i < 4; i++)
				rgba[i] = p[i] << 4;

			for (int j = c1; j < c2; j++){
				p -= 4;
				for (int i = 0; i < 4; i++)
					p[i] = (rgba[i] += (((p[i] << 4) - rgba[i])) >> 1) >> 4;
			}
		}
	}
	return 1;
}

// Stack Blur Algorithm by Mario Klingemann <mario@quasimondo.com>
static const unsigned int stack_blur8_mul[] =
{
    512,512,456,512,328,456,335,512,405,328,271,456,388,335,292,512,
    454,405,364,328,298,271,496,456,420,388,360,335,312,292,273,512,
    482,454,428,405,383,364,345,328,312,298,284,271,259,496,475,456,
    437,420,404,388,374,360,347,335,323,312,302,292,282,273,265,512,
    497,482,468,454,441,428,417,405,394,383,373,364,354,345,337,328,
    320,312,305,298,291,284,278,271,265,259,507,496,485,475,465,456,
    446,437,428,420,412,404,396,388,381,374,367,360,354,347,341,335,
    329,323,318,312,307,302,297,292,287,282,278,273,269,265,261,512,
    505,497,489,482,475,468,461,454,447,441,435,428,422,417,411,405,
    399,394,389,383,378,373,368,364,359,354,350,345,341,337,332,328,
    324,320,316,312,309,305,301,298,294,291,287,284,281,278,274,271,
    268,265,262,259,257,507,501,496,491,485,480,475,470,465,460,456,
    451,446,442,437,433,428,424,420,416,412,408,404,400,396,392,388,
    385,381,377,374,370,367,363,360,357,354,350,347,344,341,338,335,
    332,329,326,323,320,318,315,312,310,307,304,302,299,297,294,292,
    289,287,285,282,280,278,275,273,271,269,267,265,263,261,259
};

static const unsigned int stack_blur8_shr[] =
{
     9, 11, 12, 13, 13, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17,
    17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19,
    19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21,
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22,
    22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
    22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24
};

enum enumColorOrder
{ 
    R, 
    G, 
    B,
    A 
};

int blurStackFastAlpha (TFRAME *src, const int x1, const int y1, const int x2, const int y2, const int radius)
{
    const unsigned int *pImage = (unsigned int*)src->pixels;
    const unsigned int w = src->width;
    const unsigned int h = src->height;
    unsigned int r = radius;
    
    unsigned x = 0, y = 0, xp = 0, yp = 0, i = 0, t = 0;
    unsigned stack_ptr = 0;
    unsigned stack_start = 0;

    unsigned char* src_pix_ptr = NULL;
    unsigned char* dst_pix_ptr = NULL;
    unsigned char* stack_pix_ptr = NULL;
    unsigned int* stack_data_ptr = NULL;
    unsigned int* lpStack = NULL;

    unsigned sum_r = 0;
    unsigned sum_g = 0;
    unsigned sum_b = 0;
    unsigned sum_a = 0;
    unsigned sum_in_r = 0;
    unsigned sum_in_g = 0;
    unsigned sum_in_b = 0;
    unsigned sum_in_a = 0;
    unsigned sum_out_r = 0;
    unsigned sum_out_g = 0;
    unsigned sum_out_b = 0;
    unsigned sum_out_a = 0;

    const unsigned wm  = (w - 1);
    const unsigned hm  = (h - 1);
    unsigned div = 0;
    unsigned mul_sum = 0;
    unsigned shr_sum = 0;
    unsigned row_addr = 0;
    unsigned stride = 0;


    if ((NULL == pImage) || (w < 1 || h < 1 || r < 1))
        return 0;
    else if (r > 254)
        r = 254;


    mul_sum = stack_blur8_mul[r];
    shr_sum = stack_blur8_shr[r];

    div = ((r + r) + 1);

    lpStack = l_malloc(sizeof(unsigned int) * div);
    stack_data_ptr = lpStack;
    y = 0;

    do
    {
        sum_r = 
        sum_g = 
        sum_b = 
        sum_a = 
        sum_in_r = 
        sum_in_g = 
        sum_in_b = 
        sum_in_a = 
        sum_out_r = 
        sum_out_g = 
        sum_out_b = 
        sum_out_a = 0;

        row_addr = (y * w);
        src_pix_ptr = (unsigned char*)(pImage + row_addr);
        i = 0;

        do
        {
            t = (i + 1);
            stack_pix_ptr = (unsigned char*)(stack_data_ptr + i);

            *(stack_pix_ptr + R) = *(src_pix_ptr + R);
            *(stack_pix_ptr + G) = *(src_pix_ptr + G);
            *(stack_pix_ptr + B) = *(src_pix_ptr + B);
            *(stack_pix_ptr + A) = *(src_pix_ptr + A);

            sum_r += (*(stack_pix_ptr + R) * t);
            sum_g += (*(stack_pix_ptr + G) * t);
            sum_b += (*(stack_pix_ptr + B) * t);
            sum_a += (*(stack_pix_ptr + A) * t);

            sum_out_r += *(stack_pix_ptr + R);
            sum_out_g += *(stack_pix_ptr + G);
            sum_out_b += *(stack_pix_ptr + B);
            sum_out_a += *(stack_pix_ptr + A);

            if (i > 0)
            {
                t = (r + 1 - i);
                
                if (i <= wm) 
                {
                    src_pix_ptr += 4; 
                }

                stack_pix_ptr = (unsigned char*)(stack_data_ptr + (i + r));

                *(stack_pix_ptr + R) = *(src_pix_ptr + R);
                *(stack_pix_ptr + G) = *(src_pix_ptr + G);
                *(stack_pix_ptr + B) = *(src_pix_ptr + B);
                *(stack_pix_ptr + A) = *(src_pix_ptr + A);

                sum_r += (*(stack_pix_ptr + R) * t);
                sum_g += (*(stack_pix_ptr + G) * t);
                sum_b += (*(stack_pix_ptr + B) * t);
                sum_a += (*(stack_pix_ptr + A) * t);

                sum_in_r += *(stack_pix_ptr + R);
                sum_in_g += *(stack_pix_ptr + G);
                sum_in_b += *(stack_pix_ptr + B);
                sum_in_a += *(stack_pix_ptr + A);
            }
        }while(++i <= r);

        stack_ptr = r;
        xp = r;

        if (xp > wm) xp = wm;

        src_pix_ptr = (unsigned char*)(pImage + (xp + row_addr));
        dst_pix_ptr = (unsigned char*)(pImage + row_addr);
        x = 0;

        do
        {
            *(dst_pix_ptr + R) = ((sum_r * mul_sum) >> shr_sum);
            *(dst_pix_ptr + G) = ((sum_g * mul_sum) >> shr_sum);
            *(dst_pix_ptr + B) = ((sum_b * mul_sum) >> shr_sum);
            *(dst_pix_ptr + A) = ((sum_a * mul_sum) >> shr_sum);

            dst_pix_ptr += 4;

            sum_r -= sum_out_r;
            sum_g -= sum_out_g;
            sum_b -= sum_out_b;
            sum_a -= sum_out_a;

            stack_start = (stack_ptr + div - r);

            if (stack_start >= div) 
            {
                stack_start -= div;
            }

            stack_pix_ptr = (unsigned char*)(stack_data_ptr + stack_start);

            sum_out_r -= *(stack_pix_ptr + R);
            sum_out_g -= *(stack_pix_ptr + G);
            sum_out_b -= *(stack_pix_ptr + B);
            sum_out_a -= *(stack_pix_ptr + A);

            if (xp < wm) {
                src_pix_ptr += 4;
                ++xp;
            }

            *(stack_pix_ptr + R) = *(src_pix_ptr + R);
            *(stack_pix_ptr + G) = *(src_pix_ptr + G);
            *(stack_pix_ptr + B) = *(src_pix_ptr + B);
            *(stack_pix_ptr + A) = *(src_pix_ptr + A);

            sum_in_r += *(stack_pix_ptr + R);
            sum_in_g += *(stack_pix_ptr + G);
            sum_in_b += *(stack_pix_ptr + B);
            sum_in_a += *(stack_pix_ptr + A);

            sum_r += sum_in_r;
            sum_g += sum_in_g;
            sum_b += sum_in_b;
            sum_a += sum_in_a;

            if (++stack_ptr >= div) 
                stack_ptr = 0;

            stack_pix_ptr = (unsigned char*)(stack_data_ptr + stack_ptr);

            sum_out_r += *(stack_pix_ptr + R);
            sum_out_g += *(stack_pix_ptr + G);
            sum_out_b += *(stack_pix_ptr + B);
            sum_out_a += *(stack_pix_ptr + A);

            sum_in_r -= *(stack_pix_ptr + R);
            sum_in_g -= *(stack_pix_ptr + G);
            sum_in_b -= *(stack_pix_ptr + B);
            sum_in_a -= *(stack_pix_ptr + A);
        }while(++x < w);
    }while(++y < h);

    stride = (w << 2);
    stack_data_ptr = lpStack;
    x = 0;
    
    do
    {
        sum_r = 
        sum_g = 
        sum_b = 
        sum_a = 
        sum_in_r = 
        sum_in_g = 
        sum_in_b = 
        sum_in_a = 
        sum_out_r = 
        sum_out_g = 
        sum_out_b = 
        sum_out_a = 0;

        src_pix_ptr = (unsigned char*)(pImage + x);
        i = 0;

        do
        {
            t = (i + 1);
        
            stack_pix_ptr = (unsigned char*)(stack_data_ptr + i);

            *(stack_pix_ptr + R) = *(src_pix_ptr + R);
            *(stack_pix_ptr + G) = *(src_pix_ptr + G);
            *(stack_pix_ptr + B) = *(src_pix_ptr + B);
            *(stack_pix_ptr + A) = *(src_pix_ptr + A);

            sum_r += (*(stack_pix_ptr + R) * t);
            sum_g += (*(stack_pix_ptr + G) * t);
            sum_b += (*(stack_pix_ptr + B) * t);
            sum_a += (*(stack_pix_ptr + A) * t);

            sum_out_r += *(stack_pix_ptr + R);
            sum_out_g += *(stack_pix_ptr + G);
            sum_out_b += *(stack_pix_ptr + B);
            sum_out_a += *(stack_pix_ptr + A);

            if (i > 0){
                t = (r + 1 - i);
                
                if (i <= hm)
                    src_pix_ptr += stride; 

                stack_pix_ptr = (unsigned char*)(stack_data_ptr + (i + r));

                *(stack_pix_ptr + R) = *(src_pix_ptr + R);
                *(stack_pix_ptr + G) = *(src_pix_ptr + G);
                *(stack_pix_ptr + B) = *(src_pix_ptr + B);
                *(stack_pix_ptr + A) = *(src_pix_ptr + A);

                sum_r += (*(stack_pix_ptr + R) * t);
                sum_g += (*(stack_pix_ptr + G) * t);
                sum_b += (*(stack_pix_ptr + B) * t);
                sum_a += (*(stack_pix_ptr + A) * t);

                sum_in_r += *(stack_pix_ptr + R);
                sum_in_g += *(stack_pix_ptr + G);
                sum_in_b += *(stack_pix_ptr + B);
                sum_in_a += *(stack_pix_ptr + A);
            }
        }while(++i <= r);

        stack_ptr = r;
        yp = r;

        if (yp > hm) yp = hm;

        src_pix_ptr = (unsigned char*)(pImage + (x + (yp * w)));
        dst_pix_ptr = (unsigned char*)(pImage + x);
        y = 0;

        do
        {
            *(dst_pix_ptr + R) = ((sum_r * mul_sum) >> shr_sum);
            *(dst_pix_ptr + G) = ((sum_g * mul_sum) >> shr_sum);
            *(dst_pix_ptr + B) = ((sum_b * mul_sum) >> shr_sum);
            *(dst_pix_ptr + A) = ((sum_a * mul_sum) >> shr_sum);

            dst_pix_ptr += stride;

            sum_r -= sum_out_r;
            sum_g -= sum_out_g;
            sum_b -= sum_out_b;
            sum_a -= sum_out_a;

            stack_start = (stack_ptr + div - r);
            if (stack_start >= div)
            {
                stack_start -= div;
            }

            stack_pix_ptr = (unsigned char*)(stack_data_ptr + stack_start);

            sum_out_r -= *(stack_pix_ptr + R);
            sum_out_g -= *(stack_pix_ptr + G);
            sum_out_b -= *(stack_pix_ptr + B);
            sum_out_a -= *(stack_pix_ptr + A);

            if (yp < hm) {
                src_pix_ptr += stride;
                ++yp;
            }

            *(stack_pix_ptr + R) = *(src_pix_ptr + R);
            *(stack_pix_ptr + G) = *(src_pix_ptr + G);
            *(stack_pix_ptr + B) = *(src_pix_ptr + B);
            *(stack_pix_ptr + A) = *(src_pix_ptr + A);

            sum_in_r += *(stack_pix_ptr + R);
            sum_in_g += *(stack_pix_ptr + G);
            sum_in_b += *(stack_pix_ptr + B);
            sum_in_a += *(stack_pix_ptr + A);

            sum_r += sum_in_r;
            sum_g += sum_in_g;
            sum_b += sum_in_b;
            sum_a += sum_in_a;

            if (++stack_ptr >= div) stack_ptr = 0;

            stack_pix_ptr = (unsigned char*)(stack_data_ptr + stack_ptr);

            sum_out_r += *(stack_pix_ptr + R);
            sum_out_g += *(stack_pix_ptr + G);
            sum_out_b += *(stack_pix_ptr + B);
            sum_out_a += *(stack_pix_ptr + A);

            sum_in_r -= *(stack_pix_ptr + R);
            sum_in_g -= *(stack_pix_ptr + G);
            sum_in_b -= *(stack_pix_ptr + B);
            sum_in_a -= *(stack_pix_ptr + A);
        }while(++y < h);
    }while(++x < w);
    
    l_free(lpStack);
	return 1;
}

#if 1


static inline int clamp (int value)
{
	if (value > 255) value = 255;
	else if (value < 0) value = 0;
	
	return value;
}

void convolveAndTranspose8 (Kernel *restrict kernel, char *restrict inPixels, char *restrict outPixels, const int width, const int height, const int edgeAction)
{
	float *matrix = kernel->matrix;
	int cols = kernel->width;
	int cols2 = cols/2.0f;

	for (int y = 0; y < height; y++){
		int index = y;
		int ioffset = y*width;
			
		for (int x = 0; x < width; x++){
			float /*r = 0, g = 0, a = 0,*/ b = 0.0f;
			int moffset = cols2;
				
			for (int col = -cols2; col <= cols2; col++){
				float f = matrix[moffset+col];

				if (f != 0){
					int ix = x+col;
					if ( ix < 0 ){
						if (edgeAction == CLAMP_EDGES)
							ix = 0;
						else if (edgeAction == WRAP_EDGES)
							ix = (x+width) % width;
					}else if (ix >= width){
						if (edgeAction == CLAMP_EDGES)
							ix = width-1;
						else if (edgeAction == WRAP_EDGES)
							ix = (x+width) % width;
					}
					int rgb = inPixels[ioffset+ix];
					//a += f * ((rgb >> 24) & 0xff);
					//r += f * ((rgb >> 16) & 0xff);
					//g += f * ((rgb >> 8) & 0xff);
					b += f * (rgb & 0xff);
				}
			}
				
			//int ia = alpha ? clamp((int)(a+0.5)) : 0xff;
			//int ir = clamp((int)(r+0.5));
			//int ig = clamp((int)(g+0.5));
			int ib = clamp((int)(b+0.5f));
			outPixels[index] = /*(ia << 24) | (ir << 16) | (ig << 8) |*/ ib;
			index += height;
		}
	}
}

void convolveAndTranspose (Kernel *kernel, int *inPixels, int *outPixels, int width, int height, const int alpha, int edgeAction)
{
	float *matrix = kernel->matrix;
	int cols = kernel->width;
	int cols2 = cols/2.0;

	for (int y = 0; y < height; y++){
		int index = y;
		int ioffset = y*width;
			
		for (int x = 0; x < width; x++){
				
			float r = 0, g = 0, b = 0, a = 0;
			int moffset = cols2;
				
			for (int col = -cols2; col <= cols2; col++){
				float f = matrix[moffset+col];

				if (f != 0) {
					int ix = x+col;
					if ( ix < 0 ) {
						if ( edgeAction == CLAMP_EDGES)
							ix = 0;
						else if ( edgeAction == WRAP_EDGES)
							ix = (x+width) % width;
					}else if ( ix >= width){
						if ( edgeAction == CLAMP_EDGES)
							ix = width-1;
						else if ( edgeAction == WRAP_EDGES)
							ix = (x+width) % width;
					}
					int rgb = inPixels[ioffset+ix];
					a += f * ((rgb >> 24) & 0xff);
					r += f * ((rgb >> 16) & 0xff);
					g += f * ((rgb >> 8) & 0xff);
					b += f * (rgb & 0xff);
				}
			}
				
			int ia = alpha ? clamp((int)(a+0.5)) : 0xff;
			int ir = clamp((int)(r+0.5));
			int ig = clamp((int)(g+0.5));
			int ib = clamp((int)(b+0.5));
			outPixels[index] = (ia << 24) | (ir << 16) | (ig << 8) | ib;
			index += height;
		}
	}
}

Kernel *makeKernel (const float radius)
{
	
	Kernel *kernel = l_calloc(1, sizeof(Kernel));
	
	int r = (int)ceil(radius);
	int rows = r*2+1;
	
	kernel->width = rows;
	kernel->matrix = l_calloc(rows, sizeof(float));
		
	float sigma = radius/3.0f;
	float sigma22 = 2*sigma*sigma;
	float sigmaPi2 = 2*M_PI*sigma;
	float sqrtSigmaPi2 = (float)sqrtf(sigmaPi2);
	float radius2 = radius*radius;
	float total = 0;
	int index = 0;
		
	for (int row = -r; row <= r; row++){
		float distance = row*row;
			
		if (distance > radius2)
			kernel->matrix[index] = 0.0f;
		else
			kernel->matrix[index] = (float)exp(-(distance)/sigma22) / sqrtSigmaPi2;
			
		total += kernel->matrix[index];
		index++;
	}
		
	for (int i = 0; i < rows; i++)
		kernel->matrix[i] /= total;

	return kernel;
}

void deleteKernel (Kernel *kernel)
{
	l_free(kernel->matrix);
	l_free(kernel);
}
/*
static inline int blurGaussian (TFRAME *src, const float radius)
{
	Kernel *kernel = makeKernel(radius);
	if (kernel){
		ubyte *tmpPixels = l_malloc(src->frameSize);
		if (tmpPixels){
			convolveAndTranspose(kernel, (int*)src->pixels, (int*)tmpPixels, src->width, src->height, 1, CLAMP_EDGES);
			convolveAndTranspose(kernel, (int*)tmpPixels, (int*)src->pixels, src->height, src->width, 1, CLAMP_EDGES);
			l_free(tmpPixels);
		}
		deleteKernel(kernel);
		return 1;
	}
	return 0;
}*/


//#else

void blurGaussian (TFRAME *src, TFRAME *des)
{
	
#define _RGB(r,g,b)	(((r) << 16) | ((g) << 8) | (b))			// Convert to RGB
#define _GetRValue(c)	((int)(((c) & 0x00FF0000) >> 16))		// Red color component
#define _GetGValue(c)	((int)(((c) & 0x0000FF00) >> 8))		// Green color component
#define _GetBValue(c)	((int)( (c) & 0x000000FF))				// Blue color component
#define itofx(x) ((x) << 8)										// Integer to int point
#define fxtoi(x) ((x) >> 8)										// Fixed point to integer
#define Mulfx(x,y) (((x) * (y)) >> 8)							// Multiply a int by a int
#define Divfx(x,y) (((x) << 8) / (y))							// Divide a int by a int


	void *m_lpData = src->pixels;
	int m_iPitch = src->pitch;
	int m_iBpp = 4;
			
	int f_16 = itofx(16);
	int f_4 = itofx(4);
	int f_2 = itofx(2);

	int dwSize = m_iPitch * src->height;
	ubyte *lpData = (ubyte*)l_malloc(dwSize);

	int dwHorizontalOffset;
	int dwVerticalOffset = 0;
	int dwTotalOffset;
	int *lpSrcData = (int*)m_lpData;
	int *lpDstData = (int*)lpData;

	for (int i = 0; i < src->height; i++){
		dwHorizontalOffset = 0;
			
		for (int j = 0; j < src->width; j++){
			dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

			int dwSrcOffset = dwTotalOffset;
			int f_red = 0, f_green = 0, f_blue = 0;
			
			for (int k = -1; k <= 1; k++){
				int m = i + k;
				if (m < 0) m = 0;
				if (m >= src->height-1) m = src->height - 1;
				
				for (int l = -1; l <= 1; l++){
					int n = j + l;
					
					if (n < 0) n = 0;
					if (n >= src->width-1) n = src->width - 1;
					dwSrcOffset = m*m_iPitch + n*m_iBpp;
					
					if ((k == 0) && (l == 0)){
						f_red += Mulfx(itofx(_GetRValue(lpSrcData[dwSrcOffset>>2])),f_4);
						f_green += Mulfx(itofx(_GetGValue(lpSrcData[dwSrcOffset>>2])),f_4);
						f_blue += Mulfx(itofx(_GetBValue(lpSrcData[dwSrcOffset>>2])),f_4);
						
					}else if (((k == -1) && (l == 0)) || ((k == 0) && (l == -1)) || ((k == 0) && (l == 1)) || ((k == 1) && (l == 0))){
						f_red += Mulfx(itofx(_GetRValue(lpSrcData[dwSrcOffset>>2])),f_2);
						f_green += Mulfx(itofx(_GetGValue(lpSrcData[dwSrcOffset>>2])),f_2);
						f_blue += Mulfx(itofx(_GetBValue(lpSrcData[dwSrcOffset>>2])),f_2);
						
					}else{
						f_red += itofx(_GetRValue(lpSrcData[dwSrcOffset>>2]));
						f_green += itofx(_GetGValue(lpSrcData[dwSrcOffset>>2]));
						f_blue += itofx(_GetBValue(lpSrcData[dwSrcOffset>>2]));
					}
				}
			}
	
			ubyte red = (ubyte)MAX(0, MIN(fxtoi(Divfx(f_red,f_16)), 255));
			ubyte green = (ubyte)MAX(0, MIN(fxtoi(Divfx(f_green,f_16)), 255));
			ubyte blue = (ubyte)MAX(0, MIN(fxtoi(Divfx(f_blue,f_16)), 255));
			lpDstData[dwTotalOffset>>2] = (255<<24) | _RGB(red, green, blue);

			dwHorizontalOffset += m_iBpp;
		}
		dwVerticalOffset += m_iPitch;
	}
	l_free(des->pixels);
	des->pixels = lpData;
}
#endif

void blurGrey (char *srcPixels, char *dstPixels, const int width, const int height, const int radius)
{
	const int windowSize = radius * 2 + 1;
	const int radiusPlusOne = radius + 1;


	int sumBlue;
	int srcIndex = 0;
	int dstIndex = 0;
	int pixel, nextPixel, previousPixel, previousPixelIndex, nextPixelIndex;
	
	const int indexLookupTableLen = radiusPlusOne;
	const int sumLookupTableLen =  256 * windowSize;
	int *sumLookupTable = l_malloc(sizeof(int) * sumLookupTableLen * indexLookupTableLen);
		
	for (int i = 0; i < sumLookupTableLen; i++)
		sumLookupTable[i] = i / windowSize;
    
	int *indexLookupTable = &sumLookupTable[sumLookupTableLen];
	if (radius < width){
		for (int i = 0; i < indexLookupTableLen; i++)
			indexLookupTable[i] = i;
	}else{
		for (int i = 0; i < width; i++)
			indexLookupTable[i] = i;
		for (int i = width; i < indexLookupTableLen; i++)
			indexLookupTable[i] = width - 1;
	}

	for (int y = 0; y < height; y++){
		sumBlue = 0;
		dstIndex = y;

		pixel = srcPixels[srcIndex];
		sumBlue  += radiusPlusOne * (pixel & 0xFF);

		for (int i = 1; i <= radius; i++) {
			pixel = srcPixels[srcIndex + indexLookupTable[i]];
			sumBlue  +=  pixel & 0xFF;
		}

		for (int x = 0; x < width; x++){
			dstPixels[dstIndex] = sumLookupTable[sumBlue];
			dstIndex += height;

			nextPixelIndex = x + radiusPlusOne;
			if (nextPixelIndex >= width) {
				nextPixelIndex = width - 1;
			}

			previousPixelIndex = x - radius;
			if (previousPixelIndex < 0) {
				previousPixelIndex = 0;
			}

			nextPixel = srcPixels[srcIndex + nextPixelIndex];
			previousPixel = srcPixels[srcIndex + previousPixelIndex];
			sumBlue += nextPixel & 0xFF;
			sumBlue -= previousPixel & 0xFF;
		}
		srcIndex += width;
	}

	l_free(sumLookupTable);
}


static inline void blurRGB (int *srcPixels, int *dstPixels, const int width, const int height, const int radius)
{
	const int windowSize = radius * 2 + 1;
	const int radiusPlusOne = radius + 1;

	//int sumAlpha;
	int sumRed;
	int sumGreen;
	int sumBlue;

	int srcIndex = 0;
	int dstIndex = 0;
	int pixel, nextPixel, previousPixel, previousPixelIndex, nextPixelIndex;
	
	const int indexLookupTableLen = radiusPlusOne;
	const int sumLookupTableLen =  256 * windowSize;
	int *sumLookupTable = l_malloc(sizeof(int) * sumLookupTableLen * indexLookupTableLen);
		
	for (int i = 0; i < sumLookupTableLen; i++)
		sumLookupTable[i] = i / windowSize;
    
	int *indexLookupTable = &sumLookupTable[sumLookupTableLen];
	if (radius < width){
		for (int i = 0; i < indexLookupTableLen; i++)
			indexLookupTable[i] = i;
	}else{
		for (int i = 0; i < width; i++)
			indexLookupTable[i] = i;
		for (int i = width; i < indexLookupTableLen; i++)
			indexLookupTable[i] = width - 1;
	}

	for (int y = 0; y < height; y++){
		//sumAlpha = 0;
		sumRed = sumGreen = sumBlue = 0;
		dstIndex = y;

		pixel = srcPixels[srcIndex];
		// sumAlpha += radiusPlusOne * ((pixel >> 24) & 0xFF);
		sumRed   += radiusPlusOne * ((pixel >> 16) & 0xFF);
		sumGreen += radiusPlusOne * ((pixel >>  8) & 0xFF);
		sumBlue  += radiusPlusOne * ( pixel        & 0xFF);

		for (int i = 1; i <= radius; i++) {
			pixel = srcPixels[srcIndex + indexLookupTable[i]];
			// sumAlpha += (pixel >> 24) & 0xFF;
			sumRed   += (pixel >> 16) & 0xFF;
			sumGreen += (pixel >>  8) & 0xFF;
			sumBlue  +=  pixel        & 0xFF;
		}

		for (int x = 0; x < width; x++){
			dstPixels[dstIndex] = /*sumLookupTable[sumAlpha] << 24 |*/
									sumLookupTable[sumRed]   << 16 |
									sumLookupTable[sumGreen] <<  8 |
									sumLookupTable[sumBlue];
			dstIndex += height;

			nextPixelIndex = x + radiusPlusOne;
			if (nextPixelIndex >= width) {
				nextPixelIndex = width - 1;
			}

			previousPixelIndex = x - radius;
			if (previousPixelIndex < 0) {
				previousPixelIndex = 0;
			}

			nextPixel = srcPixels[srcIndex + nextPixelIndex];
			previousPixel = srcPixels[srcIndex + previousPixelIndex];

			//sumAlpha += (nextPixel     >> 24) & 0xFF;
			//sumAlpha -= (previousPixel >> 24) & 0xFF;
			sumRed += (nextPixel     >> 16) & 0xFF;
			sumRed -= (previousPixel >> 16) & 0xFF;
			sumGreen += (nextPixel     >> 8) & 0xFF;
			sumGreen -= (previousPixel >> 8) & 0xFF;
			sumBlue += nextPixel & 0xFF;
			sumBlue -= previousPixel & 0xFF;
		}
		srcIndex += width;
	}

	l_free(sumLookupTable);
}

static inline void blurARGB (int *srcPixels, int *dstPixels, const int width, const int height, const int radius)
{
	const int windowSize = radius * 2 + 1;
	const int radiusPlusOne = radius + 1;

	int sumAlpha;
	int sumRed = 0;
	int sumGreen = 0;
	int sumBlue = 0;

	int srcIndex = 0;
	int dstIndex = 0;
	int pixel;

	const int indexLookupTableLen = radiusPlusOne;
	const int sumLookupTableLen =  256 * windowSize;
	int *sumLookupTable = l_malloc(sizeof(int) * sumLookupTableLen * indexLookupTableLen);
		
	for (int i = 0; i < sumLookupTableLen; i++)
		sumLookupTable[i] = i / windowSize;
    
	int *indexLookupTable = &sumLookupTable[sumLookupTableLen];
        
	if (radius < width){
		for (int i = 0; i < indexLookupTableLen; i++)
			indexLookupTable[i] = i;
	}else{
		for (int i = 0; i < width; i++)
			indexLookupTable[i] = i;
		for (int i = width; i < indexLookupTableLen; i++)
			indexLookupTable[i] = width - 1;
	}

	for (int y = 0; y < height; y++){
		sumAlpha = 0;
		sumRed = sumGreen = sumBlue = 0;
		dstIndex = y;

		pixel = srcPixels[srcIndex];
		sumAlpha += radiusPlusOne * ((pixel >> 24) & 0xFF);
		sumRed   += radiusPlusOne * ((pixel >> 16) & 0xFF);
		sumGreen += radiusPlusOne * ((pixel >>  8) & 0xFF);
		sumBlue  += radiusPlusOne * ( pixel        & 0xFF);

		for (int i = 1; i <= radius; i++) {
			pixel = srcPixels[srcIndex + indexLookupTable[i]];
			sumAlpha += (pixel >> 24) & 0xFF;
			sumRed   += (pixel >> 16) & 0xFF;
			sumGreen += (pixel >>  8) & 0xFF;
			sumBlue  +=  pixel        & 0xFF;
		}

		for (int x = 0; x < width; x++){
			dstPixels[dstIndex] =   sumLookupTable[sumAlpha] << 24 |
									sumLookupTable[sumRed]   << 16 |
									sumLookupTable[sumGreen] <<  8 |
									sumLookupTable[sumBlue];
			dstIndex += height;

			int nextPixelIndex = x + radiusPlusOne;
			if (nextPixelIndex >= width) {
				nextPixelIndex = width - 1;
			}

			int previousPixelIndex = x - radius;
			if (previousPixelIndex < 0) {
				previousPixelIndex = 0;
			}

			int nextPixel = srcPixels[srcIndex + nextPixelIndex];
			int previousPixel = srcPixels[srcIndex + previousPixelIndex];

			sumAlpha += (nextPixel     >> 24) & 0xFF;
			sumAlpha -= (previousPixel >> 24) & 0xFF;
			sumRed += (nextPixel     >> 16) & 0xFF;
			sumRed -= (previousPixel >> 16) & 0xFF;
			sumGreen += (nextPixel     >> 8) & 0xFF;
			sumGreen -= (previousPixel >> 8) & 0xFF;
			sumBlue += nextPixel & 0xFF;
			sumBlue -= previousPixel & 0xFF;
		}
		srcIndex += width;
	}

	l_free(sumLookupTable);
}

int blur (TFRAME *src, TFRAME *workingBuffer, const int applyAlpha, const int radius)
{
	if (src->bpp == LFRM_BPP_32A || src->bpp == LFRM_BPP_32){
		if (applyAlpha == 1){
			blurARGB((int*)src->pixels, (int*)workingBuffer->pixels, src->width, src->height, radius);
			blurARGB((int*)workingBuffer->pixels, (int*)src->pixels, src->height, src->width, radius);
			return 1;
		}else{
			blurRGB((int*)src->pixels, (int*)workingBuffer->pixels, src->width, src->height, radius);
			blurRGB((int*)workingBuffer->pixels, (int*)src->pixels, src->height, src->width, radius);
			return 1;			
		}
	}
	return -1;
}

int blurArea (TFRAME *src, const int x1, const int y1, const int x2, const int y2, const int radius)
{
	if (src->bpp == LFRM_BPP_32A || src->bpp == LFRM_BPP_32)
		return blurHuhtanen(src, x1, y1, x2, y2, radius);
	return -1;
}

int blurImage (TFRAME *src, const int blurOp, const int radius)
{
	if (src->bpp == LFRM_BPP_32A || src->bpp == LFRM_BPP_32){
		if (blurOp == lBLUR_HUHTANEN){
			return blurHuhtanen(src, 0, 0, src->width-1, src->height-1, radius);
			
		}else if (blurOp == lBLUR_STACKFAST){
			return blurStackFastAlpha(src, 0, 0, src->width-1, src->height-1, radius);
			
		}else if (blurOp == lBLUR_GAUSSIAN){
			for (int i = 0; i < radius; i++)
				blurGaussian(src, src);
			return 1;
		}
	}
	return -1;
}

static inline int convolve2DSeparableRGB (int *in, int* out, const int dataSizeX, const int dataSizeY, 
                         const float* const kernelX, const int kSizeX, const float* const kernelY, const int kSizeY)
{
    
    // check validity of params
    if (!in || !out || !kernelX || !kernelY) return 0;
    if (dataSizeX <= 0 || kSizeX <= 0) return 0;

	size_t tmpLen = dataSizeX * dataSizeY;
	size_t sumLen = dataSizeX;
	size_t memSize = (3 * tmpLen  * sizeof(float)) + (3 * sumLen * sizeof(float));
	float *temp = l_malloc(memSize);
	if (!temp) return 0;

    float *tmp[3];                               // intermediate data buffer
	float *sum[3];
    tmp[0] = &temp[0];
    tmp[1] = &temp[tmpLen];
    tmp[2] = &temp[2 * tmpLen];

    sum[0] = &temp[(3 * tmpLen)];               // intermediate data buffer
    sum[1] = &temp[sumLen + (3 * tmpLen)];
    sum[2] = &temp[(2 * sumLen) + (3 * tmpLen)];
    
   // if(!sum) return 0;  // memory allocation error

    // covolve horizontal direction ///////////////////////

    // find center position of kernel (half of kernel size)
    int kCenter = kSizeX >> 1;                          // center index of kernel array
    int endIndex = dataSizeX - kCenter;                 // index for full kernel convolution

    // init working pointers
    int *inPtr = in;
    float *tmpPtr[3];
    tmpPtr[0] = tmp[0];                                   // store intermediate results from 1D horizontal convolution
    tmpPtr[1] = tmp[1];
    tmpPtr[2] = tmp[2];


    // start horizontal convolution (x-direction)
    for (int i=0; i < dataSizeY; ++i){                    // number of rows
        int kOffset = 0;                                // starting index of partial kernel varies for each sample

        // COLUMN FROM index=0 TO index=kCenter-1
        for (int j=0; j < kCenter; ++j){
            *tmpPtr[0] = 0;                            // init to 0 before accumulation
			*tmpPtr[1] = 0;
			*tmpPtr[2] = 0;
			
            for (int k = kCenter + kOffset, m = 0; k >= 0; --k, ++m){ // convolve with partial of kernel
                *tmpPtr[0] += (*(inPtr + m)&0x0000FF) * kernelX[k];
                *tmpPtr[1] += (*(inPtr + m)&0x00FF00) * kernelX[k];
                *tmpPtr[2] += (*(inPtr + m)&0xFF0000) * kernelX[k];
            }
            ++tmpPtr[0];                               // next output
            ++tmpPtr[1];
            ++tmpPtr[2];
            ++kOffset;                              // increase starting index of kernel
        }

        // COLUMN FROM index=kCenter TO index=(dataSizeX-kCenter-1)
        for(int j = kCenter; j < endIndex; ++j){
            *tmpPtr[0] = 0;                            // init to 0 before accumulate
            *tmpPtr[1] = 0;
            *tmpPtr[2] = 0;
            
            for(int k = kSizeX-1, m = 0; k >= 0; --k, ++m){  // full kernel
                *tmpPtr[0] += ((inPtr[m] )&0x0000FF) * kernelX[k];
                *tmpPtr[1] += ((inPtr[m] )&0x00FF00) * kernelX[k];
                *tmpPtr[2] += ((inPtr[m] )&0xFF0000) * kernelX[k];
            }
            ++inPtr;                                // next input
            ++tmpPtr[0];                               // next output
            ++tmpPtr[1];
            ++tmpPtr[2];
        }

        kOffset = 1;                                // ending index of partial kernel varies for each sample

        // COLUMN FROM index=(dataSizeX-kCenter) TO index=(dataSizeX-1)
        for(int j = endIndex; j < dataSizeX; ++j){
            *tmpPtr[0] = 0;                            // init to 0 before accumulation
            *tmpPtr[1] = 0;
            *tmpPtr[2] = 0;

            for(int k = kSizeX-1, m=0; k >= kOffset; --k, ++m){   // convolve with partial of kernel
                *tmpPtr[0] += (*(inPtr + m)&0x0000FF) * kernelX[k];
                *tmpPtr[1] += (*(inPtr + m)&0x00FF00) * kernelX[k];
                *tmpPtr[2] += (*(inPtr + m)&0xFF0000) * kernelX[k];
            }
            ++inPtr;                                // next input
            ++tmpPtr[0];                               // next output
            ++tmpPtr[1];
            ++tmpPtr[2];
            ++kOffset;                              // increase ending index of partial kernel
        }

        inPtr += kCenter;                           // next row
    }
    // END OF HORIZONTAL CONVOLUTION //////////////////////

    // start vertical direction ///////////////////////////

    // find center position of kernel (half of kernel size)
    kCenter = kSizeY >> 1;                          // center index of vertical kernel
    endIndex = dataSizeY - kCenter;                 // index where full kernel convolution should stop

    // set working pointers
    int *outPtr = out;
    float *tmpPtr2[3];
    tmpPtr[0] = tmpPtr2[0] = tmp[0];
    tmpPtr[1] = tmpPtr2[1] = tmp[1];
    tmpPtr[2] = tmpPtr2[2] = tmp[2];
    
    // clear out array before accumulation
    for (int i = 0; i < dataSizeX; ++i){
        sum[0][i] = 0;
        sum[1][i] = 0;
        sum[2][i] = 0;
	}

    // start to convolve vertical direction (y-direction)

    // ROW FROM index=0 TO index=(kCenter-1)
    int kOffset = 0;                                    // starting index of partial kernel varies for each sample
    for (int i=0; i < kCenter; ++i){
        for (int k = kCenter + kOffset; k >= 0; --k){     // convolve with partial kernel
        	float ktmp = kernelY[k];
            for (int j=0; j < dataSizeX; ++j){
                sum[0][j] += ((*tmpPtr[0]++)) * ktmp;
                sum[1][j] += ((*tmpPtr[1]++)) * ktmp;
                sum[2][j] += ((*tmpPtr[2]++)) * ktmp;
            }
        }

        for(int n = 0; n < dataSizeX; ++n){              // convert and copy from sum to out
 			*outPtr = ((int)sum[0][n]&0x0000FF) | ((int)sum[1][n]&0x00FF00) | ((int)sum[2][n]&0xFF0000);
            ++outPtr;                               // next element of output 			
            sum[0][n] = 0;                             // reset to zero for next summing
            sum[1][n] = 0;
            sum[2][n] = 0;
        }

        tmpPtr[0] = tmpPtr2[0];                           // reset input pointer
        tmpPtr[1] = tmpPtr2[1];
        tmpPtr[2] = tmpPtr2[2];
        ++kOffset;                                  // increase starting index of kernel
    }

    // ROW FROM index=kCenter TO index=(dataSizeY-kCenter-1)
    
    for (int i = kCenter; i < endIndex; ++i){
        for (int k = kSizeY -1; k >= 0; --k){             // convolve with full kernel
        	float ktmp = kernelY[k];
            for(int j = 0; j < dataSizeX; ++j){
                sum[0][j] += (*tmpPtr[0]++) * ktmp;
                sum[1][j] += (*tmpPtr[1]++) * ktmp;
                sum[2][j] += (*tmpPtr[2]++) * ktmp;
            }
        }

        for (int n = 0; n < dataSizeX; ++n){              // convert and copy from sum to out
            *outPtr++ = ((int)sum[0][n]&0x0000FF)  | ((int)sum[1][n]&0x00FF00) | ((int)sum[2][n]&0xFF0000);
            sum[0][n] = 0;                             // reset to 0 before next summing
            sum[1][n] = 0;
            sum[2][n] = 0;
        }

        // move to next row
        tmpPtr2[0] += dataSizeX;
        tmpPtr2[1] += dataSizeX;
        tmpPtr2[2] += dataSizeX;
        tmpPtr[0] = tmpPtr2[0];
        tmpPtr[1] = tmpPtr2[1];
        tmpPtr[2] = tmpPtr2[2];

    }

    // ROW FROM index=(dataSizeY-kCenter) TO index=(dataSizeY-1)
    kOffset = 1;                                    // ending index of partial kernel varies for each sample
    for (int i=endIndex; i < dataSizeY; ++i) {
        for(int k = kSizeY-1; k >= kOffset; --k){        // convolve with partial kernel
        	float ktmp = kernelY[k];
            for(int j=0; j < dataSizeX; ++j){
                sum[0][j] += (*tmpPtr[0]++) * ktmp;
                sum[1][j] += (*tmpPtr[1]++) * ktmp;
                sum[2][j] += (*tmpPtr[2]++) * ktmp;
            }
        }

        for(int n = 0; n < dataSizeX; ++n){              // convert and copy from sum to out
            *outPtr = ((int)sum[0][n]&0x0000FF) | ((int)sum[1][n]&0x00FF00) | ((int)sum[2][n]&0xFF0000);
            sum[0][n] = 0;                             // reset before next summing
            sum[1][n] = 0;
            sum[2][n] = 0;
            ++outPtr;                               // next output
        }

        // move to next row
        tmpPtr2[0] += dataSizeX;
        tmpPtr2[1] += dataSizeX;
        tmpPtr2[2] += dataSizeX;
        tmpPtr[0] = tmpPtr2[0];                           // next input
        tmpPtr[1] = tmpPtr2[1];
        tmpPtr[2] = tmpPtr2[2];        
        ++kOffset;                                  // increase ending index of kernel
    }
    // END OF VERTICAL CONVOLUTION ////////////////////////

    // deallocate temp buffers
    l_free(temp);

    return 1;
}

static inline int convolve2DSeparableARGB (int* in, int* out, const int dataSizeX, const int dataSizeY, 
                         const float* const kernelX, const int kSizeX, const float* const kernelY, const int kSizeY)
{
    float *tmp[4], *sum[4];                               // intermediate data buffer
    int *inPtr, *outPtr;                    // working pointers
    float *tmpPtr[4], *tmpPtr2[4];                        // working pointers
    int kCenter, kOffset, endIndex;                 // kernel indice
    float ktmp;
    
    // check validity of params
    if(!in || !out || !kernelX || !kernelY) return 0;
    if(dataSizeX <= 0 || kSizeX <= 0) return 0;

	size_t tmpLen = dataSizeX * dataSizeY;
	size_t sumLen = dataSizeX;

	size_t memSize = (4 * tmpLen  * sizeof(float)) + (4 * sumLen * sizeof(float));
	float *temp = malloc(memSize);
	if (!temp) return 0;

    tmp[0] = &temp[0];
    tmp[1] = &temp[tmpLen];
    tmp[2] = &temp[2 * tmpLen];
    tmp[3] = &temp[3 * tmpLen];

    sum[0] = &temp[(4 * tmpLen)];
    sum[1] = &temp[sumLen + (4 * tmpLen)];
    sum[2] = &temp[(2 * sumLen) + (4 * tmpLen)];
    sum[3] = &temp[(3 * sumLen) + (4 * tmpLen)];
    
   // if(!sum) return 0;  // memory allocation error

    // covolve horizontal direction ///////////////////////

    // find center position of kernel (half of kernel size)
    kCenter = kSizeX >> 1;                          // center index of kernel array
    endIndex = dataSizeX - kCenter;                 // index for full kernel convolution

    // init working pointers
    inPtr = in;
    tmpPtr[0] = tmp[0];                                   // store intermediate results from 1D horizontal convolution
    tmpPtr[1] = tmp[1];
    tmpPtr[2] = tmp[2];
    tmpPtr[3] = tmp[3];


    // start horizontal convolution (x-direction)
    for(int i = 0; i < dataSizeY; ++i){                    // number of rows
        kOffset = 0;                                // starting index of partial kernel varies for each sample

        // COLUMN FROM index=0 TO index=kCenter-1
        for(int j = 0; j < kCenter; ++j){
            *tmpPtr[0] = 0;                            // init to 0 before accumulation
			*tmpPtr[1] = 0;
			*tmpPtr[2] = 0;
			*tmpPtr[3] = 0;
			
            for(int k = kCenter + kOffset, m = 0; k >= 0; --k, ++m){ // convolve with partial of kernel
                *tmpPtr[0] += (*(inPtr + m)&0x000000FF) * kernelX[k];
                *tmpPtr[1] += (*(inPtr + m)&0x0000FF00) * kernelX[k];
                *tmpPtr[2] += (*(inPtr + m)&0x00FF0000) * kernelX[k];
                *tmpPtr[3] += (*(inPtr + m)&0xFF000000) * kernelX[k];
            }
            ++tmpPtr[0];                               // next output
            ++tmpPtr[1];
            ++tmpPtr[2];
            ++tmpPtr[3];
            ++kOffset;                              // increase starting index of kernel
        }

        // COLUMN FROM index=kCenter TO index=(dataSizeX-kCenter-1)
        for(int j = kCenter; j < endIndex; ++j){
            *tmpPtr[0] = 0;                            // init to 0 before accumulate
            *tmpPtr[1] = 0;
            *tmpPtr[2] = 0;
            *tmpPtr[3] = 0;
            
            for(int k = kSizeX-1, m = 0; k >= 0; --k, ++m){  // full kernel
                *tmpPtr[0] += ((inPtr[m] )&0x000000FF) * kernelX[k];
                *tmpPtr[1] += ((inPtr[m] )&0x0000FF00) * kernelX[k];
                *tmpPtr[2] += ((inPtr[m] )&0x00FF0000) * kernelX[k];
                *tmpPtr[3] += ((inPtr[m] )&0xFF000000) * kernelX[k];
            }
            ++inPtr;                                // next input
            ++tmpPtr[0];                               // next output
            ++tmpPtr[1];
            ++tmpPtr[2];
            ++tmpPtr[3];
        }

        kOffset = 1;                                // ending index of partial kernel varies for each sample

        // COLUMN FROM index=(dataSizeX-kCenter) TO index=(dataSizeX-1)
        for(int j = endIndex; j < dataSizeX; ++j){
            *tmpPtr[0] = 0;                            // init to 0 before accumulation
            *tmpPtr[1] = 0;
            *tmpPtr[2] = 0;
            *tmpPtr[3] = 0;

            for(int k = kSizeX-1, m=0; k >= kOffset; --k, ++m){   // convolve with partial of kernel
                *tmpPtr[0] += (*(inPtr + m)&0x000000FF) * kernelX[k];
                *tmpPtr[1] += (*(inPtr + m)&0x0000FF00) * kernelX[k];
                *tmpPtr[2] += (*(inPtr + m)&0x00FF0000) * kernelX[k];
                *tmpPtr[3] += (*(inPtr + m)&0xFF000000) * kernelX[k];
            }
            ++inPtr;                                // next input
            ++tmpPtr[0];                               // next output
            ++tmpPtr[1];
            ++tmpPtr[2];
            ++tmpPtr[3];
            
            ++kOffset;                              // increase ending index of partial kernel
        }

        inPtr += kCenter;                           // next row
    }
    // END OF HORIZONTAL CONVOLUTION //////////////////////

    // start vertical direction ///////////////////////////

    // find center position of kernel (half of kernel size)
    kCenter = kSizeY >> 1;                          // center index of vertical kernel
    endIndex = dataSizeY - kCenter;                 // index where full kernel convolution should stop

    // set working pointers
    outPtr = out;
    tmpPtr[0] = tmpPtr2[0] = tmp[0];
    tmpPtr[1] = tmpPtr2[1] = tmp[1];
    tmpPtr[2] = tmpPtr2[2] = tmp[2];
    tmpPtr[3] = tmpPtr2[3] = tmp[3];
    
    // clear out array before accumulation
    for(int i = 0; i < dataSizeX; ++i){
        sum[0][i] = 0;
        sum[1][i] = 0;
        sum[2][i] = 0;
        sum[3][i] = 0;
	}

    // start to convolve vertical direction (y-direction)

    // ROW FROM index=0 TO index=(kCenter-1)
    kOffset = 0;                                    // starting index of partial kernel varies for each sample
    for(int i = 0; i < kCenter; ++i){
        for(int k = kCenter + kOffset; k >= 0; --k){     // convolve with partial kernel
        	ktmp = kernelY[k];
            for(int j = 0; j < dataSizeX; ++j){
                sum[0][j] += ((*tmpPtr[0]++)) * ktmp;
                sum[1][j] += ((*tmpPtr[1]++)) * ktmp;
                sum[2][j] += ((*tmpPtr[2]++)) * ktmp;
                sum[3][j] += ((*tmpPtr[3]++)) * ktmp;
            }
        }

        for(int n = 0; n < dataSizeX; ++n){              // convert and copy from sum to out
 			*outPtr = ((int)sum[0][n]&0x0000FF) | ((int)sum[1][n]&0x00FF00) | ((int)sum[2][n]&0xFF0000) | ((int)sum[3][n]&0xFF000000);
            ++outPtr;                               // next element of output 			
            sum[0][n] = 0;                             // reset to zero for next summing
            sum[1][n] = 0;
            sum[2][n] = 0;
            sum[3][n] = 0;
        }

        tmpPtr[0] = tmpPtr2[0];                           // reset input pointer
        tmpPtr[1] = tmpPtr2[1];
        tmpPtr[2] = tmpPtr2[2];
        tmpPtr[3] = tmpPtr2[3];
        ++kOffset;                                  // increase starting index of kernel
    }

    // ROW FROM index=kCenter TO index=(dataSizeY-kCenter-1)
    
    for (int i = kCenter; i < endIndex; ++i){
        for (int k = kSizeY -1; k >= 0; --k){             // convolve with full kernel
        	ktmp = kernelY[k];
            for (int j = 0; j < dataSizeX; ++j){
                sum[0][j] += (*tmpPtr[0]++) * ktmp;
                sum[1][j] += (*tmpPtr[1]++) * ktmp;
                sum[2][j] += (*tmpPtr[2]++) * ktmp;
                sum[3][j] += (*tmpPtr[3]++) * ktmp;
            }
        }

        for(int n = 0; n < dataSizeX; ++n){              // convert and copy from sum to out
            *outPtr++ = ((int)sum[0][n]&0x0000FF)  | ((int)sum[1][n]&0x00FF00) | ((int)sum[2][n]&0xFF0000) | ((int)sum[3][n]&0xFF000000);
            sum[0][n] = 0;                             // reset to 0 before next summing
            sum[1][n] = 0;
            sum[2][n] = 0;
            sum[3][n] = 0;
        }

        // move to next row
        tmpPtr2[0] += dataSizeX;
        tmpPtr2[1] += dataSizeX;
        tmpPtr2[2] += dataSizeX;
        tmpPtr2[3] += dataSizeX;
        tmpPtr[0] = tmpPtr2[0];
        tmpPtr[1] = tmpPtr2[1];
        tmpPtr[2] = tmpPtr2[2];
        tmpPtr[3] = tmpPtr2[3];

    }

    // ROW FROM index=(dataSizeY-kCenter) TO index=(dataSizeY-1)
    kOffset = 1;                                    // ending index of partial kernel varies for each sample
    for (int i = endIndex; i < dataSizeY; ++i){
        for (int k = kSizeY-1; k >= kOffset; --k){        // convolve with partial kernel
        	ktmp = kernelY[k];
            for (int j = 0; j < dataSizeX; ++j){
                sum[0][j] += (*tmpPtr[0]++) * ktmp;
                sum[1][j] += (*tmpPtr[1]++) * ktmp;
                sum[2][j] += (*tmpPtr[2]++) * ktmp;
                sum[3][j] += (*tmpPtr[3]++) * ktmp;
            }
        }

        for (int n = 0; n < dataSizeX; ++n){              // convert and copy from sum to out
            *outPtr = ((int)sum[0][n]&0x0000FF) | ((int)sum[1][n]&0x00FF00) | ((int)sum[2][n]&0xFF0000) | ((int)sum[3][n]&0xFF000000);
            sum[0][n] = 0;                             // reset before next summing
            sum[1][n] = 0;
            sum[2][n] = 0;
            sum[3][n] = 0;
            ++outPtr;                               // next output
        }

        // move to next row
        tmpPtr2[0] += dataSizeX;
        tmpPtr2[1] += dataSizeX;
        tmpPtr2[2] += dataSizeX;
        tmpPtr2[3] += dataSizeX;
        tmpPtr[0] = tmpPtr2[0];                           // next input
        tmpPtr[1] = tmpPtr2[1];
        tmpPtr[2] = tmpPtr2[2];
        tmpPtr[3] = tmpPtr2[3];
        ++kOffset;                                  // increase ending index of kernel
    }
    // END OF VERTICAL CONVOLUTION ////////////////////////

    // deallocate temp buffers
    free(temp);

    return 1;
}

int convolve2DSeparable (TFRAME *in, TFRAME *out, const float* const kernelX, const int kSizeX, const float* const kernelY, const int kSizeY)
{
	if (in->bpp == LFRM_BPP_32A && (out->bpp == LFRM_BPP_32A || out->bpp == LFRM_BPP_32)){
		convolve2DSeparableARGB((int*)in->pixels, (int*)out->pixels,
		  in->width, in->height, 
		  kernelX, kSizeX,
		  kernelY, kSizeY);
		return 1;
	}else if (in->bpp == LFRM_BPP_32 && (out->bpp == LFRM_BPP_32 || out->bpp == LFRM_BPP_32A)){
		convolve2DSeparableRGB((int*)in->pixels, (int*)out->pixels,
		  in->width, in->height,
		  kernelX, kSizeX,
		  kernelY, kSizeY);
		return 1;
	}
	return 0;
}

static inline int convolve2DRGB (int *in, int *out, const int dataSizeX, const int dataSizeY, 
               float *const kernel, const int kernelSizeX, const int kernelSizeY)
{
    int *inPtr, *inPtr2;
    int rowMin, rowMax;                             // to check boundary of input array
    int colMin, colMax;                             //
    //float sum;                                      // temp accumulation buffer
    float sum[4];

    // check validity of params
    if (!in || !out || !kernel) return 0;
    if (dataSizeX <= 0 || kernelSizeX <= 0) return 0;

    // find center position of kernel (half of kernel size)
    int kCenterX = kernelSizeX >> 1;
    int kCenterY = kernelSizeY >> 1;

    // init working  pointers
    inPtr = inPtr2 = &in[dataSizeX * kCenterY + kCenterX];  // note that  it is shifted (kCenterX, kCenterY),
    int *outPtr = out;
    float *kPtr = kernel;

    // start convolution
    for (int i = 0; i < dataSizeY; ++i){                   // number of rows
        // compute the range of convolution, the current row of kernel should be between these
        rowMax = i + kCenterY;
        rowMin = i - dataSizeY + kCenterY;

		outPtr = &out[i * dataSizeX];

        for (int j = 0; j < dataSizeX; ++j){              // number of columns
            // compute the range of convolution, the current column of kernel should be between these
            colMax = j + kCenterX;
            colMin = j - dataSizeX + kCenterX;

            sum[0] = 0;                                // set to 0 before accumulate
            sum[1] = 0;
            sum[2] = 0;

            // flip the kernel and traverse all the kernel values
            // multiply each kernel value with underlying input data
            for (int m = 0; m < kernelSizeY; ++m){        // kernel rows
                // check if the index is out of bound of input array
                if (m <= rowMax && m > rowMin){
                    for (int n = 0; n < kernelSizeX; ++n){
                        // check the boundary of array
                        if (n <= colMax && n > colMin){
                            sum[0] += ((int)*(inPtr - n)&0xFF0000) * *kPtr;
                            sum[1] += ((int)*(inPtr - n)&0x00FF00) * *kPtr;
                            sum[2] += ((int)*(inPtr - n)&0x0000FF) * *kPtr;
                          }

                        ++kPtr;                     // next kernel
                    }
                }else{
                    kPtr += kernelSizeX;            // out of bound, move to next row of kernel
				}
                inPtr -= dataSizeX;                 // move input data 1 raw up
            }

            *outPtr++ = 0xFF000000|(((int)sum[0]&0xFF0000) | ((int)sum[1]&0x00FF00) | ((int)sum[2]&0x0000FF));
            kPtr = kernel;                          // reset kernel to (0,0)
            inPtr = ++inPtr2;                       // next input
        }
    }

    return 1;
}


static inline int convolve2DARGB (int *in, int *out, const int dataSizeX, const int dataSizeY, 
               float *const kernel, const int kernelSizeX, const int kernelSizeY)
{
    int *inPtr, *inPtr2;
    int rowMin, rowMax;                             // to check boundary of input array
    int colMin, colMax;                             //
    float sum[4];                                      // temp accumulation buffer

    // check validity of params
    if (!in || !out || !kernel) return 0;
    if (dataSizeX <= 0 || kernelSizeX <= 0) return 0;

    // find center position of kernel (half of kernel size)
    int kCenterX = kernelSizeX >> 1;
    int kCenterY = kernelSizeY >> 1;

    // init working  pointers
    inPtr = inPtr2 = &in[dataSizeX * kCenterY + kCenterX];  // note that it is shifted (kCenterX, kCenterY),
    int *outPtr = out;
    float *kPtr = kernel;

    // start convolution
    for (int i= 0; i < dataSizeY; ++i){                   // number of rows
        // compute the range of convolution, the current row of kernel should be between these
        rowMax = i + kCenterY;
        rowMin = i - dataSizeY + kCenterY;

		outPtr = &out[i * dataSizeX];

        for (int j = 0; j < dataSizeX; ++j){              // number of columns
            // compute the range of convolution, the current column of kernel should be between these
            colMax = j + kCenterX;
            colMin = j - dataSizeX + kCenterX;

            sum[0] = 0;                                // set to 0 before accumulate
            sum[1] = 0;
            sum[2] = 0;
            sum[3] = 0;

            // flip the kernel and traverse all the kernel values
            // multiply each kernel value with underlying input data
            for (int m = 0; m < kernelSizeY; ++m){        // kernel rows
                // check if the index is out of bound of input array
                if (m <= rowMax && m > rowMin){
                    for (int n = 0; n < kernelSizeX; ++n){
                        // check the boundary of array
                        if (n <= colMax && n > colMin){
                            sum[0] += ((int)*(inPtr - n)&0xFF0000) * *kPtr;
                            sum[1] += ((int)*(inPtr - n)&0x00FF00) * *kPtr;
                            sum[2] += ((int)*(inPtr - n)&0x0000FF) * *kPtr;
                            sum[3] += ((int)*(inPtr - n)&0xFF000000) * *kPtr;
                          }

                        ++kPtr;                     // next kernel
                    }
                }else{
                    kPtr += kernelSizeX;            // out of bound, move to next row of kernel
				}
                inPtr -= dataSizeX;                 // move input data 1 raw up
            }

            *outPtr++ = ((int)sum[0]&0xFF0000) | ((int)sum[1]&0x00FF00) | ((int)sum[2]&0x0000FF) | ((int)sum[3]&0xFF000000);
            kPtr = kernel;                          // reset kernel to (0,0)
            inPtr = ++inPtr2;                       // next input
        }
    }

    return 1;
}

int convolve2D (TFRAME *const in, TFRAME *out, float* const kernel, const int kSizeX, const int kSizeY)
{
	if (in->bpp == LFRM_BPP_32A && (out->bpp == LFRM_BPP_32A || out->bpp == LFRM_BPP_32)){
		convolve2DARGB((int*)in->pixels, (int*)out->pixels,
		  in->width, in->height,  kernel, kSizeX, kSizeY);
		return 1;
	}else if (in->bpp == LFRM_BPP_32 && (out->bpp == LFRM_BPP_32 || out->bpp == LFRM_BPP_32A)){
		convolve2DRGB((int*)in->pixels, (int*)out->pixels,
		  in->width, in->height,  kernel, kSizeX, kSizeY);
		return 1;
	}
	return 0;
}


#else

int invertFrame (TFRAME *frm){return 0;}
int invertArea (TFRAME *frm, int x1, int y1, int x2, int y2){return 0;}
int drawRectangleDotted (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour){return 0;}
int drawRectangleFilled (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour){return 0;}



#endif

