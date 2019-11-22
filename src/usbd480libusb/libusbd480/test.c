
// libusbd480 1.1 - http://mylcd.sourceforge.net/

// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2010  Michael McElligott
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


#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>

#include "libusbd480.h"


/*
gcc test.c libusbd480.c hid.c -lusb -lsetupapi -lhid
*/



int main (int argc, char **argv)
{
	TUSBD480 di;
	
	if (libusbd480_OpenDisplay(&di, 0)){		// open first device
		printf("Name:%s\nWidth:%i\nHeight:%i\nVersion:0x%X\nSerial:%s\n", di.Name, di.Width, di.Height, di.DeviceVersion, di.Serial);
/*	
		int i;
		for (i = 0; i < 256; i++){
			libusbd480_SetBrightness(&di, i);	// do something
			Sleep(10);
		}
*/
		libusbd480_SetConfigValue(&di, CFG_USB_ENUMERATION_MODE, 1);
		libusbd480_CloseDisplay(&di);			// shutdown
	}
	return 1;
}
