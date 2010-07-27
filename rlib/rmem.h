#ifndef _RMEM_H_
#define _RMEM_H_


#include "rtypes.h"

#ifdef __cplusplus
extern "C" {
#endif


rpointer r_malloc(rsize_t size);
rpointer r_realloc(rpointer ptr, rsize_t size);
rpointer r_calloc(rsize_t nmemb, rsize_t size);
void r_free(rpointer ptr);
rpointer r_zmalloc(rsize_t size);
rpointer r_memset(rpointer s, rint c, rsize_t n);
rpointer r_memcpy(rpointer dest, rconstpointer src, rsize_t n);
rpointer r_memmove(rpointer dest, rconstpointer src, rsize_t n);
rsize_t r_debug_get_allocmem();
rsize_t r_debug_get_maxmem();
void r_debug_reset_maxmem();


#ifdef __cplusplus
}
#endif

#endif
