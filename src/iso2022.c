
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



#include "mylcd.h"

#if (__BUILD_CHRDECODE_SUPPORT__)

#include <string.h>
#include "chardecode.h"
#include "fonts.h"


#define NL          10
#define FF          12
#define CR          13
#define SO          14
#define SI          15
#define ESC         27
#define TRUE        1
#define FALSE       0 




// decodes ISO_2022_KR only
int ISO2022KRToUTF32 (THWD *hw, ubyte *inbuffer, UTF32 *outbuffer)
{
	// ISO2022 Shift State
	static int shiftOut_kr = 0;

	ubyte p1,p2;
	int tchars = 0;
	ubyte *in = inbuffer;
	UTF32 *out = outbuffer;
	UTF32 ch;
	int chars = 0;

	while ((p1 = *in)){
		if (!p1){
			return 0; //(int)in-(int)inbuffer;
		}else if (p1 == NL){
			in++;
			*out = NL;
			shiftOut_kr = FALSE;
			return (intptr_t)in-(intptr_t)inbuffer;
		}else if (p1 == SO){
			in++;
			shiftOut_kr = TRUE;
		}else if (p1 == SI){
			in++;
			shiftOut_kr = FALSE;
		}else if (p1 == ESC){		//	$)C
			if (in[1] == '$')
				in += 4;
		}else if (p1 < 33){
			in++;
			*out = p1;
			return (intptr_t)in-(intptr_t)inbuffer;
		}else if (p1 == FF){
			in++;
			shiftOut_kr = FALSE;
			return (intptr_t)in-(intptr_t)inbuffer;
		}else if (p1 == CR){
			in++;
			shiftOut_kr = FALSE;
			return (intptr_t)in-(intptr_t)inbuffer;
		}else{
			chars = isCharRef(hw, (char*)in, &ch);
			if (chars){
				in += chars;
				p1 = (ubyte)ch&0xFF;
			}else{
				in++;
			}

			if (!shiftOut_kr){
				*out = p1;
			}else{
				p2 = *in;
				chars = isCharRef(hw, (char*)in, &ch);
				if (chars){
					in += chars;
					p2 = (ubyte)ch&0xFF;
				}else{
					in++;
				}
				*out = remapChar(hw, (p1<<8)|p2);
			}
			return (intptr_t)in-(intptr_t)inbuffer;
		}
	}
	return tchars;
}


#endif
