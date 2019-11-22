
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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mylcd.h"

#if (__BUILD_PGM_SUPPORT__)

#include "memory.h"
#include "fileio.h"
#include "utils.h"
#include "lstring.h"
#include "frame.h"
#include "pixel.h"
#include "misc.h"
#include "pgm.h"
#include "image.h"



static inline int pamGetHeaderInfo (FILE *f, int *type, int *width, int *height, void *maxval);


#define lineLen 127
	
static inline int getline (FILE *f, char *text, unsigned int len)
{
	text[0] = 0;
	if (fgets(text, len, f)){
		while(text[0] == '#') return getline(f, text, len);
		text[l_strlen(text)] = 0;
		return 1;
	}
	return 0;
}

static inline int pgmGetHeaderInfo (FILE *f, char *type, int *width, int *height, void *maxval)
{

	/*int _type;
	int _width;
	int _height;
	int _maxval;*/
	
	//pamGetHeaderInfo(f, &_type, &_width, &_height, &_maxval);
	l_fseek(f, 0, 0);

	char text[lineLen+1] = {0};
	
	getline(f, text, lineLen);
	if (!l_strncmp(text, type, 2)){
		
		getline(f, text, lineLen);
		if (type[1] == 'F'){
			sscanf(text,"%i", width);
			getline(f, text, lineLen);
			sscanf(text,"%i", height);
			if (*width && *height){
				getline(f, text, lineLen);
				sscanf(text,"%f", (float*)maxval);
				return 1;
			}
		}else{
			sscanf(text,"%i %i", width, height);
			if (*width && *height){
				// get current position
				// get line
				// check if line is a maxval, if not then reset current position then treat data as if pixel data
				
				
				getline(f, text, lineLen);
				sscanf(text,"%i", (int*)maxval);
				return 1;
			}
		}
	}
	return 0;
}

int savePgm (TFRAME *frame, const wchar_t *filename, int width, int height)
{
	FILE *fp = l_wfopen(filename, L"wb");
	if (!fp){
		return 0;
	}

	ubyte *pgmdata = l_malloc(height * width);
	if (!pgmdata ){
		l_fclose(fp);
		return 0;
	}

	const double yscale = frame->height / (double)height;
	const double xscale = frame->width / (double)width;
	int pos = 0;
	
	for (double y = 0; y<(double)frame->height; y+=yscale){
		for (double x = 0; x<(double)frame->width; x+=xscale){
			if (l_getPixel_NB(frame,x,y))
				*(pgmdata+pos++) = lPGMPIXELSET;
			else
				*(pgmdata+pos++) = lPGMPIXELCLEAR;
		}
	}
	
	fprintf(fp,"P5\n%i %i\n%i\n",width,height,lPGMPIXELTOTAL);
	l_fwrite(pgmdata, height * width, 1, fp);
	l_free(pgmdata);
	l_fclose(fp);
	return 1;	
}

int loadPgm (TFRAME *frame, const int flags, const wchar_t *filename, int ox, int oy)
{
	FILE *f = l_wfopen(filename, L"r");
	if (!f) return 0;

	if (flags&LOAD_SIZE_RESTRICT){
		ox = 0;
		oy = 0;
	}
	
	int width, height, maxval;
	
	int ret = pgmGetHeaderInfo(f, "P5", &width, &height, &maxval);
	if (!ret || !width || !height || !maxval){
		l_fclose(f);
		return 0;
	}
	
	ubyte *pgmStorage = (ubyte*)l_malloc(height * width);
	if (!pgmStorage){
		l_fclose(f);
		return 0;
	}
	
	l_fread(pgmStorage,1,height*width,f);
	l_fclose(f);
	
	if (flags & LOAD_RESIZE)
		_resizeFrame(frame, width, height, 0);
	
	int i = 0;
	maxval = (maxval>>1)+1;

	if (frame->bpp == LFRM_BPP_1){
		for (int y = 0; y < frame->height; y++){
			for (int x = 0; x < frame->width; x++){
				if (pgmStorage[i++] >= maxval)
					l_setPixel(frame, x+ox, y+oy, LSP_SET);
			}
		}
	}else if (frame->bpp == LFRM_BPP_32 || frame->bpp == LFRM_BPP_32A || frame->bpp == LFRM_BPP_24){
		//const int colour = 0xFF000000 | maxval<<16 | maxval<<8 | maxval;
		const int ink = getForegroundColour(frame->hw);
		const int back = 0; //lGetBackgroundColour(frame->hw);
		
		for (int y = 0; y < frame->height; y++){
			for (int x = 0; x < frame->width; x++){
				if (!pgmStorage[i++] /*>= maxval*/)
					l_setPixel(frame, x+ox, y+oy, ink);
				else
					l_setPixel(frame, x+ox, y+oy, back);
			}
		}
	}
	l_free(pgmStorage);
	return 1;
	
}


int loadPpm (TFRAME *frame, const int flags, const wchar_t *filename, int ox, int oy)
{
	//wprintf(L"ppmLoad '%s'\n", filename);
	
	FILE *f = l_wfopen(filename, L"r+b");
	if (!f) return 0;

	if (flags&LOAD_SIZE_RESTRICT){
		ox = 0;
		oy = 0;
	}

	int width, height, maxval, isP3 = 1;
	
	const int isP6 = pgmGetHeaderInfo(f, "P6", &width, &height, &maxval);
	if (!isP6){
		l_fseek(f, 0, 0);
		isP3 = pgmGetHeaderInfo(f, "P3", &width, &height, &maxval);
	}
	//printf("pgmGetHeaderInfo %i/%i: %i %i %i\n", isP6, isP3, width, height, maxval);
	
	if (!isP3 || !width || !height || !maxval || maxval > 255){
		l_fclose(f);
		return 0;
	}
	
	uint64_t len = l_lof(f);
	ubyte *storage = (ubyte*)l_malloc(len+16);
	ubyte *ppmStorage = storage;
	if (!ppmStorage){
		l_fclose(f);
		return 0;
	}
	
	
	if (flags & LOAD_RESIZE)
		_resizeFrame(frame, width, height, 0);

	if (isP6){
		l_fread(ppmStorage, 1, len-l_ftell(f), f);
		l_fclose(f);
			
		for (int y = 0; y < frame->height; y++){
			for (int x = 0; x < frame->width; x++){
				int r = ppmStorage[0]<<16;
				int g = ppmStorage[1]<<8;
				int b = ppmStorage[2];
				l_setPixel(frame, x+ox, y+oy, 0xFF000000 | r | g | b);
				ppmStorage += 3;
			}
		}
	}else if (isP3){
		unsigned char r,g,b;
		uint64_t pos = l_ftell(f);
		size_t bread = l_fread(ppmStorage, 1, len-pos, f);
		l_fclose(f);
		if (bread < 3){
			l_free(storage);
			return 0;
		}
		
		ubyte *ppmStorageEnd = ppmStorage + len - pos;
		
		for (int y = 0; y < height; y++){
			for (int x = 0; (x < width) && ppmStorage < ppmStorageEnd; x++){
				while(isspace(*ppmStorage)) ppmStorage++;
				r = l_atoi((char*)ppmStorage++);				
				while(isalnum(*ppmStorage)) ppmStorage++;
				if (*ppmStorage == '\r' || *ppmStorage == '\n'){
					if (++ppmStorage < ppmStorageEnd-1){
						//break;
					}else{
						l_free(storage);
						return 0;
					}
					while(isspace(*ppmStorage)) ppmStorage++;
				}
								
				while(isspace(*ppmStorage)) ppmStorage++;
				g = l_atoi((char*)ppmStorage++);
				while(isalnum(*ppmStorage)) ppmStorage++;
				if (*ppmStorage == '\r' || *ppmStorage == '\n'){
					if (++ppmStorage < ppmStorageEnd-1){
						//break;
					}else{
						l_free(storage);
						return 0;
					}
					while(isspace(*ppmStorage)) ppmStorage++;
				}
												
				while(isspace(*ppmStorage)) ppmStorage++;
				b = l_atoi((char*)ppmStorage++);
				while(isalnum(*ppmStorage)) ppmStorage++;
				if (*ppmStorage == '\r' || *ppmStorage == '\n'){
					if (++ppmStorage < ppmStorageEnd-1){
						//break;
					}else{
						l_free(storage);
						return 0;
					}
					while(isspace(*ppmStorage)) ppmStorage++;
				}
								
				//printf("%i/%i, %i %i %i\n", x, y, r, g, b);
				l_setPixel(frame, x+ox, y+oy, 0xFF000000 | r<<16 | g<<8 | b);
				//ppmStorage += ret;
			}
		}
	}

	l_free(storage);
	return 1;
	
}

int savePpm (TFRAME *frame, const wchar_t *filename, const int width, const int height)
{
	FILE *out = l_wfopen(filename, L"wb");
	if (!out) return 0;

  
	fprintf(out, "P6\n");  
	fprintf(out, "%d %d\n%d\n", width, height, 255); 
  
	ubyte *ptr = (ubyte*)frame->pixels;
	const double yscale = frame->height / (double)height;
	const double xscale = frame->width / (double)width;
	
	for (double y = 0; y<(double)frame->height; y+=yscale){
		const double row = width * y;
		
		for (double x = 0; x<(double)frame->width; x+=xscale){
			ubyte b = *(ptr+4*(unsigned int)(row+x));  
			ubyte g = *(ptr+4*(unsigned int)(row+x)+1);
			ubyte r = *(ptr+4*(unsigned int)(row+x)+2);
			fprintf(out, "%c%c%c", r, g, b);
		}
	}
	
	fprintf(out, "\n");
	l_fclose(out);
	return 1;
}


int loadPbm (TFRAME *frame, const int flags, const wchar_t *filename, int ox, int oy)
{
	FILE *f = l_wfopen(filename, L"r+b");
	if (!f) return 0;

	if (flags&LOAD_SIZE_RESTRICT){
		ox = 0;
		oy = 0;
	}

	int width, height;
	float maxval;
	
	const int isPf = pgmGetHeaderInfo(f, "PF", &width, &height, &maxval);
	//printf("%i %i %i %f\n", isPf, width, height, maxval);

	if (!isPf || !width || !height){
		l_fclose(f);
		return 0;
	}
	
	uint64_t len = l_lof(f);
	if (len < width*height*sizeof(float)*3){
		l_fclose(f);
		return 0;
	}	
	ubyte *storage = (ubyte*)l_malloc(len+16);
	float *pbmStorage = (float*)storage;
	if (!pbmStorage){
		l_fclose(f);
		return 0;
	}
	
	
	if (flags & LOAD_RESIZE)
		_resizeFrame(frame, width, height, 0);

	l_fread(pbmStorage, 1, len-l_ftell(f), f);
	l_fclose(f);
			
	//printf("%i %i, %f, %f %f %f\n", width, height, maxval, pbmStorage[0], pbmStorage[1], pbmStorage[2]);
			
	const float gain = 255.0f * 1.00f;
	
	for (int y = 0; y < frame->height; y++){
		for (int x = 0; x < frame->width; x++){
			int r = (int)(pbmStorage[0]*gain);
			if (r > 255) r = 255;
			else if (r < 0) r = 0;
			
			int g = (int)(pbmStorage[1]*gain);
			if (g > 255) g = 255;
			else if (g < 0) g = 0;
			
			int b = (int)(pbmStorage[2]*gain);
			if (b > 255) b = 255;
			else if (b < 0) b = 0;
			
			l_setPixel(frame, x+ox, y+oy, 0xFF000000 | (r<<16) | (g<<8) | b);
			pbmStorage += 3;
		}
	}

	l_free(storage);
	return 1;
	
}

int savePbm (TFRAME *frame, const wchar_t *filename, const int width, const int height)
{
	// TODO
	return 0;
}
		
		
int pgmGetMetrics (const wchar_t *filename, int *_width, int *_height, int *bpp)
{
	//wprintf(L"pgmGetMetrics '%s'\n", filename);
	
	if (bpp) *bpp = LFRM_BPP_32;
	
	FILE *f = l_wfopen(filename, L"r");
	if (!f) return 0;

	int width, height, maxval;
	int ret = pgmGetHeaderInfo(f, "P5", &width, &height, &maxval);
	l_fclose(f);
	
	if (_width) *_width = width;
	if (_height) *_height = height;
	
	//wprintf(L"pgmGetMetrics %i %i", width, height);
	
	return ret;
}

int ppmGetMetrics (const wchar_t *filename, int *_width, int *_height, int *bpp)
{
	//wprintf(L"ppmGetMetrics '%s'\n", filename);
	
	if (bpp) *bpp = LFRM_BPP_32;
	
	FILE *f = l_wfopen(filename, L"r");
	if (!f) return 0;

	int width, height, maxval;
	int ret = pgmGetHeaderInfo(f, "P3", &width, &height, &maxval);
	if (!ret){
		l_fseek(f, 0, 0);
		ret = pgmGetHeaderInfo(f, "P6", &width, &height, &maxval);
	}
	l_fclose(f);
	
	if (_width) *_width = width;
	if (_height) *_height = height;
	
	//wprintf(L"ppmGetMetrics %i %i\n", width, height);
	
	return ret;
}

int pbmGetMetrics (const wchar_t *filename, int *_width, int *_height, int *bpp)
{
	//wprintf(L"pgmGetMetrics '%s'\n", filename);
	
	if (bpp) *bpp = LFRM_BPP_32;
	
	FILE *f = l_wfopen(filename, L"r");
	if (!f) return 0;

	int width, height, maxval;
	int ret = pgmGetHeaderInfo(f, "PF", &width, &height, &maxval);
	l_fclose(f);
	
	if (_width) *_width = width;
	if (_height) *_height = height;
	
	//wprintf(L"pbmGetMetrics %i %i", width, height);
	
	return ret;
}

static inline int pamGetHeaderInfo (FILE *f, int *type, int *width, int *height, void *maxval)
{

	char buffer[lineLen+1] = {0};
	
	
	getline(f, buffer, lineLen);
	
	if (l_strlen(buffer) != 3){
		printf("invalid header ident %i \n", (int)l_strlen(buffer));
		return 0;
	}
	
	*type = 0;
	*width = 0;
	*height = 0;
	//if (maxval) *(int*)maxval = 0;
	
	if (buffer[0] == 'P' && (buffer[2] == '\r' || buffer[2] == '\n')){
		if (buffer[1] == '1')
			*type = PAM_P1;
		else if (buffer[1] == '2')
			*type = PAM_P2;
		else if (buffer[1] == '3')
			*type = PAM_P3;
		else if (buffer[1] == '4')
			*type = PAM_P4;
		else if (buffer[1] == '5')
			*type = PAM_P5;
		else if (buffer[1] == '6')
			*type = PAM_P6;
		else if (buffer[1] == 'F')		// produced via photoshop (32bit colour mode then 'save as')
			*type = PAM_PF;
	}
	
	if (!*type){
		printf("invalid header marker\n");
		return 0;
	}

	printf("type: %i\n", *type);
	
	getline(f, buffer, lineLen);
	sscanf(buffer,"%i %i", width, height);
	if (!*height){
		getline(f, buffer, lineLen);
		sscanf(buffer,"%i", height);
	}
	
	
	if (!*width || !*height){
		printf("invalid width/height %i %i\n", *width, *height);
		return 0;
	}

	printf("width/height: %i %i\n", *width, *height);
			
	getline(f, buffer, lineLen);
	if (maxval){
		if (*type != PAM_PF){
			sscanf(buffer,"%i", (int*)maxval);
			printf("max: %i\n", *(int*)maxval);
		}else{
			sscanf(buffer,"%f", (float*)maxval);
			printf("max: %f\n", *(float*)maxval);
		}
	}
	
	
	return 1;

}

int pamReadFloatRGB (TFRAME *frame, float *storage, const int swidth, const int sheight, const float maxval, const int ox, const int oy)
{
	
	const float gain = 1.00f * 255.0f;
	const int pitch = swidth * sizeof(float);
	const int width = MIN(swidth, frame->width);
	const int height = MIN(sheight, frame->height);
	
	for (int y = 0; y < height; y++){
		float *row = storage + (y*pitch);
		
		for (int x = 0; x < width; x++){
			int r = (int)(row[0]*gain);
			if (r > 255) r = 255;
			else if (r < 0) r = 0;
			
			int g = (int)(row[1]*gain);
			if (g > 255) g = 255;
			else if (g < 0) g = 0;
			
			int b = (int)(row[2]*gain);
			if (b > 255) b = 255;
			else if (b < 0) b = 0;
			
			l_setPixel(frame, x+ox, y+oy, 0xFF000000 | (r<<16) | (g<<8) | b);
			row += 3;
		}
	}
	return 1;
}

int pamReadInt32RGB (TFRAME *frame, int *storage, const int swidth, const int sheight, const float maxval, const int ox, const int oy)
{
	const int pitch = swidth * sizeof(uint32_t);
	const int width = MIN(swidth, frame->width);
	const int height = MIN(sheight, frame->height);
	
	for (int y = 0; y < height; y++){
		int *row = storage + (y*pitch);
		
		for (int x = 0; x < width; x++){
			int r = row[0]<<16;
			int g = row[1]<<8;
			int b = row[2];
			l_setPixel(frame, x+ox, y+oy, 0xFF000000 | r | g | b);
			row += 3;
		}
	}
	return 1;
}

int pamReadInt8Gray (TFRAME *frame, uint8_t *storage, const int swidth, const int sheight, const float maxval, const int ox, const int oy)
{
	const int pitch = swidth * sizeof(uint8_t);
	const int width = MIN(swidth, frame->width);
	const int height = MIN(sheight, frame->height);
	const float gain = 255.0f / maxval;

	for (int y = 0; y < height; y++){
		unsigned char *row = storage + (y*pitch);
		
		for (int x = 0; x < width; x++){
			uint8_t pixel = (int)((float)row[x] * gain)&0xFF;
			int r = pixel << 16;
			int g = pixel << 8;
			int b = pixel;
			l_setPixel(frame, x+ox, y+oy, 0xFF000000 | r | g | b);
		}
	}

	return 1;
}

int pamReadInt1BW (TFRAME *frame, uint8_t *storage, const int swidth, const int sheight, const float maxval, const int ox, const int oy)
{
	const int pitch = swidth * sizeof(uint8_t);
	const int width = MIN(swidth, frame->width);
	const int height = MIN(sheight, frame->height);
	const float gain = 255.0f / maxval;

	for (int y = 0; y < height; y++){
		unsigned char *row = storage + (y*pitch);
		
		for (int x = 0; x < width; x++){
			uint8_t pixel = (int)((float)row[x] * gain)&0xFF;
			int r = pixel << 16;
			int g = pixel << 8;
			int b = pixel;
			l_setPixel(frame, x+ox, y+oy, 0xFF000000 | r | g | b);
		}
	}

	return 1;
}

int loadPam (TFRAME *frame, const int flags, const wchar_t *filename, int ox, int oy)
{
	wprintf(L"loadPam '%s'\n", filename);
	
	FILE *f = l_wfopen(filename, L"r+b");
	if (!f) return 0;

	if (flags&LOAD_SIZE_RESTRICT){
		ox = 0;
		oy = 0;
	}

	int type, width, height, maxval;
	int readOk = pamGetHeaderInfo(f, &type, &width, &height, &maxval);
	if (!readOk) return 0;

	if (flags & LOAD_RESIZE)
		_resizeFrame(frame, width, height, 0);
		
		
	uint64_t len = l_lof(f);
	void *storage = (ubyte*)l_malloc(len+16);
	l_fread(storage, 1, len-l_ftell(f), f);
	
	switch (type){
	case PAM_P1: break;
	case PAM_P2: break;
	case PAM_P3: break;
	case PAM_P4:
		pamReadInt1BW(frame, storage, width, height, maxval, ox, oy);
		break;
	case PAM_P5:
		pamReadInt8Gray(frame, storage, width, height, maxval, ox, oy);
		break;
	case PAM_P6:
		pamReadInt32RGB(frame, storage, width, height, maxval, ox, oy);
		break;
	case PAM_PF:
		pamReadFloatRGB(frame, storage, width, height, maxval, ox, oy);
		break;
	}
	
	
	l_free(storage);
	l_fclose(f);
	return 1;
}



#else


int savePgm (TFRAME *frame, const wchar_t *filename, int width, int height){return 0;}
int loadPgm (TFRAME *frame, const wchar_t *filename, int style){return 0;}
int savePpm (TFRAME *frame, const wchar_t *filename, int width, int height){return 0;}
int loadPpm (TFRAME *frame, const wchar_t *filename, int style){return 0;}
int savePbm (TFRAME *frame, const wchar_t *filename, int width, int height){return 0;}
int loadPbm (TFRAME *frame, const wchar_t *filename, int style){return 0;}
int pbmGetMetrics (const wchar_t *filename, int *_width, int *_height, int *bpp){return 0;}
int ppmGetMetrics (const wchar_t *filename, int *_width, int *_height, int *bpp){return 0;}
int pgmGetMetrics (const wchar_t *filename, int *_width, int *_height, int *bpp){return 0;}

#endif

