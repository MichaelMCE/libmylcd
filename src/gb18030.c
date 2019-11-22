
// libmylcd - http://mylcd.sourceforge.net/
// An LCD framebuffer library
// Michael McElligott
// okio@users.sourceforge.net

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



#include "mylcd.h"


/*
GB18030-2000 encodes characters in sequences of one, two, or four bytes.
The following are valid byte sequences:
Single-byte: 0x00-0x7f 
Two-byte: 0x81-0xfe + 0x40-0x7e, 0x80-0xfe 
Four-byte: 0x81-0xfe + 0x30-0x39 + 0x81-0xfe + 0x30-0x39
*/
int GB18030CodeToUTF32 (const ubyte *str, UTF32 *wc)
{
	ubyte c1 = *str++;

	if (c1 >= 0x81 && c1 <= 0xFE){
		ubyte *p = (ubyte *)wc;
		ubyte c2 = (ubyte)*str++;
		
		if ((c2 >= 0x40 && c2 <= 0x7E) || (c2 >= 0x80 && c2 <= 0xFE)){
			*p++ = c2;
			*p = c1;
			return 2;
		}else if (c2 >= 0x30 && c2 <= 0x39){
			int c3 = (int)*str++;
			int c4 = (int)*str;
			if (c3 >= 0x81 && c3 <= 0xFE){
				if (c4 >= 0x30 && c4 <= 0x39){
					*p++ = c4;
					*p++ = c3;
					*p++ = c2;
					*p = c1;
					return 4;
				}else{
					return -3;
				}
			}else{
				return -2;
			}
		}else{
			return -1;
		}

	}else{
		*wc = (UTF32)c1;
		return 1;
	}
}
