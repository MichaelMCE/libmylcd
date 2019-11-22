
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


#ifndef _DRAW_H_
#define _DRAW_H_




#define WRAP_EDGES		1
#define CLAMP_EDGES		2

typedef struct{
	int width;
	float *matrix;
}Kernel;


typedef struct{
	T2POINT *stack;
	size_t size;	// maximum number of x/y points that may be stored
	int position;
	int width;
	int height;
	TFRAME *frame;
}TFILL;



int drawTriangleFilled (TFRAME *frame, const int x1, const int y1, const int x2, const int y2, const int x3, const int y3, const int colour);
int drawTriangle (TFRAME *frame, int x1, int y1, int x2, int y2, int x3, int y3, const int colour);
int drawRectangleDottedFilled (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour);
int drawRectangleDotted (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour);
int drawRectangleFilled (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour);
int drawRectangle (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour);
int drawLine (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour);
int drawLineDotted (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour);
int drawCircle (TFRAME *frm, const int xc, const int yc, const int radius, const int colour);
int drawCircleFilled (TFRAME *frame, int xc, int yc, int radius, const int colour);

int drawMask (TFRAME *src, TFRAME *mask, TFRAME *des, int maskXOffset, int maskYOffset, const int mode);
int drawMaskA (const TFRAME *src, const TFRAME *mask, TFRAME *des, const int Xoffset, const int Yoffset, const int srcX1, const int srcY1, const int srcX2, const int srcY2, const float alpha);
int invertArea (TFRAME *frm, int x1, int y1, int x2, int y2);
int invertFrame (TFRAME *frm);

int drawArc (TFRAME *frm, int x, int y, int r1, int r2, double a1, double a2, const int colour);
int drawEnclosedArc (TFRAME *frm, const int x, const int y, const int r1, const int r2, double a1, double a2, const int colour);
int drawEllipse (TFRAME *frm, const int x, const int y, const int r1, const int r2, const int colour);
int drawPolyLineTo (TFRAME *frm, TPOINTXY *pt, int tPoints, const int colour);
int drawPolyLineDottedTo (TFRAME *frm, T2POINT *pt, int tPoints, const int colour);
int drawPolyLine (TFRAME *frm, T2POINT *pt, int tPoints, const int colour);
int drawPolyLineEx (TFRAME *frm, TLPOINTEX *pt, int n, const int colour);
int floodFill (TFRAME *frame, int x, int y, const int colour);
int edgeFill (TFRAME *frame, int x, int y, int fillColour, int edgeColour);

int blur (TFRAME *src, TFRAME *workingBuffer, const int applyAlpha, const int radius);
int blurArea (TFRAME *src, const int x1, const int y1, const int x2, const int y2, const int radius_iter);
int blurImage (TFRAME *src, const int blurOp, const int radius_iter);

int convolve2DSeparable (TFRAME *const src, TFRAME *des, const float* const kernelX, const int kSizeX, const float* const kernelY, const int kSizeY);
int convolve2D (TFRAME *const src, TFRAME *des, float* const kernel, const int kSizeX, const int kSizeY);

void blurGrey (char *srcPixels, char *dstPixels, const int width, const int height, const int radius);

Kernel *makeKernel (float radius);
void deleteKernel (Kernel *kernel);
void convolveAndTranspose8 (Kernel *kernel, char *inPixels, char *outPixels, int width, int height, int edgeAction);

#endif 
