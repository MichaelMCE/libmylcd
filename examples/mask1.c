
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


static int RED;
static int GREEN;
static int BLUE;
static int WHITE;
static int BLACK;

int main(int argc, char* argv[])
{
	if (!utilInitConfig("config.cfg"))
		return EXIT_FAILURE;

	RED = lGetRGBMask(frame, LMASK_RED);
	GREEN = lGetRGBMask(frame, LMASK_GREEN);
	BLUE = lGetRGBMask(frame, LMASK_BLUE);
	BLACK = lGetRGBMask(frame, LMASK_BLACK);
	WHITE = lGetRGBMask(frame, LMASK_WHITE);

	lSetBackgroundColour(hw, BLACK);
	lSetForegroundColour(hw, WHITE);

	const int width = frame->width;
	const int height = frame->height;
	const int third = width/3.0f;
	
	TFRAME *src = lNewImage(hw, L"images/outline.png", DBPP);
	if (!src) goto cleanup;
	TFRAME *mask = lNewFrame(hw, width, height, DBPP);

	lDrawCircleFilled(mask, 63, 31, 30, WHITE);
	lDrawCircleFilled(mask, 63, 31, 15, BLACK);
	lDrawRectangleFilled(mask, 0, height/2, width-1, height-1, WHITE);
	lDrawRectangleFilled(mask, 0.0*third, height/2, (0*third)+third, height-30, BLUE);
	lDrawRectangleFilled(mask, 1.0*third, height/2, (1*third)+third, height-30, GREEN);
	lDrawRectangleFilled(mask, 2.0*third, height/2, (2*third)+third, height-30, RED);
	lDrawRectangleFilled(mask, 31, height-48, width-30, height-15, BLACK);
	lDrawRectangleDottedFilled(mask, 30, height-48, width-30, height-15, WHITE);

	lClearFrame(frame);
	lDrawMask(src, mask, frame, 0, 0, LMASK_AND);
	lRefresh(frame);
	lDeleteFrame(src);
	lDeleteFrame(mask);

	lSleep(1000);

cleanup:	
	utilCleanup();
	return EXIT_SUCCESS;
}


