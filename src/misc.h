
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


#ifndef _MISC_H_
#define _MISC_H_


int initDebugPrint ();
void closeDebugPrint ();

int getBackgroundColour (THWD *hw);
int setBackgroundColour (THWD *hw, const int colour);
int getForegroundColour (THWD *hw);
int setForegroundColour (THWD *hw, const int colour);
int getFilterAttribute (THWD *hw, const int filter, const int attribute);
int setFilterAttribute (THWD *hw, const int filter, const int attribute, const int value);

void l_sleep (const int time);
unsigned int getTicks ();
double getTicksD ();

int my_printf (const char *str, ...);
int my_wprintf (const wchar_t *wstr, ...);

#endif

