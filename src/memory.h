
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


#ifndef _MEMORY_H_
#define _MEMORY_H_



int initMemory();
void closeMemory();



#if (__DEBUG_MEM__)

#define funcname		__func__
#define linenumber		(__LINE__)

#define l_malloc(n)		my_malloc(n, funcname, linenumber)
#define l_calloc(n, e)	my_calloc(n, e, funcname, linenumber)
#define l_realloc(p, n)	my_realloc(p, n, funcname, linenumber)
#define l_free(p)		my_free(p, funcname, linenumber)

void *my_malloc (size_t size, const char *func, const int line)
	__attribute__((malloc));

void *my_calloc (size_t nelem, size_t elsize, const char *func, const int line)
	__attribute__((malloc));
void *my_realloc (void *ptr, size_t size, const char *func, const int line)
	__attribute__((malloc));

void my_free (void *ptr, const char *func, const int line);

char *my_strdup (const char *str, const char *func, const int line)
	__attribute__((nonnull(1)));

wchar_t * my_wcsdup (const wchar_t *str, const char *func, const int line)
	__attribute__((nonnull(1)));

void *mmx_memcpy_dbg (void *s1, const void *s2, size_t n);
void *memcpy_dbg (void *s1, const void *s2, size_t n);


#else

#define l_malloc(n)		my_malloc(n)
#define l_calloc(n, e)	my_calloc(n, e)
#define l_realloc(p, n)	my_realloc(p, n)
#define l_free(p)		my_free(p)

void *my_malloc (size_t size)
	__attribute__((malloc));

void *my_calloc (size_t nelem, size_t elsize)
	__attribute__((malloc));
void *my_realloc (void *ptr, size_t size)
	__attribute__((malloc));

void my_free (void *ptr);

char *my_strdup (const char *str)
	__attribute__((nonnull(1)));

wchar_t * my_wcsdup (const wchar_t *str)
	__attribute__((nonnull(1)));
	

#endif


void *l_memset (void *s, int c, size_t n)
	__attribute__((nonnull(1)));
	
int l_memcmp (void *s1, const void *s2, size_t count)
	__attribute__((nonnull(1, 2)));
	
void dumpMemStats (THWD *hw);

#ifdef USE_MMX

# ifndef __DEBUG_MEM__
# define l_memcpy mmx_memcpy
# else
# define l_memcpy mmx_memcpy_dbg
# endif

#else

# ifndef __DEBUG_MEM__
# define l_memcpy memcpy
# else
# define l_memcpy memcpy_dbg
# endif

#endif


#define MIN_LEN			(64)  /* 64-byte blocks */
#define MMX1_MIN_LEN	(64)

#ifdef USE_MMX
#define _mmx_memcpy(to,from,len) \
{\
  if (len >= MMX1_MIN_LEN)\
  {\
    size_t __i = len >> 6;\
    len &= 63;\
    for(; __i>0; __i--)\
    {\
      __asm__ __volatile__ (\
      "movq (%0), %%mm0\n"\
      "movq 8(%0), %%mm1\n"\
      "movq 16(%0), %%mm2\n"\
      "movq 24(%0), %%mm3\n"\
      "movq 32(%0), %%mm4\n"\
      "movq 40(%0), %%mm5\n"\
      "movq 48(%0), %%mm6\n"\
      "movq 56(%0), %%mm7\n"\
      "movq %%mm0, (%1)\n"\
      "movq %%mm1, 8(%1)\n"\
      "movq %%mm2, 16(%1)\n"\
      "movq %%mm3, 24(%1)\n"\
      "movq %%mm4, 32(%1)\n"\
      "movq %%mm5, 40(%1)\n"\
      "movq %%mm6, 48(%1)\n"\
      "movq %%mm7, 56(%1)\n"\
      :: "r" (from), "r" (to) : "memory");\
      from += 64;\
      to += 64;\
    }\
    __asm__ __volatile__ ("emms":::"memory");\
  }\
  if (len) memcpy(to, from, len);\
}
#endif

#ifdef USE_MMX2
#define _mmx2_memcpy(to,from,len) \
{\
	__asm__ __volatile__ (\
    "   prefetch (%0)\n"\
    "   prefetch 64(%0)\n"\
    : : "r" (from) );\
	if (len >= MIN_LEN){\
        size_t __i = len >> 6;\
    len &= 63;\
    for(; __i>0; __i--){\
      __asm__ __volatile__ (\
      "prefetch 64(%0)\n"\
      "movq (%0), %%mm0\n"\
      "movq 8(%0), %%mm1\n"\
      "movq 16(%0), %%mm2\n"\
      "movq 24(%0), %%mm3\n"\
      "movq 32(%0), %%mm4\n"\
      "movq 40(%0), %%mm5\n"\
      "movq 48(%0), %%mm6\n"\
      "movq 56(%0), %%mm7\n"\
      "movntq %%mm0, (%1)\n"\
      "movntq %%mm1, 8(%1)\n"\
      "movntq %%mm2, 16(%1)\n"\
      "movntq %%mm3, 24(%1)\n"\
      "movntq %%mm4, 32(%1)\n"\
      "movntq %%mm5, 40(%1)\n"\
      "movntq %%mm6, 48(%1)\n"\
      "movntq %%mm7, 56(%1)\n"\
      :: "r" (from), "r" (to) : "memory");\
      from += 64;\
      to += 64;\
    }\
    __asm__ __volatile__ ("sfence":::"memory");\
    __asm__ __volatile__ ("emms":::"memory");\
  }\
  if (len) memcpy(to, from, len);\
}

#define mmx_memcpy(_to ,_from, _len) \
{\
  	unsigned char * restrict __to = (unsigned char * restrict)(_to);\
  	unsigned char * restrict __from = (unsigned char * restrict)(_from);\
	size_t __len = (size_t)(_len);\
 	if (__len < 128){\
		if (__len&3){\
			for (int i = 0; i < __len; i++)\
				__to[i] = __from[i];\
		}else{\
			unsigned int *restrict __src = (unsigned int *restrict)__from;\
			unsigned int *restrict __des = (unsigned int *restrict)__to;\
			__len >>= 2;\
			for (int i = 0; i < __len; i++)\
				__des[i] = __src[i];\
		}\
  	}else if (__len < 301184){\
		_mmx_memcpy(__to,__from,__len);\
	}else{\
		_mmx2_memcpy(__to,__from,__len);\
	}\
}
#else
#define mmx_memcpy(_to ,_from, _len) \
{\
  	unsigned char * restrict __to = (unsigned char * restrict)(_to);\
  	unsigned char * restrict __from = (unsigned char * restrict)(_from);\
	size_t __len = (size_t)(_len);\
 	if (__len < 128){\
		if (__len&3){\
			for (int i = 0; i < __len; i++)\
				__to[i] = __from[i];\
		}else{\
			unsigned int *restrict __src = (unsigned int *restrict)__from;\
			unsigned int *restrict __des = (unsigned int *restrict)__to;\
			__len >>= 2;\
			for (int i = 0; i < __len; i++)\
				__des[i] = __src[i];\
		}\
	}else{\
		_mmx_memcpy(__to,__from,__len);\
	}\
}
#endif

#endif

