
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



void listdrv (THWD *hw, int which)
{
	TREGDRV *drv = lEnumerateDriversBegin(hw, which);
	if (drv){
		printf("Total: %i\n",drv->total);
		do{
			printf("%i  \"%s\"\t\"%s\"\n", drv->index, drv->name, drv->comment);
		}while(lEnumerateDriverNext(drv));
		lEnumerateDriverEnd(drv);
	}
	printf("\n");
}

int main (int argc, char* argv[])
{
	THWD *hw = lOpen(NULL, NULL);
	if (hw == NULL){
		printf("lOpenLibrary() failed\n");
		return EXIT_FAILURE;
	}
	
	listdrv(hw, LDRV_DISPLAY);
	listdrv(hw, LDRV_PORT);
	lClose(hw);
	
	return EXIT_SUCCESS;
}

