
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


#ifndef _G15D_H_
#define _G15D_H_

#if defined(__WIN32__)
#include "lglcd.dll/lglcd.h"


#define G15_PRIORITY_ASYNC		(LGLCD_ASYNC_UPDATE(LGLCD_PRIORITY_ALERT))
#define G15_PRIORITY_SYNC		(LGLCD_SYNC_UPDATE(LGLCD_PRIORITY_NORMAL))
#define G15_WHEEL_ANTICLOCKWISE VK_VOLUME_DOWN
#define G15_WHEEL_CLOCKWISE		VK_VOLUME_UP
#define G15_KEY_MUTE			VK_VOLUME_MUTE
#define G15_KEY_FORWARD			VK_MEDIA_NEXT_TRACK
#define G15_KEY_BACK			VK_MEDIA_PREV_TRACK
#define G15_KEY_STOP			VK_MEDIA_STOP
#define G15_KEY_PLAY			VK_MEDIA_PLAY_PAUSE

#if 1
#define G15_SOFTKEY1			LGLCDBUTTON_BUTTON0
#define G15_SOFTKEY2			LGLCDBUTTON_BUTTON1
#define G15_SOFTKEY3			LGLCDBUTTON_BUTTON2
#define G15_SOFTKEY4			LGLCDBUTTON_BUTTON3

#else
#define G15_SOFTKEY1			VK_OEM_1
#define G15_SOFTKEY2			VK_OEM_2
#define G15_SOFTKEY3			VK_OEM_3
#define G15_SOFTKEY4			VK_OEM_4
#endif

struct TMYLCDG15;
typedef DWORD (*psoftkeycb) (int device, DWORD dwButtons, struct TMYLCDG15 *mylcdg15);

#ifndef _G15_FRAMESIZE
#define _G15_FRAMESIZE
#define G15_FRAMESIZE (LGLCD_BMP_HEIGHT*(LGLCD_BMP_WIDTH/8))
#endif

#ifndef G15_TBUFFERS
#define G15_TBUFFERS 1
#endif


#ifndef _TG15BUFFER
#define _TG15BUFFER
typedef struct{
	ubyte data[G15_FRAMESIZE];
}TG15BUFFER;
#endif


#ifndef _TG15BUFFERS
#define _TG15BUFFERS
typedef struct{
	TG15BUFFER buffer[G15_TBUFFERS];
	int total;
}TG15BUFFERS;
#endif

typedef struct{
	lgLcdConnectContext	connectContext;
	lgLcdDeviceDesc		deviceDescription;
	lgLcdOpenContext	openContext;
	lgLcdBitmap160x43x1 bmp;
}TLOGITECHG;


typedef struct{
	psoftkeycb	softkeycb;
	void		*ptrUser;	// user space
	int			intUser;	// user space
	int			priority;
	TLOGITECHG	*lg;
	TG15BUFFERS frames;
	TFRAME *back;
	volatile int threadState;
	TTHRDSYNCCTRL tsc;
	unsigned int updateCt;
	unsigned int t0;
	unsigned int t1; 
}TMYLCDG15;

 
#endif
#endif // #ifndef _G15DISPLAY_H_


