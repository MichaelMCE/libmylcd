
// DLPORTIO interface driver

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

#if ((__BUILD_DLPORTIO__) && (__BUILD_WIN32__))

#include <windows.h>
#include "../memory.h"
#include "../device.h"
#include "../lstring.h"
#include "../misc.h"
#include "dlp.h"



typedef void  (__stdcall *pWrite) (ULONG Port, ubyte Value);
typedef ubyte (__stdcall *pRead)  (ULONG Port);

typedef struct {
	HANDLE hLib;
	pWrite	Write;
	pRead	Read;
}TMYLCDDLP;

static TMYLCDDLP mylcddlp;
static TMYLCDDLP *dlp = &mylcddlp;



int loadDLPLib (TMYLCDDLP *dlp, char *dllpathname)
{
	if (dlp->hLib)
		return 1;

	dlp->hLib = LoadLibrary(dllpathname);
	if (dlp->hLib == NULL){
		mylog("libmylcd: '%s' not found\n", dllpathname);
		return 0;
	}

	dlp->Write = (pWrite)GetProcAddress(dlp->hLib, "DlPortWritePortUchar");
	dlp->Read = (pRead)GetProcAddress(dlp->hLib, "DlPortReadPortUchar");

	if ((intptr_t)dlp->Write && (intptr_t)dlp->Read)
		return 1;
	else
		return 0;
}

int initDLP (TREGISTEREDDRIVERS *rd)
{
	l_memset(dlp, 0 ,sizeof(TMYLCDDLP));

	TPORTDRIVER pd;
	l_strcpy(pd.name, "DLPortIO");
	l_strcpy(pd.comment, "DriverLINX Port I/O Driver Interface (DLPORTIO.dll).");
	pd.open = DlPort_OpenDriver;
	pd.close = DlPort_CloseDriver;
	pd.write8 = DlPort_WritePort8;
	pd.write16 = DlPort_WritePort16;
	pd.write32 = DlPort_WritePort32;
	pd.writeBuffer = DlPort_WriteBuffer;
	pd.flush = DlPort_Flush;
	pd.read = DlPort_ReadPort;
	pd.setOption = NULL;
	pd.getOption = NULL;
	pd.status = LDRV_CLOSED;
	pd.optTotal = 0;

	return (registerPortDriver(rd, &pd) > 0);
}

void closeDLP (TREGISTEREDDRIVERS *rd)
{
	if (dlp->hLib)
		FreeLibrary(dlp->hLib);
	dlp->hLib = NULL;
	return;
}

int DlPort_OpenDriver(TPORTDRIVER *pd, int port)
{

	if (!pd)
		return 0;

	if (!loadDLPLib(&mylcddlp, "DLPORTIO.dll"))
		return 0;

	if (port){
		pd->status = LDRV_READY;
		return 1;
	}else{
		pd->status = LDRV_CLOSED;
		return 0;
	}
}

int DlPort_CloseDriver (TPORTDRIVER *pd)
{
	if (!pd)
		return 0;

	if (dlp->hLib)
		FreeLibrary(dlp->hLib);
	dlp->hLib = NULL;
	pd->status = LDRV_CLOSED;
	return 1;
}

ubyte DlPort_ReadPort (const int port)
{
	return dlp->Read(port);
}

int DlPort_WritePort8 (const int port, const ubyte data)
{
	dlp->Write(port,data);
	return 1;
}

int DlPort_WritePort16 (const int port, const unsigned short data)
{
	dlp->Write(port,(ubyte)((data>>8)&0xFF));
	dlp->Write(port,(ubyte)(data&0xFF));
	return 1;
}

int DlPort_WritePort32 (const int port, const unsigned int data)
{
	dlp->Write(port,(ubyte)((data>>24)&0xFF));
	dlp->Write(port,(ubyte)((data>>16)&0xFF));
	dlp->Write(port,(ubyte)((data>>8)&0xFF));
	dlp->Write(port,(ubyte)(data&0xFF));
	return 1;
}


int DlPort_WriteBuffer (const int port, void *buffer, size_t tbytes)
{
	return 0;
}

int DlPort_Flush (TPORTDRIVER *pd)
{
	return 0;
}

#else

int initDLP(void *rd){return 1;}
void closeDLP(void *rd){return;}


#endif

