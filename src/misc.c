
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



#ifdef __WIN32__
#include <windows.h>
#include <string.h>
#else
#include <time.h>
#endif

#include "mylcd.h"
#include "lstring.h"
#include "sync.h"
#include "misc.h"


#ifdef __WIN32__
static TTHRDSYNCCTRL dbsync;
#endif


int initDebugPrint ()
{
	lock_create(&dbsync);
	return 1;
}

void closeDebugPrint ()
{
	lock(&dbsync);
	lock_delete(&dbsync);
}


int my_printf (const char *str, ...)
{
	int ret = -1;
#ifdef __WIN32__
	if (lock(&dbsync)){
		VA_OPEN(ap, str);
		ret = vprintf(str, ap);
		VA_CLOSE(ap);
		unlock(&dbsync);
	}
	
#else
	VA_OPEN(ap, str);
	ret = vprintf(str, ap);
	VA_CLOSE(ap);
#endif

	return ret;
}

int my_wprintf (const wchar_t *wstr, ...)
{
	int ret = -1;
#ifdef __WIN32__
	if (lock(&dbsync)){
		VA_OPEN(ap, wstr);
		ret = vwprintf(wstr, ap);
		VA_CLOSE(ap);
		unlock(&dbsync);
	}
	
#else
	VA_OPEN(ap, wstr);
	ret = vwprintf(wstr, ap);
	VA_CLOSE(ap);
#endif
	return ret;
}


int setForegroundColour (THWD *hw, const int colour)
{
	if (hw){
		if (hw->render){
			const int current = hw->render->foreGround;
			hw->render->foreGround = colour;
			return current;
		}
	}
	return 0;
}

int setBackgroundColour (THWD *hw, const int colour)
{
	if (hw){
		if (hw->render){
			const int current = hw->render->backGround;
			hw->render->backGround = colour;
			return current;
		}
	}
	return 0;
}

int setFilterAttribute (THWD *hw, const int filter, const int attribute, const int value)
{
	if (hw){
		if (hw->render){
			if (hw->render->loc){
				const int current = hw->render->loc->attributes[filter][attribute];
				hw->render->loc->attributes[filter][attribute] = value;
				return current;
			}
		}
	}
	return 0;
}

int getFilterAttribute (THWD *hw, const int filter, const int attribute)
{
	if (hw){
		if (hw->render){
			if (hw->render->loc)
				return hw->render->loc->attributes[filter][attribute];
		}
	}
	return 0;
}

int getForegroundColour (THWD *hw)
{
	if (hw){
		if (hw->render)
			return hw->render->foreGround;
	}
	return 0;
}

int getBackgroundColour (THWD *hw)
{
	if (hw){
		if (hw->render)
			return hw->render->backGround;
	}
	return 0;
}

void l_sleep (const int time)
{
#if (defined(__WIN32__))
	  Sleep(time);
#elif (defined(__LINUX__))
	  usleep(time*1000);
#endif
}

double getTicksD ()
{
#ifdef __WIN32__

	static double resolution;
	static uint64_t freq;
	static uint64_t tStart;
	
	if (!tStart){
		QueryPerformanceCounter((LARGE_INTEGER*)&tStart);
		QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
		resolution = 1.0 / (double)freq;
	}
	uint64_t t1 = tStart;
	QueryPerformanceCounter((LARGE_INTEGER*)&t1);
	return ((double)((uint64_t)(t1 - tStart) * resolution) * 1000.0);
	
#else
	return clock();
#endif
}

unsigned int getTicks ()
{
#ifdef __WIN32__

	static float resolution;
	static uint64_t freq;
	static uint64_t tStart;
	
	if (!tStart){
		QueryPerformanceCounter((LARGE_INTEGER*)&tStart);
		QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
		resolution = 1.0 / (float)freq;
	}
	uint64_t t1 = tStart;
	QueryPerformanceCounter((LARGE_INTEGER*)&t1);
	return ((float)((uint64_t)(t1 - tStart) * resolution) * 1000.0);
	
#else
	return clock();
#endif
}