
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


#define IMGFILETYPE	IMG_PNG
#define IMGEXT		L"png"


/*
 -i: read from file
 -s: read string from cmd line
 -e: select encoding
 -v: be verbose
 -f: select font
 -g: set print flags (PF_xxxxx)
 -c: set text ink colour
 -b: set text background colour
 -w: set maximum width of image
 -h: set maximum height of image
 -d: force maximum width and/or height (-w and -h)
 -o: output filename

 example: txt2img.exe -i "filerendering/utf8demo.txt" -e utf8 -v -f 2034 -g 32768
 UTF8 encoding
 Verbose output
 LFTW_10x20 font
 PF_DISABLEAUTOWIDTH print flag
 
*/


//set defaults
typedef struct {
	int Encoding;
	int Font;
	int Flags;
	int MaxWidth;
	int MaxHeight;
	int ForceDim;
	wchar_t *string;
	
	wchar_t inFilename[MaxPath];
	wchar_t outFilename[MaxPath];
	
	char alias[256];
	int isInfileSet;
	int isInStringSet;
	int verbose;
}TFILERENDER;


static TFILERENDER file;
int renderFile (TFRAME *frame, TFILERENDER *file);
int renderBuffer (TFRAME *frame, char *buffer, TFILERENDER *file);
void saveFrame (TFRAME *frame, wchar_t *filename, wchar_t *ext, int imagetype);



void setDefaults (TFILERENDER *file)
{
	file->isInfileSet = 0;
	file->isInStringSet = 0;
	file->verbose = 0;
	file->Encoding = CMT_UTF16;
	strcpy(file->alias, "utf16");
	file->Font = LFTW_UNICODE;
	file->Flags = 0;
	file->MaxWidth = 0;
	file->MaxHeight = 0;
	file->ForceDim = 0;
	wcscpy(file->outFilename, L"out."IMGEXT);
}

void printhelp ()
{
	printf(
		"Usage:\n"
		" -i: read from file\n"
		" -s: read (string) from command line\n"
		" -e: select encoding\n"
		" -v: be verbose\n"
		" -f: select font (LFTW_xxxx)\n"
		" -g: set print flags (PF_xxxxx)\n"
		" -c: set text ink colour\n"
		" -b: set text background colour\n"
		" -w: set maximum width of image\n"
		" -h: set maximum height of image\n"
		" -d: force maximum width and/or height (-w and -h)\n"
		" -o: output filename\n"
		"\n"
		"Example:\n"
		"txt2img.exe -s \"Hello World\"\n"
		"Render string with the default font (LFTW_UNICODE:%i)\n"
		"\n"
		"Example:\n"
		"txt2img.exe -i \"filerendering/utf8demo.txt\" -e utf8 -v -f 2034 -g 32768\n"
		" UTF8 encoding\n"
		" Verbose output\n"
		" LFTW_10x20 font\n"
		" PF_DISABLEAUTOWIDTH print flag\n",
		LFTW_UNICODE
	);
}

int main (void)
{

	int argc = 0, totalChars = 0;
	wchar_t **argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (argv == NULL || argc < 2){
		printhelp();
		return EXIT_FAILURE;
	}

	if (!(hw=lOpen(NULL,NULL))){
    	printf("lOpen() failed\n");
		return EXIT_FAILURE;
    }
    
	TFRAME *frame = lNewFrame(hw, 8, 8, LFRM_BPP_24);
	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));
	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
	lSetPixelWriteMode(frame, LSP_SET);
	setDefaults(&file);
	
	int i = 0;
	for (i = 0; i < argc; i++){
		wchar_t *cmd = argv[i];
		//wprintf(L"%s\n", cmd);

		if (!wcscmp(L"-i", cmd)){			// input text file, path
			if (i+1 >= argc) break;
			wcsncpy(file.inFilename, argv[i+1], MaxPath-sizeof(wchar_t));
			file.isInfileSet = 1;
			i++;
		}else if (!wcscmp(L"-o", cmd)){		// output image file, path
			if (i+1 >= argc) break;
			wcsncpy(file.outFilename, argv[i+1], MaxPath-sizeof(wchar_t));
			i++;
		}else if (!wcscmp(L"-c", cmd)){		// ink/forground colour
			if (i+1 >= argc) break;
			hw->render->foreGround = (int)wcstol(argv[i+1], NULL, 10);
			i++;
		}else if (!wcscmp(L"-b", cmd)){		// 	paper/background colour
			if (i+1 >= argc) break;
			hw->render->backGround = (int)wcstol(argv[i+1], NULL, 10);
			i++;
		}else if (!wcscmp(L"-e", cmd)){		// encoding alias, string
			if (i+1 >= argc) break;
			memset(file.alias, 0, sizeof(file.alias));
			if (WideCharToMultiByte(CP_ACP, 0, argv[i+1], -1, file.alias, sizeof(file.alias), NULL, NULL)){
				int encoding = lEncodingAliasToID(hw, file.alias);
				if (encoding)
					file.Encoding = encoding;
			}
			i++;
		}else if (!wcscmp(L"-f", cmd)){		// font id, digit
			if (i+1 >= argc) break;
			file.Font = (int)wcstol(argv[i+1], NULL, 10);
			i++;
		}else if (!wcscmp(L"-g", cmd)){		// print flags, digit
			if (i+1 >= argc) break;
			file.Flags = (int)wcstol(argv[i+1], NULL, 10);
			i++;
		}else if (!wcscmp(L"-w", cmd)){		// max width, digit
			if (i+1 >= argc) break;
			file.MaxWidth = (int)wcstol(argv[i+1], NULL, 10);
			i++;
		}else if (!wcscmp(L"-h", cmd)){		// max height, digit
			if (i+1 >= argc) break;
			file.MaxHeight = (int)wcstol(argv[i+1], NULL, 10);
			i++;
		}else if (!wcscmp(L"-d", cmd)){		// force diminsions, none
			file.ForceDim = 1;

		}else if (!wcscmp(L"-v", cmd)){		// verbose,none
			file.verbose = 1;

		}else if (!wcscmp(L"-vv", cmd)){		// verbose,none
			file.verbose = 2;
						
		}else if (!wcscmp(L"-s", cmd)){		// input string
			if (i+1 >= argc) break;
			file.string = calloc(sizeof(wchar_t), wcslen(argv[i+1])+1);
			if (file.string != NULL){
				wcscpy(file.string, argv[i+1]);
				file.isInStringSet = 1;
			}
			i++;
		}
	}

	if (file.verbose){
		printf("\nencoding: %s (%i)\n", file.alias, file.Encoding);
		printf("font: %i\n",file.Font);
		printf("flags: %i\n",file.Flags);
		printf("max width: %i\n",file.MaxWidth);
		printf("max height: %i\n",file.MaxHeight);
		printf("force dim: %i\n",file.ForceDim);
		//printf("string: %i\n",file.isInStringSet);
		if (file.isInStringSet)
			wprintf(L"in: \"%s\"",file.string);
		else
			wprintf(L"in: %s",file.inFilename);
		printf("\n");
		wprintf(L"out: %s\n",file.outFilename);
		printf("\n");
	}
	
	
	if (file.isInfileSet){
		if ((totalChars=renderFile(frame, &file))){
			//if (file.verbose)
				//printf("saving...\n");
			//saveFrame(frame, file.outFilename, IMGEXT, IMGFILETYPE);
		}
	}else if (file.isInStringSet){
		if ((totalChars=renderBuffer(frame, (char*)file.string, &file))){
			//saveFrame(frame, file.outFilename, IMGEXT, IMGFILETYPE);
			free(file.string);
		}
	}

	if (totalChars){
		if (file.verbose){
			printf("total chars: %i\n", totalChars);
			printf("image size: %ix%i\n",frame->width, frame->height);
			printf("saving...\n\n");
		}
		saveFrame(frame, file.outFilename, IMGEXT, IMGFILETYPE);
	}


	lDeleteFrame(frame);
    lClose(hw);
    return EXIT_SUCCESS;
}


uint64_t lof (FILE *stream)
{
	fpos_t pos;
	fgetpos(stream,&pos);
	fseek(stream,0,SEEK_END);
	uint64_t fl = (uint64_t)ftell(stream);
	fsetpos(stream,&pos);
	return fl;
}


uint64_t loadfile (wchar_t *path, char **buffer)
{
    FILE *fp = _wfopen(path, L"rb");
    if (fp){
    	uint64_t len = lof(fp)+1;
    	*buffer = (char *)calloc(len,1);
		fread(*buffer,1,len-1,fp);
		fclose(fp);
		return len;
	}else{
		return 0;
	}
}

void saveFrame (TFRAME *frame, wchar_t *filename, wchar_t *ext, int imagetype)
{
	//wchar_t filepath[MaxPath];
	//swprintf(filepath, L"%s.%s", filename, ext);
	if (!lSaveImage(frame, filename, imagetype, frame->width, frame->height))
		printf("saveFrame failed\n");
}

int renderFile (TFRAME *frame, TFILERENDER *file)
{
	if (file->verbose)
		printf("loading...\n");
	
	char *buffer = NULL;
	if (loadfile(file->inFilename, &buffer)){
		int total = renderBuffer(frame, buffer, file);
		free(buffer);
		return total;
	}else{
		wprintf(L"can not read '%s'", file->inFilename);
		printf("\n");
		return 0;
	}
}

int renderBuffer (TFRAME *frame, char *buffer, TFILERENDER *file)
{
	if (file->verbose)
		printf("processing...\n");
		

	lSetCharacterEncoding(hw, file->Encoding);
	int total = lCountCharacters(hw, buffer);
	if (!total) return 0;
	unsigned int *glist = (unsigned int *)malloc(sizeof(unsigned int)*total);
	if (!glist) return 0;

	TLPRINTR rect = {0,0,0,0,0,0,0,0};
	
	if (file->MaxWidth < 0) file->MaxWidth = 0;
	if (file->MaxWidth)
		rect.bx2 = file->MaxWidth-1;

	if (file->MaxHeight < 0) file->MaxHeight = 0;
	if (file->MaxHeight)
		rect.by2 = file->MaxHeight-1;
	
	lDecodeCharacterBuffer(hw, buffer, glist, total);

	if (file->verbose == 2){
		printf("string: ");
		for (int i = 0; i < total; i++)
			printf("%X ", glist[i]);
		printf("\n");
	}
	
	
	lCacheCharacterBuffer(hw, glist, total, file->Font);
	if (!file->ForceDim || !rect.by2 || !rect.bx2){
		lGetTextMetricsList(hw, glist, 0, total-1, file->Flags|PF_FORCEAUTOWIDTH|PF_CLIPWRAP, file->Font, &rect);
		rect.bx2 = abs(rect.bx2 - rect.bx1) + 1;
		rect.by2++;
	}
	
	if (file->ForceDim){
		if (file->MaxWidth)
			rect.bx2 = file->MaxWidth;
		if (file->MaxHeight)
			rect.by2 = file->MaxHeight;
	}

	lResizeFrame(frame, rect.bx2, rect.by2, 0);
	lClearFrame(frame);
	
	rect.bx2--;
	rect.by1 = 0;
	rect.bx1 = 0;
	rect.sx = 0;
	rect.ex = 0;
	rect.sy = 0;
	rect.ey = 0;

	lPrintList(frame, glist, 0, total, &rect, file->Font, file->Flags|PF_FORCEAUTOWIDTH|PF_CLIPWRAP|PF_RESETXY, LPRT_CPY);
	free(glist);
	return total;
}

