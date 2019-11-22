
// libusbd480 - http://mylcd.sourceforge.net/

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


//#include "mylcd.h"

//#if (__BUILD_USBD480__ && __WIN32__)

#include <windows.h>
#include <setupapi.h>

//#define _ANONYMOUS_UNION __extension__
//#define _ANONYMOUS_STRUCT __extension__
#define DDKAPI __cdecl
typedef LONG NTSTATUS;

#if defined(WIN64)
#include <hidsdi.h>
#else
#include <hidsdi.h>
#endif

typedef struct{
	char header;
	TUSBD480TOUCHCOORD16 pos;
}__attribute__((packed))TUSBD480HIDREPORT;

typedef struct{
	HIDP_CAPS capabilities;
	OVERLAPPED overlapped;
	HANDLE deviceHandle;
	HANDLE hRead;
	HANDLE hIOPort;
	int key;
	int tReports;
	int idx;
	int state;
	char devicePathname[1024];
	TUSBD480HIDREPORT report[32];
}TUSBD480HID;

#define INPUT_REPORT_SIZE (sizeof(TUSBD480TOUCHCOORD16))



int hid_FindUSBD480 (TUSBD480HID *hid, char *displaySerial, int useSerial);
int hid_OpenUSBD480 (TUSBD480HID *hid);
void hid_SignalCloseReady (TUSBD480HID *hid);
void hid_CloseUSBD480 (TUSBD480HID *hid);
int hid_GetTouchPosition (TUSBD480 *di, TUSBD480HID *hid, TTOUCHCOORD *pos);

void hid_ClearQueue (TUSBD480HID *hid);




//#endif

