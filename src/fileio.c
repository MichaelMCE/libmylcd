
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
#include "fileio.h"

#include "sync.h"
#include "misc.h"



//FILE* _wfopen (const wchar_t*, const wchar_t*);


#ifdef __DEBUG__
static volatile int fopenCount;
static volatile int fcloseCount;
#endif


int initFileIO (THWD *hw)
{
#ifdef __DEBUG__
	fopenCount = 0;
	fcloseCount = 0;
#endif
	return 1;
}

void closeFileIO (THWD *hw)
{
#ifdef __DEBUG__
	mylog("fopen:\t%i\nfclose:\t%i\n",fopenCount, fcloseCount);
#endif
}

FILE * l_wfopen (const wchar_t *filename, const wchar_t *mode)
{
#ifdef __DEBUG_SHOWFILEIO__
	
	mylogw(L"wfopen(): (%s) '%s' ", mode, filename);
#endif
	FILE *ret = _wfopen(filename, mode);
#ifdef __DEBUG_SHOWFILEIO__	
	mylog("%p\n", ret);
#endif

#ifdef __DEBUG__
	if (ret) fopenCount++;
#endif
	  return ret;
}

FILE * l_fopen (const ubyte *filename, const char *mode)
{
#ifdef __DEBUG_SHOWFILEIO__
	mylog("fopen(): (%s) '%s' ",mode, filename);
#endif
	  FILE *ret = fopen((char*)filename,mode);
#ifdef __DEBUG_SHOWFILEIO__	
	mylog("%p\n", ret);
#endif
	  
#ifdef __DEBUG__
	if (ret) fopenCount++;
#endif
	return ret;
}

int l_fclose (FILE *stream)
{
#ifdef __DEBUG_SHOWFILEIO__
	mylog("fclose() %p\n", stream);
#endif
		
#ifdef __DEBUG__
	fcloseCount++;
#endif
	return fclose (stream);
}

uint64_t l_fread (void *buffer, size_t size, size_t count, FILE *stream)
{
#ifdef __DEBUG_SHOWFILEIO__
	mylog("fread() %p, %i, %i, %p\n", buffer, size, count, stream);
#endif
	return (uint64_t)fread(buffer,size,count,stream);
}

size_t l_fwrite (const void *buffer, size_t size, size_t count, FILE *stream)
{
#ifdef __DEBUG_SHOWFILEIO__
	mylog("fwrite() %p, %i, %i, %p\n", buffer, size, count, stream);
#endif
	return fwrite(buffer,size,count,stream);
}

int l_fseek (FILE *stream, long offset, int whence)
{
#ifdef __DEBUG_SHOWFILEIO__
	mylog("fseek() %p, %u, %i\n", stream, offset, whence);
#endif
	return fseek(stream,offset,whence);
}

uint64_t l_ftell (FILE *stream)
{
	return (uint64_t)ftell(stream);
}

void l_rewind (FILE *stream)
{
#ifdef __DEBUG_SHOWFILEIO__
	mylog("rewind() %\n", stream);
#endif
	rewind(stream);
}

uint64_t l_fgetpos (FILE *stream, fpos_t *pos)
{
	return (uint64_t)fgetpos(stream, pos);
}

int l_fsetpos (FILE *stream, fpos_t *pos)
{
	return fsetpos(stream,pos);
}

int l_fflush (FILE *stream)
{
#ifdef __DEBUG_SHOWFILEIO__
	mylog("fflush() %\n", stream);
#endif
	return fflush(stream);
}

uint64_t l_lof (FILE *stream)
{
	fpos_t pos;
	l_fgetpos(stream, &pos);
	l_fseek(stream, 0, SEEK_END);
	uint64_t fl = l_ftell(stream);
	l_fsetpos(stream, &pos);
	return fl;
}


uint64_t getFileLength (const ubyte *path)
{
	FILE *fp = NULL;
	
	if ((fp=l_fopen (path,"r"))){
		uint64_t len = l_lof(fp);
		l_fclose(fp);
		return len;
	}
	return 0;
}


uint64_t getFileLengthw (const wchar_t *path)
{
	FILE *fp=NULL;
	
	if ((fp=l_wfopen(path, L"r"))){
		uint64_t len = l_lof(fp);
		l_fclose(fp);
		return len;
	}
	return 0;
}

