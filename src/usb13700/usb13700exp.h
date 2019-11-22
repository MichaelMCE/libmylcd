
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

#ifndef _USB13700EXP_H_
#define _USB13700EXP_H_

#if ((__BUILD_USB13700LIBUSB__) && (__BUILD_USB13700EXP__) && !(__BUILD_USB13700DLL__))


#include "../usb13700libusb/libusb13700/libusb13700.h"


#define DisplayLib_ExPortSPIConfig libusb13700_ExPortSPIConfig
#define DisplayLib_GetNumberOfDisplays libusb13700_GetNumberOfDisplays
#define DisplayLib_ExPortIOGet libusb13700_ExPortIOGet
#define DisplayLib_ExPortConfig libusb13700_ExPortConfig
#define DisplayLib_ExPortIOConfig libusb13700_ExPortIOConfig
#define DisplayLib_ExPortSPIConfig libusb13700_ExPortSPIConfig
#define DisplayLib_ExPortIOSet libusb13700_ExPortIOSet
#define DisplayLib_OpenDisplay libusb13700_OpenDisplay
#define DisplayLib_CloseDisplay libusb13700_CloseDisplay
#define DisplayLib_GetDisplayConfiguration libusb13700_GetDisplayConfiguration
#define DisplayLib_ExPortSPIReceive libusb13700_ExPortSPIReceive
#define DisplayLib_ExPortIOGet libusb13700_ExPortIOGet
#define DisplayLib_ExPortSPISend libusb13700_ExPortSPISend

#else
#include "display_lib_USB13700.h"
#endif
#include "usb13700_common.h"

int initUSB13700EXP(TREGISTEREDDRIVERS *rd);
void closeUSB13700EXP(TREGISTEREDDRIVERS *rd);

#endif
