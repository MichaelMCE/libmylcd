
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


#define UNI_SUR_HIGH_START  (UTF32)0xD800
#define UNI_SUR_HIGH_END    (UTF32)0xDBFF
#define UNI_SUR_LOW_START   (UTF32)0xDC00
#define UNI_SUR_LOW_END     (UTF32)0xDFFF

static int _UTF16ToUTF32 (const UTF16** sourceStart, const UTF16* sourceEnd, UTF32** targetStart, UTF32* targetEnd);

static int utf16_state = 0;



static INLINE UTF16 reverse16 (UTF16 a)
{
	UTF16 b = (a&0xFF00)>>8;
	return (b|(a&0x00FF)<<8);
}

// auto endian
int utf162utf32 (const ubyte *buffer, UTF32 *ch)
{
	if (*(UTF16*)buffer == 0xFEFF){
		utf16_state = 0;
		return -1;
	}else if (*(UTF16*)buffer == 0xFFFE){
		utf16_state = 1;
		return -1;
	}
	
	UTF16 str[4];
	if (!utf16_state){
		str[0] = *(UTF16 *)buffer;
		str[1] = *(UTF16 *)(buffer+2);
	}else{
		str[0] = reverse16(*(UTF16 *)buffer);
		str[1] = reverse16(*(UTF16 *)(buffer+2));
	}
	str[2] = 0;
	str[3] = 0;

	// check for utf16 line/paragraph separators
	// replace with \n
	if (str[0] == 0x2029 || str[0] == 0x2028 || str[0] == 0x0085){
		*ch = '\n';
		return 1;
	}

	UTF16 *_str = (UTF16 *)&str;
	UTF32 wc[2] = {0, 0};
	UTF32 *_wc = (UTF32 *)&wc;

	if (!_UTF16ToUTF32((const UTF16 **)&_str, (const UTF16 *)&_str[1], (UTF32**)&_wc, (UTF32*)&_wc[1])){
		*ch = wc[0];
		return 1;
	}else{
		*ch = 0;
		return 0;
	}
}

int utf16be2utf32 (const ubyte *buffer, UTF32 *ch)
{
	if (*(UTF16*)buffer == 0xFFFE)
		return -1;

	UTF16 str[4];
	str[0] = reverse16(*(UTF16 *)buffer);
	str[1] = reverse16(*(UTF16 *)(buffer+2));
	str[2] = 0;
	str[3] = 0;

	// check for utf16 line/paragraph separators
	// replace with \n
	if (str[0] == 0x2029 || str[0] == 0x2028 || str[0] == 0x0085){
		*ch = '\n';
		return 1;
	}

	UTF16 *_str = (UTF16 *)&str;
	UTF32 wc[2] = {0, 0};
	UTF32 *_wc = (UTF32 *)&wc;

	if (!_UTF16ToUTF32((const UTF16 **)&_str, (const UTF16 *)&_str[1], (UTF32**)&_wc, (UTF32*)&_wc[1])){
		*ch = wc[0];
		return 1;
	}else{
		*ch = 0;
		return 0;
	}
}

int utf16le2utf32 (const ubyte *buffer, UTF32 *ch)
{
	if (*(UTF16*)buffer == 0xFEFF)
		return -1;

	UTF16 str[4];
	str[0] = *(UTF16 *)buffer;
	str[1] = *(UTF16 *)(buffer+2);
	str[2] = 0;
	str[3] = 0;

	// check for utf16 line/paragraph separators
	// replace with \n
	if (str[0] == 0x2029 || str[0] == 0x2028 || str[0] == 0x0085){
		*ch = '\n';
		return 1;
	}

	UTF16 *_str = (UTF16 *)&str;
	UTF32 wc[2] = {0, 0};
	UTF32 *_wc = (UTF32 *)&wc;

	if (!_UTF16ToUTF32((const UTF16 **)&_str, (const UTF16 *)&_str[1], (UTF32**)&_wc, (UTF32*)&_wc[1])){
		*ch = wc[0];
		return 1;
	}else{
		*ch = 0;
		return 0;
	}
}

static int _UTF16ToUTF32 (const UTF16** sourceStart, const UTF16* sourceEnd, UTF32** targetStart, UTF32* targetEnd)
{

	//const int flags = 1;
    int result = 0;
    const UTF16* source = *sourceStart;
    UTF32* target = *targetStart;
    UTF32 ch, ch2;

    while (source < sourceEnd){
    	
		const UTF16* oldSource = source; /*  In case we have to back up because of target overflow. */
		ch = *source++;
		
		/* If we have a surrogate pair, convert to UTF32 first. */
		if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END){
	    	/* If the 16 bits following the high surrogate are in the source buffer... */
	    	if (source < sourceEnd){
				ch2 = *source;
				/* If it's a low surrogate, convert to UTF32. */
				if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END){
		    		ch = ((ch - UNI_SUR_HIGH_START) << 10)
					+ (ch2 - UNI_SUR_LOW_START) + 0x0010000UL;
		    		++source;
		    	#if 0
				}else if (flags == 0){ /* it's an unpaired high surrogate */
		    		--source; /* return to the illegal value itself */
		    		result = 1;
		    		break;
		    	#endif
				}
	    	}else{ /* We don't have the 16 bits following the high surrogate. */
				--source; /* return to the high surrogate */
				result = 2;
				break;
	    	}
#if 0
		}else if (flags == 0){
	    	/* UTF-16 surrogate values are illegal in UTF-32 */
	    	if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END){
				--source; /* return to the illegal value itself */
				result = 1;
				break;
	    	}
#endif
		}
		
	
		if (target >= targetEnd){
	    	source = oldSource; /* Back up source pointer! */
	    	result = 2; break;
		}
		*target++ = ch;
    }

    *sourceStart = source;
    *targetStart = target;
    return result;
}
