#include <stdlib.h>
#include <string.h>

#include "rspinlock.h"
#include "rmem.h"

static rspinlock_t g_lock = R_SPINLOCK_INIT;
static rsize_t g_allocmem = 0;
static rsize_t g_maxmem = 0;

typedef struct rmallocvtable_s {
	rpointer (*malloc)(rsize_t size);
	void (*free)(rpointer ptr);
	rpointer (*realloc)(rpointer ptr, rsize_t size);
	rpointer (*calloc)(rsize_t nmemb, rsize_t size);
} rmallocvtable_t;


static rpointer r_std_malloc(rsize_t size)
{
	return malloc((size_t)size);
}


static rpointer r_std_calloc(rsize_t nmemb, rsize_t size)
{
	return calloc((size_t)nmemb, (size_t)size);
}


static void r_std_free(rpointer ptr)
{
	free((void*)ptr);
}


static rpointer r_std_realloc(rpointer ptr, rsize_t size)
{
	return realloc((void*)ptr, (size_t)size);
}


static rpointer r_dbg_calloc(rsize_t nmemb, rsize_t size)
{
	rword *mem = NULL;

	nmemb += (size < sizeof(rword)) ? sizeof(rword) : 1;
	mem = (rword*)calloc((size_t)nmemb, (size_t)size);
	*((rword*)mem) = size * nmemb;
	r_spinlock_lock(&g_lock);
	g_allocmem += size * nmemb;
	if (g_maxmem < g_allocmem)
		g_maxmem = g_allocmem;
	r_spinlock_unlock(&g_lock);
	mem += 1;
	return (void*)mem;
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


static rmallocvtable_t g_stdMemAlloc = {
	r_std_malloc,
	r_std_free,
	r_std_realloc,
	r_std_calloc,
};


static rmallocvtable_t g_dbgMemAlloc = {
	r_dbg_malloc,
	r_dbg_free,
	r_dbg_realloc,
	r_dbg_calloc,
};


#ifdef R_DEBUG_MEMALLOC
static rmallocvtable_t *g_pMemAlloc = &g_dbgMemAlloc;
#else
static rmallocvtable_t *g_pMemAlloc = &g_stdMemAlloc;
#endif


rpointer r_malloc(rsize_t size)
{
	return g_pMemAlloc->malloc(size);
}


rpointer r_realloc(rpointer ptr, rsize_t size)
{
	return g_pMemAlloc->realloc(ptr, size);
}


rpointer r_calloc(rsize_t nmemb, rsize_t size)
{
	return g_pMemAlloc->calloc(nmemb, size);
}


void r_free(rpointer ptr)
{
	g_pMemAlloc->free(ptr);
}


rpointer r_zmalloc(rsize_t size)
{
	void *mem;

	if ((mem = r_malloc(size)))
		r_memset(mem, 0, size);
	return mem;
}


rpointer r_memset(rpointer s, rinteger c, rsize_t n)
{
	return memset((void*)s, (int)c, (size_t)n);
}


rpointer r_memcpy(rpointer dest, rconstpointer src, rsize_t n)
{
	return memcpy((void*)dest, (const void*)src, (size_t)n);
}


rpointer r_memmove(rpointer dest, rconstpointer src, rsize_t n)
{
	return memmove((void*)dest, (const void*)src, (size_t)n);
}


rsize_t r_debug_get_allocmem()
{
	return g_allocmem;
}


rsize_t r_debug_get_maxmem()
{
	return g_maxmem;
}


void r_debug_reset_maxmem()
{
	g_maxmem = g_allocmem;
}


/*
 * This function has no meaning it is used just to
 * suppress some compiler warnings.
 */
void r_memalloc_do_not_use()
{
	rmallocvtable_t *pDbgMemAlloc = &g_dbgMemAlloc;
	rmallocvtable_t *pStdMemAlloc = &g_stdMemAlloc;

	if (pDbgMemAlloc == pStdMemAlloc)
		return;
	return;
}
