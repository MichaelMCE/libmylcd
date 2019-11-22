
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



#ifndef _FTDI_H_
#define _FTDI_H_

#if (__BUILD_FTDI__)
#include "libs/libftdi.h"


#define FT_VID 0x0403
#define FT_PID 0x6001

#ifndef FT_OK
#define FT_OK 0
#endif
#define FT_BB_ENABLED 1
#define FT_BB_DISABLED 1

#define DEFAULT_BAUD 758900
#define BUFFERLENGTH 64000

int resetDevice ();
int setBaudRate (int baud);
int setLatency (ubyte ucLatency);
int setBitMode (ubyte mask, ubyte mode);
#endif

int initFTDI (TREGISTEREDDRIVERS *rd);
void closeFTDI (TREGISTEREDDRIVERS *rd);



#endif

