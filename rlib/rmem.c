#include <stdlib.h>
#include <string.h>

#include "rmem.h"

long int g_allocmem = 0;
long int g_maxmem = 0;


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


rboolean r_atomic_int_compare_and_exchange (volatile rint *atomic, rint oldval, rint newval)
{
  rint result;

  __asm__ __volatile__ ("lock; cmpxchgl %2, %1"
            : "=a" (result), "=m" (*atomic)
            : "r" (newval), "m" (*atomic), "0" (oldval));

  return result == oldval;
}
