
// USBD480, for the Sharp LQ043 LCD (as found in the PSP-1000) @ www.lcdinfo.com

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
//  GNU LIBRARY GENERAL PUBLIC LICENSE for more details.



#include "mylcd.h"

#if (__BUILD_USBD480DLL__)

#include "../frame.h"
#include "../device.h"
#include "../memory.h"
#include "../lstring.h"
#include "../sync.h"
#include "../lmath.h"
#include "../misc.h"
#include "USBD480_lib.h"
#include "usbd480dll.h"
#include "touch.h"

#define DEBOUNCE(x) ((x)->pos.dt >= DEBOUNCETIME)
#define PIXELDELTAX(di, x) ((int)((x)*(4096.0/(float)(di)->Width)))
#define PIXELDELTAY(di, y) ((int)((y)*(4096.0/(float)(di)->Height)))


static void cleanQueue (TTOUCHDLL *touch)
{
	int i;	
	for (i = 0; i < 16; i++){
		touch->tin[i].mark = 2;
		touch->tin[i].loc.x = 0;
		touch->tin[i].loc.y = 0;
		touch->tin[i].loc.pen = -1;
		touch->tin[i].loc.pressure = -1;
	}
	touch->dragState = 0;
	touch->dragStartCt = 0;
    touch->last.x = 0;
    touch->last.y = 0;
    touch->last.pen = -1;
	touch->last.time = getTicks();
	touch->pos.time = touch->last.time;
}

void initTouchDll (TUSBD480DIDLL *di, TTOUCHDLL *touch)
{
	cleanQueue(touch);
	touch->count = 0;
	touch->dragState = 0;
	touch->last.pen = -1;
	
	touch->cal.left = 210;		// your panel may differ 
	touch->cal.right = 210;
	touch->cal.top = 255;
	touch->cal.bottom = 300;
	touch->cal.hrange = (4095-touch->cal.left)-touch->cal.right;
	touch->cal.vrange = (4095-touch->cal.top)-touch->cal.bottom;
	touch->cal.hfactor = di->Width/touch->cal.hrange;
	touch->cal.vfactor = di->Height/touch->cal.vrange;
}

// convert from touch panel location to display coordinates
static int filter0 (TUSBD480DIDLL *di, TTOUCHDLL *touch, TTOUCHCOORD *pos)
{
	if ((pos->y > touch->cal.top) && (pos->y < 4095-touch->cal.bottom) && (pos->x > touch->cal.left) && (pos->x < 4095-touch->cal.right)){
		pos->x = di->Width-(touch->cal.hfactor*(pos->x - touch->cal.right));
		pos->y = touch->cal.vfactor*(pos->y - touch->cal.top);
		touch->pos.x = pos->x;
		touch->pos.y = pos->y;
		return 1;
	}else{
		return 0;
	}
}

// detect and remove erroneous coordinates plus apply a little smoothing
static int filter1 (TUSBD480DIDLL *di, TTOUCHDLL *touch)
{
	TTOUCHCOORD *pos = &touch->pos;
	
#if 0
	if ((touch->last->x == pos->x) || (touch->last->y == pos->y))
		return 0;
#endif
		
	l_memcpy(&touch->last, &touch->pos, sizeof(TTOUCHCOORD));

	// check current with next (pos)
	int dx = abs(touch->tin[9].loc.x - pos->x);
	int dy = abs(touch->tin[9].loc.y - pos->y);
	if (dx > PIXELDELTAX(di, 3) || dy > PIXELDELTAY(di, 3))
		touch->tin[10].mark = 1;
	else
		touch->tin[10].mark = 0;
	l_memcpy(&touch->tin[10].loc, pos, sizeof(TTOUCHCOORD));

	// check current with previous
	if (touch->tin[9].mark != 2 && touch->tin[8].mark != 2){
		dx = abs(touch->tin[9].loc.x - touch->tin[8].loc.x);
		dy = abs(touch->tin[9].loc.y - touch->tin[8].loc.y);
		if (dx > PIXELDELTAX(di, 3) || dy > PIXELDELTAY(di, 3))
			touch->tin[9].mark = 1;
		else
			touch->tin[9].mark = 0;
	}

	int i;
	for (i = 0; i < 10; i++)
		l_memcpy(&touch->tin[i], &touch->tin[i+1], sizeof(TINDLL));

	if (touch->tin[10].mark != 0)
		return 0;

	int total = 0;
	pos->x = 0;
	pos->y = 0;
	
	for (i = 6; i < 10; i++){
		if (!touch->tin[i].mark && touch->tin[i].loc.y){
			pos->x += touch->tin[i].loc.x;
			pos->y += touch->tin[i].loc.y;
			total++;
		}
	}
	if (total){
		const int t = getTicks();
		pos->dt = t - pos->time;
		pos->time = t;
		pos->x /= (float)total;
		pos->y /= (float)total;
		pos->count = touch->count++;
		return total;
	}else{
		return 0;
	}
}

static int detectdrag (TUSBD480DIDLL *di, TTOUCHDLL *touch)
{
	if (touch->pos.dt < DRAGDEBOUNCETIME){
		if (touch->dragState)
			return 1;
		
		const int dtx = abs(touch->pos.x - touch->last.x);
		const int dty = abs(touch->pos.y - touch->last.y);
		if ((dtx > PIXELDELTAX(di, 0) || dty > PIXELDELTAY(di, 0)) && dtx && dty){
			if (++touch->dragStartCt == 2){
				touch->dragState = 1;
				return 1;
			}
		}
	}
	return 0;
}

int usbd480DLL_GetTouchPosition (TUSBD480DIDLL *di, TTOUCHCOORD *pos)
{	
	if (di){
		int result = USBD480_GetTouchReport(&di->di, (/*TouchReport*/void*)&di->upos);
		if (result != USBD480_OK){
			mylog("usbd480DLL_GetTouchPosition(): USBD480_GetTouchReport() returned %d\n", result);
			return -1;
		}else{
			pos->x = di->upos.x;
			pos->y = di->upos.y;
			pos->z1 = di->upos.z1;
			pos->z2 = di->upos.z2;
			pos->pen = di->upos.pen;
			pos->pressure = di->upos.pressure;
			l_memcpy(&di->touch.pos, pos, sizeof(TTOUCHCOORD));

			if (!pos->pressure && di->touch.last.pen == di->touch.pos.pen){
				cleanQueue(&di->touch);
				return -2;
			}else if (pos->pen == 1 && di->touch.last.pen == 0){	// pen up
				cleanQueue(&di->touch);
				return 3;
			}else if (pos->pen == 0 && di->touch.last.pen == 1){	// pen down
				cleanQueue(&di->touch);
			}
		
			if (filter1(di, &di->touch)){
				if (detectdrag(di, &di->touch)){
					if (filter0(di, &di->touch, &di->touch.pos)){
						pos->x = di->touch.pos.x;
						pos->y = di->touch.pos.y;
						pos->dt = di->touch.pos.dt;
						pos->time = di->touch.pos.time;
						pos->z1 = di->touch.pos.z1;
						pos->z2 = di->touch.pos.z2;
						pos->pen = di->touch.pos.pen;
						pos->pressure = di->touch.pos.pressure;
						pos->count = di->touch.pos.count;
						return 2;
					}
				}else if (DEBOUNCE(&di->touch)){
					if (filter0(di, &di->touch, &di->touch.pos)){
						pos->x = di->touch.pos.x;
						pos->y = di->touch.pos.y;
						pos->dt = di->touch.pos.dt;
						pos->time = di->touch.pos.time;
						pos->z1 = di->touch.pos.z1;
						pos->z2 = di->touch.pos.z2;
						pos->pen = di->touch.pos.pen;
						pos->pressure = di->touch.pos.pressure;
						pos->count = di->touch.pos.count;
						return 1;
					}
				}
			}
			return -2;
		}
	}
	return 0;
}


#endif

