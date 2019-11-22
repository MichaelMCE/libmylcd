
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


#ifndef _NULLDISPLAY_H_
#define _NULLDISPLAY_H_


#define quadBoxW 8
#define quadBoxH 8

int initNullDisplay (TREGISTEREDDRIVERS *rd);
void closeNullDisplay (TREGISTEREDDRIVERS *rd);


int nullDisplay_OpenDisplay (TDRIVER *drv);
int nullDisplay_CloseDisplay (TDRIVER *drv);
int nullDisplay_ClearDisplay (TDRIVER *drv);
int nullDisplay_Refresh (TDRIVER *drv, TFRAME *frm);
int nullDisplay_RefreshArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);

void initiateNullDisplay (TDRIVER *drv);
void shutdownNullDisplay (TDRIVER *drv);

int nullDisplay_updateAreaFlippedH (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);
int nullDisplay_updateArea (TDRIVER *drv, TFRAME *frm, int x1, int y1, int x2, int y2);



#endif 


