
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


#ifndef _LINUX_H_
#define _LINUX_H_


int initLinux (TREGISTEREDDRIVERS *rd);
void closeLinux (TREGISTEREDDRIVERS *rd);


int linux_OpenDriver (TPORTDRIVER *pd, int port);
int linux_CloseDriver (TPORTDRIVER *pd);
int linux_WritePort8 (const int addr, const ubyte data);
int linux_WritePort16 (const int addr, const unsigned short data);
int linux_WritePort32 (const int addr, const unsigned int data);
int linux_WriteBuffer (const int port, void *buffer, size_t tbytes);
int linux_Flush (TPORTDRIVER *pd);
ubyte linux_ReadPort (const int addr);


#endif 


