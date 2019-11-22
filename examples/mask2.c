
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



#define BALLWIDTH	30

static int ballX, ballY;
static int deltaX, deltaY;
static int deltaValue = 1;
static int RED;
static int GREEN;
static int BLUE;
static int WHITE;
static int BLACK;

TFRAME *mask = NULL;
TFRAME *src = NULL;





static inline void drawMask (TFRAME *frame)
{
	lDrawMask(src, mask, frame, ballX-BALLWIDTH, ballY-BALLWIDTH, LMASK_XOR);
}

static inline void updateMaskPosition (TFRAME *frame)
{

   ballX += deltaX;
   ballY += deltaY;

   if (ballX < BALLWIDTH){
      ballX = BALLWIDTH;
      deltaX = deltaValue;
      
   }else if (ballX+BALLWIDTH > frame->width-1){
      ballX = frame->width - BALLWIDTH;
      deltaX = -deltaValue;
   }
   
   if (ballY < BALLWIDTH){
      ballY = BALLWIDTH;
      deltaY = deltaValue;
      
   }else if (ballY+BALLWIDTH  > frame->height-1){
      ballY = frame->height - BALLWIDTH;
      deltaY = -deltaValue;
   }
}

int main ()
{
	if (!utilInitConfig("config.cfg"))
		return EXIT_FAILURE;

	int ALPHA;
	if (DBPP == LFRM_BPP_32A)
		ALPHA = 0xFF000000;
	else
		ALPHA = 0x00000000;
	
	RED =   ALPHA | lGetRGBMask(frame, LMASK_RED);
	GREEN = ALPHA | lGetRGBMask(frame, LMASK_GREEN);
	BLUE =  ALPHA | lGetRGBMask(frame, LMASK_BLUE);
	BLACK = ALPHA | lGetRGBMask(frame, LMASK_BLACK);
	WHITE = ALPHA | lGetRGBMask(frame, LMASK_WHITE);

	lSetBackgroundColour(hw, ALPHA|BLACK);
	lSetForegroundColour(hw, ALPHA|WHITE);
	lClearFrame(frame);
	
	const int width = frame->width;
	const int height = frame->height;
	
	src = lNewFrame(hw, width, height, DBPP);
	mask = lNewFrame(hw, BALLWIDTH*2.1f, BALLWIDTH*2.1f, DBPP);
	lClearFrame(src);
	lClearFrame(mask);
	
	lSetPixelWriteMode(src, LSP_OR);
	lDrawCircleFilled(src, width/2.0f, height/3.0f, width/3.0f, ALPHA|RED);
	lDrawCircleFilled(src, width/3.0f, height/1.5f, width/3.0f, ALPHA|GREEN);
	lDrawCircleFilled(src, width/1.5f, height/1.5f, width/3.0f, ALPHA|BLUE);

	lDrawCircleFilled(mask, BALLWIDTH, BALLWIDTH, BALLWIDTH, ALPHA|WHITE);
	lDrawCircleFilled(mask, BALLWIDTH, BALLWIDTH, BALLWIDTH/2, ALPHA|BLACK);
	
	srand(GetTickCount());
	ballX = (rand()%DWIDTH);
	ballY = (rand()%DHEIGHT);

	if (ballX < BALLWIDTH)
		ballX = BALLWIDTH;
	else if (ballX > frame->width-BALLWIDTH-1)
		ballX = frame->width-BALLWIDTH-1;
	
	if (ballY < BALLWIDTH)
		ballY = BALLWIDTH;
	else if (ballY > frame->height-BALLWIDTH-1)
		ballY = frame->height-BALLWIDTH-1;
	
	deltaX = deltaValue;
	deltaY = deltaValue;
	lSetPixelWriteMode(frame, LSP_SET);
	
	int x = 1000;
	
	do{
		drawMask(frame);
		lRefresh(frame);
		updateMaskPosition(frame);
		lSleep(1);
	}while(x--);

	lDeleteFrame(src);
	lDeleteFrame(mask);
	lSleep(1000);
	
	utilCleanup();
	return EXIT_SUCCESS;
}
