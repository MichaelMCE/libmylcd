
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


#ifndef _SERIAL_H_
#define _SERIAL_H_


#define DEVICE_NAME				"\\\\.\\COM%d"
#define DEVICE_NAME_SIZE		(sizeof(DEVICE_NAME)+16*sizeof(ubyte))



int initSerial (TREGISTEREDDRIVERS *rd);
void closeSerial (TREGISTEREDDRIVERS *rd);

 
#if ((__BUILD_SERIAL__) && (__BUILD_WIN32__))
#include <windows.h>

MYLCD_EXPORT int  lSerialSetCommState (DCB *dcb);
MYLCD_EXPORT int  lSerialGetCommState (DCB *dcb);

int setupSerialPort (HANDLE serial);
int serial_OpenDriver (TPORTDRIVER *pd, int comPort);
int serial_CloseDriver (TPORTDRIVER *pd);
int serial_WritePort8 (const int port, const ubyte data);
int serial_WritePort16 (const int port, const unsigned short data);
int serial_WritePort32 (const int port, const unsigned int data);
int serial_WriteBuffer (const int port, void *buffer, size_t tbytes);
int serial_Flush (TPORTDRIVER *pd);
ubyte serial_ReadPort (const int port);


#endif

#endif
