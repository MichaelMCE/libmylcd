
// libmylcd
// An LCD framebuffer library
// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2012  Michael McElligott
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


#ifndef _SBUICB20_H_
#define _SBUICB20_H_

#ifndef SBUISDK_VERSION
#define SBUISDK_VERSION	"2.0.1"
#endif

#ifndef TSBUIRENDERSTATSRUCT
#define TSBUIRENDERSTATSRUCT 1

typedef struct{
	int count;
	int maxTime;
	int lastTime;
	int averageTime;
}TSBUIRENDERSTATS;


typedef struct{
	int type;
	int params;
	int x;
	int y;
	int z;
	
	unsigned int ct;
	int64_t time;
	int64_t timePrev;
	unsigned int dt;
	unsigned int id;
}TSBGESTURE;


typedef struct{
	size_t size;	// size of this struct
	int dk;
	int state;
	wchar_t *path;
}TSBGESTURESETDK;
 

typedef struct{
	int op;
	int gesture;
	int state;
}TSBGESTURECBCFG;

enum _SBUI_DK
{
    SBUI_DK_INVALID = 0,
    SBUI_DK_1,
    SBUI_DK_2,
    SBUI_DK_3,
    SBUI_DK_4,
    SBUI_DK_5,
    SBUI_DK_6,
    SBUI_DK_7,
    SBUI_DK_8,
    SBUI_DK_9,
    SBUI_DK_10,
    SBUI_DK_UNDEFINED
};


#define	SBUI_DK_RZBUTTON		(0x400)
#define	SBUI_DK_CLOSE			(SBUI_DK_RZBUTTON+0)
#define	SBUI_DK_EXIT			(SBUI_DK_RZBUTTON+1)
#define	SBUI_DK_ACTIVATE		(SBUI_DK_RZBUTTON+2)
#define	SBUI_DK_DEACTIVATE		(SBUI_DK_RZBUTTON+3)


enum _SBUI_DK_STATE
{
    SBUI_DK_DISABLED = 0,
    SBUI_DK_UP = 1,
    SBUI_DK_DOWN = 2
};

//gestures
// keep in sync with SwitchBladeSDK_types.h::RZSDKGESTURETYPE
#define SBUICB_GESTURE_INVALID    0x00000000
#define SBUICB_GESTURE_NONE       0x00000000
#define SBUICB_GESTURE_PRESS      0x00000001
#define SBUICB_GESTURE_TAP        0x00000002
#define SBUICB_GESTURE_FLICK      0x00000004
#define SBUICB_GESTURE_ZOOM       0x00000008
#define SBUICB_GESTURE_ROTATE     0x00000010
#define SBUICB_GESTURE_ALL        0xFFFF
#define SBUICB_GESTURE_UNDEFINED  SBUICB_GESTURE_INVALID

#endif

//op functions
#define SBUICB_OP_GestureEnable					1
#define SBUICB_OP_GestureSetNotification		2
#define SBUICB_OP_GestureSetOSNotification		3




//states
#define SBUICB_STATE_DISABLED	0
#define SBUICB_STATE_ENABLED 	1


typedef int (*pgesturecb) (const TSBGESTURE *sbg, void *ptr);
typedef int (*pdkcb) (const int dk, const int state, void *ptr);

int sbuiDKCB (const int dk, const int state, void *ptr);


#endif

