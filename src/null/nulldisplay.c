
// null display driver

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

#if (__BUILD_NULLDISPLAY__)

#include "../utils.h"
#include "../frame.h"
#include "../device.h"
#include "../lstring.h"
#include "../pixel.h"
#include "../memory.h"
#include "../copy.h"
#include "nulldisplay.h"




static INLINE void clearDisplay (TDRIVER *drv);

//static INLINE void copyquad (TFRAME *src, TFRAME *des, int x, int y);
static INLINE void uploadquad (TDRIVER *drv, TFRAME *frm, int x, int y);
static INLINE int quadrefresh (TDRIVER *drv, TFRAME *frm);
static INLINE int cmpquad (TFRAME *a, TFRAME *b, int x, int y);



int initNullDisplay (TREGISTEREDDRIVERS *rd)
{
	TDISPLAYDRIVER dd;

	l_strcpy(dd.name,"NULL");
	l_strcpy(dd.comment,"Null display driver.");
	dd.open = nullDisplay_OpenDisplay;
	dd.close = nullDisplay_CloseDisplay;
	dd.clear = nullDisplay_ClearDisplay;
	dd.refresh = nullDisplay_Refresh;
	dd.refreshArea = nullDisplay_RefreshArea;
	dd.setOption = NULL;
	dd.getOption = NULL;
	dd.status = LDRV_CLOSED;
	dd.optTotal = 0;

	return (registerDisplayDriver(rd, &dd) > 0);
}

void closeNullDisplay (TREGISTEREDDRIVERS *rd)
{
	return;
}

int nullDisplay_OpenDisplay (TDRIVER *drv)
{

	if (drv){
		if (drv->dd->status == LDRV_CLOSED){
			drv->dd->back = _newFrame(drv->dd->hw, drv->dd->width, drv->dd->height, 1, LFRM_BPP_1);
			if (drv->dd->back){
				// add initiate display code here
				drv->dd->status = LDRV_READY;
				drv->dd->clear(drv);
				return 1;
			}
		}
	}
	return 0;
}


int nullDisplay_CloseDisplay (TDRIVER *drv)
{

	if (drv){
		if (drv->dd->status != LDRV_CLOSED){
			drv->dd->status = LDRV_CLOSED;
			// add shutdown display code here
			if (drv->dd->back)
				deleteFrame(drv->dd->back);
			return 1;
		}
	}
	return 0;
}

int nullDisplay_ClearDisplay (TDRIVER *drv)
{
	if (drv){
		if (drv->dd->status == LDRV_READY){
			clearDisplay(drv);
			return 1;
		}
	}
	return 0;
}


int nullDisplay_Refresh (TDRIVER *drv, TFRAME *frm)
{
	if (!frm||!drv)
		return 0;
	else{
		if (drv->dd->status == LDRV_READY){
			if (frm->hw->caps[CAP_BACKBUFFER]==CAP_STATE_ON)
				return quadrefresh(drv, frm);
			else
				return nullDisplay_RefreshArea(drv, frm, 0, 0, frm->width-1, frm->height-1);
		}
	}
	return 0;
}

int nullDisplay_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	if (!frm){
		return 0;
	}

	if (drv->dd->status != LDRV_READY)
		return 0;

	// sync buffers
	if (frm->hw->caps[CAP_BACKBUFFER] == CAP_STATE_ON)
		copyArea(frm, drv->dd->back, x1, y1, x1, y1, x2, y2);

	x2 = MIN(MIN(frm->width, drv->dd->width), x2+1);
	y2 = MIN(MIN(frm->height, drv->dd->height), y2+1);
	x1 = MAX(x1,0);
	y1 = MAX(y1,0);

	//if (frm->hw->caps[CAP_NULLDISPLAY_FLIPH]==CAP_STATE_ON)
	//	return nullDisplay_updateAreaFlippedH(drv, frm,x1,y1,x2,y2);
	//else
		return nullDisplay_updateArea(drv,frm,x1,y1,x2,y2);
}


// send data to display
int nullDisplay_updateArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	if (drv && frm &&  x1 && y1 &&  x2 && y2){}
	return 0;
}

// send data to display
int nullDisplay_updateAreaFlippedH (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2)
{
	if (drv && frm && x1 && y1 &&  x2 && y2){}
	return 0;
}

static INLINE void clearDisplay (TDRIVER *drv)
{
	if (drv){}
}


static INLINE void uploadquad (TDRIVER *drv, TFRAME *frm, int x, int y)
{
	nullDisplay_RefreshArea(drv, frm,x,y,x+quadBoxW-1,y+quadBoxH-1);
}

/*
static INLINE void copyquad (TFRAME *src, TFRAME *des, int x, int y)
{
	int i,j;
	int xPLUS8=x+quadBoxW;
	int yPLUS8=y+quadBoxH;

	for (i=x;i<xPLUS8;i++)
		for (j=y;j<yPLUS8;j++)
			setPixel_NB(des,i,j,getPixel_NB(src,i,j));
}
*/
static INLINE int cmpquad (TFRAME *a, TFRAME *b, int x, int y)
{
	int i,j;
	int xPLUS8=x+quadBoxW;
	int yPLUS8=y+quadBoxH;

	for (i=x;i<xPLUS8; i++)
		for (j=y;j<yPLUS8; j++)
			if (getPixel_NB(a,i,j) ^ getPixel_NB(b,i,j))
				return 0;
	return 1;
}

static INLINE int quadrefresh (TDRIVER *drv, TFRAME *frm)
{
	int x2 = MIN(frm->width, drv->dd->width);
	int y2 = MIN(frm->height, drv->dd->height);
	int x1 = 0;
	int y1 = 0;
	int updates=0;
	int x,y;

	for (y=y1;y<y2; y+=quadBoxH){
		for (x=x1;x<x2; x+=quadBoxW){
			if (!cmpquad(frm, drv->dd->back, x, y)){
				//copyquad (frm,frm->hw->back,x,y);
				uploadquad (drv, frm,x,y);
				updates++;
			}
		}
	}
	return updates;
}

#else

int initNullDisplay(void *rd){return 1;}
void closeNullDisplay(void *rd){return;}

#endif

