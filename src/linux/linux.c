
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

#if (__BUILD_LINUXPORT__)

#include <sys/io.h>
#include <sys/perm.h>

#include "../device.h"
#include "../lstring.h"
#include "linux.h"


static int IsDriverInitialized=0;
static int initport (const int port);


int initLinux (TREGISTEREDDRIVERS *rd)
{
	TPORTDRIVER pd;
	
	l_strcpy(pd.name, "Linux");
	l_strcpy(pd.comment, "Linux LPT port driver. Work in progress.");
	pd.open = linux_OpenDriver;
	pd.close = linux_CloseDriver;
	pd.write8 = linux_WritePort8;
	pd.write16 = linux_WritePort16;
	pd.write32 = linux_WritePort32;
	pd.writeBuffer = linux_WriteBuffer;
	pd.flush = linux_Flush;
	pd.read = linux_ReadPort;
	pd.status = LDRV_CLOSED;
	pd.setOption = NULL;
	pd.getOption = NULL;
	pd.optTotal = 0;

	return (registerPortDriver(rd, &pd) > 0);
}

void closeLinux (TREGISTEREDDRIVERS *rd)
{
	return;
}

int linux_OpenDriver (TPORTDRIVER *pd, int port)
{
	if (!pd) return 0;
	if (((pd->status == LDRV_READY) || IsDriverInitialized) && port)
		return 1;
	else{
		if (initport(port)){
			pd->status = LDRV_READY;
			return 1;
		}else{
			pd->status = LDRV_CLOSED;
			return 0;
		}
	}
}

int linux_CloseDriver (TPORTDRIVER *pd)
{
	pd->status = LDRV_CLOSED;
	IsDriverInitialized=0;
	return 1;
}


ubyte linux_ReadPort (const int port)
{
	if (IsDriverInitialized)
		return inb(port);

	return 0;
}

int linux_WritePort8 (const int port, const ubyte data)
{
	if (IsDriverInitialized){
		outb(data,port);
		return 1;
	}

	return 0;
}

int linux_WritePort16 (const int port, const unsigned short data)
{
	if (IsDriverInitialized){
		outb((ubyte)((data>>8)&0xFF),port);
		outb((ubyte)(data&0xFF),port);
		return 1;
	}

	return 0;
}


int linux_WritePort32 (const int port, const unsigned int data)
{
	if (IsDriverInitialized){
		outb((ubyte)((data>>24)&0xFF),port);
		outb((ubyte)((data>>16)&0xFF),port);
		outb((ubyte)((data>>8)&0xFF),port);
		outb((ubyte)(data&0xFF),port);
		return 1;
	}

	return 0;
}

static int initport (const int port)
{
	if (!ioperm(port,3,1)){
		//mylog("access to port 0x%x granted\n", port);
		IsDriverInitialized=1;
		return 1;
	}else{
		mylog("libmylcd: access to port 0x%x denied\n", port);
		IsDriverInitialized=0;
		return 0;
	}
}

int linux_WriteBuffer (const int port, void *buffer, size_t tbytes)
{
	return 0;
}

int linux_Flush (TPORTDRIVER *pd)
{
	return 0;
}

#else

int initLinux(void *rd){return 1;}
void closeLinux(void *rd){return;}

#endif

