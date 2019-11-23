
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
#include <conio.h>

#define MIN(a, b) ((a)<(b)?(a):(b))
#define MAX(a, b) ((a)>(b)?(a):(b))




static int dumpfont (TFRAME *frame, TWFONT *font)
{
	// sanity check
	if (!font->built) return 0;

	unsigned int *cachedlist = (unsigned int *)malloc(sizeof(unsigned int)*font->GlyphsInFont);
	if (!cachedlist) return 0;
	const int gtotal = abs(lGetCachedGlyphs(hw, cachedlist, font->GlyphsInFont, font->FontID));
	unsigned int *glist = (unsigned int *)malloc(sizeof(unsigned int)*gtotal*4);
	if (!glist) return 0;
	
	char buffer[16];
	unsigned int *pglist = glist;
	unsigned int *pcachedlist = cachedlist;
	unsigned int ch;
	int total=0,ct=0,w=0,h=0;
	int i;
	unsigned int gtotal_1 = gtotal-1;
	unsigned int firstGlyph = *pcachedlist;

	if (*pcachedlist <= 0xFFFF)
		_snprintf(buffer, 16, "%.4X| ", *pcachedlist);
	else
		_snprintf(buffer, 16, "%X| ", *pcachedlist);

	for (i = 0; i < 6; i++)
		*pglist++ = (unsigned int)buffer[i];
	total += 6;

	for (ch = 0; ch < gtotal; ch++){
		*pglist++ = *pcachedlist++;

		if (++ct == 16 || *pcachedlist >= firstGlyph+16){		// 16 glyphs per row
			if (ch < gtotal_1){
				ct = 0;
				*pglist++ = '\n';		// add a new line
			
				firstGlyph = *pcachedlist;
				if (*pcachedlist <= 0xFFFF)
					_snprintf(buffer, 16, "%.4X| ", *pcachedlist);
				else
					_snprintf(buffer, 16, "%X| ", *pcachedlist);

				for (i = 0; i < 6; i++)
					*pglist++ = (unsigned int)buffer[i];
				total += 6;
			}
		}else{
			*pglist++ = ' ';		// ensure there is a space between glyphs
		}
		total += 2;
	}

	TLPRINTR rect = {0,0,0,0,0,0,0,0};
	int flags = PF_CLIPWRAP|PF_DISABLEAUTOWIDTH|PF_DONTFORMATBUFFER;
	lGetTextMetricsList(hw, glist, 0, total-1, flags, font->FontID, &rect);
	lSetCharacterEncoding(hw, CMT_UTF16);	//  is ignored by the lxxxxxList API
	lGetTextMetrics(hw, (char*)font->file, 0, font->FontID, &w, &h);		// font path

	rect.sx = rect.sy = 0;
	rect.bx2 = MAX(rect.bx2,w)+1;
	rect.by2 += h+1;

	lResizeFrame(frame, rect.bx2, rect.by2, 0);
	lClearFrame(frame);
	
	lPrintEx(frame, &rect, font->FontID, PF_DONTFORMATBUFFER, LPRT_CPY, (char*)font->file);
	rect.ey++;
	lPrintList(frame, glist, 0, total-1, &rect, font->FontID, flags|PF_NEWLINE, LPRT_CPY);
	free(glist);
	free(cachedlist);
	
	return total;
}


int main (void)
{
	int argc = 0;
	wchar_t **argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (argv == NULL/* || argc < 2*/){
		printf("not enough arguments\n");
		return 0;
	}

	if (!utilInitConfig("config.cfg"))
		return EXIT_FAILURE;

	wchar_t savename[MAX_PATH];
	wchar_t name[MAX_PATH];	name[0] = 0;
	wchar_t dir[MAX_PATH];
	wchar_t drive[MAX_PATH];
	
	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));
	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
	lCombinedCharDisable(hw);
	lHTMLCharRefDisable(hw);

	int i = argc;
	if (i > 1){
		while (--i){
			wprintf(L"\n%s\n", argv[i]);

			TWFONT regwfont;
			regwfont.FontID = 10000+i;
			regwfont.CharSpace = 2;
			regwfont.LineSpace = lLineSpaceHeight;
	
			if (lRegisterFontW(hw, argv[i], &regwfont)){
				TWFONT *font = (TWFONT*)lFontIDToFont(hw, regwfont.FontID);
				if (font){
					int ret = lCacheCharactersAll(hw, regwfont.FontID);
					if (ret){
						if (font->GlyphsInFont - font->CharsBuilt)
							printf("%i glyphs built from %i reported, %i repeated or invalid\n", ret, font->GlyphsInFont, font->GlyphsInFont - font->CharsBuilt);
						else
							printf("%i glyphs built from %i reported\n", ret, font->GlyphsInFont);
						
						dumpfont(frame, font);
						lRefresh(frame);
#if 1
						_wsplitpath(font->file, drive, dir, name, NULL);
						_swprintf(savename, L"%s%s%s.png", drive, dir, name);
						
						if (lSaveImage(frame, savename, IMG_PNG, frame->width, frame->height))
							wprintf(L"-> %s\n", savename);
						else
							wprintf(L"could not write file:%s\n", savename);
#endif
					}
				}
			}
		}
		//lSleep(3000);
		utilCleanup();
		return EXIT_SUCCESS;
	}

	TENUMFONT *enf = lEnumFontsBeginW(hw);
	do{
		wprintf(L"%i %s   \n", enf->id, enf->wfont->file);
		
		if (lCacheCharactersAll(hw, enf->id)){
			dumpfont(frame, enf->wfont);
			lFlushFont(hw, enf->id);
			lRefresh(frame);

			*name = 0;
			_wsplitpath(enf->wfont->file, NULL, NULL, name, NULL);
			if (*name)
				_swprintf(savename, L"%s.tga", name);
			else
				_swprintf(savename, L"%i.tga", enf->id);
			
			lSaveImage(frame, savename, IMG_TGA, 0, 0);
		}
	}while(!kbhit() && lEnumFontsNextW(enf));
	lEnumFontsDeleteW(enf);
	
	utilCleanup();
	return EXIT_SUCCESS;
}



