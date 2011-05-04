#ifndef _RPACACHE_H_
#define _RPACACHE_H_

#include "rtypes.h"
#include "rarray.h"
#include "rparecord.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RPA_MCACHE_BITS 9
#define RPA_MCACHE_SIZE (1 << RPA_MCACHE_BITS)
#define RPA_MCACHE_MASK (RPA_MCACHE_SIZE - 1)


typedef struct rpacachedentry_s {
	rlong ruleid;
	rlong top;
	rlong ret;
	rlong startrec;
	rlong recsize;
	rulong serial;
	rarray_t *records;
} rpacachedentry_t;

typedef struct rpacache_s {
	rpacachedentry_t entry[RPA_MCACHE_SIZE];
	rlong hit;
	rlong disalbled;
	rulong serial;
} rpacache_t;


rpacache_t *rpa_cache_create();
void rpa_cache_destroy(rpacache_t *cache);
void rpa_cache_disable(rpacache_t *cache, rlong disable);
void rpa_cache_invalidate(rpacache_t *cache);
void rpa_cache_set(rpacache_t *cache, rlong top, rlong ruleid, rlong ret, rarray_t *records, rlong startrec, rlong size);
rpacachedentry_t *rpa_cache_lookup(rpacache_t *cache, rlong top, rlong ruleid);
#ifdef __cplusplus
}
#endif

#endif
