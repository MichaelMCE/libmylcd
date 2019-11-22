
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
//  GNU LIBRARY GENERAL PUBLIC LICENSE for details.



#include "utils/utils.h"

	
char txt[] = "`他们为什么不说中文'";  //= 'why don't you speak chinese'


/*
typedef struct{
	unsigned int encoding;	// unicode reference code point
	int	xoffset;			// [bbx] x offset of glyph within frame
	int	yoffset;			// [bbx] y offset of glyph within frame
	int	w;					// [bbx] width of this glyph
	int	h;					// [bbx] height of this glyph
	int	dwidth;				// bdf 'DWIDTH' field
	TRECT box;	
	TGLYPHPOINTS *gp;
	TGLYPHPOINTS *ep[LTR_TOTAL];
}TWCHAR;
*/

/*
typedef struct{
	uint16_t encoding;		// unicode reference code point
	uint16_t dwidth;		// bdf 'DWIDTH' field
	uint8_t w;				// [bbx] width of this glyph
	uint8_t h;				// [bbx] height of this glyph
	int8_t xoffset;			// [bbx] x offset of glyph within frame
	int8_t yoffset;			// [bbx] y offset of glyph within frame
}_char_t;

*/
static TFRAME *glyphToFrame (THWD *hw, TWCHAR *g)
{
	TFRAME *surface = lNewFrame(hw, g->w, g->h, LFRM_BPP_24);
	lClearFrame(surface);
	
	TGLYPHPOINTS *gp = g->gp;
	const int ink = 0xFF<<24|lGetForegroundColour(hw);
	
	for (int i = 0; i < gp->pointsTotal; i++)
		lSetPixel_NB(surface, gp->points[i].x, gp->points[i].y, ink);
	return surface;
}

int main (int argc, char* argv[])
{

	if (!utilInitConfig("config.cfg"))
		return EXIT_FAILURE;

	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));
	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
	lClearFrame(frame);

	// lGetGlyph() will return NULL if requested code point (glyph) isn't available
	TWCHAR *wc = lGetGlyph(hw, "&euro;", 0, LFTW_B24);
	if (wc){
		TFRAME *glyph = glyphToFrame(hw, wc);
		if (glyph){
			lDrawRectangle(glyph, 0, 0, glyph->width-1, glyph->height-1, lGetRGBMask(glyph, LMASK_BLACK));
			lSaveImage(glyph, L"euro.tga", IMG_TGA, glyph->width*2, glyph->height*2);
		
			lCopyFrame(glyph, frame);
			lMoveArea(frame, 0, 0, glyph->width-1, DHEIGHT, DHEIGHT/2, LMOV_CLEAR, LMOV_DOWN);
			lRefresh(frame);
		
			int flen = glyph->width * glyph->height * 3;
			char *temp = (char *)calloc(1, flen+1);
			lFrame1BPPToRGB(glyph, temp, LFRM_BPP_24, (240<<16) | (240<<8) | 240, (110<<16) | (110<<8) | 110);
		
			FILE *fp = fopen("euroRGB24_12x24.raw","wb");
			if (fp){
				fwrite(temp, sizeof(char), flen, fp);
				fclose(fp);
			}

			lDeleteFrame(glyph);
			free(temp);
		}
	}

	wc = lGetGlyph(hw, NULL, 0x2500, LFTW_WENQUANYI9PT);
	if (wc){
		TFRAME *glyph = glyphToFrame(hw, wc);
		if (glyph){
			lSaveImage(glyph, L"0x2500.bmp", IMG_BMP, glyph->width*2, glyph->height*2);
			lDeleteFrame(glyph);
		}
	}

	wc = lGetGlyph(hw, NULL, 0x9F98, LFTW_B24);
	if (wc){
		TFRAME *glyph = glyphToFrame(hw, wc);
		if (glyph){
			lSaveImage(glyph, L"0x9F98.bmp", IMG_BMP, glyph->width*2, glyph->height*2);
			lDeleteFrame(glyph);
		}
	}

	wc = lGetGlyph(hw, NULL, 0xFFFD, LFTW_UNICODE);
	if (wc){
		TFRAME *glyph = glyphToFrame(hw, wc);
		if (glyph){
			lSaveImage(glyph, L"0xFFFD.bmp", IMG_BMP, glyph->width*2, glyph->height*2);
			lDeleteFrame(glyph);
		}
	}


	lSetForegroundColour(hw, (0xFF<<24)|lGetRGBMask(frame, LMASK_BLUE));
	lSetCharacterEncoding(hw, CMT_GB18030);

	TFRAME *gb = lNewString(hw, LFRM_BPP_32A, PF_VERTICALRTL|PF_CLIPWRAP, LFTW_B24, txt);
	if (gb){
		lRefresh(gb);
		lSaveImage(gb, L"gb.png", IMG_PNG|IMG_KEEPALPHA, gb->width, gb->height);
		lDeleteFrame(gb);
	}

	utilCleanup();
	return EXIT_SUCCESS;
}


