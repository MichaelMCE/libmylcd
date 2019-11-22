
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


#ifndef _NULLPORT_H_
#define _NULLPORT_H_



int initNullPort (TREGISTEREDDRIVERS *rd);
void closeNullPort (TREGISTEREDDRIVERS *rd);


int nullPort_OpenDriver (TPORTDRIVER *pd, int port);
int nullPort_CloseDriver (TPORTDRIVER *pd);
int nullPort_WritePort8 (const int port, const ubyte data);
int nullPort_WritePort16 (const int port, const unsigned short data);
int nullPort_WritePort32 (const int port, const unsigned int data);
int nullPort_WriteBuffer (const int port, void *buffer, size_t tbytes);
int nullPort_Flush (TPORTDRIVER *pd);
ubyte nullPort_ReadPort (const int port);



#endif

