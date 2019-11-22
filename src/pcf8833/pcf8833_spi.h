
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


#ifndef _PCF8833_SPI_H_
#define _PCF8833_SPI_H_


#include "../usb13700/usb13700_common.h"



int initPCF8833_USB (TREGISTEREDDRIVERS *rd);
void closePCF8833_USB (TREGISTEREDDRIVERS *rd);





#endif



