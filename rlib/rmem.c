/*
 *  Regular Pattern Analyzer Toolkit (RPA/Tk)
 *  Copyright (c) 2009-2012 Martin Stoilov
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Martin Stoilov <martin@rpasearch.com>
 */

#include <stdlib.h>
#include <string.h>

#include "rlib/rspinlock.h"
#include "rlib/rmem.h"

static rspinlock_t g_lock = R_SPINLOCK_INIT;
static rword g_allocmem = 0;
static rword g_maxmem = 0;

typedef struct rmallocvtable_s {
	rpointer (*malloc)(size_t size);
	void (*free)(rpointer ptr);
	rpointer (*realloc)(rpointer ptr, size_t size);
	rpointer (*calloc)(size_t nmemb, size_t size);
} rmallocvtable_t;


static rpointer r_std_malloc(size_t size)
{
	return malloc(size);
}


static rpointer r_std_calloc(size_t nmemb, size_t size)
{
	return calloc(nmemb, size);
}


static void r_std_free(rpointer ptr)
{
	free((void*)ptr);
}


static rpointer r_std_realloc(rpointer ptr, size_t size)
{
	return realloc((void*)ptr, size);
}


static rpointer r_dbg_calloc(size_t nmemb, size_t size)
{
	ruword *mem = NULL;

	nmemb += (size < sizeof(ruword)) ? sizeof(ruword) : 1;
	mem = (ruword*)calloc(nmemb, size);
	*((ruword*)mem) = size * nmemb;
	r_spinlock_lock(&g_lock);
	g_allocmem += size * nmemb;
	if (g_maxmem < g_allocmem)
		g_maxmem = g_allocmem;
	r_spinlock_unlock(&g_lock);
	mem += 1;
	return (void*)mem;
}


static rpointer r_dbg_malloc(size_t size)
{
	ruword *mem = NULL;
	size += sizeof(ruword);
	mem = (ruword*)malloc((size));
	*((ruword*)mem) = size;
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
	ruword *mem = (void*)(((ruword*)ptr) - 1);
	ruword size;
	if (!ptr)
		return;
	size = *mem;
	r_spinlock_lock(&g_lock);
	g_allocmem -= size;
	r_spinlock_unlock(&g_lock);
	free((void*)mem);
}


static rpointer r_dbg_realloc(rpointer ptr, size_t size)
{
	ruword *mem = (void*)(((ruword*)ptr) - 1);
	ruword csize;
	if (!ptr)
		return r_malloc(size);
	csize = *mem;
	r_spinlock_lock(&g_lock);
	g_allocmem -= csize;
	r_spinlock_unlock(&g_lock);
	size += sizeof(ruword);
	mem = (ruword*)realloc((void*)mem, (size));
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


rpointer r_malloc(size_t size)
{
	return g_pMemAlloc->malloc(size);
}


rpointer r_realloc(rpointer ptr, size_t size)
{
	return g_pMemAlloc->realloc(ptr, size);
}


rpointer r_calloc(size_t nmemb, size_t size)
{
	return g_pMemAlloc->calloc(nmemb, size);
}


void r_free(rpointer ptr)
{
	g_pMemAlloc->free(ptr);
}


rpointer r_zmalloc(size_t size)
{
	void *mem;

	if ((mem = r_malloc(size)))
		r_memset(mem, 0, size);
	return mem;
}


rpointer r_memset(rpointer s, int c, size_t n)
{
	return memset((void*)s, (int)c, n);
}


rpointer r_memcpy(rpointer dest, rconstpointer src, size_t n)
{
	return memcpy((void*)dest, (const void*)src, n);
}


rpointer r_memmove(rpointer dest, rconstpointer src, size_t n)
{
	return memmove((void*)dest, (const void*)src, n);
}


size_t r_debug_get_allocmem()
{
	return g_allocmem;
}


size_t r_debug_get_maxmem()
{
	return g_maxmem;
}


void r_debug_reset_maxmem()
{
	g_maxmem = g_allocmem;
}





#if 0
static inline rpointer r_std_malloc(size_t size)
{
	return malloc(size);
}

static inline rpointer r_std_calloc(size_t nmemb, size_t size)
{
	return calloc(nmemb, size);
}

static inline void r_std_free(rpointer ptr)
{
	free((void*)ptr);
}

static inline rpointer r_std_realloc(rpointer ptr, size_t size)
{
	return realloc((void*)ptr, size);
}

rpointer r_malloc(size_t size)
{
	return r_std_malloc(size);
}

rpointer r_realloc(rpointer ptr, size_t size)
{
	return r_std_realloc(ptr, size);
}

rpointer r_calloc(size_t nmemb, size_t size)
{
	return r_std_calloc(nmemb, size);
}

void r_free(rpointer ptr)
{
	r_std_free(ptr);
}

rpointer r_zmalloc(size_t size)
{
	void *mem;

	if ((mem = r_malloc(size)))
		r_memset(mem, 0, size);
	return mem;
}

rpointer r_memset(rpointer s, int32_t c, size_t n)
{
	return memset((void*)s, (int)c, n);
}

rpointer r_memcpy(rpointer dest, rconstpointer src, size_t n)
{
	return memcpy((void*)dest, (const void*)src, n);
}

rpointer r_memmove(rpointer dest, rconstpointer src, size_t n)
{
	return memmove((void*)dest, (const void*)src, n);
}

size_t r_debug_get_allocmem()
{
	return 0;
}

size_t r_debug_get_maxmem()
{
	return 0;
}

void r_debug_reset_maxmem()
{

}

#endif

