
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

TASCIILINE *readFileA (const char *filename);
void freeASCIILINE (TASCIILINE *al);



void printusage()
{
	printf("\nVersion %s\n",(ubyte*)lVersion());
	printf("Usage: hex2bdf in.hex out.bdf\n");
}

static INLINE int H2D (ubyte *text)
{
	if (!text){
		return 0;
	}else{
		int x = 0;
		sscanf((char*)text,"%x",&x);
		return x;
	}
}


int main(int argc, char* argv[])
{
	if (argc<3){
		printusage();
		return 0;
	}

	char *in = argv[1];
	char *out = argv[2];
	TASCIILINE *hex = readFileA(in);
	if (!hex) return 0;
	
	printf ("in\t%s\nout\t%s\n",in,out);
	    	
	FILE *fp = fopen(out,"wb");
    if (fp){
    	fprintf(fp,"STARTFONT 2.1\n");
    	fprintf(fp,"FONT -GNU-Unifont-Medium-R-Normal--16-160-75-75-C-80-ISO10646-1\n");
		fprintf(fp,"SIZE 16 75 75\n");
		fprintf(fp,"PIXEL_SIZE 16\n");
		fprintf(fp,"FONTBOUNDINGBOX 16 16 0 -2\n");
		fprintf(fp,"CHARSET_REGISTRY \"ISO10646\"\n");
		fprintf(fp,"FAMILY_NAME \"Unicode\"\n");
		fprintf(fp,"STARTPROPERTIES 3\n");
		fprintf(fp,"FONT_ASCENT 14\n");
		fprintf(fp,"FONT_DESCENT 2\n");
		fprintf(fp,"DEFAULT_CHAR 32\n");
		fprintf(fp,"ENDPROPERTIES\n");
		fprintf(fp,"CHARS %i\n",hex->tlines);
		fflush(fp);
    	
    	ubyte *encoding=NULL,*line=NULL;;
		ubyte buffer[128]={0};
		int len=0,c=0;
		int i=0,j=0;
		
    	do{
    		if (strlen(hex->lines[i]) > 8){
    		
    		encoding =(ubyte*)strtok((char*)hex->lines[i],":");
    		line = (ubyte*)strtok(NULL,"\n");
    		if (strlen((char*)line) > 32) len = 2;
    		else len = 1;

			fprintf(fp,"STARTCHAR U+%s\n",encoding);
			fprintf(fp,"ENCODING %i\n",H2D(encoding));
			fprintf(fp,"SWIDTH %i 0\n",len*500);
			fprintf(fp,"DWIDTH %i 0\n",len<<3);
			fprintf(fp,"BBX %i 16 0 -2\n",len<<3);
			fprintf(fp,"BITMAP\n");

			j = c = 0;
			do{
			
				if (len==1){
					buffer[j++]=line[c++];
					buffer[j++]=line[c++];
					buffer[j++]='\n';
				}else{
					buffer[j++]=line[c++];
					buffer[j++]=line[c++];
					buffer[j++]=line[c++];
					buffer[j++]=line[c++];
					buffer[j++]='\n';
				}
			}while(*(line+c));
			
			buffer[j]=0;
			fputs((char*)buffer,fp);
			fprintf(fp,"ENDCHAR\n");
			}
    	}while(++i < hex->tlines);

		fprintf(fp,"ENDFONT\n");
		fflush(fp);
		fclose(fp);
	}
	
	printf("%i glyphs added to %s\n", hex->tlines,out);
	freeASCIILINE(hex);
	return 1;
}

