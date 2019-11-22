
// libmylcd
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
//  GNU LIBRARY GENERAL PUBLIC LICENSE for details.
//
//	You should have received a copy of the GNU Library General Public
//	License along with this library; if not, write to the Free
//	Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


#include <string.h>
#include <mylcd.h>

typedef struct{
	unsigned int tlines;
	ubyte	**lines;
	ubyte	*data;
}TASCIILINE;


static INLINE int H2D (ubyte *text);
TASCIILINE *readFileA (const char *filename);
void freeASCIILINE (TASCIILINE *al);


int main (int argc, char* argv[])
{
	if (argc<1)
		return 0;

	unsigned int page = 0;
	unsigned int pageIndex = 0;
	unsigned int j, i = 2;
	ubyte *line;


	TASCIILINE *enc = readFileA(argv[1]);
	if (!enc) return 0;

	FILE *fp = fopen(strtok(argv[1],"."),"wb");
	if (*enc->lines[0] == '#'){
		fprintf(fp, "%s\n", enc->lines[0]);
		i = 3;
	}
	
	do{
		line = enc->lines[i];
		if ((*line == 'R' ) || (*line == 'r' )){
			i = enc->tlines;
		}else if (strlen((char*)line) < 5){
			page = H2D(line);
			pageIndex = 0;
			printf("page: %X (%i)\n", page, page);
		}else{
			for (j = 0; j < 64; j+=4){
				if ((*(line+j) != '0') || (*(line+j+1) != '0') || (*(line+j+2) != '0') || (*(line+j+3) != '0')){
					if (!page)
						fprintf(fp,"%.2X\t%.4s\n", pageIndex, line+j);
					else
						fprintf(fp,"%.2X%.2X\t%.4s\n",page, pageIndex, line+j);
				}					
				pageIndex++;
			}
		}
	}while(++i < enc->tlines);

	fclose(fp);	
	freeASCIILINE(enc);
	return 1;	
}

static INLINE int H2D (ubyte *text)
{
	if (!text){
		return 0;
	}else{
		unsigned int x = 0;
		sscanf((char*)text,"%x",&x);
		return x;
	}
}


