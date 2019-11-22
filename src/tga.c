
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

#if (__BUILD_TGA_SUPPORT__)

#include "memory.h"
#include "frame.h"
#include "utils.h"
#include "fileio.h"
#include "tga.h"
#include "pixel.h"
#include "image.h"
#include "convert.h"

#include "sync.h"
#include "misc.h"

static INLINE void TGA_WriteRLEPacket (int total, ubyte *in, ubyte *out);
static INLINE void TGA_WriteRawPacket (int total, ubyte *in, ubyte *out);
static INLINE void TGA_ReadPixel (TRGB *p, ubyte *in);
static INLINE int TGA_isEqual (TRGB *a, TRGB *b);

static void TGA_GetPackets (ubyte *data, int width, int height, int depth, FILE *stream);
static inline int TGA_CountMatched (ubyte *in, int p, int inlen);
static inline int TGA_CountUnMatched (ubyte *in, int p, int inlen);
static inline int TGA_Encode (ubyte *in, int inlen, ubyte *out, int outlen);
static inline int TGA_Encode_GetSize (ubyte *in, int inlen);

#if (!__BUILD_2432BITONLY_SUPPORT__)
static inline int _32bppTGA1bppFrame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy);
static inline int _24bppTGA1bppFrame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy);
static inline int _32bppTGA12bppFrame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy);
static inline int _24bppTGA12bppFrame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy);
static inline int _32bppTGA8bppFrame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy);
static inline int _24bppTGA8bppFrame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy);
static inline int _24bppTGA16bpp565Frame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy);
static inline int _32bppTGA16bpp565Frame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy);
static inline int _24bppTGA16bpp555Frame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy);
static inline int _32bppTGA16bpp555Frame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy);
#endif
static inline int _24bppTGA24bppFrame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy);
static inline int _24bppTGA32bppFrame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy);
static inline int _24bppTGA32AbppFrame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy);
static inline int _32bppTGA24bppFrame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy);
static inline int _32bppTGA32bppFrame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy);
static inline int _32bppTGA32AbppFrame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy);


static inline int frame24ToTGA24 (TFRAME *frame, ubyte *data, double lcd_width, double lcd_height, double xscale, double yscale);
#if (!__BUILD_2432BITONLY_SUPPORT__)
static inline int frame16ToTGA24 (TFRAME *frame, ubyte *data, double lcd_width, double lcd_height, double xscale, double yscale);
static inline int frame15ToTGA24 (TFRAME *frame, ubyte *data, double lcd_width, double lcd_height, double xscale, double yscale);
static inline int frame12ToTGA24 (TFRAME *frame, ubyte *data, double lcd_width, double lcd_height, double xscale, double yscale);
static inline int frame8ToTGA24 (TFRAME *frame, ubyte *data, double lcd_width, double lcd_height, double xscale, double yscale);
static inline int frame1ToTGA24  (TFRAME *frame, ubyte *data, double lcd_width, double lcd_height, double xscale, double yscale);
#endif



static void TGA_GetPackets (ubyte *data, int width, int height, int depth, FILE *stream)
{
	ubyte header;
	ubyte buffer8[4];
	int run_length, i;
	int current_byte = 0;
	unsigned short buffer16;
	int bpp;

	if(depth==16) bpp=3; else bpp=depth>>3;

	while(current_byte < width*height*bpp){
		l_fread(&header, sizeof(ubyte), 1, stream);
		run_length=(header&0x7F)+1;

		if(header&0x80){
			if(depth == 32)
				l_fread(buffer8, sizeof(ubyte), 4, stream);

			if(depth == 24)
				l_fread(buffer8, sizeof(ubyte), 3, stream);

			if(depth == 16)
				l_fread(&buffer16, sizeof(unsigned short), 1, stream);

			if(depth == 8)
				l_fread(buffer8, sizeof(ubyte), 1, stream);

			for(i=0;i<run_length;i++){
				if(depth == 32){
					data[current_byte++]=buffer8[0];
					data[current_byte++]=buffer8[1];
					data[current_byte++]=buffer8[2];
					data[current_byte++]=buffer8[3];
				}

				if(depth == 24){
					data[current_byte++]=buffer8[0];
					data[current_byte++]=buffer8[1];
					data[current_byte++]=buffer8[2];
				}

				if(depth == 16){
					data[current_byte++]=(buffer16&0x1F)<<3;
					data[current_byte++]=((buffer16>>5)&0x1F)<<3;
					data[current_byte++]=((buffer16>>10)&0x1F)<<3;
				}

				if(depth == 8)
					data[current_byte++]=buffer8[0];
			}
		}else{
			for(i=0;i<run_length;i++){
				if(depth == 32){
					l_fread(buffer8, sizeof(ubyte), 4, stream);
					data[current_byte++]=buffer8[0];
					data[current_byte++]=buffer8[1];
					data[current_byte++]=buffer8[2];
					data[current_byte++]=buffer8[3];
				}

				if(depth == 24){
					l_fread(buffer8, sizeof(ubyte), 3, stream);
					data[current_byte++]=buffer8[0];
					data[current_byte++]=buffer8[1];
					data[current_byte++]=buffer8[2];
				}

				if(depth == 16){
					l_fread(&buffer16, sizeof(unsigned short), 1, stream);
					data[current_byte++]=(buffer16&0x1F)<<3;
					data[current_byte++]=((buffer16>>5)&0x1F)<<3;
					data[current_byte++]=((buffer16>>10)&0x1F)<<3;
				}

				if(depth == 8){
					l_fread(buffer8, sizeof(ubyte), 1, stream);
					data[current_byte++]=buffer8[0];
				}
			}
		}
	}
}

static ubyte readByte (FILE *stream)
{
	ubyte data;
	l_fread(&data, 1, 1, stream);
	return data;
}

static unsigned short readShort (FILE *stream)
{
	unsigned short data;
	l_fread(&data, 2, 1, stream);
	return data;
}

static inline int writeByte (ubyte data, FILE *stream)
{
	return (int)l_fwrite(&data, 1, 1, stream);
}

static inline int writeShort (unsigned short data, FILE *stream)
{
	return (int)l_fwrite(&data, 2, 1, stream);
}

int tgaGetMetrics (const wchar_t *filename, int *width, int *height, int *bpp)
{
    FILE *fp = l_wfopen(filename, L"rb");
   	if (!fp) return 0;
	
	TTGA tga;
	tga.identsize = readByte(fp);
	tga.colourmaptype = readByte(fp);
	tga.imagetype = readByte(fp);
	tga.colourmapstart = readShort(fp);
	tga.colourmaplength = readShort(fp);
	tga.colourmapbits = readByte(fp);
	tga.xstart = readShort(fp);
	tga.ystart = readShort(fp);
	tga.width = readShort(fp);
	tga.height = readShort(fp);
	tga.bpp = readByte(fp);

	l_fclose(fp);

	if (tga.imagetype){
		if (width) *width = tga.width;
		if (height) *height = tga.height;
		if (bpp){
			switch(tga.bpp){
			case 24: *bpp = LFRM_BPP_24; break;
			case 32: *bpp = LFRM_BPP_32; break;
			case 16: *bpp = LFRM_BPP_16; break;
			case  8: *bpp = LFRM_BPP_8; break;
			default: return 0;
			}
		}
		return 1;
	}
	return 0;
}

int loadTga (TFRAME *frm, const int flags, const wchar_t *filename, int ox, int oy)
{
	ubyte *ColorMap = NULL, *Data = NULL;
	FILE *stream;
	TTGA tga;

	if ((stream=l_wfopen(filename, L"rb")) == NULL)
		return 0;


	//l_fread(&tga, sizeof(tga), 1, stream);

	tga.identsize = readByte(stream);
	tga.colourmaptype = readByte(stream);
	tga.imagetype = readByte(stream);
	tga.colourmapstart = readShort(stream);
	tga.colourmaplength = readShort(stream);
	tga.colourmapbits = readByte(stream);
	tga.xstart = readShort(stream);
	tga.ystart = readShort(stream);
	tga.width = readShort(stream);
	tga.height = readShort(stream);
	tga.bpp = readByte(stream);
	tga.descriptor = readByte(stream);

	l_fseek(stream, tga.identsize, SEEK_CUR);

	switch(tga.imagetype){
	  case 1:
		if (tga.colourmaptype == 1 && tga.colourmapbits == 24){
			ColorMap=(ubyte *)l_malloc(tga.colourmaplength*(tga.colourmapbits>>3));
			if (!ColorMap) return 0;
				l_fread(ColorMap, sizeof(ubyte), tga.colourmaplength*(tga.colourmapbits>>3), stream);
		}else{
			l_fclose(stream);
			return 0;
		}
		break;
	  case 9:
		if (tga.colourmaptype == 1 && tga.colourmapbits == 24){
			ColorMap=(ubyte *)l_malloc(tga.colourmaplength*(tga.colourmapbits>>3));
			if (!ColorMap){
				l_fclose(stream);
				return 0;
			}
			l_fread(ColorMap, sizeof(ubyte), tga.colourmaplength*(tga.colourmapbits>>3), stream);
		}else{
			l_fclose(stream);
			return 0;
		}
		break;

	  case 2: break;
	  case 3: break;
	  case 10: break;
	  case 11: break;
	  default:
		l_fclose(stream);
		return 0;
	}

	switch(tga.bpp){
		case 32:
			Data =(ubyte *)l_malloc(tga.width*tga.height*4);
			if (Data==NULL){
				l_fclose(stream);
				return 0;
			}

			if(tga.imagetype == 2){
				l_fread(Data, sizeof(ubyte), tga.width*tga.height*4, stream);
				break;
			}

			if(tga.imagetype == 10){
				TGA_GetPackets(Data, tga.width, tga.height, tga.bpp, stream);
				break;
			}
			break;

		case 24:
			Data=(ubyte *)l_malloc(tga.width*tga.height*3);
			if (Data==NULL){
				l_fclose(stream);
				return 0;
			}

			if(tga.imagetype == 2){
				l_fread(Data, sizeof(ubyte), tga.width*tga.height*3, stream);
				break;
			}

			if(tga.imagetype == 10){
				TGA_GetPackets(Data, tga.width, tga.height, tga.bpp, stream);
				break;
			}
			break;

		case 16:
			Data=(ubyte *)l_malloc(tga.width*tga.height*3);
			if (Data==NULL){
				l_fclose(stream);
				return 0;
			}

			if (tga.imagetype == 2){
				unsigned short *buffer=(unsigned short *)l_malloc(sizeof(unsigned short)*tga.width*tga.height);
				if (buffer==NULL){
					l_free(Data);
					l_fclose(stream);
					return 0;
				}
				int i;

				l_fread(buffer, sizeof(unsigned short), tga.width*tga.height, stream);

				for (i=0;i<tga.width*tga.height;i++){
					Data[3*i]=(buffer[i]&0x1F)<<3;
					Data[3*i+1]=((buffer[i]>>5)&0x1F)<<3;
					Data[3*i+2]=((buffer[i]>>10)&0x1F)<<3;
				}

				l_free(buffer);
				tga.bpp=24;
				break;
			}

			if (tga.imagetype == 10){
				TGA_GetPackets(Data, tga.width, tga.height, tga.bpp, stream);
				tga.bpp=24;
				break;
			}
			break;

		case 8:
			if (tga.colourmaptype==1&&tga.colourmapbits==24){
				ubyte *buffer;

				Data=(ubyte *)l_malloc(tga.width*tga.height*3);
				if (Data==NULL){
					l_fclose(stream);
					return 0;
				}
				buffer=(ubyte *)l_malloc(tga.width*tga.height);
				if (buffer==NULL){
					l_free(Data);
					l_fclose(stream);
					return 0;
				}

				if (tga.imagetype == 9)
					TGA_GetPackets(buffer, tga.width, tga.height, tga.bpp, stream);
				else
					l_fread(buffer, sizeof(ubyte), tga.width*tga.height, stream);

				int i;
				for (i=0;i<tga.width*tga.height;i++){
					Data[3*i]=ColorMap[3*buffer[i]];
					Data[3*i+1]=ColorMap[3*buffer[i]+1];
					Data[3*i+2]=ColorMap[3*buffer[i]+2];
				}

				l_free(buffer);
				l_free(ColorMap);

				tga.bpp=24;
				break;
			}

			if (tga.imagetype == 3){
				Data=(ubyte *)l_malloc(tga.width*tga.height);
				if (Data==NULL){
					l_fclose(stream);
					return 0;
				}
				l_fread(Data, sizeof(ubyte), tga.width*tga.height, stream);
				break;
			}

			if (tga.imagetype == 11){
				Data=(ubyte *)l_malloc(tga.width*tga.height);
				if (Data==NULL){
					l_fclose(stream);
					return 0;
				}
				TGA_GetPackets(Data, tga.width, tga.height, tga.bpp, stream);
				break;
			}
			break;

		default:
			l_fclose(stream);
			return 0;
	}

	l_fclose(stream);
	if (!(tga.height&&tga.width)){
		l_free(Data);
		return 0;
	}

	if (flags & LOAD_RESIZE)
		_resizeFrame(frm, tga.width, tga.height, 0);
	int ret = 0;


	switch(tga.bpp){
	  case 32:
		if (frm->bpp == LFRM_BPP_32)
			ret = _32bppTGA32bppFrame(frm, Data, &tga, ox, oy);
		else if (frm->bpp == LFRM_BPP_32A)
			ret = _32bppTGA32AbppFrame(frm, Data, &tga, ox, oy);
		else if (frm->bpp == LFRM_BPP_24)
			ret = _32bppTGA24bppFrame(frm, Data, &tga, ox, oy);
#if (!__BUILD_2432BITONLY_SUPPORT__)
		else if (frm->bpp == LFRM_BPP_1)
			ret = _32bppTGA1bppFrame(frm, Data, &tga, ox, oy);
		else if (frm->bpp == LFRM_BPP_12)
			ret = _32bppTGA12bppFrame(frm, Data, &tga, ox, oy);
		else if (frm->bpp == LFRM_BPP_8)
			ret = _32bppTGA8bppFrame(frm, Data, &tga, ox, oy);
		else if (frm->bpp == LFRM_BPP_16)
			ret = _32bppTGA16bpp565Frame(frm, Data, &tga, ox, oy);
		else if (frm->bpp == LFRM_BPP_15)
			ret = _32bppTGA16bpp555Frame(frm, Data, &tga, ox, oy);
#endif
		break;
	  case 24:
		if (frm->bpp == LFRM_BPP_32)
			ret = _24bppTGA32bppFrame(frm, Data, &tga, ox, oy);
		else if (frm->bpp == LFRM_BPP_32A)
			ret = _24bppTGA32AbppFrame(frm, Data, &tga, ox, oy);
		else if (frm->bpp == LFRM_BPP_24)
			ret = _24bppTGA24bppFrame(frm, Data, &tga, ox, oy);
#if (!__BUILD_2432BITONLY_SUPPORT__)
		else if (frm->bpp == LFRM_BPP_1)
			ret = _24bppTGA1bppFrame(frm, Data, &tga, ox, oy);
		else if (frm->bpp == LFRM_BPP_12)
			ret = _24bppTGA12bppFrame(frm, Data, &tga, ox, oy);
		else if (frm->bpp == LFRM_BPP_8)
			ret = _24bppTGA8bppFrame(frm, Data, &tga, ox, oy);
		else if (frm->bpp == LFRM_BPP_16)
			ret = _24bppTGA16bpp565Frame(frm, Data, &tga, ox, oy);
		else if (frm->bpp == LFRM_BPP_15)
			ret = _24bppTGA16bpp555Frame(frm, Data, &tga, ox, oy);
#endif
		break;
		default :	ret = 0;
	}

	l_free(Data);
	return ret;
}

#if (!__BUILD_2432BITONLY_SUPPORT__)
static inline int _32bppTGA16bpp565Frame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy)
{

	int x,y,r,g,b;
	int p=0;

	for (y=tga->height-1; y>=0; y--){
		for (x=0;x<tga->width;x++){
			b = (data[p++]&0xF8)>>3;	// 5
			g = (data[p++]&0xFC)<<3;	// 6
			r = (data[p++]&0xF8)<<8;	// 5
			l_setPixel(frame, x+ox, y+oy, r|g|b);
			p++;
		}
	}
	return 1;
}

static inline int _32bppTGA16bpp555Frame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy)
{
	int x,y,r,g,b;
	int p=0;

	for (y=tga->height-1; y>=0; y--){
		for (x=0;x<tga->width;x++){
			b = (data[p++]&0xF8)>>3;	// 5
			g = (data[p++]&0xF8)<<2;	// 5
			r = (data[p++]&0xF8)<<7;	// 5
			l_setPixel(frame, x+ox, y+oy, r|g|b);
			p++;
		}
	}
	return 1;
}

static inline int _24bppTGA16bpp555Frame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy)
{
	int x,y,r,g,b;
	int p=0;

	for (y=tga->height-1; y>=0; y--){
		for (x=0;x<tga->width;x++){
			b = (data[p++]&0xF8)>>3;	// 5
			g = (data[p++]&0xF8)<<2;	// 5
			r = (data[p++]&0xF8)<<7;	// 5
			l_setPixel(frame, x+ox, y+oy, r|g|b);
		}
	}
	return 1;
}

static inline int _24bppTGA16bpp565Frame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy)
{
	int x,y,r,g,b;
	int p=0;

	for (y=tga->height-1; y>=0; y--){
		for (x=0;x<tga->width;x++){
			b = (data[p++]&0xF8)>>3;	// 5
			g = (data[p++]&0xFC)<<3;	// 6
			r = (data[p++]&0xF8)<<8;	// 5
			l_setPixel(frame, x+ox, y+oy, r|g|b);
		}
	}
	return 1;
}
#endif
static inline int _32bppTGA32bppFrame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy)
{
	int x,y;
	int p=0;

	for (y=tga->height-1; y>=0; y--){
		for (x=0;x<tga->width;x++){
			l_setPixel(frame, x+ox, y+oy, data[p]|data[p+1]<<8|data[p+2]<<16);
			p += 4;
		}
	}
	return 1;
}

static inline int _32bppTGA32AbppFrame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy)
{
	int x,y;
	int p=0;

	for (y=tga->height-1; y>=0; y--){
		for (x=0;x<tga->width;x++){
			l_setPixel(frame, x+ox, y+oy, data[p]|data[p+1]<<8|data[p+2]<<16|data[p+3]<<24);
			p += 4;
		}
	}
	return 1;
}

static inline int _32bppTGA24bppFrame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy)
{
	int x,y;
	int p=0;

	for (y=tga->height-1; y>=0; y--){
		for (x=0;x<tga->width;x++){
			l_setPixel(frame, x+ox, y+oy, data[p]|data[p+1]<<8|data[p+2]<<16);
			p += 4;
		}
	}
	return 1;
}

static inline int _24bppTGA32bppFrame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy)
{
	int x,y;
	int p=0;

	for (y=tga->height-1; y>=0; y--){
		for (x=0;x<tga->width;x++){
			l_setPixel(frame, x+ox, y+oy, data[p] | data[p+1]<<8 | data[p+2]<<16);
			p += 3;
		}
	}
	return 1;
}

#if 0
#ifdef USE_MMX
#include "mmx.h"
#include "mmx_rgb.h"
#endif
#endif

static inline int _24bppTGA32AbppFrame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy)
{
#if 0
	printf("_24bppTGA32AbppFrame %i %i %i %i\n", frame->width, tga->width, frame->height, tga->height);

#ifdef USE_MMX
	rgb_24_to_rgba_32_mmx(data, tga->width, tga->height, lGetPixelAddress(frame, 0, 0));
	return 1;
#endif
#endif

	int x,y;
	int p=0;

	for (y=tga->height-1; y>=0; y--){
		for (x=0;x<tga->width;x++){
			l_setPixel(frame, x+ox, y+oy, 0xFF000000 | data[p] | data[p+1]<<8 | data[p+2]<<16);
			p += 3;
		}
	}
	return 1;
}
#if (!__BUILD_2432BITONLY_SUPPORT__)
static inline int _24bppTGA8bppFrame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy)
{
	int x,y,r,g,b;
	int p=0;


	for (y=tga->height-1; y>=0; y--){
		for (x=0;x<tga->width;x++){
			b = (data[p++]&0xC0)>>6;
			g = (data[p++]&0xE0)>>3;
			r = (data[p++]&0xE0);
			l_setPixel(frame, x+ox, y+oy, r|g|b);
		}
	}
	return 1;
}

static inline int _32bppTGA8bppFrame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy)
{
	int x,y,r,g,b;
	int p=0;

	for (y=tga->height-1; y>=0; y--){
		for (x=0;x<tga->width;x++){
			b = (data[p++]&0xC0)>>6;
			g = (data[p++]&0xE0)>>3;
			r = (data[p++]&0xE0);
			l_setPixel(frame, x+ox, y+oy, r|g|b);
			p++;
		}
	}
	return 1;
}

static inline int _32bppTGA12bppFrame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy)
{
	int x,y,r,g,b;
	int p=0;

	for (y=tga->height-1; y>=0; y--){
		for (x=0;x<tga->width;x++){
			b = (data[p++]&0xF0)>>4;
			g = (data[p++]&0xF0);
			r = (data[p++]&0xF0)<<4;
			l_setPixel(frame, x+ox, y+oy, r|g|b);
			p++;
		}
	}
	return 1;
}

static inline int _24bppTGA12bppFrame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy)
{
	int x,y,r,g,b;
	int p=0;

	for (y=tga->height-1; y>=0; y--){
		for (x=0;x<tga->width;x++){
			b = (data[p++]&0xF0)>>4;
			g = (data[p++]&0xF0);
			r = (data[p++]&0xF0)<<4;
			l_setPixel(frame, x+ox, y+oy, r|g|b);
		}
	}
	return 1;
}
#endif
static inline int _24bppTGA24bppFrame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy)
{
	int x,y;
	int p=0;

	for (y=tga->height-1; y>=0; y--){
		for (x=0;x<tga->width;x++){
			l_setPixel(frame, x+ox, y+oy, data[p]|data[p+1]<<8|data[p+2]<<16);
			p += 3;
		}
	}
	return 1;
}
#if (!__BUILD_2432BITONLY_SUPPORT__)
static inline int _24bppTGA1bppFrame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy)
{

	int x,y;
	int p=0;

	for (y=tga->height-1; y>=0; y--){
		for (x=0;x<tga->width;x++){
			setPixel_BC(frame, x+ox, y+oy, (data[p]<TGATRESHOLD) || (data[p+1]<TGATRESHOLD) || (data[p+2]<TGATRESHOLD));
			p += 3;
		}
	}
	return 1;
}

static inline int _32bppTGA1bppFrame (TFRAME *frame, ubyte *data, TTGA *tga, int ox, int oy)
{

	int x,y;
	int p=0;

	for (y=tga->height-1; y>=0; y--){
		for (x=0;x<tga->width;x++){
			setPixel_BC(frame, x+ox, y+oy, (data[p]<TGATRESHOLD) || (data[p+1]<TGATRESHOLD) || (data[p+2]<TGATRESHOLD));
			p += 4;
		}
	}
	return 1;
}

static inline int frame1ToTGA24 (TFRAME *frame, ubyte *data, double lcd_width, double lcd_height, double xscale, double yscale)
{
	double x=0.0, y=0.0;

	for (y=lcd_height-yscale; y>-yscale; y-=yscale){
		for (x=0;x<lcd_width;x+=xscale){
			if (getPixel_NB(frame,(int)x,(int)y)){
				*(data++) = 255;
				*(data++) = 255;
				*(data++) = 255;
			}else{
				*(data++) = 0;
				*(data++) = 0;
				*(data++) = 0;
			}
		}
	}
	return 1;
}
#endif
static inline int frame24ToTGA24 (TFRAME *frame, ubyte *data, double lcd_width, double lcd_height, double xscale, double yscale)
{
	double x=0.0, y=0.0;
	int colour;

	for (y=lcd_height-yscale; y>-yscale; y-=yscale){
		for (x=0;x<lcd_width;x+=xscale){
			colour = l_getPixel(frame,(int)x,(int)y);
			*(data++) =  colour&0x0000FF;		// blue
			*(data++) = (colour&0x00FF00)>>8;	// green
			*(data++) = (colour&0xFF0000)>>16;	// red
		}
	}
	return 1;
}

#if (!__BUILD_2432BITONLY_SUPPORT__)
static inline int frame8ToTGA24 (TFRAME *frame, ubyte *data, double lcd_width, double lcd_height, double xscale, double yscale)
{
	double x=0.0, y=0.0;
	int colour;

	for (y=lcd_height-yscale; y>-yscale; y-=yscale){
		for (x=0;x<lcd_width;x+=xscale){
			colour = l_getPixel(frame,(int)x,(int)y);
			*(data++) = (colour&0x03)<<6;	// blue
			*(data++) = (colour&0x1C)<<3;	// green
			*(data++) = (colour&0xE0);		// red
		}
	}
	return 1;
}

static inline int frame12ToTGA24 (TFRAME *frame, ubyte *data, double lcd_width, double lcd_height, double xscale, double yscale)
{
	double x=0.0, y=0.0;
	int colour;

	for (y=lcd_height-yscale; y>-yscale; y-=yscale){
		for (x=0;x<lcd_width;x+=xscale){
			colour = l_getPixel(frame,(int)x,(int)y);
			*(data++) = (colour&0x00F)<<4;	// blue
			*(data++) = (colour&0x0F0);		// green
			*(data++) = (colour&0xF00)>>4;	// red
		}
	}
	return 1;
}

static inline int frame15ToTGA24 (TFRAME *frame, ubyte *data, double lcd_width, double lcd_height, double xscale, double yscale)
{
	double x=0.0, y=0.0;
	int colour;

	for (y=lcd_height-yscale; y>-yscale; y-=yscale){
		for (x=0;x<lcd_width;x+=xscale){
			colour = l_getPixel(frame,(int)x,(int)y);
			*(data++) = (colour&0x001F)<<3;		// blue
			*(data++) = (colour&0x03E0)>>2;		// green
			*(data++) = (colour&0x7C00)>>7;		// red
		}
	}
	return 1;
}

static inline int frame16ToTGA24 (TFRAME *frame, ubyte *data, double lcd_width, double lcd_height, double xscale, double yscale)
{
	double x=0.0, y=0.0;
	int colour;

	for (y=lcd_height-yscale; y>-yscale; y-=yscale){
		for (x=0;x<lcd_width;x+=xscale){
			colour = l_getPixel(frame,(int)x,(int)y);
			*(data++) = (colour&0x001F)<<3;		// blue
			*(data++) = (colour&0x07E0)>>3;		// green
			*(data++) = (colour&0xF800)>>8;		// red
		}
	}
	return 1;
}
#endif
int saveTga (TFRAME *frm, const wchar_t *filename, int width, int height)
{
	if (!frm || !filename)
		return -3;

	int ibufflen = height * width * 3;
	ubyte *tgadata = (ubyte *)l_malloc(ibufflen);
	if (tgadata == NULL) return -4;

	double yscale = frm->height / (double)height;
	double xscale = frm->width / (double)width;
	double lcd_height = (double)(MIN(frm->height,65535));
	double lcd_width = (double)(MIN(frm->width,65535));

	if (frm->bpp == LFRM_BPP_24)
		frame24ToTGA24(frm, tgadata, lcd_width, lcd_height, xscale, yscale);
	else if (frm->bpp == LFRM_BPP_32 || frm->bpp == LFRM_BPP_32A)
		frame24ToTGA24(frm, tgadata, lcd_width, lcd_height, xscale, yscale);
#if (!__BUILD_2432BITONLY_SUPPORT__)
	else if (frm->bpp == LFRM_BPP_1)
		frame1ToTGA24(frm, tgadata, lcd_width, lcd_height, xscale, yscale);
	else if (frm->bpp == LFRM_BPP_12)
		frame12ToTGA24(frm, tgadata, lcd_width, lcd_height, xscale, yscale);
	else if (frm->bpp == LFRM_BPP_8)
		frame8ToTGA24(frm, tgadata, lcd_width, lcd_height, xscale, yscale);
	else if (frm->bpp == LFRM_BPP_15)
		frame15ToTGA24(frm, tgadata, lcd_width, lcd_height, xscale, yscale);
	else if (frm->bpp == LFRM_BPP_16)
		frame16ToTGA24(frm, tgadata, lcd_width, lcd_height, xscale, yscale);
#endif

	FILE *fp = l_wfopen(filename, L"wb");
	if (!fp){
		mylogw(L"libmylcd: saveTGA(): could not open '%s'\n", filename);
		l_free(tgadata);
		return -5;
	}

	TTGA tga ={0,0,10,0,0,0,0,0,MIN(width, 0xFFFF),MIN(height, 0xFFFF),24,0};

	int encodedtotal = TGA_Encode_GetSize(tgadata,ibufflen);
	if (encodedtotal >= ibufflen){
		// use raw tga
		tga.imagetype = 2;
		l_fwrite(&tga, sizeof(tga), 1, fp);
    	l_fwrite(tgadata, sizeof(ubyte), ibufflen, fp);
    	l_free(tgadata);
    }else{
    	// use compressed tga
		ubyte *out = (ubyte*)l_malloc(encodedtotal);
		if (out == NULL){
			l_free (tgadata);
			l_fclose(fp);
			return -6;
		}
    	TGA_Encode(tgadata,ibufflen,out,encodedtotal);
    	l_free(tgadata);
    	
    	//l_fwrite(&tga, sizeof(tga), 1, fp);
    	
    	writeByte(tga.identsize, fp);
		writeByte(tga.colourmaptype, fp);
		writeByte(tga.imagetype, fp);
		writeShort(tga.colourmapstart, fp);
		writeShort(tga.colourmaplength, fp);
		writeByte(tga.colourmapbits, fp);
		writeShort(tga.xstart, fp);
		writeShort(tga.ystart, fp);
		writeShort(tga.width, fp);
		writeShort(tga.height, fp);
		writeByte(tga.bpp, fp);
		writeByte(tga.descriptor, fp);

		l_fwrite(out, sizeof(ubyte), encodedtotal, fp);
		l_free(out);
	}

    l_fclose(fp);
    return 1;
}

// TGA RLE encoding
static inline int TGA_Encode (ubyte *in, int inlen, ubyte *out, int outlen)
{
	ubyte *pout = out;
	int ct=0,i;
	(void)outlen;

	for (i=0;i<inlen;i+=(ct+ct+ct)){
		if (!(ct = TGA_CountMatched(in,i,inlen))){
			if ((ct = TGA_CountUnMatched(in,i,inlen))){
				TGA_WriteRawPacket(ct,in+i,pout);
				pout += 1+(ct+ct+ct);
			}
		}else{
			TGA_WriteRLEPacket (ct,in+i,pout);
			pout += 4;
		}
	}
	return (pout-out);
}


// TGA RLE return compressed length
static inline int TGA_Encode_GetSize (ubyte *in, int inlen)
{
	ubyte *pout = in;
	int ct=0,i;

	for (i=0; i<inlen; i+=ct*3){
		if (!(ct = TGA_CountMatched(in,i,inlen))){
			if ((ct = TGA_CountUnMatched(in,i,inlen)))
				pout += 1+(ct*3);
		}else{
			pout += 4;
		}
	}
	return (pout-in);
}


static INLINE void TGA_ReadPixel (TRGB *p, ubyte *in)
{
	p->r = *(in++);
	p->g = *(in++);
	p->b = *in;
}

static INLINE void TGA_WriteRawPacket (int total, ubyte *in, ubyte *out)
{
	if (!total) return;

	// a packet header byte, sets number of raw pixels to follow
	*(out++) = (total-1)&0x7F;

	int i;
	for (i=0;i<total;i++){
		*(out++) = *(in++);
		*(out++) = *(in++);
		*(out++) = *(in++);
	}
}

static INLINE void TGA_WriteRLEPacket (int total, ubyte *in, ubyte *out)
{
	if (!total) return;

	TRGB rgb;
	TGA_ReadPixel(&rgb,in);

	// a packet header byte. following pixel is repeated 'total' times
	*(out++) = (total-1)|0x80;
	*(out++) = rgb.r;
	*(out++) = rgb.g;
	*out 	 = rgb.b;
}

static INLINE int TGA_isEqual (TRGB *a, TRGB *b)
{
	if ((a->r != b->r) || (a->g != b->g) || (a->b != b->b))
		return 0;
	else
		return 1;
}

static inline int TGA_CountMatched (ubyte *in, int p, int inlen)
{
	int ct=0;
	TRGB c,n;

	do{
		TGA_ReadPixel(&c,in+p);
		TGA_ReadPixel(&n,in+p+3);

		if (TGA_isEqual(&c,&n)){
			ct++;
			if (ct == 128) return ct;
		}else{
			if (ct) ct++;
			return ct;
		}

		p+=3;
	}
	while (p < inlen-3);

	if (ct) ct++;
	return ct;
}

static inline int TGA_CountUnMatched (ubyte *in, int p, int inlen)
{
	int ct=0;
	TRGB c,n;

	do{
		TGA_ReadPixel(&c,in+p);
		TGA_ReadPixel(&n,in+p+3);

		if (!TGA_isEqual(&c,&n)){
			ct++;
			if (ct == 128) return ct;
		}else
			return ct;

		p+=3;
	}
	while (p < inlen-3);

	if (ct) ct++;
	return ct;
}

#else


int loadTga (TFRAME *frame, const wchar_t* filename, int style){return 0;}
int saveTga (TFRAME *frame, const wchar_t* filename, int width, int height){return 0;}
int tgaGetMetrics (const wchar_t *filename, int *width, int *height, int *bpp){return 0;}

#endif

