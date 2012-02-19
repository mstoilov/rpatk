/*
 *  Regular Pattern Analyzer Toolkit (RPA/Tk)
 *  Copyright (c) 2009-2010 Martin Stoilov
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
	rpointer (*malloc)(unsigned long size);
	void (*free)(rpointer ptr);
	rpointer (*realloc)(rpointer ptr, unsigned long size);
	rpointer (*calloc)(unsigned long nmemb, unsigned long size);
} rmallocvtable_t;


static rpointer r_std_malloc(unsigned long size)
{
	return malloc((size_t)size);
}


static rpointer r_std_calloc(unsigned long nmemb, unsigned long size)
{
	return calloc((size_t)nmemb, (size_t)size);
}


static void r_std_free(rpointer ptr)
{
	free((void*)ptr);
}


static rpointer r_std_realloc(rpointer ptr, unsigned long size)
{
	return realloc((void*)ptr, (size_t)size);
}


static rpointer r_dbg_calloc(unsigned long nmemb, unsigned long size)
{
	ruword *mem = NULL;

	nmemb += (size < sizeof(ruword)) ? sizeof(ruword) : 1;
	mem = (ruword*)calloc((size_t)nmemb, (size_t)size);
	*((ruword*)mem) = size * nmemb;
	r_spinlock_lock(&g_lock);
	g_allocmem += size * nmemb;
	if (g_maxmem < g_allocmem)
		g_maxmem = g_allocmem;
	r_spinlock_unlock(&g_lock);
	mem += 1;
	return (void*)mem;
}


static rpointer r_dbg_malloc(unsigned long size)
{
	ruword *mem = NULL;
	size += sizeof(ruword);
	mem = (ruword*)malloc((size_t)(size));
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


static rpointer r_dbg_realloc(rpointer ptr, unsigned long size)
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
	mem = (ruword*)realloc((void*)mem, (size_t)(size));
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


rpointer r_malloc(unsigned long size)
{
	return g_pMemAlloc->malloc(size);
}


rpointer r_realloc(rpointer ptr, unsigned long size)
{
	return g_pMemAlloc->realloc(ptr, size);
}


rpointer r_calloc(unsigned long nmemb, unsigned long size)
{
	return g_pMemAlloc->calloc(nmemb, size);
}


void r_free(rpointer ptr)
{
	g_pMemAlloc->free(ptr);
}


rpointer r_zmalloc(unsigned long size)
{
	void *mem;

	if ((mem = r_malloc(size)))
		r_memset(mem, 0, size);
	return mem;
}


rpointer r_memset(rpointer s, int c, unsigned long n)
{
	return memset((void*)s, (int)c, (size_t)n);
}


rpointer r_memcpy(rpointer dest, rconstpointer src, unsigned long n)
{
	return memcpy((void*)dest, (const void*)src, (size_t)n);
}


rpointer r_memmove(rpointer dest, rconstpointer src, unsigned long n)
{
	return memmove((void*)dest, (const void*)src, (size_t)n);
}


unsigned long r_debug_get_allocmem()
{
	return (unsigned long)g_allocmem;
}


unsigned long r_debug_get_maxmem()
{
	return (unsigned long)g_maxmem;
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
