
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


int main (int argc, char* argv[])
{
	THWD *hw = lOpen(NULL, NULL);
	if (hw == NULL){
    	printf("lOpen() failed\n");
		return EXIT_FAILURE;
    }	
	
	char *alias;
	int langID;
	wchar_t *filepath = NULL;
	
	
	/*
	lEnumLanguageTables (char **alias, int *cmt_id, wchar_t **filepath)
	Cycles through encodings, returning language alias, id and filepath,
	in alias, cmt_id and filepath respectively.
	
	'filepath' will return a NULL pointer if internal decoding method uses
	an internal lookup table(s). eg: HZ-GB-2312.
	
	
	returns 1 to indicate another/next encoding is available.
	returns 0 when end of list has been reached, at which point list is reset,
	ready for the next iteration.
	*/
	
	while(lEnumLanguageTables(hw, &alias, &langID, &filepath)){
		printf("%s    \t", alias);
		if (filepath){
			printf("%i\t%s   \t", langID, alias);
			wprintf(L"%s\n", filepath);
		}else{
			printf("%i\t%s\n", langID, alias);
		}
	}
	
	printf("\ntis620 = %i\n", lEncodingAliasToID(hw, "tis-620"));
	printf("big5 = %i\n", lEncodingAliasToID(hw, "bIg5"));
	printf("utf16 = %i\n", lEncodingAliasToID(hw, "UTF16"));	
	printf("cp1251 = %i\n", lEncodingAliasToID(hw, "Cp1251"));
	printf("iso8859-1 = %i\n", lEncodingAliasToID(hw, "8859-1"));
	printf("iso8859-16 = %i\n", lEncodingAliasToID(hw, "8859-16"));
	printf("\n");

    lClose(hw);
	return EXIT_SUCCESS;
}

