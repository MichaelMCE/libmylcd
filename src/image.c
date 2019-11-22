
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
#include "memory.h"
#include "frame.h"
#include "fileio.h"
#include "lstring.h"
#include "utils.h"
#include "image.h"
#include "pixel.h"
#include "copy.h"
#include "bmp.h"
#include "tga.h"
#include "pgm.h"
#include "lpng.h"
#include "ljpg.h"
#include "gif.h"
#include "psd/psd.h"
#include "icon.h"

#include "sync.h"
#include "misc.h"

static inline int _loadImage (TFRAME *frame, const int flags, const wchar_t *filename, const int type, const int x, const int y);
static inline int _saveImage (TFRAME *frame, const wchar_t *filename, const int type, const int w, const int h);

static inline int saveRaw (TFRAME *frm, const wchar_t *filename);
static inline int loadRaw (TFRAME *frm, int flags, const wchar_t *filename, int x, int y);
static inline int guessImageType (const wchar_t *filename);


	

int saveImage (TFRAME *frame, const wchar_t *filename, const int img_type, int w, int h)
{

	if (filename == NULL){
		mylog("libmylcd: invalid filename\n");
		return -1;
	}
	if (frame == NULL){
		mylog("libmylcd: invalid input surface\n");
		return -1;
	}
	if (w < 1) w = frame->width;
	if (h < 1) h = frame->height;
	w = MAX(MINFRAMESIZEW, w);
	h = MAX(MINFRAMESIZEH, h);
	return _saveImage(frame, filename, img_type, w, h);
}

TFRAME *newImage (THWD *hw, const wchar_t *filename, const int frameBPP)
{
	if (filename == NULL){
		mylog("libmylcd: invalid filename\n");
		return NULL;
	}
	if (hw == NULL){
		mylog("libmylcd: invalid device handle\n");
		return NULL;
	}
	return _newImage(hw, filename, guessImageType(filename), frameBPP);
}
	
int loadImage (TFRAME *frame, const wchar_t *filename)
{
	if (filename == NULL){
		mylog("libmylcd: invalid filename\n");
		return -1;
	}
	if (frame == NULL){
		mylog("libmylcd: invalid input surface\n");
		return -1;
	}
	return _loadImage(frame, LOAD_RESIZE|LOAD_PIXEL_CPY, filename, guessImageType(filename), 0, 0);
}

int loadImageEx (TFRAME *frame, const wchar_t *filename, const int flags, const int ox, const int oy)
{
	if (filename == NULL){
		mylog("libmylcd: invalid filename\n");
		return -1;
	}
	if (frame == NULL){
		mylog("libmylcd: invalid input surface\n");
		return -1;
	}
	return _loadImage(frame, flags, filename, guessImageType(filename), ox, oy);
}

static inline int extMatch (const wchar_t *string, const wchar_t *ext)
{
	const int len = l_wcslen(string)-3;

	for (int i = len; i > len-4 && string[i]; i--){
		if (string[i] == ext[0] || string[i]-32 == ext[0]){
			if (string[i+1] == ext[1] || string[i+1]-32 == ext[1]){
				if (string[i+2] == ext[2] || string[i+2]-32 == ext[2]){
					if (string[i+3] == ext[3] || string[i+3]-32 == ext[3])
						return 1;
				}
			}
		}
	}
	return 0;
}

static inline int guessImageType (const wchar_t *filename)
{
    int type = -1;
   	FILE *fp = l_wfopen(filename, L"rb");
   	if (!fp) return 0;	

	if (l_lof(fp) > 16){
    	ubyte data[16] = {0};
    	l_memset(data, 0, sizeof(data));
    	l_fread(&data, sizeof(data), 1, fp);
    	l_fclose(fp);
        
        //printf("guess %i %i %i %i\n", data[0], data[1], data[2], data[3]);

    	if (data[0] == 0x89 && data[1] == 'P' && data[2] == 'N' && data[3] == 'G'){
	    	type = IMG_PNG;
    	}else if (data[0] == 0xFF && data[1] == 0xD8 && data[2] == 0xFF){
	    	type = IMG_JPG;
		}else if (data[0] == 'G' && data[1] == 'I' && data[2] == 'F'){
			type = IMG_GIF;
	    }else if (data[0] == 'B' && data[1] == 'M'){
    		type = IMG_BMP;
	    }else if (data[0] == 'P' && data[1] == '5' && (data[2] == 0x0A || data[2] == 0x0D)){
    		type = IMG_PGM;
		}else if (data[0] == 'P' && (data[1] == '3' || data[1] == '5' || data[1] == '6') && (data[2] == 0x0A || data[2] == 0x0D)){
    		type = IMG_PPM;
		}else if (data[0] == 'P' && data[1] == 'F' && (data[2] == 0x0A || data[2] == 0x0D)){
    		type = IMG_PBM;
		}else if (data[0] == '8' && data[1] == 'B' && data[2] == 'P' && data[3] == 'S'){
			type = IMG_PSD;
    	}else if (data[3] == 1 || data[3] == 9 || data[3] == 2 || data[3] == 3 || data[3] == 10 || data[3] == 11){
	    	type = IMG_TGA;
    	}else if (extMatch(filename, L".bmp")){
	    	type = IMG_BMP;
		/*}else if (extMatch(filename, L".png")){
    		type = IMG_PNG;
		}else if (extMatch(filename, L".jpg")){
	    	type = IMG_JPG;*/
		}else if (extMatch(filename, L".tga")){
    		type = IMG_TGA;
    	}else if (extMatch(filename, L".pgm")){
    		type = IMG_PGM;
    	}else if (extMatch(filename, L".exe") || extMatch(filename, L".ico") || extMatch(filename, L".dll")){
    		type = IMG_ICO;
    	//}else if (extMatch(filename, L".psd")){
    	//	type = IMG_PSD;
    	//}else if (extMatch(filename, L".gif")){
    	//	type = IMG_GIF;
    	/*}else if (extMatch(filename, L".raw")){
	    	type = IMG_RAW;*/
	    }
	}else{
	    l_fclose(fp);
	}

    if (type == -1){
    	mylogw(L"libmylcd: format of file not recognized '%s'", filename);
    	mylogw(L"\n");
    }
    
	return type;
}

int imageGetMetrics (const wchar_t *filename, int type, int *width, int *height, int *bpp)
{
	if (type == -1){
		type = guessImageType(filename);
		if (type == -1) return 0;
	}
	
	int ret = 0;
	switch (type){
	  case IMG_PNG: ret = pngGetMetrics(filename, width, height, bpp); break;
	  case IMG_JPG: ret = jpgGetMetrics(filename, width, height, bpp); break;
	  case IMG_BMP: ret = bmpGetMetrics(filename, width, height, bpp); break;
	  case IMG_TGA: ret = tgaGetMetrics(filename, width, height, bpp); break;
	  case IMG_GIF: ret = gifGetMetrics(filename, width, height, bpp); break;
	  case IMG_PSD: ret = psdGetMetrics(filename, width, height, bpp); break;
	  case IMG_PPM: ret = ppmGetMetrics(filename, width, height, bpp); break;
	  case IMG_PGM: ret = pgmGetMetrics(filename, width, height, bpp); break;
	  case IMG_PBM: ret = pbmGetMetrics(filename, width, height, bpp); break;
	  case IMG_ICO: ret = icoGetMetrics(filename, width, height, bpp); break;
	  default:
	  	ret = 0;
	}
	
	return ret;
}



#ifdef __DEBUG_SHOWFILEIO__
#define DUMPIO(f,t) \
	mylogw(L"libmylcd: loadImage() "t": '%s'", f); \
	mylog("\n");
#else
#define DUMPIO(f,t)
#endif

static inline int _loadImage (TFRAME *frame, const int flags, const wchar_t *filename, const int type, const int ox, const int oy)
{
	
	int ret = 0;

	if (type == IMG_PNG){
		DUMPIO(filename,"PNG");
		ret = loadPng(frame, flags, filename, ox, oy);
	}else if (type == IMG_JPG){
		DUMPIO(filename,"JPG");
		ret = loadJpg(frame, flags, filename, ox, oy);
	}else if (type == IMG_TGA){
		DUMPIO(filename,"TGA");
		ret = loadTga(frame, flags, filename, ox, oy);
	}else if (type == IMG_BMP){
		DUMPIO(filename,"BMP");
		ret = loadBmp(frame, flags, filename, ox, oy);
	}else if (type == IMG_GIF){
		DUMPIO(filename,"GIF");
		ret = loadGif(frame, flags, filename, ox, oy);
	}else if (type == IMG_PSD){
		DUMPIO(filename,"PSD");
		ret = loadPsd(frame, flags, filename, ox, oy);
	}else if (type == IMG_PGM/* && frame->bpp == LFRM_BPP_1*/){
		DUMPIO(filename,"PGM");
		ret = loadPgm(frame, flags, filename, ox, oy);
	}else if (type == IMG_PPM){
		DUMPIO(filename,"PPM");
		ret = loadPpm(frame, flags, filename, ox, oy);
	}else if (type == IMG_PBM){
		DUMPIO(filename,"PBM");
		ret = loadPbm(frame, flags, filename, ox, oy);
	
	}else if (type == IMG_ICO){
		DUMPIO(filename,"ICO");
		ret = loadIco(frame, flags, filename, ox, oy);
	
	}else if (0 && type == IMG_RAW){
		DUMPIO(filename,"RAW");
		ret = loadRaw(frame, flags, filename, ox, oy);
	}else{
		mylogw(L"libmylcd: loadImage(): unsupported: '%s'", filename); mylog("\n");
		ret = 0;
	}
	return ret;
}

TFRAME *_newImage (THWD *hw, const wchar_t *filename, const int type, const int frameBPP)
{
	TFRAME *frame = _newFrame(hw, 8, 8, 1, frameBPP);
	if (frame != NULL){
		if (_loadImage(frame, LOAD_RESIZE|LOAD_PIXEL_CPY, filename, type, 0, 0) > 0){
			return frame;
		}
		deleteFrame(frame);
	}
	return NULL;
}

static inline int _saveImage (TFRAME *frame, const wchar_t *filename, const int type, const int w, const int h)
{	
	int ret;
	
	switch(type&0xFF){
	  case IMG_PNG:
	  	mylogw(L"libmylcd: lSaveImage() PNG,%ix%i: '%s'", w, h, filename); mylog("\n");
		ret = savePng(frame, filename, w, h, type&IMG_KEEPALPHA);
		break;
	  case IMG_JPG:
	  	mylogw(L"libmylcd: lSaveImage() JPG,%ix%i: '%s'", w, h, filename); mylog("\n");
	  	ret = saveJpg(frame, filename, w, h, 90);
		break;
	  case IMG_BMP:
	  	mylogw(L"libmylcd: lSaveImage() BMP,%ix%i: '%s'", w, h, filename); mylog("\n");
		ret = saveBmp(frame, filename, w, h);
		break;
	  case IMG_TGA:
	  	mylogw(L"libmylcd: lSaveImage() TGA,%ix%i: '%s'", w, h, filename); mylog("\n");
		ret = saveTga(frame, filename, w, h);
		break;
	  case IMG_PGM:
	  	mylogw(L"libmylcd: lSaveImage() PGM,%ix%i: '%s'", w, h, filename); mylog("\n");
		ret = savePgm(frame, filename, w, h);
		break;
	  case IMG_PPM:
	  	mylogw(L"libmylcd: lSaveImage() PPM,%ix%i: '%s'", w, h, filename); mylog("\n");
		ret = savePpm(frame, filename, w, h);
		break;
	  case IMG_GIF:
	  	mylogw(L"libmylcd: lSaveImage() GIF,%ix%i: '%s'", w, h, filename); mylog("\n");
		ret = saveGif(frame, filename, w, h, 0);
		break;
	  case IMG_PSD:
	  	mylogw(L"libmylcd: lSaveImage() PSD,%ix%i: '%s'", w, h, filename); mylog("\n");
		ret = savePsd(frame, filename, w, h, 0);
		break;
	  case IMG_RAW:
		mylogw(L"libmylcd: lSaveImage() RAW,%ix%i: '%s'", w, h, filename); mylog("\n");
		ret = saveRaw(frame, filename);
		break;
	  case IMG_ICO:		// not implemented
	  default: 
	  	ret = -2;
	}
	return ret;
}

static inline int loadRaw (TFRAME *frm, int flags, const wchar_t *filename, int x, int y)
{
	FILE *fp = NULL;
	unsigned int flen=0;
	ubyte *buffer=NULL;
	
    if (!(fp=l_wfopen(filename, L"r")))
    	return 0;
    
    l_fseek(fp,0, SEEK_END);
	if (!(flen=l_ftell(fp))){
		l_fclose (fp);
		return 0;
	}
	if (!(buffer=(ubyte*)l_malloc(flen*sizeof(ubyte*)))){
		l_fclose (fp);
		return 0;
	}
	unsigned int bread = l_fread(buffer,1,flen,fp);
	l_fclose(fp);
	if (bread != flen){
		l_free (buffer);
		return 0;
	}
	l_memcpy(frm->pixels, buffer, MIN(frm->frameSize, flen));
	l_free(buffer);
	return (bread);
}


static inline int saveRaw (TFRAME *frm, const wchar_t *filename)
{
	if (!frm || !filename)
		return 0;
	
    FILE *fp = l_wfopen(filename, L"wb");
    if (!fp)
    	return 0;

    size_t bwritten = l_fwrite(frm->pixels, 1, frm->frameSize, fp);
	l_fclose(fp);
    if (bwritten < frm->frameSize)
    	return 0;
	else
		return (int)bwritten;
}

