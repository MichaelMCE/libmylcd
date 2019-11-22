
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


#ifndef _OPENPARPORT_H_
#define _OPENPARPORT_H_


int initOPP (TREGISTEREDDRIVERS *rd);
void closeOPP (TREGISTEREDDRIVERS *rd);

int opp_OpenDriver (TPORTDRIVER *pd, int port);
int opp_CloseDriver (TPORTDRIVER *pd);
int opp_WritePort8 (const int addr, const ubyte data);
int opp_WritePort16 (const int addr, const unsigned short data);
int opp_WritePort32 (const int addr, const unsigned int data);
int opp_WriteBuffer (const int port, void *buffer, size_t tbytes);
int opp_Flush (TPORTDRIVER *pd);
ubyte opp_ReadPort (const int addr);


#endif
