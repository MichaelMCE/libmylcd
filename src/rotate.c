
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

#if (__BUILD_ROTATE_SUPPORT__)

#include <math.h>
#include "frame.h"
#include "utils.h"
#include "pixel.h"
#include "lmath.h"


static inline double rot_x (const double angle, const double x, const double y);
static inline double rot_y (const double angle, const double x, const double y);
static inline int getPixel32a_BL (TFRAME *frame, float x, float y);	// bilinear filtering



int rotateFrameEx (TFRAME *src, TFRAME *des, const float xang, const float yang, const float zang, float flength, const float zoom, const int destx, const int desty)
{
	float ox, oy;
	//float oz;
	float nx, ny, nz;
	float tmp;

	const int xw = src->width>>1;
	const int yh = src->height>>1;
		
	// pre calc some values
	const float ThetaX = xang/180.0f*M_PI;
	const float ThetaY = yang/180.0f*M_PI;
	const float ThetaZ = zang/180.0f*M_PI;
	
	const float cx = l_cosf(ThetaX);
    const float cy = l_cosf(ThetaY);
    const float cz = l_cosf(ThetaZ);
    const float sx = l_sinf(ThetaX);
    const float sy = l_sinf(ThetaY);
    const float sz = l_sinf(ThetaZ);
	
	//X axis
	const float xx = cy*cz;
	const float xy = sx*sy*cz - cx*sz;
//	const float xz = cx*sy*cz + sx*sz;

	//Y axis
	const float yx = cy*sz;
	const float yy = cx*cz + sx*sy*sz;
//	const float yz = -sx*cz + cx*sy*sz;

	//Z axis
	const float zx = -sy;
	const float zy = sx*cy;
//	const float zz = cx*cy;
	
//	oz = 0.0;
	const int w = src->width-1;
	const int h = src->height-1;
	
  	for (float y = h; y >= 0.0f; y -= 0.5f){
		oy = (y-yh);   	
		for (float x = w; x >= 0.0f; x -= 0.5f){
			ox = (x-xw);
			//oz = 0.0;

			nx = ox*xx + oy*xy /*+ oz*xz*/;
			ny = ox*yx + oy*yy /*+ oz*yz*/;
			nz = ox*zx + oy*zy /*+ oz*zz*/;

			if (nz-zoom >= 0.0f){
				tmp = flength/(nz-zoom);
				l_setPixel(des,(nx*tmp)+destx, (ny*tmp)+desty, l_getPixel_NB(src,x,y));
				//l_setPixel(des,(nx*tmp)+destx, (ny*tmp)+desty, getPixel32a_BL(src,x,y));
				
			}
   		}
	}
	return 1;
}

int rotate (TFRAME *src, TFRAME *des, const int desx, const int desy, double angle)
{

    const int d_w = des->width;
    const int d_h = des->height;
    const double dx_x = rot_x(-angle, 1.0, 0.0);
    const double dx_y = rot_y(-angle, 1.0, 0.0);
    const double dy_x = rot_x(-angle, 0.0, 1.0);
    const double dy_y = rot_y(-angle, 0.0, 1.0);
    double x1 = rot_x(-angle, -d_w/2.0, -d_h/2.0) - (double)desx + d_w/2.0;
    double y1 = rot_y(-angle, -d_w/2.0, -d_h/2.0) - (double)desy + d_h/2.0;
    double x2;
	double y2;
	
	if (src->bpp == LFRM_BPP_32A){
		for (int y = 0; y < d_h; y++){
			x2 = x1;
			y2 = y1;

			for (int x = 0; x < d_w; x++){
				if (!checkbounds(src, x2, y2)){
					l_setPixel_NB(des, x, y, getPixel32a_BL(src, x2, y2));
				}
				x2 += dx_x;
				y2 += dx_y;
			}
			x1 += dy_x;
			y1 += dy_y;
		}
	}else{
		for (int y = 0; y < d_h; y++){
			x2 = x1;
			y2 = y1;
			for (int x = 0; x < d_w; x++){
				if (!checkbounds(src, x2, y2))
					l_setPixel_NB(des, x, y, l_getPixel_NB(src, x2, y2));
				/*else
					l_setPixel_NB(des, x, y, background);*/
				x2 += dx_x;
				y2 += dx_y;
			}
			x1 += dy_x;
			y1 += dy_y;
		}
	}
    return 1;
}

int rotateFrameR90 (TFRAME *frm)
{
	TFRAME *tmp = NULL;
	tmp = _newFrame(frm->hw, frm->height, frm->width, 1, frm->bpp);
	if (tmp == NULL) return 0;
	
	int x,y;
	int dx=tmp->width-1,dy=0;
	for (y=0;y < frm->height;y++){
		dy=0;
		for (x=0;x < frm->width;x++){
			l_setPixel(tmp,dx,dy,l_getPixel_NB(frm,x,y));
			dy++;
		}
		dx--;
	}

	ubyte *buffer = frm->pixels;		//ensure source frame is freed
	frm->pixels = tmp->pixels;
	tmp->pixels = buffer;
	frm->height = tmp->height;
	frm->width = tmp->width;
	frm->pitch = tmp->pitch;
	frm->bpp = tmp->bpp;
	frm->style = tmp->style;
	deleteFrame(tmp);
	
	return 1;
}

int rotateFrameL90 (TFRAME *frm)
{
	TFRAME *tmp = _newFrame(frm->hw, frm->height, frm->width, 1, frm->bpp);
	if (tmp == NULL) return 0;

	int x,y;
	int dx=0,dy=0;

	for (y=0; y < frm->height; y++){
		dy = tmp->height-1;
		if (tmp->bpp == LFRM_BPP_32A){
			for (x=0; x < frm->width; x++){
				l_setPixel(tmp,dx,dy, 0xFF000000|l_getPixel_NB(frm,x,y));
				dy--;
			}
		}else{
			for (x=0; x < frm->width; x++){
				l_setPixel(tmp,dx,dy, l_getPixel_NB(frm,x,y));
				dy--;
			}
		}
		dx++;
	}

	ubyte *buffer = frm->pixels;		//ensure source frame is freed
	frm->pixels = tmp->pixels;
	tmp->pixels = buffer;
	frm->height = tmp->height;
	frm->width = tmp->width;
	frm->pitch = tmp->pitch;
	frm->bpp = tmp->bpp;
	frm->style = tmp->style;
	deleteFrame(tmp);

	return 1;
}

void rotateX (const float angle, const float y, const float z, float *yr, float *zr)
{
	*yr = y*l_cosf(angle) - z*l_sinf(angle);
	*zr = y*l_sinf(angle) + z*l_cosf(angle);
}

void rotateY (const float angle, const float x, const float z, float *xr, float *zr)
{
	*xr =  x*l_cosf(angle) + z*l_sinf(angle);
	*zr = -x*l_sinf(angle) + z*l_cosf(angle);
}

void rotateZ (const float angle, const float x, const float y, float *xr, float *yr)
{
	*xr = x*l_cosf(angle) - y*l_sinf(angle);
	*yr = x*l_sinf(angle) + y*l_cosf(angle);
}

void point3DTo2D (const float x, const float y, const float z, const float flength, const float camz, const int x0, const int y0, int *screenx, int *screeny)
{
	if (z-camz < 0.0f) return;
	const float tmp = flength/(z-camz);
	*screenx = (x*tmp)+x0;
	*screeny = (y*tmp)+y0;
}

static inline double rot_x (const double angle, const double x, const double y)
{
    return (x * l_cosf(angle/180.0*M_PI) + y * -l_sinf(angle/180.0*M_PI));
}

static inline double rot_y (const double angle, const double x, const double y)
{
    return (x * l_sinf(angle/180.0*M_PI) + y * l_cosf(angle/180.0*M_PI));
}

static inline int getPixel32a_BL (TFRAME *frame, float x, float y)
{
	#define PSEUDO_FLOOR( V ) ((V) >= 0 ? (int)(V) : (int)((V) - 1))
	
	const int width = frame->width;
	const int height = frame->height;
	const int pitch = frame->pitch;
	const int spp = 4;	// samples per pixel
		
	if (x < 0) x = 0;

	float x1 = PSEUDO_FLOOR(x);
	float x2 = x1+1.0;
	
	if (x2 >= width) {
		x = x2 = (float)width-1.0;
		x1 = x2 - 1;
	}
	const int wx1 = (int)(256*(x2 - x));
	const int wx2 = 256-wx1;

	if (y < 0) y = 0;
	
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
	unsigned char *px1y1 = &frame->pixels[pitch * (int)y1 + spp * (int)x1];
	unsigned char *px2y1 = px1y1 + spp;
	unsigned char *px1y2 = px1y1 + pitch;
	unsigned char *px2y2 = px1y1 + pitch+spp;


	TCOLOUR4 clr;
	TCOLOUR4 *c = &clr;
	const TCOLOUR4 *restrict cx1y1 = (TCOLOUR4*)px1y1;
	const TCOLOUR4 *restrict cx2y1 = (TCOLOUR4*)px2y1;
	const TCOLOUR4 *restrict cx1y2 = (TCOLOUR4*)px1y2;
	const TCOLOUR4 *restrict cx2y2 = (TCOLOUR4*)px2y2;
	
	c->u.bgra.r = (wx1y1 * cx1y1->u.bgra.r + wx2y1 * cx2y1->u.bgra.r + wx1y2 * cx1y2->u.bgra.r + wx2y2 * cx2y2->u.bgra.r + 128*256) / (256*256);
	c->u.bgra.g = (wx1y1 * cx1y1->u.bgra.g + wx2y1 * cx2y1->u.bgra.g + wx1y2 * cx1y2->u.bgra.g + wx2y2 * cx2y2->u.bgra.g + 128*256) / (256*256);
	c->u.bgra.b = (wx1y1 * cx1y1->u.bgra.b + wx2y1 * cx2y1->u.bgra.b + wx1y2 * cx1y2->u.bgra.b + wx2y2 * cx2y2->u.bgra.b + 128*256) / (256*256);
	c->u.bgra.a = (wx1y1 * cx1y1->u.bgra.a + wx2y1 * cx2y1->u.bgra.a + wx1y2 * cx1y2->u.bgra.a + wx2y2 * cx2y2->u.bgra.a + 128*256) / (256*256);
	return c->u.colour;
}

#endif
