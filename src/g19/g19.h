
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


#ifndef _G19_H_
#define _G19_H_



#if defined(__WIN32__)
#include "../g15/lglcd.dll/lglcd.h"


#define G19_PRIORITY_ASYNC		(LGLCD_ASYNC_UPDATE(LGLCD_PRIORITY_NORMAL))
#define G19_PRIORITY_SYNC		(LGLCD_SYNC_UPDATE(LGLCD_PRIORITY_NORMAL))


#define G19_VOLUME_DOWN			VK_VOLUME_DOWN
#define G19_VOLUME_UP			VK_VOLUME_UP
#define G19_KEY_MUTE			VK_VOLUME_MUTE
#define G19_KEY_FORWARD			VK_MEDIA_NEXT_TRACK
#define G19_KEY_BACK			VK_MEDIA_PREV_TRACK
#define G19_KEY_STOP			VK_MEDIA_STOP
#define G19_KEY_PLAY			VK_MEDIA_PLAY_PAUSE

#define G19_KEY_LEFT			LGLCDBUTTON_LEFT
#define G19_KEY_RIGHT			LGLCDBUTTON_RIGHT
#define G19_KEY_OK				LGLCDBUTTON_OK
#define G19_KEY_CANCEL			LGLCDBUTTON_CANCEL
#define G19_KEY_UP				LGLCDBUTTON_UP
#define G19_KEY_DOWN			LGLCDBUTTON_DOWN
#define G19_KEY_MENU			LGLCDBUTTON_MENU

#define G19_SOFTKEY1			LGLCDBUTTON_BUTTON0
#define G19_SOFTKEY2			LGLCDBUTTON_BUTTON1
#define G19_SOFTKEY3			LGLCDBUTTON_BUTTON2
#define G19_SOFTKEY4			LGLCDBUTTON_BUTTON3
#define G19_SOFTKEY5			LGLCDBUTTON_BUTTON5
#define G19_SOFTKEY6			LGLCDBUTTON_BUTTON6
#define G19_SOFTKEY7			LGLCDBUTTON_BUTTON7


struct TMYLCDG19;
typedef DWORD (*psoftkeyg19cb) (int device, DWORD dwButtons, struct TMYLCDG19 *mylcdg19);

#define G19_FRAMESIZE (LGLCD_QVGA_BMP_WIDTH * LGLCD_QVGA_BMP_HEIGHT * LGLCD_QVGA_BMP_BPP)
#define G19_TBUFFERS 1


typedef struct{
	ubyte data[G19_FRAMESIZE];
}TG19BUFFER;

typedef struct{
	TG19BUFFER buffer[G19_TBUFFERS];
	int total;
}TG19BUFFERS;

typedef struct{
	lgLcdConnectContextExA connectContext;
	lgLcdOpenByTypeContext openContext;
	lgLcdBitmapQVGAx32 bmp;
	int contextStatus;		// applet is enabled/disable by user, or device remove/enabled
}TLOGITECHG19;

typedef struct{
	psoftkeyg19cb softkeycb;
	void *ptrUser;	// user space
	int intUser;	// user space
	intptr_t priority;
	TLOGITECHG19 *lg;
	TG19BUFFERS frames;
	TFRAME *back;
	volatile int threadState;
	//TTHRDSYNCCTRL tsc;
	unsigned int updateCt;
	unsigned int t0;
	unsigned int t1; 
}TMYLCDG19;

 
#endif
#endif // #ifndef _G19DISPLAY_H_


