#include <stdlib.h>
#include <string.h>

#include "rspinlock.h"
#include "rmem.h"

static rspinlock g_lock = R_SPINLOCK_INIT;
static long int g_allocmem = 0;
static long int g_maxmem = 0;

typedef struct _RMemAllocVTable {
	rpointer (*malloc)(rsize_t size);
	void (*free)(rpointer ptr);
	rpointer (*realloc)(rpointer ptr, rsize_t size);
} RMemAllocVTable;


static rpointer r_std_malloc(rsize_t size)
{
	return malloc((size_t)size);
}


static void r_std_free(rpointer ptr)
{
	free((void*)ptr);
}


static rpointer r_std_realloc(rpointer ptr, rsize_t size)
{
	return realloc((void*)ptr, (size_t)size);
}


static rpointer r_dbg_malloc(rsize_t size)
{
	rword *mem = NULL;
	size += sizeof(rword);
	mem = (rword*)malloc((size_t)(size));
	*((rword*)mem) = size;
	r_spinlock_lock(&g_lock);
	g_allocmem += size;
	if (g_maxmem < g_allocmem)
		g_maxmem = g_allocmem;
	r_spinlock_unlock(&g_lock);
	mem += 1;
	return (void*)mem;
}


static void r_dbg_free(rpointer ptr)
{
	rword *mem = (void*)(((rword*)ptr) - 1);
	rword size;
	if (!ptr)
		return;
	size = *mem;
	r_spinlock_lock(&g_lock);
	g_allocmem -= size;
	r_spinlock_unlock(&g_lock);
	free((void*)mem);
}


static rpointer r_dbg_realloc(rpointer ptr, rsize_t size)
{
	rword *mem = (void*)(((rword*)ptr) - 1);
	rword csize;
	if (!ptr)
		return r_malloc(size);
	csize = *mem;
	r_spinlock_lock(&g_lock);
	g_allocmem -= csize;
	r_spinlock_unlock(&g_lock);
	size += sizeof(long);
	mem = (rword*)realloc((void*)mem, (size_t)(size));
	*mem = size;
	r_spinlock_lock(&g_lock);
	g_allocmem += size;
	if (g_maxmem < g_allocmem)
		g_maxmem = g_allocmem;
	r_spinlock_unlock(&g_lock);
	mem += 1;
	return (void*)mem;
}


static RMemAllocVTable g_stdMemAlloc = {
	r_std_malloc,
	r_std_free,
	r_std_realloc,
};


static RMemAllocVTable g_dbgMemAlloc = {
	r_dbg_malloc,
	r_dbg_free,
	r_dbg_realloc,
};


rpointer r_malloc(unsigned long size)
{
#ifdef RPA_DEBUG_MEM
	rword *mem = NULL;
	size += sizeof(rword);
	mem = (rword*)malloc((size_t)(size));
	*((rword*)mem) = size;
	g_allocmem += size;
	if (g_maxmem < g_allocmem)
		g_maxmem = g_allocmem;
	mem += 1;
	return (void*)mem;
#else
	return malloc((size_t)size);
#endif
}


void *r_zmalloc(unsigned long size) {
	void *mem;

	if ((mem = r_malloc(size)))
		r_memset(mem, 0, size);
	return mem;
}

void r_free(void *ptr)
{
#ifdef RPA_DEBUG_MEM
	rword *mem = (void*)(((rword*)ptr) - 1);
	rword size;
	if (!ptr)
		return;
	size = *mem;
	g_allocmem -= size;
	free((void*)mem);
#else
	free(ptr);
#endif
}


void *r_realloc(void *ptr, unsigned long size)
{
#ifdef RPA_DEBUG_MEM
	rword *mem = (void*)(((rword*)ptr) - 1);
	rword csize;
	if (!ptr)
		return r_malloc(size);
	csize = *mem;
	g_allocmem -= csize;
	size += sizeof(long);
	mem = (rword*)realloc((void*)mem, (size_t)(size));
	*mem = size;
	g_allocmem += size;
	if (g_maxmem < g_allocmem)
		g_maxmem = g_allocmem;
	mem += 1;
	return (void*)mem;
#else
	return realloc(ptr, (size_t)size);
#endif
}


void *r_memset(void *s, int c, unsigned long n)
{
	return memset(s, c, (size_t)n);
}


void *r_memcpy(void *dest, const void *src, unsigned long n)
{
	return memcpy(dest, src, (size_t)n);
}

