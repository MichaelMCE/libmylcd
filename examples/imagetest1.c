
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
#include "images/child.h"
#include "images/girl.h"
#include "images/startframe.h"




static inline wchar_t wreverse16 (wchar_t a)
{
	const wchar_t b = (a&0xFF00)>>8;
	return (b|(a&0x00FF)<<8);
}

static inline void loadBuffer1 (TFRAME *frame, ubyte *buffer, int size)
{
	char *des = (char*)lGetPixelAddress(frame, 0, 0);
	char *src = (char*)buffer;
	
	int tpixels = frame->height*(frame->width/8);
	while(tpixels--)
		des[tpixels] = src[tpixels];
}

static inline void loadBufferRev16 (TFRAME *frame, ubyte *buffer, int size)
{
	wchar_t *des = (wchar_t*)lGetPixelAddress(frame, 0, 0);
	wchar_t *src = (wchar_t*)buffer;
	
	int tpixels = frame->height * frame->width;
	while(tpixels--)
		des[tpixels] = wreverse16(src[tpixels]);
}

static inline void displayBuffer1 (void *buffer, const size_t size, const int width, const int height)
{
	TFRAME *src = lNewFrame(hw, width, height, LFRM_BPP_1);
	if (!src) return;
	TFRAME *des = lNewFrame(hw, width, height, DBPP);
	if (!des){
		lDeleteFrame(src);
		return;
	}
	
	loadBuffer1(src, buffer, size);
	
	lFrame1BPPToRGB(src, lGetPixelAddress(des, 0, 0), DBPP, 
	  lGetRGBMask(des, LMASK_YELLOW), lGetRGBMask(des, LMASK_MAGENTA));
	
	lRefresh(des);
	lDeleteFrame(des);
	lDeleteFrame(src);
	
	lSleep(1000);
}

static inline void displayBuffer16 (void *buffer, const size_t size, const int width, const int height, const int bpp)
{
	TFRAME *tmp = lNewFrame(hw, width, height, bpp);
	if (!tmp) return;
	loadBufferRev16(tmp, buffer, size);
	lRefresh(tmp);
	
	lDeleteFrame(tmp);
	lSleep(1000);
}

int main (int argc_a, char *argv_a[])
{
	if (!utilInitConfig("config.cfg"))
		return EXIT_FAILURE;

	lClearFrame(frame);
	lRefresh(frame);

	displayBuffer16(child130x130_12, sizeof(child130x130_12), 130, 130, LFRM_BPP_12);
	displayBuffer16(girl130x130_12, sizeof(girl130x130_12), 130, 130, LFRM_BPP_12);
	displayBuffer1(startframe160x43, sizeof(startframe160x43), 160, 43);
	
	lSleep(1000);
	
	utilCleanup();
	return EXIT_SUCCESS;
	
}

