
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

#if (__BUILD_CHRDECODE_SUPPORT__)

#include "utf.h"
#include "fonts.h"
#include "lstring.h"
#include "tis620.h"
#include "lmath.h"
#include "utf16.h"
#include "hz.h"
#include "gb18030.h"
#include "gb2313.h"
#include "chardecode.h"


#if (__BUILD_INTERNAL_JISX0213__)
 #include "jis.h"
#endif


#if (__BUILD_INTERNAL_BIG5__)
 #include "big5.h"
 
#else


#define IsBig5Char1(c)	(((c) > 0x80) && ((c) < 0xFF))
#define IsBig5Char2(c)	(((c) > 0x3F) && ((c) < 0xFF))
		
int BIG5ToUTF32 (THWD *hw, const ubyte *buffer, UTF32 *ch)
{
	if (IsBig5Char1(*buffer)){
		UTF32 c2 = *(buffer+1);
		if (IsBig5Char2(c2)){
			*ch = remapChar(hw, (*buffer<<8)|c2);
			return 2;
		}else{
			*ch = '?';
			if (!c2)
				return -1;
			else
				return -2;
		}
	}else{
		*ch = remapChar(hw, *buffer);
		return 1;
	}
}
#endif


int EUCCNToUTF32 (THWD *hw, const ubyte *buffer, UTF32 *wc)
{
	if (buffer[0] < 0x80){
		*wc = remapChar(hw, (UTF32)*buffer);
		return 1;
	}else if ((buffer[0] > 0xA0) && (buffer[0] < 0xFF)){
		if ((buffer[1] > 0xA0) && (buffer[1] < 0xFF)){
			*wc = remapChar(hw, to_gb2321(buffer[0] - 0xA1, buffer[1] - 0xA1));
			return 2;
		}else{
			*wc = '?';
			return 1;
		}
	}
	return -1;
}

/*
Extended Unix Code for Traditional Chinese
An 8-bit encoding form of single-byte ASCII and multi-byte CNS 11643-1992.
It uses two code sets to encode CNS:
Code Set 1 encodes Plane 1 characters in 2-byte sequences
Code Set 2 encodes characters of any plane in 4-byte sequences
This is what each byte stands for in Code Set 2:
     1st = code set: 0x8E
     2nd = plane:    0xA1-0xA7
     3rd = row:      0xA1-0xFE
     4th = cell:     0xA1-0xFE  */
int EUCTWToUTF32 (THWD *hw, const ubyte *buffer, UTF32 *ch)
{
	if (*buffer&0x80){
		if (*buffer == 0x8E){
			*ch = remapChar(hw, 0x8E000000|(*buffer<<16)|(buffer[1]<<8)|buffer[2]);
			return 4;
		}else{
			*ch = remapChar(hw, (*buffer<<8)|buffer[1]);
			return 2;
		}
	}else{
		*ch = remapChar(hw, (UTF32)*buffer);
		return 1;
	}
}

int EUCKRToUTF32 (THWD *hw, const ubyte *buffer, UTF32 *ch)
{
	if (*buffer > 0xA0){
		*ch = remapChar(hw, (*buffer<<8)|buffer[1]);
		return 2;
	}else{
		*ch = remapChar(hw, (UTF32)*buffer);
		return 1;
	}
}

int EUCJPToUTF32 (const ubyte *buffer, UTF32 *ch)
{
	UTF32 out;
	int tchars = eucjp2sjis(buffer, &out);
	if (tchars > 0)
		JISX0213ToUTF32((ubyte*)&out, ch);
	return tchars;
}

int JISX0213ToUTF32 (const ubyte *buffer, UTF32 *ch)
{
#if (__BUILD_INTERNAL_JISX0213__)
	  return shift_jisx0213_mbtowc(ch, buffer);
#else
	  *ch = 0x20;
	  return 1;
#endif
}

int HZToUTF32 (const ubyte *buffer, UTF32 *ch)
{
	int ret = hzgb2312ToUTF32(buffer, ch);
	if (ret == -11){
		if (hzgb2312ToUTF32(buffer+1, ch) == 1)
			return 2;
		else
			return -2;
	}else{
		return ret;
	}
}

int GB18030ToUTF32 (THWD *hw, const ubyte *str, UTF32 *wc)
{
	int chars = GB18030CodeToUTF32(str, wc);
	*wc = remapChar(hw, *wc);
	return chars;
}

int GBKToUTF32 (THWD *hw, const ubyte *str, UTF32 *wc)
{
	if (*str > 0x80){
		if (*str != 0xFF){
			*wc = remapChar(hw, (*str<<8)|str[1]);
			return 2;
		}
	}
	*wc = remapChar(hw, (UTF32)*str);
	return 1;

}

int UTF16ToUTF32 (const ubyte *buffer, UTF32 *ch)
{
	return utf162utf32(buffer, ch);
}

int UTF16BEToUTF32 (const ubyte *buffer, UTF32 *ch)
{
	return utf16be2utf32(buffer, ch);
}

int UTF16LEToUTF32 (const ubyte *buffer, UTF32 *ch)
{
	return utf16le2utf32(buffer, ch);
}

int UTF8ToUTF32 (const UTF8 *buffer, UTF32 *pwc)
{
	
	unsigned int n;// = l_strlen(buffer);
	for (n = 0; n<7; n++){
		if (!buffer[n])
			break;
	}

	UTF8 *s = (UTF8 *)buffer;
	UTF8 c = s[0];
	const UTF32 invalidChar = '?';
	
	if (c < 0x80){
		*pwc = c;
		return 1;
	}else if (c < 0xc2){
		*pwc = invalidChar;
		return 1;

	}else if (c < 0xe0){
		if (n < 2){
			*pwc = invalidChar;
			return 1;
		}
			
		if (!((s[1] ^ 0x80) < 0x40)){
			*pwc = invalidChar;
			if (s[1]&0x80)
				return 2;
			else
				return 1;
		}
			
		*pwc = ((UTF32)(c & 0x1f) << 6) | (UTF32)(s[1] ^ 0x80);
		return 2;
	}else if (c < 0xf0){
		if (n < 3){
			*pwc = invalidChar;
			return 2;
		}
		
		if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40 && (c >= 0xe1 || s[1] >= 0xa0))){
			*pwc = invalidChar;
			if (s[1]&0x80){
				if (s[2]&0x80)
					return 3;
				else
					return 2;
			}else{
				return 1;
			}
		}

		*pwc = ((UTF32)(c & 0x0f) << 12) | ((UTF32)(s[1] ^ 0x80) << 6) | (UTF32)(s[2] ^ 0x80);
		return 3;
	}else if (c < 0xf8){
		if (n < 4){
			*pwc = invalidChar;
			return 3;
		}
			
		if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40 && (s[3] ^ 0x80) < 0x40 && (c >= 0xf1 || s[1] >= 0x90))){
			*pwc = invalidChar;
			if (s[1]&0x80){
				if (s[2]&0x80){
					if (s[3]&0x80)
						return 4;
					else
						return 3;
				}else{
					return 2;
				}
			}else{
				return 1;
			}
		}
			
		*pwc = ((UTF32)(c & 0x07) << 18) | ((UTF32)(s[1] ^ 0x80) << 12) | ((UTF32)(s[2] ^ 0x80) << 6) | (UTF32)(s[3] ^ 0x80);
		return 4;
	}else if (c < 0xfc){
		if (n < 5){
			*pwc = invalidChar;
			return 4;
		}
			
		if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40 && (s[3] ^ 0x80) < 0x40 && (s[4] ^ 0x80) < 0x40 && (c >= 0xf9 || s[1] >= 0x88))){
			*pwc = invalidChar;
			if (s[1]&0x80){
				if (s[2]&0x80){
					if (s[3]&0x80){
						if (s[4]&0x80){
							return 5;
						}else{
							return 4;
						}
					}else{
						return 3;
					}
				}else{
					return 2;
				}
			}else{
				return 1;
			}
		}
		
		*pwc = ((UTF32)(c & 0x03) << 24) | ((UTF32)(s[1] ^ 0x80) << 18) | ((UTF32)(s[2] ^ 0x80) << 12) | ((UTF32)(s[3] ^ 0x80) << 6) | (UTF32)(s[4] ^ 0x80);
		return 5;
	}else if (c < 0xfe){
		if (n < 6){
			*pwc = invalidChar;
			return 5;
		}
		
		if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40 && (s[3] ^ 0x80) < 0x40 && (s[4] ^ 0x80) < 0x40 && (s[5] ^ 0x80) < 0x40 && (c >= 0xfd || s[1] >= 0x84))){
			*pwc = invalidChar;
			if (s[1]&0x80){
				if (s[2]&0x80){
					if (s[3]&0x80){
						if (s[4]&0x80){
							if (s[5]&0x80){
								return 6;
							}else{
								return 5;
							}
						}else{
							return 4;
						}
					}else{
						return 3;
					}
				}else{
					return 2;
				}
			}else{
				return 1;
			}
		}
		
		*pwc = ((UTF32)(c & 0x01) << 30) | ((UTF32)(s[1] ^ 0x80) << 24) | ((UTF32)(s[2] ^ 0x80) << 18) | ((UTF32)(s[3] ^ 0x80) << 12) | ((UTF32)(s[4] ^ 0x80) << 6) | (UTF32)(s[5] ^ 0x80);
		return 6;
	}else{
		*pwc = invalidChar;
		return 1;
	}
}


#endif


