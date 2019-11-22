
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


#include <string.h>

#include "mylcd.h"
#include "memory.h"
#include "lstring.h"
#include "lmath.h"




int l_vaswprintf (wchar_t **result, const wchar_t *format, va_list *args)
{
	
  const wchar_t *p = format;
  /* Add one to make sure that it is never zero, which might cause malloc
     to return NULL.  */
  int total_width = l_wcslen(format) + sizeof(wchar_t);
  va_list ap;

  l_memcpy(&ap, args, sizeof(va_list));

  while (*p != L'\0')
    {
      if (*p++ == L'%')
        {
          while (l_wcschr(L"-+ #0", *p))
            ++p;
          if (*p == L'*')
            {
              ++p;
              total_width += l_abs(va_arg(ap, int));
            }
          else
            total_width += l_wcstoul(p, (wchar_t**)&p, 10);
          if (*p == L'.')
            {
              ++p;
              if (*p == L'*')
                {
                  ++p;
                  total_width += l_abs(va_arg(ap, int));
                }
              else
                total_width += l_wcstoul(p, (wchar_t**)&p, 10);
            }
          while(l_wcschr(L"hlL", *p))
            ++p;
          /* Should be big enough for any format specifier except %s.  */
          total_width += 30;
          switch (*p)
            {
            case L'd':
            case L'i':
            case L'o':
            case L'u':
            case L'x':
            case L'X':
            case L'c':
              (void) va_arg(ap, int);
              break;
            case L'f':
            case L'e':
            case L'E':
            case L'g':
            case L'G':
              (void) va_arg(ap, double);
              break;
            case L's':
              total_width += l_wcslen(va_arg(ap, wchar_t *));
              break;
            case L'p':
            case L'n':
              (void) va_arg(ap, wchar_t *);
              break;
            }
        }
    }

  if (!total_width) return 0;
  *result = l_calloc(sizeof(wchar_t), 1+total_width);
  if (*result != NULL)
    return l_vsnwprintf(*result, total_width, format, *args);
  else
    return 0;
}


int l_vasprintf (char **result, const char *format, va_list *args)
{
	
  const char *p = format;
  int total_width = l_strlen(format) + sizeof(char);
  va_list ap;

  l_memcpy(&ap, args, sizeof(va_list));

  while (*p != '\0')
    {
      if (*p++ == '%')
        {
          while (l_strchr("-+ #0", *p))
            ++p;
          if (*p == '*')
            {
              ++p;
              total_width += l_abs(va_arg(ap, int));
            }
          else
            total_width += l_strtoul(p, (char**)&p, 10);
          if (*p == '.')
            {
              ++p;
              if (*p == '*')
                {
                  ++p;
                  total_width += l_abs(va_arg(ap, int));
                }
              else
                total_width += l_strtoul(p, (char**)&p, 10);
            }
          while (l_strchr ("hlL", *p))
            ++p;
          /* Should be big enough for any format specifier except %s.  */
          total_width += 30;
          switch (*p)
            {
            case 'd':
            case 'i':
            case 'o':
            case 'u':
            case 'x':
            case 'X':
            case 'c':
              (void)va_arg(ap, int);
              break;
            case 'f':
            case 'e':
            case 'E':
            case 'g':
            case 'G':
              (void)va_arg(ap, double);
              break;
            case 's':
              total_width += l_strlen(va_arg(ap, char *));
              break;
            case 'p':
            case 'n':
              (void)va_arg(ap, char *);
              break;
            }
        }
    }
  if (!total_width) return 0;
	
  *result = l_calloc(sizeof(char), 1+total_width);
  if (*result != NULL)
    return l_vsnprintf(*result, total_width, format, *args);
  else
    return 0;
}


/*int l_vsnprintf (char *buffer, const int count, const char *format, va_list argptr)
{
	return vsnprintf(buffer, count, format, argptr);
}

int l_vsnwprintf (wchar_t *buffer, const int count, const wchar_t *format, va_list argptr)
{
	return vsnwprintf(buffer, count, format, argptr);
}*/

char *l_stristr (const char *in, const char *find)
{
	const unsigned int inlen = l_strlen(in);
	const unsigned int findlen = l_strlen(find);
	
	if ((findlen > inlen) || !inlen || !findlen)
		return NULL;
	
	char *tmpin = (char*)l_malloc(inlen+1);
	char *tmpfind = (char*)l_malloc(findlen+1);
	if ((tmpfind == NULL) || (tmpin == NULL))
		return NULL;

	l_strtolower(tmpin, in, inlen);
	tmpin[inlen] = 0;
	l_strtolower(tmpfind, find, findlen);
	tmpfind[findlen] = 0;
	char *result = l_strstr(tmpin, tmpfind);
	l_free(tmpin);
	l_free(tmpfind);
	return result;
}


/*
void l_strtolower (char *dest, const char *src, unsigned int srclen)
{
	while(srclen--){
		*dest++ = tolower((int)*src++);
	}
}
*/

/*char *l_strstr (const char *s1, const char *s2)
{
	return strstr((char *)s1, (char *)s2);
}

char *l_strtok (char *str1, const char *str2)
{
	return strtok (str1, str2);
}

int l_strncasecmp (const char *s1, const char *s2, size_t n)
{
	return _strnicmp(s1, s2, n);
}

int l_strcasecmp (const char *s1, const char *s2)
{
	return _stricmp(s1, s2);
}*/



//size_t l_wcslen (const wchar_t *s)
//{
//	return wcslen(s);
//}

/*int l_atoi (const char *s)
{
	return atoi(s);
}

size_t l_strlen (const char *s)
{
	return strlen(s);
}

int l_wcsncmp (const wchar_t *s1, const wchar_t *s2, size_t n)
{
	return wcsncmp(s1, s2, n);
}

int l_strncmp (const char *s1, const char *s2, size_t n)
{
	return strncmp(s1, s2, n);
}

int l_strcmp (const char *s1, const char *s2)
{
	return strcmp(s1, s2);
}
*/



//wchar_t *l_wcsncpy (wchar_t *dest, const wchar_t *src, size_t n)
//{
//	return wcsncpy(dest,src,n);
//}

//wchar_t *l_wcscpy (wchar_t *dest, const wchar_t *src)
//{
//	return wcscpy(dest, src);
//}

//char *l_strncpy (char *dest, const char *src, size_t n)
//{
//	return strncpy(dest, src, n);
//}

//char *l_strcpy (char *dest, const char *src)
//{
//	return strcpy(dest, src);
	
	/*
	int d0, d1, d2;
	__asm__ __volatile__("1:\tlodsb\n\t"
					   "stosb\n\t"
					   "testb %%al,%%al\n\t"
					   "jne 1b"
					 : "=&S" (d0), "=&D" (d1), "=&a" (d2)
                     : "0" (src),"1" (dest) 
                     : "memory");
	return (ubyte *)dest;
	*/
//}

int l_wcatoi2 (const wchar_t *str, int *value)
{
	
	if (!str||!value) return 0;
	int c = 0;
	*value = 0;
	
	while (*str) {
		if ((*str >= L'0') && (*str <= L'9')){
			*value *= 10;
			*value += (*str-L'0');
			c++;
		}else{
			return c;
		}
		str++;
	}
	return c;
}

int l_atoi2 (const char *str, int *value)
{
	if (!str || !value) return 0;

	int c = 0;
	*value = 0;
	
	while (*str) {
		if ((*str>47)&&(*str<58)){
			*value *= 10;
			*value += (*str-'0');
			c++;
		}else{
			return c;
		}
		str++;
	}
	return c;
}

/*unsigned long l_wcstoul (const wchar_t *a, wchar_t **b, int c)
{
	return wcstoul(a, b, c);
}

unsigned long l_strtoul (const char *a, char **b, int c)
{
	return strtoul(a, b, c);
}

char *l_strchr (const char *s, int c)
{
	return strchr(s, c);
}

wchar_t *l_wcschr (const wchar_t *s, int c)
{
	return wcschr(s, c);
}

char *l_fgets (char *s, int n, const FILE *stream)
{
	return fgets(s, n,(FILE*)stream);
}
*/
