
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


#ifndef _USBD480DLL_H_
#define _USBD480DLL_H_


int initUSBD480DLL (TREGISTEREDDRIVERS *rd);
void closeUSBD480DLL (TREGISTEREDDRIVERS *rd);


#define DEVICE_NAMELENGTH		64

#define PIXELDELTA				50		/* panel delta. TODO: make this dynamic */ 
#define DEBOUNCETIME			120		/* millisecond period*/
#define DRAGDEBOUNCETIME		40

#define USBD480_TOUCH_MODE_4BYTEOLD		0	// original 4byte report
#define USBD480_TOUCH_MODE_16BYTEFILTER	1	// 16 byte with simple filtering that tries to discard most erroneous samples, no smoothing.
#define USBD480_TOUCH_MODE_16BYTERAW	2	// 16 byte report, raw data without processing.


typedef struct {
	unsigned short x;
	unsigned short y;
	unsigned short z1;
	unsigned short z2;
	unsigned char pen;
	unsigned char pressure;
	char padto16[6];
}TUSBD480TOUCHCOORD16DLL;

typedef struct {
	int mark;
	TTOUCHCOORD loc;
}TINDLL;

typedef struct {
	int left;
	int right;
	int top;
	int bottom;
	float hrange;
	float vrange;
	float hfactor;
	float vfactor;
}TCALIBRATIONDLL;

typedef struct {
	TTOUCHCOORD pos;		// current position
	TTOUCHCOORD last;		// previous position
	TINDLL tin[16];
	TCALIBRATIONDLL cal;
	unsigned int count;		// number of filtered locations processed and accepted
	unsigned int dragState;
	int	dragStartCt;
}TTOUCHDLL;

typedef int (*lpTouchCBdll) (TTOUCHCOORD *pos, int flags, intptr_t *userPtr);


typedef struct{
	uint32_t Width;
    uint32_t Height;
    uint32_t PixelFormat;
	uint16_t DeviceVersion;
	
	char name[DEVICE_NAMELENGTH];   
    char username[DEVICE_NAMELENGTH];

	int currentPage;
	intptr_t *userPtr;
	int threadState;
	
	TTOUCHDLL touch;
	TTHRDSYNCCTRL hTouchThread;
	lpTouchCBdll TouchCB;
	TUSBD480TOUCHCOORD16DLL upos;
	DisplayInfo di;
	unsigned int *convertBuffer;
}TUSBD480DIDLL;



#endif 


