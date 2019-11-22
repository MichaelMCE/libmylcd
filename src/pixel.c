
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




#include "mylcd.h"
#include "pixel.h"


int initPixel (THWD *hw)
{

	hw->render->maskLookup[LFRM_BPP_8][LMASK_RED] = RGB_8_RED;
	hw->render->maskLookup[LFRM_BPP_8][LMASK_GREEN] = RGB_8_GREEN;
	hw->render->maskLookup[LFRM_BPP_8][LMASK_BLUE] = RGB_8_BLUE;
	hw->render->maskLookup[LFRM_BPP_8][LMASK_WHITE] = RGB_8_WHITE;
	hw->render->maskLookup[LFRM_BPP_8][LMASK_BLACK] = RGB_8_BLACK;
	hw->render->maskLookup[LFRM_BPP_8][LMASK_MAGENTA] = RGB_8_MAGENTA;
	hw->render->maskLookup[LFRM_BPP_8][LMASK_YELLOW] = RGB_8_YELLOW;
	hw->render->maskLookup[LFRM_BPP_8][LMASK_CYAN] = RGB_8_CYAN;
	
	hw->render->maskLookup[LFRM_BPP_12][LMASK_RED] = RGB_12_RED;
	hw->render->maskLookup[LFRM_BPP_12][LMASK_GREEN] = RGB_12_GREEN;
	hw->render->maskLookup[LFRM_BPP_12][LMASK_BLUE] = RGB_12_BLUE;
	hw->render->maskLookup[LFRM_BPP_12][LMASK_WHITE] = RGB_12_WHITE;
	hw->render->maskLookup[LFRM_BPP_12][LMASK_BLACK] = RGB_12_BLACK;
	hw->render->maskLookup[LFRM_BPP_12][LMASK_MAGENTA] = RGB_12_MAGENTA;
	hw->render->maskLookup[LFRM_BPP_12][LMASK_YELLOW] = RGB_12_YELLOW;
	hw->render->maskLookup[LFRM_BPP_12][LMASK_CYAN] = RGB_12_CYAN;
	
	hw->render->maskLookup[LFRM_BPP_15][LMASK_RED] = RGB_15_RED;
	hw->render->maskLookup[LFRM_BPP_15][LMASK_GREEN] = RGB_15_GREEN;
	hw->render->maskLookup[LFRM_BPP_15][LMASK_BLUE] = RGB_15_BLUE;
	hw->render->maskLookup[LFRM_BPP_15][LMASK_WHITE] = RGB_15_WHITE;
	hw->render->maskLookup[LFRM_BPP_15][LMASK_BLACK] = RGB_15_BLACK;
	hw->render->maskLookup[LFRM_BPP_15][LMASK_MAGENTA] = RGB_15_MAGENTA;
	hw->render->maskLookup[LFRM_BPP_15][LMASK_YELLOW] = RGB_15_YELLOW;
	hw->render->maskLookup[LFRM_BPP_15][LMASK_CYAN] = RGB_15_CYAN;	
	
	hw->render->maskLookup[LFRM_BPP_16][LMASK_RED] = RGB_16_RED;
	hw->render->maskLookup[LFRM_BPP_16][LMASK_GREEN] = RGB_16_GREEN;
	hw->render->maskLookup[LFRM_BPP_16][LMASK_BLUE] = RGB_16_BLUE;
	hw->render->maskLookup[LFRM_BPP_16][LMASK_WHITE] = RGB_16_WHITE;
	hw->render->maskLookup[LFRM_BPP_16][LMASK_BLACK] = RGB_16_BLACK;
	hw->render->maskLookup[LFRM_BPP_16][LMASK_MAGENTA] = RGB_16_MAGENTA;
	hw->render->maskLookup[LFRM_BPP_16][LMASK_YELLOW] = RGB_16_YELLOW;
	hw->render->maskLookup[LFRM_BPP_16][LMASK_CYAN] = RGB_16_CYAN;

	hw->render->maskLookup[LFRM_BPP_24][LMASK_RED] = RGB_24_RED;
	hw->render->maskLookup[LFRM_BPP_24][LMASK_GREEN] = RGB_24_GREEN;
	hw->render->maskLookup[LFRM_BPP_24][LMASK_BLUE] = RGB_24_BLUE;
	hw->render->maskLookup[LFRM_BPP_24][LMASK_WHITE] = RGB_24_WHITE;
	hw->render->maskLookup[LFRM_BPP_24][LMASK_BLACK] = RGB_24_BLACK;
	hw->render->maskLookup[LFRM_BPP_24][LMASK_MAGENTA] = RGB_24_MAGENTA;
	hw->render->maskLookup[LFRM_BPP_24][LMASK_YELLOW] = RGB_24_YELLOW;
	hw->render->maskLookup[LFRM_BPP_24][LMASK_CYAN] = RGB_24_CYAN;
	
	hw->render->maskLookup[LFRM_BPP_32][LMASK_RED] = RGB_32_RED;
	hw->render->maskLookup[LFRM_BPP_32][LMASK_GREEN] = RGB_32_GREEN;
	hw->render->maskLookup[LFRM_BPP_32][LMASK_BLUE] = RGB_32_BLUE;
	hw->render->maskLookup[LFRM_BPP_32][LMASK_WHITE] = RGB_32_WHITE;
	hw->render->maskLookup[LFRM_BPP_32][LMASK_BLACK] = RGB_32_BLACK;
	hw->render->maskLookup[LFRM_BPP_32][LMASK_MAGENTA] = RGB_32_MAGENTA;
	hw->render->maskLookup[LFRM_BPP_32][LMASK_YELLOW] = RGB_32_YELLOW;
	hw->render->maskLookup[LFRM_BPP_32][LMASK_CYAN] = RGB_32_CYAN;

	hw->render->maskLookup[LFRM_BPP_32A][LMASK_RED] = RGB_32A_RED;
	hw->render->maskLookup[LFRM_BPP_32A][LMASK_GREEN] = RGB_32A_GREEN;
	hw->render->maskLookup[LFRM_BPP_32A][LMASK_BLUE] = RGB_32A_BLUE;
	hw->render->maskLookup[LFRM_BPP_32A][LMASK_WHITE] = RGB_32A_WHITE;
	hw->render->maskLookup[LFRM_BPP_32A][LMASK_BLACK] = RGB_32A_BLACK;
	hw->render->maskLookup[LFRM_BPP_32A][LMASK_MAGENTA] = RGB_32A_MAGENTA;
	hw->render->maskLookup[LFRM_BPP_32A][LMASK_YELLOW] = RGB_32A_YELLOW;
	hw->render->maskLookup[LFRM_BPP_32A][LMASK_CYAN] = RGB_32A_CYAN;
	
	//initializeBlendingTable();
	return 1;
}

void closePixel (THWD *hw)
{
}

int getRGBMask (const TFRAME *frame, const int colourMaskIdx)
{
	if (frame->bpp != LFRM_BPP_1 && colourMaskIdx < 8)
		return frame->hw->render->maskLookup[frame->bpp][colourMaskIdx];
	else
		return (colourMaskIdx != LMASK_BLACK);
}

// return row containing first pixel
// return -1 if none
static inline int l_getTopPixel (const TFRAME *const frm)
{
	int x,y;
	for (y=0;y<frm->height;y++){
		for (x=frm->width;x--;){
			if (l_getPixel_NB(frm,x,y))
				return y;
		}
	}
	return -1;
}

// return row containing last pixel, ie, first pixel from the bottom up
// return -1 if none
static inline int l_getBottomPixel (const TFRAME *const frm)
{
	int x,y;
	for (y=frm->height;y--;){
		for (x=frm->width;x--;){
			if (l_getPixel_NB(frm,x,y))
				return y;
		}
	}
			
	return -1;
}

// return column of containing first pixel from left in
// return -1 if none
static inline int l_getLeftPixel (const TFRAME *const frm)
{
	int x,y;
	for (x=0;x<frm->width;x++){
		for (y=frm->height;y--;){
			if (l_getPixel_NB(frm,x,y))
				return x;
		}
	}
	return -1;
}

// return column of containing first pixel from right
// return -1 if none
static inline int l_getRightPixel (const TFRAME *const frm)
{
	int x,y;
	for (x=frm->width;x--;){
		for (y=frm->height;y--;){
			if (getPixel_NB(frm,x,y))
				return x;
		}
	}
	return -1;
}

int getImageBoundingBox (TFRAME *frame, TLPOINTEX *p)
{
	p->x1 = l_getLeftPixel(frame);
	p->y1 = l_getTopPixel(frame);
	p->x2 = l_getRightPixel(frame);
	p->y2 = l_getBottomPixel(frame);
	return 1;
}

