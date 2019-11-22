
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


void listbdf (THWD *hw)
{
	TENUMFONT *enf = lEnumFontsBeginW(hw);
	if (enf != NULL){
		while(lEnumFontsNextW(enf)){
			wprintf(L"%i '%s'\n",enf->id, enf->wfont->file);
		}
		lEnumFontsDeleteW(enf);
		printf("\n");
	}else{
		printf("lEnumWFontsBegin returned NULL\n");
	}
}

void listimgbitmap (THWD *hw)
{
	TENUMFONT *enf = lEnumFontsBeginA(hw);
	if (enf != NULL){
		while(lEnumFontsNextA(enf)){
			wprintf(L"%i '%s'\n",enf->id, enf->font->file);
		}
		lEnumFontsDeleteA(enf);
		printf("\n");
	}else{
		printf("lEnumFontsBegin() returned NULL\n");
	}
}

int main (int argc, char* argv[])
{
	THWD *hw = lOpen(NULL, NULL);
	if (hw == NULL){
		printf("lOpenLibrary() failed\n");
		return 0;
	}

	listimgbitmap(hw);
	listbdf(hw);
	lClose(hw);
	
	return EXIT_SUCCESS;
}

