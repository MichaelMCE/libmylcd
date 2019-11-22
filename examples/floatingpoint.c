
// libmylcd
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
//  GNU LIBRARY GENERAL PUBLIC LICENSE for details.


#include "utils/utils.h"
                   

int main ()
{
	if (!utilInitConfig("config.cfg"))
		return EXIT_FAILURE;

	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));


	for (float i = 0.001f; i < 4.0f; i += 0.002f){
		for (int y = 0; y < frame->height; ++y){
			for (int x = 0; x < frame->width; ++x){
				float r = 0.15f + (x/i + y*i) * 0.0015f;
				float g = 0.3f + (x*i + y) * 0.0010f;
				float b = 0.6f + (x + y) * 0.0005f;
				lSetPixelf(frame, x, y, r, g, b);
			}
		}
		lRefresh(frame);
	}
	lSaveImage(frame, L"fp.png", IMG_PNG, frame->width, frame->height);
	lSleep(2000);
	
	utilCleanup();
	return EXIT_SUCCESS;
}



