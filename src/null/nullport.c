
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

#if (__BUILD_NULLPORT__)

#include "../device.h"
#include "../lstring.h"
#include "nullport.h"


int initNullPort (TREGISTEREDDRIVERS *rd)
{
	TPORTDRIVER pd;
	
	l_strcpy(pd.name,"NULL");
	l_strcpy(pd.comment,"Null port driver.");
	pd.open = nullPort_OpenDriver;
	pd.close = nullPort_CloseDriver;
	pd.write8 = nullPort_WritePort8;
	pd.write16 = nullPort_WritePort16;
	pd.write32 = nullPort_WritePort32;
	pd.writeBuffer = nullPort_WriteBuffer;
	pd.flush = nullPort_Flush;
	pd.read = nullPort_ReadPort;
	pd.status = LDRV_CLOSED;
	pd.setOption = NULL;
	pd.getOption = NULL;
	pd.optTotal = 0;
	return (registerPortDriver(rd, &pd) > 0);
}

void closeNullPort (TREGISTEREDDRIVERS *rd)
{
	return;
}

int nullPort_OpenDriver (TPORTDRIVER *pd, int port)
{
	pd->status = LDRV_READY;
	if (port){}
	return 1;
}

int nullPort_CloseDriver (TPORTDRIVER *pd)
{
	pd->status = LDRV_CLOSED;
	return 1;
}

ubyte nullPort_ReadPort (const int port)
{
	if (port){}
	return 0;
}

int nullPort_WritePort8 (const int port, const ubyte data)
{
	if (port){}
	if (data){}
	return 1;
}

int nullPort_WritePort16 (const int port, const unsigned short data)
{
	if (port){}
	if (data){}
	return 1;
}

int nullPort_WritePort32 (const int port, const unsigned int data)
{
	if (port){}
	if (data){}
	return 1;
}

int nullPort_WriteBuffer (const int port, void *buffer, size_t tbytes)
{
	return 0;
}

int nullPort_Flush (TPORTDRIVER *pd)
{
	return 0;
}


#else

int initNullPort(void *rd){return 1;}
void closeNullPort(void *rd){return;}

#endif


