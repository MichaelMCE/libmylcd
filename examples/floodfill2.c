
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


int main (int argc, char* argv[])
{

	if (!utilInitConfig("config.cfg"))
		return EXIT_FAILURE;
		

	const int RED = lGetRGBMask(frame, LMASK_RED);
	const int CYAN = lGetRGBMask(frame, LMASK_CYAN);
	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));
	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));

	lLoadImage(frame, L"images/outline.png");
	lFloodFill(frame, 280, 90, CYAN);
	lSetPixel(frame, 280, 90, RED);

	lSaveImage(frame,L"floodoutline.bmp", IMG_BMP, 0, 0);
	lRefresh(frame);
	lSleep(1000);
	
	utilCleanup();
	return EXIT_SUCCESS;
}


