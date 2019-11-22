
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

#if ((__BUILD_FT245__) && (__BUILD_WIN32__))

#include <windows.h>
#include "../device.h"
#include "../memory.h"
#include "../lstring.h"
#include "../misc.h"
#include "FTD2XX.H"
#include "ft245.h"

#define FTD2XX_DLL "FTD2XX.dll"
#define bufferSize (64000)


typedef FT_STATUS (WINAPI *pOpen) (int deviceNumber, FT_HANDLE *pHandle);
typedef FT_STATUS (WINAPI *pOpenEx) (PVOID pArg1, DWORD Flags, FT_HANDLE *pHandle);
typedef FT_STATUS (WINAPI *pClose) (FT_HANDLE ftHandle);
typedef FT_STATUS (WINAPI *pRead) (FT_HANDLE ftHandle, LPVOID lpBuffer, DWORD nBufferSize, LPDWORD lpBytesReturned);
typedef FT_STATUS (WINAPI *pWrite) (FT_HANDLE ftHandle, LPVOID lpBuffer, DWORD nBufferSize, LPDWORD lpBytesWritten);
typedef FT_STATUS (WINAPI *pSetUSBParameters) (FT_HANDLE ftHandle, ULONG ulInTransferSize, ULONG ulOutTransferSize);
typedef FT_STATUS (WINAPI *pSetDivisor) (FT_HANDLE ftHandle, USHORT Divisor);
typedef FT_STATUS (WINAPI *pSetBaudRate) (FT_HANDLE ftHandle, ULONG BaudRate);
typedef FT_STATUS (WINAPI *pSetLatencyTimer) (FT_HANDLE ftHandle, UCHAR ucLatency);
typedef FT_STATUS (WINAPI *pSetBitMode) (FT_HANDLE ftHandle, UCHAR ucMask, UCHAR ucEnable);
typedef FT_STATUS (WINAPI *pPurge) (FT_HANDLE ftHandle, ULONG Mask);
typedef FT_STATUS (WINAPI *pResetDevice) (FT_HANDLE ftHandle);

typedef struct {
	HANDLE hLib;
	pOpen Open;
	pOpenEx	OpenEx;
	pClose	Close;
	pRead	Read;
	pWrite Write;
	pSetUSBParameters SetUSBParameters;
	pSetDivisor SetDivisor;
	pSetBaudRate SetBaudRate;
	pSetLatencyTimer SetLatencyTimer;
	pSetBitMode SetBitMode;
	pPurge Purge;
	pResetDevice ResetDevice;
}TMYLCDFT;

static int ft245_OpenDriver (TPORTDRIVER *pd, int port);
static int ft245_CloseDriver (TPORTDRIVER *pd);
static int ft245_WritePort8 (const int port, const ubyte data);
static int ft245_WritePort16 (const int port, const unsigned short data);
static int ft245_WritePort32 (const int port, const unsigned int data);
static int ft245_WriteBuffer (const int port, void *buffer, size_t tbytes);
static int ft245_Flush (TPORTDRIVER *pd);
static int ft245_setOption (TDRIVER *drv, int option, intptr_t *value);
static int ft245_getOption (TDRIVER *drv, int option, intptr_t *value);
static ubyte ft245_ReadPort (const int port);

static int resetDevice();
static int setBaudRate (int baud);
static int setLatency (ubyte ucLatency);
static int setBitMode (ubyte mask, ubyte mode);

static TMYLCDFT mylcdft;
static TMYLCDFT *ft = NULL;
static FT_HANDLE ftHandle = 0;
static unsigned int bytesToWrite = 0;			// queue position within buffer
static ubyte *writeBuffer = NULL ; //[bufferSize];

//#define USB_DEVICE_DESCRIPTION "USB Serial Converter"
#define USB_DEVICE_DESCRIPTION "USB <-> Serial"


int initFT245 (TREGISTEREDDRIVERS *rd)
{
	TPORTDRIVER pd;

	ft = &mylcdft;
	memset(ft, 0 ,sizeof(TMYLCDFT));

	l_strcpy(pd.name, "FT245");
	l_strcpy(pd.comment, "USB - FTDI FT245xx. D2XX direct driver interface");
	pd.open = ft245_OpenDriver;
	pd.close = ft245_CloseDriver;
	pd.write8 = ft245_WritePort8;
	pd.write16 = ft245_WritePort16;
	pd.write32 = ft245_WritePort32;
	pd.writeBuffer = ft245_WriteBuffer;
	pd.flush = ft245_Flush;
	pd.read = ft245_ReadPort;
	pd.status = LDRV_CLOSED;
	pd.setOption = ft245_setOption;
	pd.getOption = ft245_getOption;
	pd.optTotal = 5;

	int drvnum = registerPortDriver(rd, &pd);
	setDefaultPortOption(rd, drvnum, lOPT_FT245_BAUDRATE, 187500);
	setDefaultPortOption(rd, drvnum, lOPT_FT245_DIVISOR, 16);
	setDefaultPortOption(rd, drvnum, lOPT_FT245_LATENCY, 2);
	setDefaultPortOption(rd, drvnum, lOPT_FT245_IODIR, 0xFF);
	setDefaultPortOption(rd, drvnum, lOPT_FT245_BITBANG, 1);
	return (drvnum > 0);
}

void closeFT245 (TREGISTEREDDRIVERS *rd)
{
	if (ft->hLib)
		FreeLibrary(ft->hLib);
	ft->hLib = 0;
	return;
}

int loadFTLib (TMYLCDFT *ft, const char *dllpathname)
{
	if (ft->hLib)
		return 1;
	ft->hLib = GetModuleHandle(dllpathname);
	if (ft->hLib == NULL)
		ft->hLib = LoadLibrary(dllpathname);
	if (ft->hLib == NULL){
		printf("libmylcd: loadFTLib(): '%s' not found\n",dllpathname);
		return 0;
	}

	ft->Open = (pOpen)GetProcAddress(ft->hLib, "FT_Open");
	ft->OpenEx = (pOpenEx)GetProcAddress(ft->hLib, "FT_OpenEx");
	ft->SetUSBParameters = (pSetUSBParameters)GetProcAddress(ft->hLib, "FT_SetUSBParameters");
	ft->Close = (pClose)GetProcAddress(ft->hLib, "FT_Close");
	ft->Read = (pRead)GetProcAddress(ft->hLib, "FT_Read");
	ft->Write = (pWrite)GetProcAddress(ft->hLib, "FT_Write");
	ft->SetDivisor = (pSetDivisor)GetProcAddress(ft->hLib, "FT_SetDivisor");
	ft->SetBaudRate = (pSetBaudRate)GetProcAddress(ft->hLib, "FT_SetBaudRate");
	ft->SetLatencyTimer = (pSetLatencyTimer)GetProcAddress(ft->hLib, "FT_SetLatencyTimer");
	ft->SetBitMode = (pSetBitMode)GetProcAddress(ft->hLib, "FT_SetBitMode");
	ft->ResetDevice = (pResetDevice)GetProcAddress(ft->hLib, "FT_ResetDevice");
	ft->Purge = (pPurge)GetProcAddress(ft->hLib, "FT_Purge");

	if ((intptr_t)ft->OpenEx)
		return 1;
	else
		return 0;
}

static int ft245_OpenDriver (TPORTDRIVER *pd, int BaudRate)
{
	if (pd == NULL)
		return 0;

	if (!loadFTLib(&mylcdft, FTD2XX_DLL))
		return 0;

	FT_STATUS ftret = 0;
	ubyte ucLatency = 2;		// latency value between 2ms and 255ms.
	//int usbdevice = 0;		// FTDI device [number] on usb hub. USBView can be used to obtain device #

	//ftret = ft->Open(usbdevice, &ftHandle);
	//ftret = ft->OpenEx("", FT_OPEN_BY_SERIAL_NUMBER, &ftHandle);
	ftret = ft->OpenEx(USB_DEVICE_DESCRIPTION, FT_OPEN_BY_DESCRIPTION, &ftHandle);
	if (ftret != FT_OK){
		mylog("libmylcd: FT_OpenEx: failed to open device %s, return code :%i\n", USB_DEVICE_DESCRIPTION, (int)ftret);
		pd->status = LDRV_CLOSED;
		ftHandle = 0;
		return 0;
	}else{
		//l_memset(writeBuffer, 0, bufferSize);
		writeBuffer = (ubyte *)l_calloc(sizeof(ubyte), bufferSize);
		if (writeBuffer == NULL)
			return 0;
		resetDevice();
		setBitMode(0xFF, 1);
		setLatency(ucLatency);
		setBaudRate(BaudRate);
		ft->SetUSBParameters(ftHandle, bufferSize, bufferSize);
		pd->write8(0,0);
		pd->status = LDRV_READY;
		return 1;
	}
}

static int ft245_CloseDriver (TPORTDRIVER *pd)
{
	if (pd && ftHandle){
		pd->write8(0,0);
		pd->flush(pd);
		pd->status = LDRV_CLOSED;
		ft->Close(ftHandle);
		if (ft->hLib)
			FreeLibrary(ft->hLib);
		ft->hLib = 0;
		if (writeBuffer)
			l_free(writeBuffer);
		writeBuffer = NULL;
		ftHandle = 0;
		bytesToWrite = 0;
		return 1;
	}else
		return 0;
}

static ubyte ft245_ReadPort (const int port)
{
	if (ftHandle){
		unsigned long buffer = 0;
		DWORD lpBytesRead = 0;
		ft->Read(ftHandle, &buffer, 1, &lpBytesRead);
		if (lpBytesRead)
			return (ubyte)buffer;
	}
	return 0;
}

static int ft245_Flush (TPORTDRIVER *pd)
{
	if (bytesToWrite){
		DWORD lpBytesWritten = 0;
		#ifdef __DEBUG__
			FT_STATUS ftret = ft->Write(ftHandle, writeBuffer, bytesToWrite, &lpBytesWritten);
		#else
			ft->Write(ftHandle, writeBuffer, bytesToWrite, &lpBytesWritten);
		#endif
		//printf("flush %i %i\n",bytesToWrite, (int)lpBytesWritten);
		if (bytesToWrite != lpBytesWritten){
			#ifdef __DEBUG__
			mylog("libmylcd: FT245: write error: %i %i %i\n",(int)ftret, (int)bytesToWrite, (int)lpBytesWritten);
			#endif
			bytesToWrite = 0;
		}else{
			bytesToWrite = 0;
			//ft->Purge(ftHandle, FT_PURGE_TX);
			return 1;
		}
	}
	return 0;
}

static int ft245_WritePort8 (const int port, const ubyte data)
{
	if (ftHandle){
		writeBuffer[bytesToWrite] = data;
		if (++bytesToWrite == bufferSize)
			return ft245_Flush(NULL);
	}
	return 0;

}

static int ft245_WritePort16 (const int port, const unsigned short data)
{
	if (ftHandle){
		ft245_WritePort8(port,(ubyte)((data>>8)&0xFF));
		ft245_WritePort8(port,(ubyte)(data&0xFF));
	}
	return 0;
}

static int ft245_WritePort32 (const int port, const unsigned int data)
{
	if (ftHandle){
		ft245_WritePort8(port,(ubyte)((data>>24)&0xFF));
		ft245_WritePort8(port,(ubyte)((data>>16)&0xFF));
		ft245_WritePort8(port,(ubyte)((data>>8)&0xFF));
		ft245_WritePort8(port,(ubyte)(data&0xFF));
	}
	return 0;
}


static int ft245_WriteBuffer (const int port, void *buffer, size_t tbytes)
{
	return 0;
}

static int ft245_setOption (TDRIVER *drv, int option, intptr_t *value)
{
	intptr_t *opt = drv->pd->opt;

	switch(option){
		case lOPT_FT245_BAUDRATE :
			if (value){
				if (setBaudRate((int)*value)){
					opt[lOPT_FT245_BAUDRATE] = *value;
					return 1;
				}
			}
			return 0;
		case lOPT_FT245_DIVISOR :
			if (ft->SetDivisor(ftHandle, *value)){
				opt[lOPT_FT245_DIVISOR] = *value;
				return 1;
			}
			return 0;
		case lOPT_FT245_LATENCY :
			if (*value&0xFE){  						// range = 2ms to 255ms
				if (setLatency(*value)){
					opt[lOPT_FT245_LATENCY] = *value;
					return 1;
				}
			}
			return 0;
		case lOPT_FT245_IODIR :
			if (setBitMode(*value&0xFF, opt[lOPT_FT245_BITBANG])){
				opt[lOPT_FT245_IODIR] = *value;
				return 1;
			}
			return 0;
		case lOPT_FT245_BITBANG :
			if (setBitMode(opt[lOPT_FT245_IODIR], *value&0xFF)){
				opt[lOPT_FT245_BITBANG] = *value;
				return 1;
			}
			return 0;
		default :
			mylog("libmylcd: FT245: setOption(): invalid option: %i:%i\n",option, value);
			return 0;
	};
	return 0;
}

static int ft245_getOption (TDRIVER *drv, int option, intptr_t *value)
{
	if (!drv || !value)	return 0;
	int *opt = (int *)drv->pd->opt;

	switch(option){
		case lOPT_FT245_BAUDRATE :
			*value = opt[lOPT_FT245_BAUDRATE];
			return 1;
		case lOPT_FT245_DIVISOR :
			*value = opt[lOPT_FT245_DIVISOR];
			return 1;
		case lOPT_FT245_BITBANG :
			*value = opt[lOPT_FT245_BITBANG];
			return 1;
		case lOPT_FT245_LATENCY :
			*value = opt[lOPT_FT245_LATENCY];
			return 1;
		case lOPT_FT245_IODIR :
			*value = opt[lOPT_FT245_IODIR];
			return 1;
		default :
			mylog("libmylcd: FT245: setOption(): invalid option:%i\n", option);
			return 0;
	};
	return 0;
}

static int setBaudRate (int baud)
{
	if (!baud) baud = 187500;

	FT_STATUS ftret = ft->SetBaudRate(ftHandle, baud);
	if (ftret != FT_OK){
		mylog("libmylcd: FT245: unable to set requested baud rate:%i %i\n",baud, (int)ftret);
		return 0;
	}else{
		//mylog("FT245: baud rate set to %i\n",baud);
		return 1;
	}
}

static int setLatency (ubyte ucLatency)
{
	FT_STATUS ftret = ft->SetLatencyTimer(ftHandle, ucLatency);
	if (ftret != FT_OK){
		mylog("libmylcd: FT245: unable to set requested latency rate:%i %i\n",(int)ucLatency, (int)ftret);
		return 0;
	}else
		return 1;
}

static int setBitMode (ubyte mask, ubyte mode)
{
	FT_STATUS ftret = ft->SetBitMode(ftHandle, mask, mode);	// set d0-d7 to outputs and enable bitbang mode
	if (ftret != FT_OK){
		mylog("libmylcd: FT245: unable to set bit bang mode:%i %i %i\n",(int)mask, (int)mode, (int)ftret);
		return 0;
	}else
		return 1;
}

static int resetDevice()
{
	FT_STATUS ftret = ft->ResetDevice(ftHandle);
	if (ftret != FT_OK){
		mylog("libmylcd: FT245: unable to reset FT device:%i\n",(int)ftret);
		return 0;
	}else
		return 1;
}

#else

int initFT245(void *rd){return 1;}
void closeFT245(void *rd){return;}

#endif


