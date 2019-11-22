// libmylcd
// An LCD framebuffer library
// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2009  Michael McElligott
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

#if ((__BUILD_OPENPARPORT__) && (__BUILD_WIN32__))

#include <windows.h>
#include <winioctl.h>
#include <winerror.h>
#include "../device.h"
#include "../lstring.h"
#include "openparport.h"


#define IOCTL_PP_WRITE_DATA			0x002C0004
#define IOCTL_PP_WRITE_CONTROL		0x002C0008
#define IOCTL_PP_READ_STATUS		0x002C000C

#define PP_DEVICE_NAME				L"\\\\.\\$VDMLPT%d"
#define PP_DEVICE_NAME_SIZE			(sizeof(PP_DEVICE_NAME)+20*sizeof(wchar_t))
#define PP_OFFSET_DATA				0
#define PP_OFFSET_STATUS			1
#define PP_OFFSET_CONTROL			2

static INLINE void _lsendbyte (const int port, const ubyte data);
static INLINE void _linitport (const int port);


static HANDLE hPP = NULL;
static int BaseAddress = 0;
static int IsDriverInitialized = 0;




int initOPP (TREGISTEREDDRIVERS *rd)
{
	TPORTDRIVER pd;

	l_strcpy(pd.name, "OPP");
	l_strcpy(pd.comment, "Open Parallel Port driver. NT 5.0 only.");
	pd.open = opp_OpenDriver;
	pd.close = opp_CloseDriver;
	pd.write8 = opp_WritePort8;
	pd.write16 = opp_WritePort16;
	pd.write32 = opp_WritePort32;
	pd.writeBuffer = opp_WriteBuffer;
	pd.flush = opp_Flush;
	pd.read = opp_ReadPort;
	pd.status = LDRV_CLOSED;
	pd.setOption = NULL;
	pd.getOption = NULL;
	pd.optTotal = 0;
	return (registerPortDriver(rd, &pd) > 0);
}

void closeOPP (TREGISTEREDDRIVERS *rd)
{
	hPP = NULL;
	IsDriverInitialized = 0;
	BaseAddress = 0;
	return;
}

int opp_OpenDriver (TPORTDRIVER *pd, int port)
{
	if (!pd)
		return 0;
	else if (IsDriverInitialized)
		return 0;
	
    wchar_t DeviceName[PP_DEVICE_NAME_SIZE];
    HANDLE h;

	int DeviceId = 1;  // LPT1
    wsprintfW(DeviceName,PP_DEVICE_NAME,DeviceId);
	
	h = CreateFileW(DeviceName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
	if (INVALID_HANDLE_VALUE==h){
        hPP = NULL;
        IsDriverInitialized=0;
        return 0;
	}else{
		hPP = h;
		IsDriverInitialized=1;
		pd->status = LDRV_READY;
		if (port) BaseAddress = port;
		return 1;
	}
}

int opp_CloseDriver (TPORTDRIVER *pd)
{
	if (!pd){
		return 0;
	}else{
		IsDriverInitialized=0;
		pd->status = LDRV_CLOSED;
		CloseHandle(hPP);
		hPP = NULL;
		return 1;
	}
}

ubyte opp_ReadPort (const int port)
{
    unsigned long Transferred;
    ubyte value;

	if (IsDriverInitialized && port){
    	DeviceIoControl(hPP,IOCTL_PP_READ_STATUS,NULL,0,&value,sizeof(UCHAR), &Transferred, NULL);
		return value;
	}else{
		return 0;
	}
}

int opp_WritePort8 (const int port, ubyte data)
{
	if (IsDriverInitialized){
		if (BaseAddress){
			_lsendbyte(port,data);
			return 1;
		}else{ 
			_linitport(port);
			if (BaseAddress){
				_lsendbyte(port,data);
				return 1;
			}
		}
	}

	return 0;

}

int opp_WritePort16 (const int port, const unsigned short data)
{
	if (IsDriverInitialized){
		if (!BaseAddress){
			_linitport(port);
			if (!BaseAddress){
				return 0;
			}
		}

		_lsendbyte(port,(ubyte)((data>>8)&0xFF));
		_lsendbyte(port,(ubyte)(data&0xFF));

		return 1;
	}else{
		return 0;
	}
}

int opp_WritePort32 (const int port, const unsigned int data)
{
	if (IsDriverInitialized){
		if (!BaseAddress){
			_linitport(port);
			if (!BaseAddress){
				return 0;
			}
		}

		_lsendbyte(port,(ubyte)((data>>24)&0xFF));
		_lsendbyte(port,(ubyte)((data>>16)&0xFF));
		_lsendbyte(port,(ubyte)((data>>8)&0xFF));
		_lsendbyte(port,(ubyte)(data&0xFF));
		
		return 1;
	}else{
		return 0;
	}

}


static INLINE void _lsendbyte (const int port, const ubyte data)
{
	unsigned long Transferred;
    unsigned long FakeByteForBug;
    ubyte _ldata = data;
    
	if (hPP){
    	if ((port-BaseAddress)==PP_OFFSET_DATA)
    		DeviceIoControl(hPP,IOCTL_PP_WRITE_DATA,&_ldata,sizeof(ubyte),&FakeByteForBug,sizeof(ULONG),&Transferred,NULL);
    	else
    		DeviceIoControl(hPP,IOCTL_PP_WRITE_CONTROL,&_ldata,sizeof(ubyte),&FakeByteForBug,sizeof(ULONG),&Transferred,NULL);
	}
}

static INLINE void _linitport (const int port)
{
	if (port) BaseAddress = port;
}


int opp_WriteBuffer (const int port, void *buffer, size_t tbytes)
{
	return 0;
}

int opp_Flush (TPORTDRIVER *pd)
{
	return 0;
}

#else

int initOPP(void *rd){return 1;}
void closeOPP(void *rd){return;}


#endif
