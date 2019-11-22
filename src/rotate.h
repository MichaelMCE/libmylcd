
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

#ifndef _ROTATE_H_
#define _ROTATE_H_



int rotateFrameEx (TFRAME *src, TFRAME *des, const float xang, const float yang, const float zang, float flength, const float zoom, const int destx, const int desty);
int rotate (TFRAME *src, TFRAME *des, const int desx, const int desy, double angle);
int rotateFrameR90 (TFRAME *frm);
int rotateFrameL90 (TFRAME *frm);

void rotateX (const float angle, const float y, const float z, float *yr, float *zr);
void rotateY (const float angle, const float x, const float z, float *xr, float *zr);
void rotateZ (const float angle, const float x, const float y, float *xr, float *yr);
void point3DTo2D (const float x,const float y,const float z,const float flength,const float camz, const int x0, const  int y0, int *screenx, int *screeny);

#endif

