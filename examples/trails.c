
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
#include <time.h>
#include "conio.h"


#define TRAILLENGTH		(10)
#define tPLOYGONS		(10)
#define TRAILSHADOW		(60)
#define TOTALPOINTS		(TRAILLENGTH*3)


typedef struct {
	int endpoint;
	int startpoint;
	int dx, dy;
	T2POINT point[TOTALPOINTS];
}TTRAIL;



static inline void drawBorder (TFRAME *frame, const int col)
{
	lDrawRectangle(frame, 0, 0, frame->width-1, frame->height-1, col);
	lDrawRectangle(frame, 1, 1, frame->width-2, frame->height-2, col);
}

static inline void advanceTrail (TFRAME *frm, int *x, int *y, int *dx, int *dy)
{

	*x += *dx;
	*y += *dy;
   
	if (*x < 0){
		*x = 0;
		*dx = 1+(rand()&1);
		
	}else if (*x > frm->width-1){
		*x = frm->width-1;
		*dx = -(1+(rand()&1));
	}
              
	if (*y < 0){
		*y = 0;
		*dy = (1+(rand()&1));
		
	}else if (*y  > frm->height-1){
		*y = frm->height-1;
		*dy = -(1+(rand()&1));
	}

}


static inline void updateTrail (TFRAME *frame, TTRAIL *trail, int ctrl)
{
	T2POINT pt[ctrl];

	for (int i = 0; i < ctrl; i++){
    	if (abs(trail[i].endpoint-trail[i].startpoint) > TRAILLENGTH){
			trail[i].startpoint++;
			if (trail[i].startpoint == TOTALPOINTS)
				trail[i].startpoint = 0;
		}

		pt[i].x = trail[i].point[trail[i].endpoint].x;
		pt[i].y = trail[i].point[trail[i].endpoint].y;

		advanceTrail(frame, &pt[i].x, &pt[i].y, &trail[i].dx, &trail[i].dy);

		trail[i].endpoint++;
		if (trail[i].endpoint == TOTALPOINTS)
			trail[i].endpoint = 0;

		trail[i].point[trail[i].endpoint].x = pt[i].x;
		trail[i].point[trail[i].endpoint].y = pt[i].y;
    }
}

static inline void drawTrail (TFRAME *frame, const TTRAIL *trail, const int ctrl, const uint32_t colour)
{
	if (trail[0].endpoint < trail[0].startpoint){
		for (int i = trail[0].startpoint; i<TOTALPOINTS; i++){
			for (int tmpctrl=0; tmpctrl < ctrl-1; tmpctrl++)
				lDrawLine(frame, trail[tmpctrl].point[i].x, trail[tmpctrl].point[i].y, trail[tmpctrl+1].point[i].x, trail[tmpctrl+1].point[i].y, colour);
			lDrawLine(frame, trail[ctrl-1].point[i].x, trail[ctrl-1].point[i].y, trail[0].point[i].x, trail[0].point[i].y, colour);
        }

		for (int i = 0; i < trail[0].endpoint; i++){
			for(int tmpctrl = 0; tmpctrl < ctrl-1; tmpctrl++)
				lDrawLine(frame, trail[tmpctrl].point[i].x, trail[tmpctrl].point[i].y, trail[tmpctrl+1].point[i].x, trail[tmpctrl+1].point[i].y, colour);
			lDrawLine(frame, trail[ctrl-1].point[i].x, trail[ctrl-1].point[i].y, trail[0].point[i].x, trail[0].point[i].y, colour);
        }
		return;
	}

	for (int i = trail[0].startpoint; i < trail[0].endpoint+1; i++){
		for (int tmpctrl = 0; tmpctrl < ctrl-1; tmpctrl++)
			lDrawLine(frame, trail[tmpctrl].point[i].x, trail[tmpctrl].point[i].y, trail[tmpctrl+1].point[i].x, trail[tmpctrl+1].point[i].y, colour);
		lDrawLine(frame, trail[ctrl-1].point[i].x, trail[ctrl-1].point[i].y, trail[0].point[i].x, trail[0].point[i].y, colour);
    }
	return;
}

static inline void setupTrail (TFRAME *frame, TTRAIL *trail, const int total)
{
	for (int i = 0; i < total; i++){
		trail[i].startpoint = 0;
		trail[i].endpoint = 0;
		trail[i].dx = 1;
		trail[i].dy = -1;
		trail[i].point[0].x = rand()%(frame->width/4);
		trail[i].point[0].y = rand()%(frame->height/4);
     }
}


int main ()
{
	if (!utilInitConfig("config.cfg"))
		return EXIT_FAILURE;

	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_RED));
	lClearFrame(frame);
	TFRAME *workingBuffer = lCloneFrame(frame);

    time_t t;
    srand((unsigned int)time(&t)%127);

	TTRAIL r_trail[TRAILSHADOW+4][tPLOYGONS+2];
	TTRAIL *trail = r_trail[0];
	setupTrail(frame, trail, tPLOYGONS);

	const uint32_t colAlpha = 20<<24;
	const uint32_t colours[] = {
		colAlpha|0xA3FA33,		// yellow/green
		colAlpha|0x508DC5		// blue sea tint
	};
	const uint32_t colour = colours[(GetTickCount()>>10)&0x01];

	do{ 
		//lClearFrame(frame);
		drawBorder(frame, 7<<24|0x00B7EB);

		for (int i = TRAILSHADOW-2; i >= 0; i--)
			drawTrail(frame, r_trail[i], tPLOYGONS, colour);

		drawTrail(frame, &trail[0], tPLOYGONS, (240<<24)|0xFF10CF);
		lBlur(frame, workingBuffer, 0, 5);
		lRefreshAsync(frame, 1);
		//lRefresh(frame);
		
		for (int i = TRAILSHADOW-2; i >= 0; i--)
			memcpy(r_trail[i+1], r_trail[i], sizeof(TTRAIL)*(tPLOYGONS));
		updateTrail(frame, trail, tPLOYGONS);

		lSleep(37);
	}while(!kbhit());


	utilCleanup();
	return EXIT_SUCCESS;
}
