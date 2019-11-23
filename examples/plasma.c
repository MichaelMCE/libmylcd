
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
#include <math.h>
#include <conio.h>



#define font LFTW_RACER102



static inline void precalculate (TFRAME *buf1, TFRAME *buf2)
{
    const int w = buf1->width>>1;
    const int h = buf1->height>>1;
    
    for (int y = 0; y < buf1->height; y++){
		for (int x = 0; x < buf1->width; x++){
			int c = (uint8_t)(64.0 + 63.0 * (sin((double)hypot(h-y, w-x)/16.0)));
			lSetPixel_NB(buf1, x, y, c);
			
			c = (uint8_t)(64.0 + 63.0 * sin((double)x/(37.0+15.0*cos((double)y/74.0)))
                                    * cos((double)y/(31.0+11.0*sin((double)x/57.0))));
			lSetPixel_NB(buf2, x, y, c);
        }
    }
}

int main (int argc, char* argv[])
{

	if (!utilInitConfig("config.cfg"))
		return EXIT_FAILURE;
	
	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));
	
	if (DBPP == LFRM_BPP_32A){
		DBPP = LFRM_BPP_32;
		lDeleteFrame(frame);
		frame = lNewFrame(hw, DWIDTH, DHEIGHT, DBPP);
	}
	
	
	TFRAME *plasma1 = lNewFrame(hw, DWIDTH*2, DHEIGHT*2, DBPP);
	TFRAME *plasma2 = lNewFrame(hw, DWIDTH*2, DHEIGHT*2, DBPP);
	precalculate(plasma1, plasma2);
	
	TFRAME *pal = lNewFrame(hw, 8, 256, DBPP);
	//lLoadImage(pal, L"images/pal.png");
	
	
	TFRAME *image = lNewString(hw, DBPP, 0, font, "12345\n6789\n0");
	if (!image) goto exit;
	if (image->height > frame->height || image->width > frame->width)
		lResizeFrame(image, min(image->width, frame->width), min(image->height, frame->height), 1);

	const double width = (image->width/2.0)-1.0;
	const double height = (image->height/2.0)-1.0;
	    
	double pifact[256];
	for (int i = 0; i < 256; i++)
		pifact[i] = (double)(i+1.0) * M_PI / 128.0;

	while (!kbhit()){
		double currentTime = GetTickCount()/17.0;
	
		for (int i = 0; i < 256; i++){
			int r = ((uint8_t)(32.0 + 31.0 * cos(pifact[i] + currentTime/74.0)))<<2;
			int g = ((uint8_t)(32.0 + 31.0 * sin(pifact[i] + currentTime/63.0)))<<2;
			int b = ((uint8_t)(32.0 - 31.0 * cos(pifact[i] + currentTime/81.0)))<<2;
			lSetPixel_NB(pal, 1, i, r<<16|g<<8|b);
		}
		
		int x1 = width+1 + (int)(width * cos(currentTime/97.0));
		int x2 = width+1 + (int)(width * sin(-currentTime/114.0));
		int x3 = width+1 + (int)(width * sin(-currentTime/137.0));
		int y1 = height+1 + (int)(height * sin(currentTime/123.0));
		int y2 = height+1 + (int)(height * cos(-currentTime/75.0));
		int y3 = height+1 + (int)(height * cos(-currentTime/108.0));
	
		for (int y = 0; y < image->height; y++){
			for (int x = 0; x < image->width; x++){
				int colour = lGetPixel(plasma1, x1+x, y1);
				colour += lGetPixel(plasma2, x2+x, y2);
				colour += lGetPixel(plasma2, x3+x, y3);
				int col = lGetPixel_NB(image, x/*-width2*/, y/*-height2*/)&(x+y);
	         	colour += col;
	
				//colour = lGetPixel(pal, 1, (colour&0xFF)>>1);
				if (col&0xFFFFFF){
					colour = lGetPixel_NB(pal, 1, colour&0xFF);
					lSetPixel_NB(frame, x, y, colour);
				}
			}
			y1++; y2++; y3++;
		}

		lRefreshAsync(frame, 0);
		// or
		//lRefresh(frame);
		lSleep(15);
	}

	lDeleteFrame(image);
	
exit:
    lDeleteFrame(pal);
	lDeleteFrame(plasma1);
	lDeleteFrame(plasma2);
	
	utilCleanup();
	return EXIT_SUCCESS;
}

