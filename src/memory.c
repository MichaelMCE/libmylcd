
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




#include <string.h>

#include "mylcd.h"
#include "memory.h"
#include "utils.h"
#include "lstring.h"
#include "convert.h"
#include "frame.h"
#include "sync.h"
#include "misc.h"




#define MAXMEMALLOC ((size_t)(200*1024*1024))
#ifndef ALIGN_TYPE
#define ALIGN_TYPE size_t
#endif

#if 1
static size_t alignSize (size_t sizeofobject)
{
	size_t odd_bytes = sizeofobject % sizeof(ALIGN_TYPE);
	if (odd_bytes > 0)
		sizeofobject += sizeof(ALIGN_TYPE) - odd_bytes;
	return sizeofobject;
}
#endif


#ifdef __DEBUG_MEM__

typedef struct {
	int status;			// not freed:1, freed:0
	void *addr;
	size_t size;
	char *func;
	int line;
}TMEMITEM;

typedef struct {
	TMEMITEM *item;
	int total;			// total allocated
	int size;			// items in stack
	//HANDLE hMutex;
	CRITICAL_SECTION cs;
	int lockCount;
	
	unsigned int allocCount;
	unsigned int freeCount;
	unsigned int reallocCount;
	unsigned int memcpyCount;
	unsigned int memsetCount;
	uint64_t maxRequestSize;
	uint64_t totalRequested;
	uint64_t totalFreed;
}TMEMALLOCSTACK;
static TMEMALLOCSTACK mstack;




#if 1

static inline int memlock (TMEMALLOCSTACK *stack)
{
	EnterCriticalSection(&stack->cs);
	stack->lockCount++;
	return 1;
}

static inline void memunlock (TMEMALLOCSTACK *stack)
{
	stack->lockCount--;
	LeaveCriticalSection(&stack->cs);
}

#else

static inline int memlock (TMEMALLOCSTACK *stack)
{
	return (WaitForSingleObject(stack->hMutex, INFINITE) == WAIT_OBJECT_0);
}

static inline void memunlock (TMEMALLOCSTACK *stack)
{
	ReleaseMutex(stack->hMutex);
}
#endif

#if 1

static void allocstack (TMEMALLOCSTACK *stack, const int total)
{
	stack->total = total;
	stack->item = calloc(stack->total, sizeof(TMEMITEM));
	stack->size = 0;
}

static void reallocstack (TMEMALLOCSTACK *stack, const int total)
{
	stack->total = total;
	stack->item = realloc(stack->item, total * sizeof(TMEMITEM));
}

// if addr exists and unfreed, return 1
// if addr exists but is freed, return -1
// else return 0
static int checkstack (TMEMALLOCSTACK *stack, void *addr)
{
	for (int i = 0; i < stack->size; i++){
		if (stack->item[i].addr == addr && stack->item[i].status == 1)
			return 1;
		else if (stack->item[i].addr == addr && stack->item[i].status == 0)
			return -1;
	}
	return 0;
}

static void removestack (TMEMALLOCSTACK *stack, const void *addr)
{
	for (int i = 0; i < stack->size; i++){
		if (stack->item[i].addr == addr){
			stack->item[i].status = 0;
			stack->totalFreed += stack->item[i].size;
			return;
		}
	}
}

static int countactivestack (TMEMALLOCSTACK *stack)
{
	unsigned int ct = 0;
	for (int i = 0; i < stack->size; i++){
		if (stack->item[i].status == 1)
			ct++;
	}
	return ct;
}

static int getslotstack (TMEMALLOCSTACK *stack, const void *addr)
{
	int found = -1;
	for (int i = 0; i < stack->size; i++){
		if (!stack->item[i].status){
			if (stack->item[i].addr == addr)
				return i;
			found = i;
		}
	}

	if (found >= 0) return found;
	/*
	for (int i = 0; i < stack->size; i++){
		if (!stack->item[i].status)
			return i;
	}*/

	if (stack->size+1 >= stack->total)
		reallocstack(stack, stack->total<<1);
	return stack->size++;
}

static void addstack (TMEMALLOCSTACK *stack, void *addr, const size_t size, const char *func, const int line)
{
	
	//printf("@@ 0x%p:%u: %s:%i\n", addr, size, func, line);
	
	const int slot = getslotstack(stack, addr);
	stack->item[slot].status = 1;
	stack->item[slot].addr = addr;
	stack->item[slot].size = size;
	stack->item[slot].line = line;
	stack->item[slot].func = (char*)func;
	stack->totalRequested += size;
}

int initMemory ()
{
	//printf("initMemory %i %i\n", mstack.size, mstack.total);
	
	if (!mstack.total){
		memset(&mstack, 0, sizeof(TMEMALLOCSTACK));
		allocstack(&mstack, 4096);
		//mstack.hMutex = CreateMutex(NULL, FALSE, NULL);
		InitializeCriticalSectionAndSpinCount(&mstack.cs, 8192);
	}
	return 1;
}

static void _dumpMemStats (TMEMALLOCSTACK *stack, const int dumpLeaks)
{
	printf("memset: %i\n", stack->memsetCount);
	printf("memcpy: %i\n", stack->memcpyCount);
	printf("memory stack size: %i of %i\n", stack->size, stack->total);
	printf("active allocs: %i\n", countactivestack(stack));
	printf("alloc: %i\n", stack->allocCount);
	printf("free: %i\n", stack->freeCount);
	if (dumpLeaks)
		printf("leaks: %i\n", stack->allocCount - stack->freeCount);
	printf("realloc: %i\n", stack->reallocCount);
	printf("largest alloc request: %I64d\n", stack->maxRequestSize);
	printf("total memory requested: %I64d\n", stack->totalRequested);
	printf("total memory freed: %I64d\n", stack->totalFreed);
	printf("alloc delta: %I64d\n", stack->totalRequested-stack->totalFreed);

	if (dumpLeaks){
		for (int i = 0; i < stack->size; i++){
			if (stack->item[i].status)
				printf("@@ leak: %p:%i %s:%i\n", stack->item[i].addr, stack->item[i].size, stack->item[i].func, stack->item[i].line);
		}
	}
}

void dumpMemStats (THWD *hw)
{
	dumpFrameCacheDetail((TFRAMECACHE*)hw->flist);
	memlock(&mstack);
	printf("mem lock ct: %i\n", mstack.lockCount);
	_dumpMemStats(&mstack, 0);
	memunlock(&mstack);
}

void closeMemory ()
{
	_dumpMemStats(&mstack, 1);
	
/*	memlock(&mstack);
	free(mstack.item);
	//CloseHandle(mstack.hMutex);
	DeleteCriticalSection(&mstack.cs);
	memset(&mstack, 0, sizeof(TMEMALLOCSTACK));*/
}
#endif


#if 1

void * my_malloc (size_t size, const char *func, const int line)
{
	if (!mstack.total) initMemory();
	
	if (((int)size == -1) || !size){
		printf("@@ malloc, invalid request, invalid size %i, %s:%i\n", (int)size, func, line);
		return NULL;
	}

	const size_t memsize = alignSize(size);
	void *addr = malloc(memsize);
	memlock(&mstack);
	if (memsize > mstack.maxRequestSize) mstack.maxRequestSize = memsize;
	addstack(&mstack, addr, memsize, func, line);
	mstack.allocCount++;
	memunlock(&mstack);
	return addr;
}

void * my_calloc (size_t nelem, size_t elsize, const char *func, const int line)
{
	if (!nelem || !elsize){
		printf("@@ calloc, bad request, invalid size %u,%u %s:%i\n",nelem, elsize, func, line);
		return NULL;
	}
	const size_t size = nelem*elsize;
	void *p = my_malloc(size, func, line);
	if (p) l_memset(p, 0, size);
	return p;
}

void * my_realloc (void *ptr, size_t size, const char *func, const int line)
{
	if (!ptr) return my_malloc(size, func, line);
	if (!mstack.total) initMemory();

	memlock(&mstack);

	const int status = checkstack(&mstack, ptr);
	if (status == -1){
		printf("@@ realloc: address was freed previously, %p:%u %s:%i\n", ptr, size, func, line);
		memunlock(&mstack);
		return NULL;
	}else if (status == 0){
		printf("@@ realloc: address not found, %p:%u %s:%i\n", ptr, size, func, line);
		memunlock(&mstack);
		return NULL;
	}else{
		removestack(&mstack, ptr);
	}

	const size_t memsize = alignSize(size);
	void *addr = realloc(ptr, memsize);
	if (addr){
		addstack(&mstack, addr, memsize, func, line);	
		if (memsize > mstack.maxRequestSize) mstack.maxRequestSize = memsize;
		mstack.reallocCount++;
	}
	memunlock(&mstack);
	return addr;
}

void my_free (void *ptr, const  char *func, const int line)
{
	if (!mstack.total) initMemory();
	
	if (ptr){
		memlock(&mstack);
		const int status = checkstack(&mstack, ptr);
		
		if (status == -1){
			memunlock(&mstack);
			printf("@@ free: double free, %p %s:%i\n", ptr, func, line);
			return;
		}else if (!status){
			memunlock(&mstack);
			printf("@@ free: nonexistant address, %p %s:%i\n", ptr, func, line);
			return;
		}

		removestack(&mstack, ptr);
		mstack.freeCount++;
		memunlock(&mstack);
		free(ptr);
	}
}

char * my_strdup (const char *str, const char *func, const int line)
{
	if (str){
		size_t n = l_strlen(str);
		char *p = (char*)my_malloc(alignSize(sizeof(char)*n + sizeof(char)), func, line);
		if (p){
			p[n] = 0;
			return l_strncpy(p, str, n);
		}
	}
	return NULL;
}

wchar_t * my_wcsdup (const wchar_t *str, const char *func, const int line)
{
	size_t n = l_wcslen(str);
	wchar_t *p = (wchar_t*)my_malloc(alignSize(sizeof(wchar_t)*n + sizeof(wchar_t)), func, line);
	if (p){
		p[n] = 0;
		return l_wcsncpy(p, str, n);
	}
	return NULL;
}


#else

#define MAGIC		((uint32_t)0xbeefdead)


static inline size_t getLength (const void *const ptr)
{
	return *(size_t*)(ptr - sizeof(size_t));
}

static inline uint32_t getMagicBefore (const void *const ptr)
{
	const unsigned char *mptr = (unsigned char*)(ptr - sizeof(size_t) - sizeof(uint32_t));
	const uint32_t magic = *(uint32_t*)mptr;
	return magic;
}

static inline uint32_t getMagicAfter (const void *const ptr)
{
	const size_t len = getLength(ptr);
	const unsigned char *mptr = (unsigned char*)ptr + len;
	const uint32_t magic = *(uint32_t*)mptr;
	return magic;
}

static inline uint32_t magicCheck (const void *const ptr)
{
	const uint32_t b = getMagicBefore(ptr);
	const uint32_t a = getMagicAfter(ptr);
	return (b == MAGIC && a == MAGIC);
}

void * my_malloc (size_t size, const char *func, const int line)
{
	
	if (!size){
		printf("malloc(%u): ZERO LENGTH '%s' %i\n", size, func, line);
		abort();
	}

	void *ptr = malloc(sizeof(uint32_t) + sizeof(size_t)+ size + sizeof(size_t));
	if (!ptr) return NULL;


	unsigned char *mptr = (unsigned char*)ptr;
	*(uint32_t*)mptr = MAGIC;

	*(size_t*)(ptr+sizeof(uint32_t)) = (size_t)size;

	mptr = (unsigned char*)ptr + sizeof(uint32_t) + sizeof(size_t) + size;
	*(uint32_t*)mptr = MAGIC;

	printf("malloc(%u): %p/%p, %s:%i\n", size, ptr, ptr + sizeof(uint32_t)+ sizeof(size_t), func, line);

	return ptr + sizeof(uint32_t)+ sizeof(size_t);
}

void * my_calloc (size_t nelem, size_t elsize, const char *func, const int line)
{

	size_t len = (nelem * elsize);
	if (!len){
		printf("calloc(%u, %u): '%s' %i\n", nelem, elsize, func, line);
		abort();
	}
	//len += 4;
	
	void *ptr = my_malloc(len, func, line);
	if (ptr)
		memset(ptr, 0, len);

	//printf("calloc(%i, %i): %p, '%s' %i\n", nelem, elsize, ptr, func, line);
	return ptr;
}

void * my_realloc (void *_ptr, size_t size, const char *func, const int line)
{
	if (!_ptr || !size)
		return my_malloc(size, func, line);

	if (!magicCheck(_ptr)){
		printf("## realloc(%p, %u): MAGIC MISMATCH %s:%i\n", _ptr, size, func, line);
		return NULL;
	}
	

	void *ptr = my_malloc(size, func, line);
	if (!ptr) return NULL;

	size_t len = getLength(_ptr);
	if (len > size) len = size;
	//printf("realloc(%p, %i): %i %p, %s:%i\n", _ptr, len, size, ptr, func, line);
	
	mmx_memcpy(ptr, _ptr, len);
	if (size > len)
		memset(ptr+len, 0, size - len);
	
	my_free(_ptr, func, line);
	//printf("realloc(%p, %i): %p, '%s' %i\n", _ptr, size, ptr, func, line);
	return ptr;
}

void my_free (void *ptr, const char *func, const int line)
{
	if (!ptr) return;
	
	if (!magicCheck(ptr)){
		printf("## free(%p): MAGIC MISMATCH %s:%i\n", ptr, func, line);
		return;
	}
	
	size_t len = getLength(ptr);
	printf("free(%p/%p): %i %s:%i\n", ptr - sizeof(uint32_t) - sizeof(size_t), ptr, len, func, line);
	
	memset(ptr - sizeof(uint32_t) - sizeof(size_t), 0, len+sizeof(uint32_t) + sizeof(size_t));
	
	free(ptr - sizeof(uint32_t) - sizeof(size_t));
}

char * my_strdup (const char *str, const char *func, const int line)
{
	//char *ptr = _strdup(str);
	int len = strlen(str);
	if (len > 0){
		char *ptr = my_malloc((len*sizeof(char)) + sizeof(char), func, line);
		if (ptr){
			mmx_memcpy(ptr, str, len*sizeof(char));
			ptr[len] = 0;
			
			//printf("strdup(%p): %p, '%s' %i\n", str, ptr, func, line);
			return ptr;
		}
	}
	
	printf("strdup(%p)", str);
	return NULL;
}

wchar_t * my_wcsdup (const wchar_t *str, const char *func, const int line)
{
	//wchar_t *ptr = _wcsdup(str);
	int len = wcslen(str);
	if (len > 0){
		wchar_t *ptr = my_malloc((len*sizeof(wchar_t)) + sizeof(wchar_t), func, line);
		if (ptr){
			mmx_memcpy(ptr, str, len*sizeof(wchar_t));
			ptr[len] = 0;

			//printf("wcsdup(%p): %p, '%s' %i\n", str, ptr, func, line);
			return ptr;
		}
	}
	
	printf("wcsdup(%p)", str);
	return NULL;
}

int initMemory ()
{
	return 1;
}

void closeMemory ()
{
}

void dumpMemStats (THWD *hw)
{
	printf("__DEBUG_MEM__ not compiled in\n");
}

#endif

int l_memcmp (void *s1, const void *s2, size_t count)
{
	return memcmp(s1, s2, count);
}

void *l_memset (void *s, int c, size_t count)
{
	mstack.memsetCount++;
	return memset(s, c, count);
}

void *mmx_memcpy_dbg (void *s1, const void *s2, size_t n)
{
	mstack.memcpyCount++;
	mmx_memcpy(s1, s2, n);
	return s1;
}

void *memcpy_dbg (void *s1, const void *s2, size_t n)
{
	mstack.memcpyCount++;
	return memcpy(s1, s2, n);
}

#else

int initMemory ()
{
	return 1;
}

void closeMemory ()
{
}

void dumpMemStats (THWD *hw)
{
	printf("__DEBUG_MEM__ not compiled in\n");
}

#if 1
void * my_malloc (size_t size)
{
	return malloc(alignSize(size));
}

void * my_calloc (size_t nelem, size_t elsize)
{
	return calloc(1, alignSize(nelem * elsize));
}

void * my_realloc (void *ptr, size_t size)
{
	return realloc(ptr, alignSize(size));
}

void my_free (void *ptr)
{
	free(ptr);
}

char * my_strdup (const char *str)
{
	return _strdup(str);
}

wchar_t * my_wcsdup (const wchar_t *str)
{
	return _wcsdup(str);
}
#endif




int l_memcmp (void *s1, const void *s2, size_t count)
{
	return memcmp(s1, s2, count);
}

void *l_memset (void *s, int c, size_t count)
{
	return memset(s, c, count);
}

#endif


