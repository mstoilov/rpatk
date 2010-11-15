/*
 *  Regular Pattern Analyzer (RPA)
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

#include "rpatypes.h"
#include "rpamem.h"

long int g_rpa_allocmem = 0;
long int g_rpa_maxmem = 0;


void *rpa_malloc(unsigned long size)
{
#ifdef RPA_DEBUG_MEM
	rpa_word_t *mem = NULL;
	size += sizeof(rpa_word_t);
	mem = (rpa_word_t*)malloc((size_t)(size));
	*((rpa_word_t*)mem) = size;
	g_rpa_allocmem += size;
	if (g_rpa_maxmem < g_rpa_allocmem)
		g_rpa_maxmem = g_rpa_allocmem;
	mem += 1;
	return (void*)mem;
#else
	return malloc((size_t)size);
#endif
}


void *rpa_zmalloc(unsigned long size) {
	void *mem;
	
	if ((mem = rpa_malloc(size)))
		rpa_memset(mem, 0, size);
	return mem;
}

void rpa_free(void *ptr)
{
#ifdef RPA_DEBUG_MEM
	rpa_word_t *mem = (void*)(((rpa_word_t*)ptr) - 1);
	rpa_word_t size;
	if (!ptr)
		return;
	size = *mem;
	g_rpa_allocmem -= size;
//	fprintf(stdout, "%p (%ld de-allocated)\n", (void*)mem, size);
	free((void*)mem);
#else
	free(ptr);
#endif
}


void *rpa_realloc(void *ptr, unsigned long size)
{
#ifdef RPA_DEBUG_MEM
	rpa_word_t *mem = (void*)(((rpa_word_t*)ptr) - 1);
	rpa_word_t csize;
	if (!ptr)
		return rpa_malloc(size);
	csize = *mem;
	g_rpa_allocmem -= csize;
	size += sizeof(long);
	mem = (rpa_word_t*)realloc((void*)mem, (size_t)(size));
	*mem = size;
	g_rpa_allocmem += size;
	if (g_rpa_maxmem < g_rpa_allocmem)
		g_rpa_maxmem = g_rpa_allocmem;
	mem += 1;
	return (void*)mem;
#else
	return realloc(ptr, (size_t)size);
#endif
}


void *rpa_memset(void *s, int c, unsigned long n)
{
	return memset(s, c, (size_t)n);
}


void *rpa_memcpy(void *dest, const void *src, unsigned long n)
{
	return memcpy(dest, src, (size_t)n);
}
