
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
//  GNU LIBRARY GENERAL PUBLIC LICENSE for details.



#include "utils/utils.h"


#define DEGTORAD 0.0174532925195


typedef struct {
	float x;
	float y;
	float z;
}TVECTORF;


static void translate (TVECTORF *v, float dx, float dy, float dz)
{
	v->x += dx;
	v->y += dy;
	v->z += dz;
}

int main (int argc, char* argv[])
{

	if (!utilInitConfig("config.cfg"))
		return EXIT_FAILURE;


	const int RED = 255<<24 | lGetRGBMask(frame, LMASK_RED);
	const int GREEN = 255<<24 | lGetRGBMask(frame, LMASK_GREEN);
	const int BLUE = 255<<24 | lGetRGBMask(frame, LMASK_BLUE);
	const int BLACK = 255<<24 | lGetRGBMask(frame, LMASK_BLACK);
	const int WHITE = 255<<24 | lGetRGBMask(frame, LMASK_WHITE);

	lSetBackgroundColour(hw, WHITE);
	lSetForegroundColour(hw, BLACK);
	int colour[6] = {BLUE, GREEN, RED, RED|GREEN, RED|BLUE, GREEN|BLUE};
	
	const float dx = frame->width/2;	// x distination of drawing(s)
	const float dy = frame->height/2;	// y distination of drawing(s)
	const float d = frame->width/2;		// size of triangles
	const float s = 10.0;		// size of inner square
	const float x = -(d/2.0);	// x center position offset
	const float y = -(d/2.0);	// y center position offset
	
	
	static TVECTORF v[3][3];	// source
	static TVECTORF vr[3][3];	// rotated
	//static TVECTORF vr[3][3];	// translated

	v[0][0].x = x+d;
	v[0][0].y = y+d;
	v[0][1].x = x+d;
	v[0][1].y = y;
	v[0][2].x = x;
	v[0][2].y = y;

	v[1][0].x = x;
	v[1][0].y = y+d;
	v[1][1].x = x+d;
	v[1][1].y = y+d;
	v[1][2].x = x;
	v[1][2].y = y;

	v[2][0].x = x+d;
	v[2][0].y = y;
	v[2][1].x = x;
	v[2][1].y = y+d;
	v[2][2].x = x;
	v[2][2].y = y;


	float xc,yc;
	
	for (float a = 0.0; a <= 360.0; a += 1.0){
		lClearFrame(frame);

		for (int i = 0; i < 2/*3*/; i++){
			for (int j = 0; j < 3; j++){
				lRotateZ(a * DEGTORAD, v[i][j].x, v[i][j].y, &vr[i][j].x, &vr[i][j].y);
				translate(&vr[i][j], dx, dy, 0.0);
			}
			lDrawTriangle(frame, vr[i][0].x, vr[i][0].y, vr[i][1].x, vr[i][1].y, vr[i][2].x, vr[i][2].y, colour[i]);
		}
		
		for (int i = 0; i < 2/*3*/; i++){
			xc = ((vr[i][0].x + vr[i][1].x + vr[i][2].x)/3.0);
			yc = ((vr[i][0].y + vr[i][1].y + vr[i][2].y)/3.0);
			lDrawRectangle(frame, xc-s, yc-s, xc+s, yc+s, 0x00);
			lFloodFill(frame, xc, yc, colour[i]);
			lDrawRectangle(frame, xc-s, yc-s, xc+s, yc+s, colour[3+i]);
			lFloodFill(frame, xc, yc, colour[2+i]);
		}
 		lRefresh(frame);
 		Sleep(20);
	}
	
	lSaveImage(frame,L"floodfill.bmp", IMG_BMP,0, 0);
	
	utilCleanup();
	return EXIT_SUCCESS;
}


