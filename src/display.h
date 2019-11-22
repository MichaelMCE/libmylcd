
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


#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "sync.h"

typedef struct{
	TTHRDSYNCCTRL tsc;
	TTHRDSYNCCTRL exitLock;
	struct TFRAME *frameAsync1;
	struct TFRAME *frameAsync2;
	volatile int thrdState;
}TASFCONTROL;

int initDisplay (THWD *hw);
void closeDisplay (THWD *hw);
void removeAsyncControl (TASFCONTROL *ctrl);


int clearDisplay (THWD *hw);
int update (const THWD *hw, const void *buffer, const size_t bufferLen);
int refreshFrame (TFRAME *frm);
int refreshFrameArea (TFRAME *frm, int left, int top, int right, int btm);
int refreshAsync (TFRAME *frm, int swap);
int setDisplayOption (THWD *hw, lDISPLAY did, int option, intptr_t *value);
int getDisplayOption (THWD *hw, lDISPLAY did, int option, intptr_t *value);
int pauseDisplay (THWD *hw, lDISPLAY did);
int resumeDisplay (THWD *hw, lDISPLAY did);


#endif 




