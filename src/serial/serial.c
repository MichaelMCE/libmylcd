
// libmylcd
// An LCD framebuffer library
// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2009  Michael McElligott
//	Core routines copyright of the OpenParPort team and project.

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

#if ((__BUILD_SERIAL__) && (__BUILD_WIN32__))

#include <windows.h>
#include "../device.h"
#include "../lstring.h"
#include "serial.h"


static INLINE void _lsendbyte (const int addr, const ubyte data);

static HANDLE hSERIAL = NULL;
static int IsDriverInitialized = 0;


int initSerial (TREGISTEREDDRIVERS *rd)
{
	TPORTDRIVER pd;
	
	l_strcpy(pd.name, "SERIAL");
	l_strcpy(pd.comment, "Serial port driver.");
	pd.open = serial_OpenDriver;
	pd.close = serial_CloseDriver;
	pd.write8 = serial_WritePort8;
	pd.write16 = serial_WritePort16;
	pd.write32 = serial_WritePort32;
	pd.writeBuffer = serial_WriteBuffer;
	pd.flush = serial_Flush;
	pd.read = serial_ReadPort;
	pd.status = LDRV_CLOSED;
	pd.setOption = NULL;
	pd.getOption = NULL;
	pd.optTotal = 0;
	return (registerPortDriver(rd, &pd) > 0);
}

void closeSerial (TREGISTEREDDRIVERS *rd)
{
	hSERIAL = NULL;
	IsDriverInitialized = 0;
	return;
}


int serial_OpenDriver (TPORTDRIVER *pd, int comPort)
{
	if (!pd){
		return 0;
	}

	if (pd->status != LDRV_CLOSED)
		return 1;
	
    char DeviceName[DEVICE_NAME_SIZE];
    wsprintf(DeviceName,DEVICE_NAME,comPort);
    HANDLE h;

	h = CreateFile(DeviceName, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == h){
        hSERIAL = NULL;
        IsDriverInitialized = 0;
        pd->status = LDRV_READY;
        return 0;
	}else{
		hSERIAL = h;
		IsDriverInitialized = 1;
		pd->status = LDRV_CLOSED;
        setupSerialPort(hSERIAL);
		return 1;
	}
}

int serial_CloseDriver (TPORTDRIVER *pd)
{
	if (!pd)
		return 0;
	else{
		if (pd->status != LDRV_CLOSED){
			IsDriverInitialized = 0;
			pd->status = LDRV_CLOSED;
			CloseHandle(hSERIAL);
			hSERIAL = NULL;
			return 1;
		}
	}
	return 0;
}


ubyte serial_ReadPort (const int addr)
{
	ubyte buffer = 0;
    unsigned long numberBytesRead = 0,comError = 0;
	COMSTAT comstat;
	
	if (IsDriverInitialized){
    	ClearCommError(hSERIAL, &comError, &comstat);
		if ((ReadFile(hSERIAL, &buffer, 1, &numberBytesRead, NULL))) return buffer;
		else return 0;
	}else{
		return 0;
	}
}

int serial_WritePort8 (const int addr, ubyte data)
{
	if (IsDriverInitialized){
		_lsendbyte(addr,data);
		return 1;
	}else{
		return 0;
	}
}

int serial_WritePort16 (const int addr, const unsigned short data)
{
	if (IsDriverInitialized){
		_lsendbyte(addr,(ubyte)((data>>8)&0xFF));
		_lsendbyte(addr,(ubyte)(data&0xFF));
		return 1;
	}else{
		return 0;
	}
}

int serial_WritePort32 (const int addr, const unsigned int data)
{
	if (IsDriverInitialized){
		_lsendbyte(addr,(ubyte)((data>>24)&0xFF));
		_lsendbyte(addr,(ubyte)((data>>16)&0xFF));
		_lsendbyte(addr,(ubyte)((data>>8)&0xFF));
		_lsendbyte(addr,(ubyte)(data&0xFF));
		return 1;
	}else{
		return 0;
	}
}

MYLCD_EXPORT int lSerialSetCommState (DCB *dcb)
{
	if (!hSERIAL || !IsDriverInitialized){
		return 0;
	}else{
		if (!SetCommState(hSERIAL,dcb))
			return 0;			
		else
			return (intptr_t)hSERIAL;
	}
}

MYLCD_EXPORT int lSerialGetCommState (DCB *dcb)
{
	if (!hSERIAL || !IsDriverInitialized){
		return 0;
	}else{
		if (!GetCommState(hSERIAL,dcb))
			return 0;			
		else
			return (intptr_t)hSERIAL;
	}
}


int setupSerialPort (HANDLE serial)
{
	COMMTIMEOUTS CommTimeouts;
	COMSTAT comstat;
  	unsigned long comError;
	DCB dcb;
			
	//if(StopBits = =  1)	STOPBITS = ONESTOPBIT;
	//if(StopBits = =  1.5) STOPBITS = ONE5STOPBITS;
	//if(StopBits = =  2)	STOPBITS = TWOSTOPBITS;
	
	lSerialGetCommState(&dcb);
	dcb.fRtsControl = RTS_CONTROL_DISABLE;
	dcb.fDtrControl = DTR_CONTROL_ENABLE;
	dcb.fOutX = dcb.fInX = FALSE;
	dcb.fOutxCtsFlow = FALSE;
	dcb.fOutxDsrFlow = 0;
	dcb.BaudRate = 9600;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	lSerialSetCommState(&dcb);

	GetCommTimeouts(serial, &CommTimeouts);
	CommTimeouts.WriteTotalTimeoutConstant = 5000;
	CommTimeouts.ReadTotalTimeoutConstant = 0;
	CommTimeouts.ReadTotalTimeoutMultiplier = 1;
	CommTimeouts.WriteTotalTimeoutMultiplier = 1;
	CommTimeouts.ReadIntervalTimeout = -1;
	SetCommTimeouts(serial, &CommTimeouts);

	PurgeComm(serial,PURGE_RXCLEAR | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_TXABORT);
 	ClearCommError(serial, &comError, &comstat);

	return 1;
}


static INLINE void _lsendbyte (const int addr, const ubyte data)
{
	unsigned long Transferred;
	ubyte _ldata = data;
	WriteFile(hSERIAL, &_ldata, 1, &Transferred, NULL);
}

int serial_WriteBuffer (const int port, void *buffer, size_t tbytes)
{
	return 0;
}

int serial_Flush (TPORTDRIVER *pd)
{
	return 0;
}


#else

int initSerial(void *rd){return 1;}
void closeSerial(void *rd){return;}

#endif

