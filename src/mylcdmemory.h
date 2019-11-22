
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


#ifndef _MYLCDMEMORY_H_
#define _MYLCDMEMORY_H_

/*
memory debugging helper methods
#include this time then call the methods as you would normally do so but prepend l_ to the func name
eg; free(ptr) becomes l_free(ptr)
*/


#if 0
#define funcname __func__
#else
#define funcname ""
#endif

#define l_malloc(n)		my_Malloc(n, funcname)
#define l_calloc(n, e)	my_Calloc(n, e, funcname)
#define l_realloc(p, n)	my_Realloc(p, n, funcname)
#define l_free(p)		my_Free(p, funcname)
#define l_strdup(s)		my_Strdup(s, funcname)
#define l_wcsdup(s)		my_Wcsdup(s, funcname)

MYLCD_EXPORT void *my_Malloc (size_t size, const char *func);
MYLCD_EXPORT void *my_Calloc (size_t nelem, size_t elsize, const char *func);
MYLCD_EXPORT void *my_Realloc (void *ptr, size_t size, const char *func);
MYLCD_EXPORT void *my_Free (void *ptr, const char *func);
MYLCD_EXPORT char *my_Strdup (const char *str, const char *func);
MYLCD_EXPORT wchar_t *my_Wcsdup (const wchar_t *str, const char *func);
MYLCD_EXPORT void *my_Memcpy (void *s1, const void *s2, size_t n);

#endif

