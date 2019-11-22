
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

#if (__BUILD_BDF_FONT_SUPPORT__)

#include "memory.h"
#include "frame.h"
#include "utils.h"
#include "fileio.h"
#include "device.h"
#include "lstring.h"
#include "bdf.h"
#include "pixel.h"
#include "fonts.h"

#include "sync.h"
#include "misc.h"


int BDFGetGlyphsByPositionList (THWD *hw, TWFONT *font, const unsigned int *glist, const int gtotal);
int BDFGetGlyphPositions (const THWD *hw, TWFONT *font, const unsigned int *glist, const int gtotal);
int BDFReadHeader (TWFONT *font);

static int hextodec[128];

static inline void glyphAddPixel (TGLYPHPOINTS *restrict gp, const int x, const int y)
{
	if (gp->pointsTotal >= gp->pointsSpace){
		gp->pointsSpace += 16;
		//gp->pointsSpace *= 2;
		gp->points = l_realloc(gp->points, (gp->pointsSpace+1) * sizeof(T2POINT));
		if (!gp->points) return;
	}

	gp->points[gp->pointsTotal].x = x;
	gp->points[gp->pointsTotal++].y = y;
	//gp->points[gp->pointsTotal++].data = 0xFFFFFFFF;
	
	//printf("gp->pointsSpace %i\n", gp->pointsSpace);
}


int initBdf (THWD *hw)
{
	hw->flags.glyphCount = 0;
	
	// create a hex2dec lookup table
	int ct = 0;
	for (int i = '0'; i <= '9'; i++)
		/*hw->flags.*/hextodec[i] = ct++;
	ct = 10;
	for (int i = 'A'; i <= 'F'; i++){
		/*hw->flags.*/hextodec[i] = ct;
		/*hw->flags.*/hextodec[i+32] = ct++;
	}
	
	return 1;
}


void closeBdf (THWD *hw)
{
#ifdef __DEBUG__
	mylog("glyphs:\t%i\n",hw->flags.glyphCount);
#endif
}

int BDFOpen (TWFONT *font)
{
	if (font->fp != NULL && font->flags&WFONT_BDF_HEADER)
		return 0;
	
	font->fp = (FILE*)l_wfopen(font->file, L"rb");
	if (!font->fp){
		mylogw(L"libmylcd: unable to open font: %s\n", font->file);
		return 0;
	}else{
		if (!(font->flags&WFONT_BDF_HEADER)){
			if (BDFReadHeader(font)){
				 font->flags |= WFONT_BDF_HEADER;
				 return 1;
			}else{
				BDFClose(font);
				return 0;
			}
		}
		// return 1 as header is already parsed
		return 1;
	}
}

int BDFClose (TWFONT *font)
{
	l_fclose(font->fp);
	font->fp = NULL;
	return 0;	
}

int BDFLoadGlyphs (unsigned int *glist, int gtotal, TWFONT *font)
{
	int total = 0;

	if (!(font->flags&WFONT_CHRPOSITIONSREAD)){
		if ((total=BDFGetGlyphPositions(font->hw, font, glist, gtotal)))
			font->flags |= WFONT_CHRPOSITIONSREAD;

	}
	if (font->flags&WFONT_CHRPOSITIONSREAD)
		total = BDFGetGlyphsByPositionList(font->hw, font, glist, gtotal);
		
#ifdef __DEBUG__
	//mylog("BDFLoadGlyphs %i %i\n", total, font->hw->flags.glyphCount);
#endif
	return total;
}

static inline char *readNextLine (char *text, const FILE *fp)
{
	*text = 0;
	return l_fgets(text, BDFLINELEN, fp);
}


// parse BDF header from file, line by line.
int BDFReadHeader (TWFONT *font)
{
	
	char *text = (char*)l_malloc(BDFLINELEN+2);
	if (text == NULL) return 0;

	while(readNextLine(text, font->fp) != 0){
		if (!l_strncmp(text,"COMMENT", 8)){
				
		}else if (!l_strncmp(text,"SPACING \"", 9)){
			font->spacing = *(text+9);

		}else if (!l_strncmp(text,"FONT ", 5)){
			l_strncpy(font->fontName, text+5, sizeof(font->fontName)-1);

		}else if (!l_strncmp(text,"PIXEL_SIZE ", 11)){
			sscanf((text+11), "%i", &font->PixelSize);

		}else if (!l_strncmp(text,"FONT_ASCENT ", 12)){
			sscanf((text+12), "%d", &font->fontAscent);
			
		}else if (!l_strncmp(text,"FONT_DESCENT ", 13)){ 
			sscanf((text+13), "%d", &font->fontDescent);

		}else if (!l_strncmp(text,"FONTBOUNDINGBOX ", 16)){
    		sscanf((text+16), "%d %d %d %d", &font->QuadWidth, &font->QuadHeight, &font->QuadXOffset, &font->QuadYOffset);
			font->QuadWidth = MAX(font->QuadWidth,0);
			font->QuadHeight = MAX(font->QuadHeight,0);

    	}else if (!l_strncmp(text,"DEFAULT_CHAR ", 13)){
			font->DefaultChar = MAX(l_atoi((text+13)),0);

		}else if (!l_strncmp(text,"CHARSET_REGISTRY ", 17)){
			sscanf((text+17), "%79s",font->CharsetRegistry);
    		
    	}else if (!l_strncmp(text,"FAMILY_NAME ", 12)){
    		sscanf((text+12), "%79s",font->FamilyName);

		}else if (!l_strncmp(text,"CHARS ",6)){
			font->GlyphsInFont = l_atoi(text+6);
    		if (!font->maxCharIdx){
				font->maxCharIdx = MAX(font->GlyphsInFont, 256);
				font->chr = (TWCHAR**)l_calloc(8+font->maxCharIdx+1, sizeof(TWCHAR*));
				if (!font->chr){
					font->maxCharIdx = 0;
					l_free(text);
					return 0;
				}
				break;
			}
		}
    	*text = 0;
	}
	l_free(text);

	if (!(font->fontAscent && font->fontDescent))
		font->fontAscent = font->QuadHeight;
	if (!font->PixelSize)
		font->PixelSize = font->QuadHeight;

	return font->GlyphsInFont != 0;
}

int BDFGetGlyph (THWD *hw, TWFONT *font, char *text, const unsigned int encoding)
{
	int width=0,height=0;
	int xoffset=0,yoffset=0;
	int DWidth=0;
	*text = 0;

	readNextLine(text, font->fp);
   	//if (!l_strncmp(text,"SWIDTH ", 7)){
		// do nothing
	//}

	readNextLine(text, font->fp);
   	if (!l_strncmp(text,"DWIDTH", 6))
		sscanf((text+7), "%d", &DWidth);

	readNextLine(text, font->fp);
	if (!l_strncmp(text,"BBX ", 4))
	//if (*(int32_t*)(text) == (int32_t)0x20584242)
		sscanf((text+4), "%d %d %d %d", &width, &height, &xoffset, &yoffset);
					
	readNextLine(text, font->fp);
	if (!l_strncmp(text,"BITM", 4)){	//BITMAP
	//if (*(int32_t*)(text) == (int32_t)0x4D544942){
		if (encoding >= font->maxCharIdx || !font->chr){
						
			const int sizeDelta = 512;
			if (font->chr == NULL){
				font->chr = l_calloc(encoding+sizeDelta+8+1, sizeof(TWCHAR*));
			}else{
				font->chr = l_realloc(font->chr,(encoding+sizeDelta+8+1) * sizeof(TWCHAR*));
				for (int i = font->maxCharIdx; i < encoding+sizeDelta+8; i++)
					font->chr[i] = NULL;
			}
			font->maxCharIdx = encoding+sizeDelta;
    	}
		if (!font->chr[encoding]){
			if (encoding == ' ' && width < 2) width = 2;

			font->chr[encoding] = l_malloc(sizeof(TWCHAR));
			if (font->chr[encoding] != NULL){
				if (!DWidth) DWidth = width; // bbx width
					
				font->chr[encoding]->gp = (TGLYPHPOINTS*)l_malloc(sizeof(TGLYPHPOINTS));
				if (font->chr[encoding]->gp){
					font->chr[encoding]->gp->pointsTotal = 0;
					font->chr[encoding]->gp->pointsSpace = 32;
					font->chr[encoding]->gp->points = l_malloc(33 * sizeof(T2POINT));
				}
					
				for (int i = 0; i < LTR_TOTAL; i++)
					font->chr[encoding]->ep[i] = NULL;
					
				font->chr[encoding]->box.left = -1;
				font->chr[encoding]->box.right = -1;
				font->chr[encoding]->box.top = -1;
				font->chr[encoding]->box.btm = -1;
					
				font->chr[encoding]->dwidth = DWidth;
 				font->chr[encoding]->w = width;
				font->chr[encoding]->h = height;
				font->chr[encoding]->xoffset = xoffset;
				font->chr[encoding]->yoffset = yoffset;
				font->chr[encoding]->encoding = encoding;
   				font->CharsBuilt++;	// actual number of glyphs processed            
  				readNextLine(text, font->fp);
  								
				if (l_strncmp(text,"ENDC",4)){
				//if (*(int32_t*)(text) != (int32_t)0x43444E45){	
					for (int ypos = 0; ypos < height; ypos++){
						//for (int xpos = width; xpos--; ){
						for (int xpos = 0; xpos < width; xpos++){
							const int val1 = text[xpos>>2];
							const int val2 = (1<<(3-(xpos&3)));
							if (/*hw->flags.*/hextodec[/*(ubyte)*/val1]&val2)
								glyphAddPixel(font->chr[encoding]->gp, xpos, ypos);
						}
						readNextLine(text, font->fp);
					}
				}
				if (encoding == font->DefaultChar)
					font->flags |= WFONT_DEFAULTCHARBUILT;
#ifdef __DEBUG__
				hw->flags.glyphCount++;
				//mylog("BDFGetGlyph %i %i\n", encoding, hw->flags.glyphCount);
#endif
			}
		}
	}
	return 1;
}

static inline int isInList (const unsigned int code, const unsigned int *restrict glist, const int gtotal)
{
	//while (gtotal--){
	for (int i = 0; i < gtotal; i++){
		if (*(glist++) == code)
			return 1;
	}
	return 0;
}

int BDFGetGlyphsByPositionList (THWD *hw, TWFONT *font, const unsigned int *glist, const int gtotal)
{
	
	int gtotalread = 0;
	char *text = (char*)l_malloc(BDFLINELEN+(sizeof(char)*2));
	if (text == NULL) return 0;

	for (int i = 0; i < font->coTotal; i++){
		if (font->chrOffset[i].offset){
			if (isInList(font->chrOffset[i].encoding, glist, gtotal) || font->chrOffset[i].encoding == font->DefaultChar){
				if (font->chrOffset[i].encoding != font->DefaultChar){
					if (gtotalread++ == gtotal)
						break;
				}
				l_fsetpos(font->fp, &font->chrOffset[i].offset);
				BDFGetGlyph(hw, font, text, font->chrOffset[i].encoding);
			}
		}else{
			break;
		}
	}
	l_free(text);
#ifdef __DEBUG__
	//mylog("BDFGetGlyphsByPositionList %i of %i\n", gtotalread, gtotal);
#endif
	return gtotalread-1;
}

static inline void addGlyphPosition (TWFONT *font, unsigned int encoding, int *pos)
{
	if (*pos < font->coTotal){
		l_fgetpos(font->fp, &font->chrOffset[*pos].offset);
		font->chrOffset[*pos].encoding = encoding;
		(*pos)++;
		return;
	}

	font->chrOffset = (TCHAROFFSET*)l_realloc(font->chrOffset, (CHODELTA+font->coTotal+1) * sizeof(TCHAROFFSET));
	if (font->chrOffset == NULL){
		mylog("addGlyphPosition: chrOffset == NULL\n");
		return;
	}

	for (int i = font->coTotal; i < font->coTotal+CHODELTA; i++)
		font->chrOffset[i].offset = 0;

	font->coTotal += CHODELTA;
	l_fgetpos(font->fp, &font->chrOffset[*pos].offset);
	font->chrOffset[*pos].encoding = encoding;
	(*pos)++;
}

int BDFGetGlyphPositions (const THWD *hw, TWFONT *font, const unsigned int *glist, const int gtotal)
{
	if (font->coTotal < font->GlyphsInFont){
		font->chrOffset = (TCHAROFFSET*)l_realloc(font->chrOffset, (1+font->GlyphsInFont) * sizeof(TCHAROFFSET));
		if (font->chrOffset == NULL){
			mylog("BDFGetGlyphPositions: chrOffset == NULL\n");
			return 0;
		}else{
			for (int i = font->coTotal; i <= font->GlyphsInFont; i++)
				font->chrOffset[i].offset = 0;
			font->coTotal = font->GlyphsInFont;
		}
	}

	char *text = l_malloc(BDFLINELEN+2);
	if (text == NULL) return 0;
	
	int gtotalread = 0;
	int index = 0;
		
	while(readNextLine(text, font->fp) != 0){
		if (!l_strncmp(text,"ENCO", 4)){		// encoding
			addGlyphPosition(font, MAX(l_atoi(text+9),0), &index);
			gtotalread++;
		}
		*text = 0;
	}
	l_free(text);
	
#ifdef __DEBUG__	
	//mylog("BDFGetGlyphPositions(): %i %i %i %i\n", font->FontID, index, font->GlyphsInFont, gtotalread);
#endif
	return gtotalread;
}

int BDFGetGlyphsAll (THWD *hw, TWFONT *font)
{

	char *text = l_malloc(BDFLINELEN+2);
	if (text == NULL) return 0;

	int gtotalread = 0;
	while(readNextLine(text, font->fp) != 0){
		if (!l_strncmp(text,"ENCO", 4)){
			gtotalread += BDFGetGlyph(hw, font, text, MAX(l_atoi(text+9), 0));
		}
		*text=0;
	}

	l_free(text);
	return gtotalread;
}



// used with buildBDFFont() 
int BDFReadFile (TWFONT *font, const wchar_t *path)
{
	int ret = 0;

	if ((font->fp=l_wfopen(path, L"r"))){
		if (!(font->flags&WFONT_BDF_HEADER)){
			if (BDFReadHeader(font))
				font->flags |= WFONT_BDF_HEADER;
		}
		if (font->flags&WFONT_BDF_HEADER)
			ret = BDFGetGlyphsAll(font->hw, font);
		l_fclose(font->fp);
		font->fp = NULL;
	}else{
		mylogw(L"libmylcd: unable to open font: '%s'\n",path);
	}
	return ret;
}

#else

int initBdf (){return 1;}
void closeBdf (){}
int BDFReadFile (TWFONT *Font, wchar_t *path){return 0;}
int BDFOpen (TWFONT *font){return 0;}
int BDFClose (TWFONT *font){return 0;}
int BDFLoadGlyphs (unsigned int *glist, int total, TWFONT *font){return 0;}

#endif




