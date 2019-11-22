
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


#ifndef _FRAME_H_
#define _FRAME_H_

#include "framec.h"


void deleteFrame (TFRAME *frm);
void deleteFrames (THWD *hw, const int groupId);
TFRAME *cloneFrame (TFRAME *src);
TFRAME *cloneFrameEx (TFRAME *current, TFRAME *src);

TFRAME *_newFrame (THWD *hw, const int width, const int height, const int gid, const ubyte bpp);
int _resizeFrame (TFRAME *frm, const int width, const int height, const int mode);

int clearFrame (TFRAME *frm);

int calcFrameSize (const int width, const int height, const ubyte bpp);
int calcPitch(const int width, const ubyte bpp);
int clearFrameClr (TFRAME *frm, const int colour);
void *createFrameTree ();
void freeFrameTree (void *plist);
TFRAME *newFrame (THWD *hw, const int width, const int height, ubyte bpp);
int resizeFrame (TFRAME *frm, int width, int height, const int mode);

void freeFrame (TFRAMECACHE *frmc, TFRAME *frm);
void unregisterFrameGroup (THWD *hw, const int gid);

void dumpFrameCacheDetail (void *flist);

#endif

