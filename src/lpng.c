
// libmylcd - http://mylcd.sourceforge.net/
// An LCD framebuffer library
// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2008  Michael McElligott
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

#if (__BUILD_PNG_SUPPORT__)

#include <png.h>

#include "memory.h"
#include "utils.h"
#include "frame.h"
#include "fileio.h"
#include "lmath.h"
#include "lpng.h"
#include "pixel.h"
#include "image.h"
#include "copy.h"

#include "sync.h"
#include "misc.h"

#if (__BUILD_PNG_READ_SUPPORT__)

static inline void readPng32To1 (TFRAME *frame, png_structp *png_ptr, int width, int height, ubyte *line, int ox, int oy, int passCount)
{
	while(passCount--)
	for (int y = 0; y < height; y++){
		png_read_row(*png_ptr, line, NULL);
		int i = 0;
		for (int x = 0; x < width; x++){
			if ((line[i]<PNG_READ_TRESHOLD) || (line[i+1]<PNG_READ_TRESHOLD) || (line[i+2]<PNG_READ_TRESHOLD))
				setPixel_BC(frame, x+ox, y+oy, frame->style);
			i += 4;
		}
	}
}

static inline void readPng32To16_565 (TFRAME *frame, png_structp *png_ptr, int width, int height, ubyte *line, int ox, int oy, int passCount)
{
	while(passCount--)
	for (int y = 0; y < height; y++){
		png_read_row(*png_ptr, line, NULL);
		int i = 0;
		for (int x = 0; x < width; x++){
			int r = (line[i++]&0xF8)<<8;	// 5
			int g = (line[i++]&0xFC)<<3;	// 6
			int b = (line[i++]&0xF8)>>3;	// 5
			l_setPixel(frame, x+ox, y+oy, r|g|b);
			i++;
		}
	}
}

static inline void readPng32To16_555 (TFRAME *frame, png_structp *png_ptr, int width, int height, ubyte *line, int ox, int oy, int passCount)
{
	while(passCount--)
	for (int y = 0; y < height; y++){
		png_read_row(*png_ptr, line, NULL);
		int i = 0;
		for (int x = 0; x < width; x++){
			int r = (line[i++]&0xF8)<<7;	// 5
			int g = (line[i++]&0xF8)<<2;	// 6
			int b = (line[i++]&0xF8)>>3;	// 5
			l_setPixel(frame, x+ox, y+oy, r|g|b);
			i++;
		}
	}
}

static inline void readPng32To12 (TFRAME *frame, png_structp *png_ptr, int width, int height, ubyte *line, int ox, int oy, int passCount)
{
	while(passCount--)
	for (int y = 0; y < height; y++){
		png_read_row(*png_ptr, line, NULL);
		int i = 0;
		for (int x = 0; x < width; x++){
			int r = (line[i++]&0xF0)<<4;
			int g = (line[i++]&0xF0);
			int b = (line[i++]&0xF0)>>4;
			l_setPixel(frame, x+ox, y+oy, r|g|b);
			i++;
		}
	}
}

static inline void readPng32To24 (TFRAME *frame, png_structp *png_ptr, int width, int height, ubyte *line, int ox, int oy, int passCount)
{
	while(passCount--)
	for (int y = 0; y < height; y++){
		png_read_row(*png_ptr, line, NULL);
		int i = 0;
		for (int x = 0; x < width; x++, i += 4){
			l_setPixel(frame, x+ox, y+oy, line[i]<<16|line[i+1]<<8|line[i+2]);
		}
	}
}

static inline void readPng32To32 (TFRAME *frame, png_structp * png_ptr, int width, int height, ubyte *line, int ox, int oy, int passCount)
{
	while(passCount--)
	for (int y = 0; y < height; y++){
		png_read_row(*png_ptr, line, NULL);
		int i = 0;
		for (int x = 0; x < width; x++, i += 4){
			l_setPixel(frame, x+ox, y+oy, line[i]<<16|line[i+1]<<8|line[i+2]);
		}
	}
}

void readPng32To32A_clone (TFRAME *frame, png_structp *png_ptr, int width, int height, ubyte *line, int ox, int oy, int passCount)
{
	while(passCount--)
	for (int y = oy; y < (const int)(oy+height); y++){
		png_read_row(*png_ptr, line, NULL);
		TRGBA *p = (TRGBA*)lGetPixelAddress(frame, ox, y);
		if (!p) continue;

		int i = 0;
		for (int x = 0; x < width; x++){
			p[x].b = line[i++];
			p[x].g = line[i++];
			p[x].r = line[i++];
			p[x].a = line[i++];
		}
	}
}

static inline void readPng32To32A (TFRAME *frame, png_structp *png_ptr, int width, int height, ubyte *line, int ox, int oy, int passCount)
{
	while(passCount--)
	for (int y = 0; y < height; y++){
		png_read_row(*png_ptr, line, NULL);

		int i = 0;
		for (int x = 0; x < width; i+=4, x++){
			l_setPixel(frame, x+ox, y+oy, line[i]<<16|line[i+1]<<8|line[i+2]|line[i+3]<<24);
		}
	}
}

static inline void readPng32To8 (TFRAME *frame, png_structp *png_ptr, int width, int height, ubyte *line, int ox, int oy, int passCount)
{
	while(passCount--)
	for (int y = 0; y < height; y++){
		png_read_row(*png_ptr, line, NULL);
		int i = 0;
		for (int x = 0; x < width; x++){
			int r = (line[i++]&0xE0);
			int g = (line[i++]&0xE0)>>3;
			int b = (line[i++]&0xC0)>>6;
			l_setPixel(frame, x+ox, y+oy, r|g|b);
			i++;
		}
	}
}
#endif

#if (__BUILD_PNG_WRITE_SUPPORT__)

static inline void writeFrame1Png24 (TFRAME *frame, png_structp *png_ptr, int width, int height, ubyte *line)
{
	const double yscale = frame->height / (double)height;
	const double xscale = frame->width / (double)width;
	const double lcd_height = (double)frame->height;
	const double lcd_width = (double)frame->width;
	double x=0.0, y=0.0;
	int i; 
	
	for (y = 0.0; y < lcd_height; y += yscale){
		for (i = 0, x = 0.0; x<lcd_width; x += xscale){
			if (!l_getPixel(frame,(int)x,(int)y)){
				line[i++] = PNG_WRITE_LO;
				line[i++] = PNG_WRITE_LO;
				line[i++] = PNG_WRITE_LO;
			}else{
				line[i++] = PNG_WRITE_HI;
				line[i++] = PNG_WRITE_HI;
				line[i++] = PNG_WRITE_HI;
			}
		}
		png_write_row(*png_ptr, line);
	}
}

static inline void writeFrame8Png24 (TFRAME *frame, png_structp *png_ptr, int width, int height, ubyte *line)
{
	const double yscale = frame->height / (double)height;
	const double xscale = frame->width / (double)width;
	const double lcd_height = (double)frame->height;
	const double lcd_width = (double)frame->width;
	double x=0.0, y=0.0;
	int colour,i; 
	
	for (y = 0.0; y < lcd_height; y += yscale){
		for (i = 0, x = 0.0; x<lcd_width; x += xscale){
			colour = l_getPixel(frame,(int)x,(int)y);
			line[i++] = (colour&0xE0);		// red
			line[i++] = (colour&0x1C)<<3;	// green
			line[i++] = (colour&0x03)<<6;	// blue
		}
		png_write_row(*png_ptr, line);
	}
}
			
static inline void writeFrame12Png24 (TFRAME *frame, png_structp *png_ptr, int width, int height, ubyte *line)
{
	const double yscale = frame->height / (double)height;
	const double xscale = frame->width / (double)width;
	const double lcd_height = (double)frame->height;
	const double lcd_width = (double)frame->width;
	double x=0.0, y=0.0;
	int colour,i; 
	
	for (y = 0.0; y < lcd_height; y += yscale){
		for (i = 0, x = 0.0; x<lcd_width; x += xscale){
			colour = l_getPixel(frame,(int)x,(int)y);
			line[i++] = (colour&0xF00)>>4;	// red
			line[i++] = (colour&0x0F0);		// green
			line[i++] = (colour&0x00F)<<4;	// blue
		}
		png_write_row(*png_ptr, line);
	}
}

static inline void writeFrame16Png24_555 (TFRAME *frame, png_structp *png_ptr, int width, int height, ubyte *line)
{
	const double yscale = frame->height / (double)height;
	const double xscale = frame->width / (double)width;
	const double lcd_height = (double)frame->height;
	const double lcd_width = (double)frame->width;
	double x=0.0, y=0.0;
	int colour,i; 
	
	for (y = 0.0; y < lcd_height; y += yscale){
		for (i = 0, x = 0.0; x<lcd_width; x += xscale){
			colour = l_getPixel(frame,(int)x,(int)y);
			line[i++] = (colour&0x7C00)>>7;		// red
			line[i++] = (colour&0x03E0)>>2;		// green
			line[i++] = (colour&0x001F)<<3;		// blue
		}
		png_write_row(*png_ptr, line);
	}
}

static inline void writeFrame16Png24_565 (TFRAME *frame, png_structp *png_ptr, int width, int height, ubyte *line)
{
	const double yscale = frame->height / (double)height;
	const double xscale = frame->width / (double)width;
	const double lcd_height = (double)frame->height;
	const double lcd_width = (double)frame->width;

	double x=0.0, y=0.0;
	int colour,i; 
	
	for (y = 0.0; y < lcd_height; y += yscale){
		for (i = 0, x = 0.0; x<lcd_width; x += xscale){
			colour = l_getPixel(frame,(int)x,(int)y);
			line[i++] = (colour&0xF800)>>8;		// red
			line[i++] = (colour&0x07E0)>>3;		// green
			line[i++] = (colour&0x001F)<<3;		// blue
		}
		png_write_row(*png_ptr, line);
	}
}

static inline void writeFrame24Png24 (TFRAME *frame, png_structp *png_ptr, int width, int height, ubyte *line)
{
	const double yscale = frame->height / (double)height;
	const double xscale = frame->width / (double)width;
	const double lcd_height = (double)frame->height;
	const double lcd_width = (double)frame->width;

	double x=0.0, y=0.0;
	int colour,i; 
	
	for (y = 0.0; y < lcd_height; y += yscale){
		for (i = 0, x = 0.0; x<lcd_width; x += xscale){
			colour = l_getPixel(frame,(int)x,(int)y);
			line[i++] = (colour&0xFF0000)>>16;	// red
			line[i++] = (colour&0x00FF00)>>8;	// green
			line[i++] = colour&0x0000FF;		// blue
		}
		png_write_row(*png_ptr, line);
	}
}

static inline void writeFrame32Png32A (TFRAME *frame, png_structp *png_ptr, int width, int height, ubyte *line)
{
	const double yscale = frame->height / (double)height;
	const double xscale = frame->width / (double)width;
	const double lcd_height = (double)frame->height;
	const double lcd_width = (double)frame->width;
	double x=0.0, y=0.0;
	int colour,i; 
	
	for (y = 0.0; y < lcd_height; y += yscale){
		for (i = 0, x = 0.0; x<lcd_width; x += xscale){
			colour = l_getPixel(frame,(int)x,(int)y);
			line[i++] = (colour&0x00FF0000)>>16;	// red
			line[i++] = (colour&0x0000FF00)>>8;		// green
			line[i++] =  colour&0x000000FF;			// blue
			line[i++] = (colour&0xFF000000)>>24;	// alpha
		}
		png_write_row(*png_ptr, line);
	}
}

static inline void writeFrame32Png24 (TFRAME *frame, png_structp *png_ptr, int width, int height, ubyte *line)
{
	const double yscale = frame->height / (double)height;
	const double xscale = frame->width / (double)width;
	const double lcd_height = (double)frame->height;
	const double lcd_width = (double)frame->width;
	double x=0.0, y=0.0;
	int colour,i; 
	
	for (y = 0.0; y < lcd_height; y += yscale){
		for (i = 0, x = 0.0; x<lcd_width; x += xscale){
			colour = l_getPixel(frame,(int)x,(int)y);
			line[i++] = (colour&0xFF0000)>>16;	// red
			line[i++] = (colour&0x00FF00)>>8;	// green
			line[i++] =  colour&0x0000FF;		// blue
		}
		png_write_row(*png_ptr, line);
	}
}


int writeFrame32AToPng32A (TFRAME *frame, png_structp *png_ptr, int width, int height)
{
	ubyte *line = (ubyte*)l_malloc(width * 4);		// 4 for RGBA
	if (line != NULL){
		writeFrame32Png32A(frame, png_ptr, width, height, line);
		l_free(line);
		return 1;
	}
	return 0;
}

int writeFrameToPng24 (TFRAME *frame, png_structp *png_ptr, int width, int height)
{
	ubyte *line = (ubyte*)l_malloc(width * 3);		// 3 for RGB
	if (line == NULL){
		return 0;
	}

	int ret = 1;
	switch (frame->bpp){
	  case LFRM_BPP_1: writeFrame1Png24(frame, png_ptr, width, height, line); break;
	  case LFRM_BPP_8: writeFrame8Png24(frame, png_ptr, width, height, line); break;
	  case LFRM_BPP_12: writeFrame12Png24(frame, png_ptr, width, height, line); break;
	  case LFRM_BPP_15: writeFrame16Png24_555(frame, png_ptr, width, height, line); break;
	  case LFRM_BPP_16: writeFrame16Png24_565(frame, png_ptr, width, height, line); break;
	  case LFRM_BPP_24: writeFrame24Png24(frame, png_ptr, width, height, line); break;
	  case LFRM_BPP_32:
	  case LFRM_BPP_32A: writeFrame32Png24(frame, png_ptr, width, height, line); break;
	  default: ret = 0;
	}

	l_free(line);
	return ret;
}
#endif		

#if (__BUILD_PNG_READ_SUPPORT__)

int readPngToFrame (TFRAME *frame, png_structp *png_ptr, int flags, int width, int height, int ox, int oy, int passCount)
{
	ubyte *line = (ubyte*)l_malloc((width * 4) + 4);		// 4 for RGBA
	if (line == NULL){
		return 0;
	}

	int ret = 1;
	switch (frame->bpp){
	  case LFRM_BPP_1: readPng32To1(frame, png_ptr, width, height, line, ox, oy, passCount); break;
	  case LFRM_BPP_8: readPng32To8(frame, png_ptr, width, height, line, ox, oy, passCount); break;
	  case LFRM_BPP_12: readPng32To12(frame, png_ptr, width, height, line, ox, oy, passCount); break;
	  case LFRM_BPP_15: readPng32To16_555(frame, png_ptr, width, height, line, ox, oy, passCount); break;
	  case LFRM_BPP_16: readPng32To16_565(frame, png_ptr, width, height, line, ox, oy, passCount); break;
	  case LFRM_BPP_24: readPng32To24(frame, png_ptr, width, height, line, ox, oy, passCount); break;
	  case LFRM_BPP_32: readPng32To32(frame, png_ptr, width, height, line, ox, oy, passCount); break;
	  case LFRM_BPP_32A:
		if (flags & LOAD_PIXEL_CPY)
			readPng32To32A_clone(frame, png_ptr, width, height, line, ox, oy, passCount);
		else
			readPng32To32A(frame, png_ptr, width, height, line, ox, oy, passCount);
		break;
	  default: ret = 0;
	}

	l_free(line);
	return ret;
}

static inline void PNGAPI warningCallback (png_structp png_ptr, png_const_charp warning_msg)
{
	char *err = png_get_error_ptr(png_ptr);
	if (err){
		mylog("libmylcd: png warning (%s), %s\n",err, (intptr_t)warning_msg);
	}else{
		mylog("libmylcd: png warning %s\n",(intptr_t)warning_msg);
	}
}

#endif

int pngGetMetrics (const wchar_t *filename, int *width, int *height, int *bpp)
{
#if (__BUILD_PNG_READ_SUPPORT__)

	unsigned int sig_read = 0;
	png_uint_32 pngWidth, pngHeight;
	int bit_depth, color_type, interlace_type;
	FILE *fp;

	if ((fp = l_wfopen(filename, L"rb")) == NULL)
		return 0;
	
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		l_fclose(fp);
		return 0;
	}

	//png_set_crc_action(png_ptr, PNG_CRC_QUIET_USE, PNG_CRC_QUIET_USE);
	png_set_error_fn(png_ptr, (png_voidp) NULL, (png_error_ptr) warningCallback, warningCallback);
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		l_fclose(fp);
		return 0;
	}

   if (setjmp(png_jmpbuf(png_ptr))){
      /* Free all of the memory associated with the png_ptr and info_ptr */
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      l_fclose(fp);
      /* If we get here, we had a problem reading the file */
      return 0;
   }

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, sig_read);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &pngWidth, &pngHeight, &bit_depth, &color_type, &interlace_type, NULL, NULL);
	if (width) *width = pngWidth;
	if (height) *height = pngHeight;
	if (bpp) *bpp = color_type;
	
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	l_fclose(fp);
	return 1;
#else
	return 0;
#endif
}

int loadPng (TFRAME *frame, const int flags, const wchar_t *filename, int ox, int oy)
{
#if (__BUILD_PNG_READ_SUPPORT__)

	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;
	png_uint_32 width, height;
	FILE *fp;

	if ((fp = l_wfopen(filename, L"rb")) == NULL)
		return 0;
	
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		l_fclose(fp);
		return 0;
	}

	png_set_crc_action(png_ptr, PNG_CRC_QUIET_USE, PNG_CRC_QUIET_USE);
	png_set_error_fn(png_ptr, (png_voidp) NULL, (png_error_ptr) warningCallback, warningCallback);
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		l_fclose(fp);
		return 0;
	}

   if (setjmp(png_jmpbuf(png_ptr))){
      /* Free all of the memory associated with the png_ptr and info_ptr */
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      l_fclose(fp);
      /* If we get here, we had a problem reading the file */
      return 0;
   }

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, sig_read);
	png_read_info(png_ptr, info_ptr);

	int bit_depth, color_type, interlace_type;
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);
	
	int passCount = 1;
	if (interlace_type == PNG_INTERLACE_ADAM7)
		passCount = png_set_interlace_handling(png_ptr);
	//printf("interlace_type %i, ct:%i, %i %i\n", interlace_type, passCount, width, height);

	
	if (bit_depth == 16)
		png_set_strip_16(png_ptr);
	png_set_packing(png_ptr);

	if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png_ptr);
	else if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_expand_gray_1_2_4_to_8(png_ptr);

	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png_ptr);
	png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
	
	if (flags&LOAD_RESIZE){
		if (!_resizeFrame(frame, width, height, 0))
			return 0;
	}

	int ret = readPngToFrame(frame, &png_ptr, flags, width, height, ox, oy, passCount);
	//png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	l_fclose(fp);
	return ret;
#else
	return 0;
#endif
}

int savePng (TFRAME *frame, const wchar_t *filename, int width, int height, int flags)
{
#if (__BUILD_PNG_WRITE_SUPPORT__)

	png_structp png_ptr;
	png_infop info_ptr;
	FILE* fp;

	if ((fp = l_wfopen(filename, L"wb")) == NULL)
		return 0;
		
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr){
		l_fclose(fp);
		return 0;
	}
	
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr){
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		l_fclose(fp);
		return 0;
	}
	
	png_init_io(png_ptr, fp);
	png_set_compression_level(png_ptr, /*Z_BEST_SPEED*/9);
	int ret;
	
	if (frame->bpp == LFRM_BPP_32A && flags&IMG_KEEPALPHA){
		png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGBA,\
		  PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
		png_write_info(png_ptr, info_ptr);
		ret = writeFrame32AToPng32A(frame, &png_ptr, width, height);
	}else{
		png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB,\
		  PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
		png_write_info(png_ptr, info_ptr);
		ret = writeFrameToPng24(frame, &png_ptr, width, height);
	}

	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
	l_fclose(fp);
	return ret;
#else
	return 0;
#endif
}



#else

int loadPng (TFRAME *frame, const int flags, const wchar_t *filename, int ox, int oy) {return 0;}
int savePng (TFRAME *frame, const wchar_t *filename, int width, int height, int flags){return 0;}
int pngGetMetrics (const wchar_t *filename, int *width, int *height, int *bpp){return 0;}

#endif


