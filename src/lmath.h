
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

#ifndef _LMATH_H_
#define _LMATH_H_

#include <math.h>

#ifndef M_PI
#define M_PI 3.141592653589793
#endif
/*
INLINE float l_sinf (float n);
INLINE float l_cosf (float n);
INLINE float l_ceilf (float x);
INLINE int l_abs (int n);
INLINE float l_fabsf (float n);
INLINE float l_sqrt (float n);
*/

#define l_sin sin
#define l_cos cos
#define l_sinf sinf
#define l_cosf cosf
#define l_ceilf ceilf
#define l_abs abs
#define l_fabsf fabsf
#define l_fabs fabs
#define l_sqrt sqrt


static INLINE int l_sgn (const int a)
{ 
	if (a > 0)
		return +1; 
	else if (a < 0)
		return -1; 
	else
		return 0; 
}


#endif

