
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
#include <string.h>
#include <stdio.h>
#include <wchar.h>

#if (__BUILD_CHRDECODE_SUPPORT__)


#include "memory.h"
#include "utils.h"
#include "cmap.h"
#include "device.h"
#include "lstring.h"
#include "fileio.h"

#include "sync.h"
#include "misc.h"

static int deleteCMapList (THWD *hw, TMAPTABLE *table);
static int parseCMapFile (TMAPTABLE *cmt, TASCIILINE *al);
static int addTable (THWD *hw, TMAPTABLE *table);
static int registerCharacterMap (THWD *hw, const char *name, const wchar_t *filename, const int mapID);
static int unregisterCharacterMap (THWD *hw, int mapID);
static TMAPTABLE *rootTable (THWD *hw);
static TMAPTABLE *newCMapTable ();
static TASCIILINE *readFileW (const wchar_t *filename);
static void free_ASCIILINE (TASCIILINE *al);



void deleteRootCMapList (THWD *hw)
{
	deleteCMapList(hw, rootTable(hw));
}

int enumLanguageTables (THWD *hw, char **alias, int *langID, wchar_t **filepath)
{
	if (hw->cmap->currentEnumTable){
		if (alias)
			*alias = hw->cmap->currentEnumTable->alias;
		if (langID)
			*langID = hw->cmap->currentEnumTable->code;
		if (filepath){
			if (*hw->cmap->currentEnumTable->file)
				*filepath = hw->cmap->currentEnumTable->file;
			else
				*filepath = NULL;
		}
		hw->cmap->currentEnumTable = hw->cmap->currentEnumTable->next;
		return 1;
	}else{
		if (alias)
			*alias = NULL;
		if (langID)
			*langID = 0;
		if (filepath)
			*filepath = NULL;

		resetTableIndex(hw);
		return 0;
	}
}


int encodingAliasToID (THWD *hw, const char *name)
{
	char *alias = NULL;
	int langID = 0;
	
	resetTableIndex(hw);
	while(enumLanguageTables(hw, &alias, &langID, NULL)){
		if (l_stristr(alias, name)){
			resetTableIndex(hw);
			return langID;
		}
	}
	resetTableIndex(hw);
	return 0;
}

wchar_t *getMapPath (THWD *hw)
{
	//return mpath;
	return hw->paths->cmap;
}

void resetTableIndex (THWD *hw)
{
	hw->cmap->currentEnumTable = rootTable(hw);
	hw->cmap->currentEnumTable = hw->cmap->currentEnumTable->next;
}

static int deleteCMapList (THWD *hw, TMAPTABLE *table)
{
	if (!table) return 0;
	TMAPTABLE *previous = NULL;
	
	do{
		previous = table;
		table = table->next;
		if (!table->next){
			unregisterCharacterMap(hw, table->code);
			l_free(table);
			previous->next = NULL;
			if (previous == rootTable(hw))
				break;
			table = rootTable(hw);
		}
	}while(table);

	return 1;
}

static int registerCharacterMap (THWD *hw, const char *alias, const wchar_t *filename, const int mapID)
{
	
	TMAPTABLE *table = newCMapTable();
	if (!table) return 0;
	
	table->active = 1;
	table->built = 0;
	table->code = mapID;
	table->table = NULL;
	table->ltable = NULL;
	table->ltotal = 0;
	table->next = NULL;
	
	
	// TODO: replace the magic number
	if (alias)
		strcpy(table->alias, alias);

	int filelen = l_wcslen(filename);
	if (filelen){
		if (filelen + l_wcslen(getMapPath(hw)) < MaxPath){
			//snwprintf(table->file, MaxPath, L"%s%s", getMapPath(hw), filename);
			table->file[0] = 0;
			wcsncat(table->file, getMapPath(hw), MaxPath);
			wcsncat(table->file, filename, MaxPath);
		}else{
			mylogw(L"libmylcd: path too long: %s%s\n", getMapPath(hw), filename);
			l_free(table->table);
			l_free(table);
			return 0;
		}
	}else{
		*table->file = 0;
	}
	
	if (!addTable(hw, table)){
		l_free(table->table);
		l_free(table);
		return 0;
	}else{
		return 1;
	}
}

static int addTable (THWD *hw, TMAPTABLE *table)
{
	TMAPTABLE *root = rootTable(hw);
	if (!root) return 0;

	if (!root->next)
		root->next = table;
	else{
		table->next = root->next;
		root->next = table;
	}
	return 1;
}

static int unregisterCharacterMap (THWD *hw, int mapID)
{
	TMAPTABLE *table = CMapIDToTable(hw, mapID);
	if (!table) return 0;

	if (table->table){
		l_free(table->table);
		table->table = NULL;
	}
	if (table->ltable){
		l_free(table->ltable);
		table->ltable = NULL;
	}

	table->active=0;
	table->built=0;
	table->code=0;
	return 1;
}

static TMAPTABLE *rootTable (THWD *hw)
{
	//return &rMapTable;
	return hw->cmap->root;
}

static TMAPTABLE *newCMapTable ()
{
	return l_calloc(1,sizeof(TMAPTABLE));
}

TMAPTABLE *CMapIDToTable (THWD *hw, int mapID)
{
	if (!mapID) return NULL;
	TMAPTABLE *t = rootTable(hw);
	if (t == NULL) return NULL;
	
	while (t){
		if (t->code == mapID)
			return t;
		else
			t=t->next;
	}
	return NULL;
}

int buildCMapTable (THWD *hw, int id)
{
	TMAPTABLE *cmt = CMapIDToTable(hw, id);
	if (cmt == NULL) return 0;
	if (cmt->built == 1) return 1;
	if (cmt->file == NULL) return 0;
		
	TASCIILINE *al = readFileW(cmt->file);
	if (!al){
		mylogw(L"libmylcd: unable to load table: %s\n", cmt->file);
		return 0;
	}else{
   		int ret = parseCMapFile(cmt, al);
		free_ASCIILINE(al);
		if (ret)
			cmt->built = 1;
		return ret;
	}
}

static unsigned int hex2Dec (ubyte *text)
{
	unsigned int x = 0;
	sscanf((char*)text,"%x",&x); 
	return x;
}

static int parseCMapFile (TMAPTABLE *cmt, TASCIILINE *al)
{
	if (!cmt || !al)
		return 0;

	ubyte *str=NULL;
	unsigned int i=0, enc=0;
	unsigned int maxEnc = MIN(0xFFFF, al->tlines)+1;
	unsigned int lindex=0;

	cmt->ltotal = 0;
	if (cmt->ltable)
		l_free(cmt->ltable);
	cmt->ltable = (TTUNIENCMAP*)l_calloc(sizeof(TTUNIENCMAP),cmt->ltotal+1);
	if (cmt->table)
		l_free(cmt->table);
	cmt->table = (UTF32*)l_calloc(sizeof(int), maxEnc);
	if (!cmt->table) return 0;

	while (al->lines[i]){
		if ((str=(ubyte *)l_strtok((char*)al->lines[i++],"\t "))){
			if (str[0]!='#'){
				enc=hex2Dec(str);
				
				if (enc > 0xFFFF){
					if (lindex > cmt->ltotal){
						cmt->ltable = (TTUNIENCMAP*)l_realloc(cmt->ltable,(sizeof(TTUNIENCMAP))*(cmt->ltotal+4097));
						if (cmt->ltable == NULL){
							mylog("libmylcd: parseCMapFile(): l_realloc returned a NULL ptr\n");
							return 0;
						}
						
						l_memset(&cmt->ltable[cmt->ltotal+1],0,4096*sizeof(TTUNIENCMAP));
						cmt->ltotal += 4096;
					}
					if ((str=(ubyte *)l_strtok(NULL,"\t ")))
						cmt->ltable[lindex].uni=hex2Dec(str);
					else
						cmt->ltable[lindex].uni=0;

					cmt->ltable[lindex++].enc = enc;
				}else{
				
					if (enc >= maxEnc){
						maxEnc = MIN(enc+2048,0xFFFF);
						cmt->table = (UTF32 *)l_realloc(cmt->table,(4+sizeof(int)) * maxEnc);
					}
					if ((str=(ubyte *)l_strtok(NULL,"\t ")))
						cmt->table[enc]=(int)hex2Dec(str);
				}
			}
		}
	}
	return i;
}

static TASCIILINE *readFileW (const wchar_t *filename)
{
	FILE *fp = NULL;

    if (!(fp=l_wfopen(filename, L"rb"))){
    	mylogw(L"libmylcd: readFileW(): unble to open '%s'\n", filename);
    	return NULL;
    }

	TASCIILINE *al = (TASCIILINE *)l_malloc(sizeof(TASCIILINE));
	if (!al){
		l_fclose(fp);
		return NULL;
	}
		

	const uint64_t flen = l_lof(fp);
	al->data = (ubyte *)l_calloc(sizeof(ubyte *),4+flen);
	if (!al->data){
		l_fclose(fp);
		l_free(al);
		return NULL;
	}

	l_fseek(fp, 0, SEEK_SET);
	unsigned int bread = l_fread(al->data, 1, flen, fp);
	l_fclose(fp);

	if (bread != flen){
		l_free(al->data);
		l_free(al);
		mylogw(L"libmylcd: readFileW(): unble to load '%s'\n", filename);
		return NULL;
	}

	al->tlines = 512;
	al->lines = (ubyte **)l_malloc((1+al->tlines) * sizeof(ubyte *));
	if (!al->lines){
		l_free(al->data);
		l_free(al);
		return NULL;
	}
	
	int i = 0;
	ubyte *str = (ubyte*)l_strtok((char*)al->data, "\12\15");
	do{
		al->lines[i++] = str;
		if (i >= al->tlines){
			al->tlines <<= 1;
			al->lines = (ubyte**)l_realloc(al->lines, (1+al->tlines) * sizeof(ubyte*));
		}
		str = (ubyte*)l_strtok(NULL, "\12\15");
	}while(str && i <= al->tlines);

	if (i < al->tlines)
		al->lines = (ubyte **)l_realloc(al->lines, (1+i) * sizeof(ubyte*));

	al->lines[i] = NULL;
	al->tlines = i;
	return al;
}

static void free_ASCIILINE (TASCIILINE *al)
{
	if (al != NULL){
		l_free(al->lines);
		l_free(al->data);
		l_free(al);
	}
}

int registerCharacterMaps (THWD *hw, const wchar_t *mapPath)
{
	if (mapPath)
		hw->paths->cmap = l_wcsdup(mapPath);
	else
		hw->paths->cmap = l_wcsdup(lCharacterMapPath);
	
	#if (!__BUILD_INTERNAL_BIG5__)
	  registerCharacterMap(hw, "BIG5,CP950", L"cp950", CMT_BIG5);
	#else
	  registerCharacterMap(hw, "BIG5,CP950", L"", CMT_BIG5);
	#endif

	registerCharacterMap(hw, "EUC-CN,EUC_CN,ZH_CN", L"", CMT_EUC_CN);
	registerCharacterMap(hw, "HZ-GB-2312,GB2312", L"", CMT_HZ_GB2312);
	registerCharacterMap(hw, "TIS-620", L"", CMT_TIS620);
	registerCharacterMap(hw, "EUC-JP,EUC_JP,JISX0208", L"", CMT_JISX0208);
	registerCharacterMap(hw, "SJIS,CP932,JISX0213", L"", CMT_JISX0213);
	registerCharacterMap(hw, "AUTO-JP,JA", L"", CMT_ISO_2022_JP_EUC_SJIS);
	registerCharacterMap(hw, "UTF32", L"", CMT_UTF32);
	registerCharacterMap(hw, "UTF16BE", L"", CMT_UTF16BE);
	registerCharacterMap(hw, "UTF16LE", L"", CMT_UTF16LE);
	registerCharacterMap(hw, "UTF16", L"", CMT_UTF16);
	registerCharacterMap(hw, "UTF8", L"", CMT_UTF8);

	registerCharacterMap(hw, "JISX0201", L"jis0201", CMT_JISX0201);
	registerCharacterMap(hw, "ISO-2022-JP", L"jis0201", CMT_ISO2022_JP);
	registerCharacterMap(hw, "ISO-2022-KR", L"ksx1001", CMT_ISO2022_KR);
	registerCharacterMap(hw, "EUC-KR,EUC_KR,CP949", L"cp949", CMT_EUC_KR);
	registerCharacterMap(hw, "EUC-TW,EUC_TW", L"euc-tw", CMT_EUC_TW);
	registerCharacterMap(hw, "GBK,CP936", L"cp936", CMT_GBK);
	registerCharacterMap(hw, "GB18030", L"gb18030", CMT_GB18030);
	registerCharacterMap(hw, "GB1988", L"gb1988", CMT_GB1988);
	registerCharacterMap(hw, "SYMBOLS", L"symbol", CMT_SYMBOL);
	registerCharacterMap(hw, "DINGBATS", L"dingbts", CMT_DINGBATS);
	registerCharacterMap(hw, "KOI8-R", L"koi8-r", CMT_KOI8_R);
	registerCharacterMap(hw, "KOI8-U", L"koi8-u", CMT_KOI8_U);
	registerCharacterMap(hw, "8859-16", L"8859-16", CMT_ISO8859_16);
	registerCharacterMap(hw, "8859-15", L"8859-15", CMT_ISO8859_15);
	registerCharacterMap(hw, "8859-14", L"8859-14", CMT_ISO8859_14);
	registerCharacterMap(hw, "8859-13", L"8859-13", CMT_ISO8859_13);
	registerCharacterMap(hw, "8859-11", L"8859-11", CMT_ISO8859_11);
	registerCharacterMap(hw, "8859-10", L"8859-10", CMT_ISO8859_10);
	registerCharacterMap(hw, "8859-9", L"8859-9", CMT_ISO8859_9);
	registerCharacterMap(hw, "8859-8", L"8859-8", CMT_ISO8859_8);
	registerCharacterMap(hw, "8859-7", L"8859-7", CMT_ISO8859_7);
	registerCharacterMap(hw, "8859-6", L"8859-6", CMT_ISO8859_6);
	registerCharacterMap(hw, "8859-5", L"8859-5", CMT_ISO8859_5);
	registerCharacterMap(hw, "8859-4", L"8859-4", CMT_ISO8859_4);
	registerCharacterMap(hw, "8859-3", L"8859-3", CMT_ISO8859_3);
	registerCharacterMap(hw, "8859-2", L"8859-2", CMT_ISO8859_2);
	registerCharacterMap(hw, "8859-1", L"8859-1", CMT_ISO8859_1);
	registerCharacterMap(hw, "CP437", L"cp437", CMT_CP437);
	registerCharacterMap(hw, "CP737", L"cp737", CMT_CP737);
	registerCharacterMap(hw, "CP775", L"cp775", CMT_CP775);
	registerCharacterMap(hw, "CP850", L"cp850", CMT_CP850);
	registerCharacterMap(hw, "CP852", L"cp852", CMT_CP852);
	registerCharacterMap(hw, "CP855", L"cp855", CMT_CP855);
	registerCharacterMap(hw, "CP857", L"cp857", CMT_CP857);
	registerCharacterMap(hw, "CP857", L"cp857", CMT_CP860);
	registerCharacterMap(hw, "CP861", L"cp861", CMT_CP861);
	registerCharacterMap(hw, "CP862", L"cp862", CMT_CP862);
	registerCharacterMap(hw, "CP863", L"cp863", CMT_CP863);
	registerCharacterMap(hw, "CP864", L"cp864", CMT_CP864);
	registerCharacterMap(hw, "CP865", L"cp865", CMT_CP865);
	registerCharacterMap(hw, "CP866", L"cp866", CMT_CP866);
	registerCharacterMap(hw, "CP869", L"cp869", CMT_CP869);
	registerCharacterMap(hw, "CP874", L"cp874", CMT_CP874);
	registerCharacterMap(hw, "CP1250", L"cp1250", CMT_CP1250);
	registerCharacterMap(hw, "CP1251", L"cp1251", CMT_CP1251);
	registerCharacterMap(hw, "CP1252", L"cp1252", CMT_CP1252);
	registerCharacterMap(hw, "CP1253", L"cp1253", CMT_CP1253);
	registerCharacterMap(hw, "CP1254", L"cp1254", CMT_CP1254);
	registerCharacterMap(hw, "CP1255", L"cp1255", CMT_CP1255);
	registerCharacterMap(hw, "CP1257", L"cp1257", CMT_CP1257);
	registerCharacterMap(hw, "CP1258", L"cp1258", CMT_CP1258);

	registerCharacterMap(hw, "ASCII", L"", CMT_NONE);
	return 1;
}


#else

void deleteRootCMapList (THWD *hw){return;}
int registerCharacterMaps (THWD *hw, const wchar_t *mapPath){return 0;}
void resetTableIndex (THWD *hw){return ;}

#endif
