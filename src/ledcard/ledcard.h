
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


#ifndef _LEDCARD_H_
#define _LEDCARD_H_

#define LEDCARD_BUFFER_WIDTH	4096
#define LEDCARD_BUFFER_HEIGHT	12
#define LEDCARD_DISPLAY_WIDTH	40
#define LEDCARD_DISPLAY_HEIGHT	12

int initLEDCARD (TREGISTEREDDRIVERS *rd);
void closeLEDCARD (TREGISTEREDDRIVERS *rd);



#endif



