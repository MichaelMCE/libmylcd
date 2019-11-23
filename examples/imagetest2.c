
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



static inline int loadImage (TFRAME *frame, wchar_t *path)
{
	TFRAME *img = lNewImage(frame->hw, path, LFRM_BPP_32A);
	if (img){
		lDrawImage(img, frame, 0, 0);
		lDeleteFrame(img);
	}
	return img != NULL;
}

static inline void displayImage (TFRAME *frame, wchar_t *path)
{
	
	wprintf(L"'%s'\n", path);
	
	do{
		if (!loadImage(frame, path)){
			wprintf(L"'%s' failed\n", path);
			break;
		}

		lRefresh(frame);
		lSleep(750);
	}while(0);
}

int main (int argc_a, char *argv_a[])
{
	if (!utilInitConfig("config.cfg"))
		return EXIT_FAILURE;

	lClearFrame(frame);


	displayImage(frame, L"images/RGB_24bits_palette_bird.bmp");
	displayImage(frame, L"images/RGB_8bits_palette.bmp");
	displayImage(frame, L"images/RGB_24bits_palette.bmp");
	displayImage(frame, L"images/RGB_24bits_palette.tga");
	displayImage(frame, L"images/RGB_12bits_palette.bmp");
	displayImage(frame, L"images/RGB_12bits_palette.tga");

	displayImage(frame, L"images/RGBR_8bit.bmp");
	displayImage(frame, L"images/24bpp_somuchwin.bmp");
	displayImage(frame, L"images/8bpp_somuchwin.bmp");

	displayImage(frame, L"images/IndexedColorSample_(Caerulea).bmp");
	displayImage(frame, L"images/IndexedColorSample_(Lapis.elephant).bmp");
	displayImage(frame, L"images/IndexedColorSample_(Lemon).bmp");
	displayImage(frame, L"images/IndexedColorSample_(Strawberries).bmp");
	displayImage(frame, L"images/IndexedColorSample_(Mosaic).bmp");

	displayImage(frame, L"images/32bpp_alpha.tga");
	displayImage(frame, L"images/RGB_Space.bmp");
	displayImage(frame, L"images/flowers.bmp");


	lRefresh(frame);

	utilCleanup();
	return EXIT_SUCCESS;
	
}

