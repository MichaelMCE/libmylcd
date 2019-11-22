
// libmylcd - http://mylcd.sourceforge.net/
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


#ifndef _DEVICE_H_
#define _DEVICE_H_



// internal struct containing registered drivers
typedef struct{
	TPORTDRIVER **pd; // list of available port drivers
	TDISPLAYDRIVER **dd; // list of available display drivers
	unsigned int dtotal;
	unsigned int ptotal;
}TREGISTEREDDRIVERS;

int registerPortDriver (TREGISTEREDDRIVERS *rd, TPORTDRIVER *pd);
int registerDisplayDriver (TREGISTEREDDRIVERS *rd, TDISPLAYDRIVER *dd);
int setDefaultPortOption (TREGISTEREDDRIVERS *rd, lDISPLAY dnumber, int option, intptr_t value);
int setDefaultDisplayOption (TREGISTEREDDRIVERS *rd, lDISPLAY dnumber, int option, intptr_t value);
TDRIVER *displayIDToDriver (THWD *hw, lDISPLAY dnumber);
lDISPLAY selectDevice (THWD *hw, const char *displayd, const char *portd, const int width, const int height, int bpp, const int port, TRECT *rect);
int setCapabilities (THWD *hw, const unsigned int flag, const int value);
int getCapabilities (THWD *hw, const unsigned int flag);
TREGDRV *enumerateDriversBegin (THWD *hw, int edrv);
void enumerateDriverEnd (TREGDRV *d);
int enumerateDriverNext (TREGDRV *d);
int closeDeviceId (THWD *hw, const lDISPLAY did);
lDISPLAY driverNameToID (THWD *hw, const char *name, int ldrv_type);

// selected/activated drivers/devices tree
THWD *newHWDeviceDesc ();
void freeHWDeviceDesc (THWD *hw);

// registered driver/device tree
TREGISTEREDDRIVERS *createDeviceTree ();
void freeDeviceTree (TREGISTEREDDRIVERS *rd);

#endif



