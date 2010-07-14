#ifndef _RMEM_H_
#define _RMEM_H_


#include "rtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

extern long int g_r_allocmem;
extern long int g_r_maxmem;


void *r_malloc(unsigned long size);
void *r_zmalloc(unsigned long size);
void r_free(void *ptr);
void *r_realloc(void *ptr, unsigned long size);
void *r_memset(void *s, int c, unsigned long n);
void *r_memcpy(void *dest, const void *src, unsigned long n);

#ifdef __cplusplus
}
#endif

#endif
