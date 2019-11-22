
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

#if ((__BUILD_WINIO__) && (__BUILD_WIN32__))

#include <windows.h>
#include <dos.h>
#include <conio.h>
#include "../device.h"
#include "../lstring.h"
#include "winiodll.h"
#include "winio.h"


static int winio_OpenDriver (TPORTDRIVER *pd, int port);
static int winio_CloseDriver (TPORTDRIVER *pd);
static int winio_WritePort8 (const int port, const ubyte data);
static int winio_WritePort16 (const int port, const unsigned short data);
static int winio_WritePort32 (const int port, const unsigned int data);
static int winio_WriteBuffer (const int port, void *buffer, size_t tbytes);
static int winio_Flush (TPORTDRIVER *pd);
static ubyte winio_ReadPort (const int port);

/*
void outp(int adresse, char wert){
  __asm("MOVW %1, %%dx    \n\t"
        "MOVB %0, %%al    \n\t"
        "OUT  %%al, %%dx"
        :
        : "g" (wert), "g" (adresse) );
} 
*/


// define in winiodll.c
extern int IsWinIoInitialized;


int initWinIO (TREGISTEREDDRIVERS *rd)
{
	TPORTDRIVER pd;
	
	l_strcpy(pd.name, "WinIO");
	l_strcpy(pd.comment, "WinIO parallel port driver (winio.sys).");
	pd.open = winio_OpenDriver;
	pd.close = winio_CloseDriver;
	pd.write8 = winio_WritePort8;
	pd.write16 = winio_WritePort16;
	pd.write32 = winio_WritePort32;
	pd.writeBuffer = winio_WriteBuffer;
	pd.flush = winio_Flush;
	pd.read = winio_ReadPort;
	pd.status = LDRV_CLOSED;
	pd.setOption = NULL;
	pd.getOption = NULL;
	pd.optTotal = 0;
	return (registerPortDriver(rd, &pd) > 0);
}

void closeWinIO (TREGISTEREDDRIVERS *rd)
{
	return;
}

static int winio_OpenDriver (TPORTDRIVER *pd, int port)
{
	
	if (IsWinIoInitialized){
		pd->status = LDRV_READY;
		return 1;
	}
	
	InitializeWinIo();
	if (!IsWinIoInitialized){
		StartWinIoDriver();
		InitializeWinIo();
	}
	if (IsWinIoInitialized){
		pd->status = LDRV_READY;
		port = 0; // keeps compiler happy
		ubyte d = pd->read(port + 0x402);
		pd->write8(port + 0x402,(d & 0x1F)+4*0x20);
	}
	
	return IsWinIoInitialized;
}

static int winio_CloseDriver (TPORTDRIVER *pd)
{
	pd->status = LDRV_CLOSED;
	return 1;
}

static ubyte winio_ReadPort (const int port)
{
	if (IsWinIoInitialized){
		ubyte ret;
		unsigned short _lport = (unsigned short)port;

		#if (defined(__GNUC__))
		  __asm ("inb %1,%0":"=a"(ret):"Nd"(_lport));
		#else
		  _inp(port,&ret);
		#endif

		return ret;
	}else
		return 0;
}

//int __cdecl _outp(unsigned short, int);

static int winio_WritePort8 (const int port, const ubyte data)
{
	if (IsWinIoInitialized){
		#if (defined(__GNUC__))
		  __asm ("outb %b0,%w1": :"a" (data), "Nd" (port));
		#else
		 _outp(port, data);
		#endif

		return 1;
	}else
		return 0;
}

static int winio_WritePort16 (const int port, const unsigned short data)
{
	if (IsWinIoInitialized){
		winio_WritePort8(port,(ubyte)((data>>8)&0xFF));
		winio_WritePort8(port,(ubyte)(data&0xFF));
		return 1;
	}else
		return 0;
}

static int winio_WritePort32 (const int port, const unsigned int data)
{
	if (IsWinIoInitialized){
		winio_WritePort8(port,(ubyte)((data>>24)&0xFF));
		winio_WritePort8(port,(ubyte)((data>>16)&0xFF));
		winio_WritePort8(port,(ubyte)((data>>8)&0xFF));
		winio_WritePort8(port,(ubyte)(data&0xFF));
		return 1;
	}else
		return 0;
}

static int winio_WriteBuffer (const int port, void *buffer, size_t tbytes)
{
	while(tbytes--)
		winio_WritePort8(port, *(ubyte*)buffer++);
		
	return 1;
}

static int winio_Flush (TPORTDRIVER *pd)
{
	return 1;
}



#else

int initWinIO(void *rd){return 1;}
void closeWinIO(void *rd){return;}

#endif

