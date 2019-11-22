
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


#ifndef _SCROLL_H_
#define _SCROLL_H_


#ifdef __DEBUG__
typedef struct{
	TLSCROLLEX **list;
	unsigned int total;
}TSCROLLCONTAINER;
#endif

int initScroll (THWD *hw);
void closeScroll (THWD *hw);

int scrCopyToClient (const TLSCROLLEX *scroll);
TLSCROLLEX *newScroll (TFRAME *source, TFRAME *client);
void deleteScroll (TLSCROLLEX *scroll);
int updateScroll (TLSCROLLEX *scroll);

#endif



