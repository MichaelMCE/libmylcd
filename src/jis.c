
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

#if (__BUILD_INTERNAL_JISX0213__)

#include "fonts.h"
#include "utf.h"
#include "chardecode.h"


#define NOT_SET     0
#define SET         1
#define NEW         1
#define OLD         2
#define NEC         3
#define EUC         4
#define SJIS        5

//#define NEW_KI      "$B"
//#define OLD_KI      "$@"
//#define NEC_KI      "K"
//#define NEW_KO      "(J"
//#define OLD_KO      "(J"
//#define NEC_KO      "H"

#define NL          10
#define FF          12
#define CR          13
#define ESC         27
#define TRUE        1
#define FALSE       0
#define TOFULLSIZE	1

#define SJIS1(A)    (((A >= 129) && (A <= 159)) || ((A >= 224) && (A <= 239)))
#define SJIS2(A)    ((A >= 64) && (A <= 252))
#define HANKATA(A)  ((A >= 161) && (A <= 223))
//#define ISEUC(A)    ((A >= 161) && (A <= 254))
#define ISEUC(A)    (A > 160)
#define NOTEUC(A,B) (((A >= 129) && (A <= 159)) && ((B >= 64) && (B <= 160)))
#define ISMARU(A)   ((A >= 202) && (A <= 206))
#define ISNIGORI(A) (((A >= 182) && (A <= 196)) || ((A >= 202) && (A <= 206)))


typedef struct{
	ubyte one;
	ubyte two;
}THAN2ZENKAN;

static const THAN2ZENKAN han2zenkan[] = {
	{129, 66},
	{129, 117},
	{129, 118},
	{129, 65},
	{129, 69},
	{131, 146},
	{131, 64},
	{131, 66},
	{131, 68},
	{131, 70},
	{131, 72},
	{131, 131},
	{131, 133},
	{131, 135},
	{131, 98},
	{129, 91},
	{131, 65},
	{131, 67},
	{131, 69},
	{131, 71},
	{131, 73},
	{131, 74},
	{131, 76},
	{131, 78},
	{131, 80},
	{131, 82},
	{131, 84},
	{131, 86},
	{131, 88},
	{131, 90},
	{131, 92},
	{131, 94},
	{131, 96},
	{131, 99},
	{131, 101},
	{131, 103},
	{131, 105},
	{131, 106},
	{131, 107},
	{131, 108},
	{131, 109},
	{131, 110},
	{131, 113},
	{131, 116},
	{131, 119},
	{131, 122},
	{131, 125},
	{131, 126},
	{131, 128},
	{131, 129},
	{131, 130},
	{131, 132},
	{131, 134},
	{131, 136},
	{131, 137},
	{131, 138},
	{131, 139},
	{131, 140},
	{131, 141},
	{131, 143},
	{131, 147},
	{129, 74},
	{129, 75}
};

static ubyte *han2zen (const ubyte *in, int *ptr1, int *ptr2, int const incode);
static int skipEscSeq (const ubyte data, int *ptr);
//static void sjis2jis (int *ptr1, int *ptr2);
static void jis2sjis (int *ptr1, int *ptr2);
static int shift2shift (const ubyte *in, ubyte *out, const int tofullsize);
static int euc2shift (const ubyte *in, ubyte *out, int tofullsize);
static int seven2shift (const ubyte *in, ubyte *out);
static int detectCodeType (const ubyte *in);


static int shiftState_jp = 0;		//0 = double byte, 1/2 = single byte

/*
	ISO-2022-JP
	ESC ( B    ASCII                                             shiftState_jp = 1
	ESC ( J    JIS X 0201-1976 Roman set (1 byte per character)	 shiftState_jp = 2
	ESC $ @    JIS X 0208-1978           (2 bytes per character) shiftState_jp = 0
	ESC $ B    JIS X 0208-1983           (2 bytes per character) shiftState_jp = 0

	ISO-2022-JP-3
	ESC ( I    JIS X 0201-1976 Kana set (1 byte per character)   shiftState_jp = 2
	ESC $ ( O  JIS X 0213-2000 Plane 1  (2 bytes per character)  shiftState_jp = 0
	ESC $ ( P  JIS X 0213-2000 Plane 2  (2 bytes per character)  shiftState_jp = 0
*/

int ISO2022JPToUTF32 (THWD *hw, const ubyte *in, UTF32 *wc)
{
	int p1,p2;

	p1 = *in++;
	if (!p1){
		return 0;
	}else if (p1 == ESC){
		if (in[0] == '('){
			if (in[1] == 'B')
				shiftState_jp = 0x01;
			else
				shiftState_jp = 0x02;
			*wc = 0;
			return -3;
		}else if (in[0] == '$'){
			shiftState_jp = 0;
			*wc = 0;
			if (in[1] == '{' && in[2])
				return -4;
			else
				return -3;
		}else{
			shiftState_jp = 0x01;
			*wc = remapChar(hw, ESC);
			return 1;
		}
	}else if (p1 == FF){
		*wc = 0;
		return -1;
	}else if (p1 == CR){
		*wc = 0;
		return -1;
	}else if (p1 == NL){
		*wc = NL;
		shiftState_jp = 0x02;	// reset to JISX0201
		return 1;
	}else{
		if (!shiftState_jp){
			p2 = *in++;
			jis2sjis(&p1, &p2);
         	ubyte str[4] = {p1, p2, 0, 0};
         	JISX0213ToUTF32(str, wc);
         	return 2;
        }else if (shiftState_jp&0x02){
			*wc = remapChar(hw, p1);	// JISX0201
			return 1;
		}else{
			*wc = p1;
			return 1;
		}
	}
}

int eucjp2sjis (const ubyte *in, UTF32 *out)
{
	int p2;
	int p1 = *in++;

	if (!p1){
		*out = 0;
		return 0;
	}else if (p1 == FF){
		*out = 0;
		return -1;
	}else{
		if (ISEUC(p1)){
			p2 = *in++;
			if (ISEUC(p2)){
				p1 &= 0x7F;
				p2 &= 0x7F;
				jis2sjis(&p1, &p2);
			}
			*out = (p2<<8)|p1;
			return 2;
		}else if (p1 == 142){
			p2 = *in++;
			if (HANKATA(p2)){
				#if (TOFULLSIZE)
					p1 = p2;
					in = han2zen(in, &p1, &p2, EUC);
          			*out = (p2<<8)|p1;
          			return 2;
				#else
					*out = p2;
					return 2;
				#endif
			}else{
        		*out = (p2<<8)|p1;
        		return 2;
           	}
       	}else{
			*out = p1;
			return 1;
		}
	}
}

int JaToSJIS (const ubyte *in, ubyte *out)
{
	switch (detectCodeType(in)){
	  case NEW :
	  case OLD :
	  case NEC :
		return seven2shift(in, out);			// 7bit, iso-2022-jp

	  case EUC :
		return euc2shift(in, out, FALSE);		// jis 208, euc-jp

	  case SJIS :
		return shift2shift(in, out, FALSE);		// shift jis
	}
	return 0;
}

static int euc2shift (const ubyte *inbuffer, ubyte *outbuffer, int tofullsize)
{
	int p1,p2;
	int tchars = 0;
	const ubyte *in = inbuffer;
	ubyte *out = outbuffer;

	while ((p1 = *in++)){
		if (!p1){
  			return tchars;
		}else if (p1 == FF){
		}else{
			if (ISEUC(p1)){
				p2 = *in++;
				if ISEUC(p2){
					p1 &= 0x7F;
					p2 &= 0x7F;
					jis2sjis(&p1, &p2);
				}
				*out++ = p1;
	        	*out++ = p2;
				tchars++;
			}else if (p1 == 142){
				p2 = *in++;
				if HANKATA(p2){
					if (tofullsize){
						p1 = p2;
						in = han2zen(in, &p1, &p2, EUC);
						*out++ = p1;
	          			*out++ = p2;
					}else{
						p1 = p2;
						*out++ = p1;
					}
				}else{
	            	*out++ = p1;
          			*out++ = p2;
            	}
            	tchars++;
        	}else{
				*out++ = p1;
				tchars++;
			}
		}
	}
	return tchars;
}

static int seven2shift (const ubyte *inbuffer, ubyte *outbuffer)
{
	int p1,p2;
	ubyte intemp;
	int tchars = 0;
	const ubyte *in = inbuffer;
	ubyte *out = outbuffer;
 	int shifted_in = FALSE;


	while ((p1 = *in++)){
		if (!p1){
			return tchars;
		}else if (p1 == ESC){
			intemp = *in++;
			in += skipEscSeq(intemp, &shifted_in);
		}else if (p1 == FF){
			//break;
		}else{
			if (!shifted_in){
				*out++ = p1;
			}else{
				p2 = *in++;
				jis2sjis(&p1, &p2);
				*out++ = p1;
          		*out++ = p2;
			}
			tchars++;
		}
	}
	return tchars;
}

static int shift2shift (const ubyte *inbuffer, ubyte *outbuffer, const int tofullsize)
{

	int p1, p2;
	int tchars = 0;
	const ubyte *in = inbuffer;
	ubyte *out = outbuffer;

	while ((p1 = *in++)){
		if (!p1){
			return tchars;
		}else if ((p1 == FF) || (p1 == CR)){

		}else if (p1 == NL){
			*out++ = NL;
			tchars++;
		}else{
			if SJIS1(p1){
				p2 = *in++;
				if SJIS2(p2){
					*out++ = p1;
	          		*out++ = p2;
	          		tchars++;
				}else{
				// bad char
				}
        	}else if (HANKATA(p1) && (tofullsize)){
          		in = han2zen(in, &p1, &p2, SJIS);
          		*out++ = p1;
          		*out++ = p2;
          		tchars++;
        	}else{
          		*out++ = p1;
          		tchars++;
			}
		}
	}

	return tchars;
}

static int detectCodeType (const ubyte *inbuffer)
{
	int p1,p2,p3;
	int whatcode = NOT_SET;
	const ubyte *in = inbuffer;

	while (((p1 = *in++) != 0) && ((whatcode == NOT_SET) || (whatcode == EUC))){
		if (p1 == ESC){
			p2 = *in++;
			if (p2 == '$'){
				p3 = *in++;
				if (p3 == 'B')
					whatcode = NEW;
				else if (p3 == '@')
					whatcode = OLD;
			}else if (p2 == 'K'){
        		whatcode = NEC;
        	}
		}else if ((p1 >= 129) && (p1 <= 254)){
			p2 = *in++;
			if NOTEUC(p1,p2)
				whatcode = SJIS;
			else if (ISEUC(p1) && ISEUC(p2))
				whatcode = EUC;
			else if (((p1 == 142)) && HANKATA(p2))
				whatcode = EUC;
		}
	}

	// we need a fall back
	if (whatcode == NOT_SET)
		return EUC;
	else
		return whatcode;
}


static ubyte *han2zen (const ubyte *inbuffer, int *one, int *two, int const incode)
{

	int maru, nigori;
	ubyte *in = (ubyte*)(inbuffer);
	maru = nigori = FALSE;

	if (incode == SJIS){
		*two = *in++;
		if (*two == 222){
			if (ISNIGORI(*one) || (*one == 179))
				nigori = TRUE;
			else
				in--;
		}else if (*two == 223){
			if ISMARU(*one)
				maru = TRUE;
			else
				in--;
		}else{
			in--;
		}
	}else if (incode == EUC){
		int junk = *in++;
		if (junk == 142){
			*two = *in++;
			if (*two == 222){
				if (ISNIGORI(*one) || (*one == 179))
					nigori = TRUE;
				else
					in -=2;
			}else if (*two == 223){
				if ISMARU(*one)
					maru = TRUE;
				else
          			in -=2;
			}else{
        		in -=2;
        	}
    	}else{
			in--;
		}
  	}

	*two = han2zenkan[*one-161].two;
	*one = han2zenkan[*one-161].one;

	if (nigori){
		if (((*two >= 74) && (*two <= 103)) || ((*two >= 110) && (*two <= 122)))
			(*two)++;
		else if ((*one == 131) && (*two == 69))
			*two = 148;
	}else if ((maru) && ((*two >= 110) && (*two <= 122))){
		*two += 2;
	}

    return in;
}

static int skipEscSeq (const ubyte temp, int *shifted_in)
{
	int ret = 0;
	if ((temp == '$') || (temp == '('))
		ret = 1;

	if ((temp == 'K') || (temp == '$'))
		*shifted_in = TRUE;
	else
		*shifted_in = FALSE;

	return ret;
}
/*
static void sjis2jis (int *p1, int *p2)
{
	register ubyte c1 = *p1;
	register ubyte c2 = *p2;
	register int adjust = c2 < 159;
	register int rowOffset = c1 < 160 ? 112 : 176;
	register int cellOffset = adjust ? (31 + (c2 > 127)) : 126;

	*p1 = ((c1 - rowOffset) << 1) - adjust;
	*p2 -= cellOffset;
}
*/
static void jis2sjis (int *p1, int *p2)
{
	const int rowOffset = *p1 < 95 ? 112 : 176;
	const int cellOffset = *p1 & 1 ? 31 + (*p2 > 95) : 126;
	*p1 = ((*p1 + 1) >> 1) + rowOffset;
	*p2 += cellOffset;
}

#else

int JaToSJIS (const ubyte *in, ubyte *out){return 0;}
int eucjp2sjis (const ubyte *in, UTF32 *out){return 0;}
int ISO2022JPToUTF32 (THWD *hw, const ubyte *in, UTF32 *wc){return 0;}
#endif

