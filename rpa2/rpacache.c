#include "rpacache.h"
#include "rmem.h"
#include "rparecord.h"

#define RPA_MCACHE_BUCKET(_top_, _ruleid_) ( ( (((rulong)(_top_))<<3) ^ ((((rulong)(_ruleid_))>>5)) ) & RPA_MCACHE_MASK)


rpacache_t *rpa_cache_create()
{
	rlong i;
	rpacache_t *cache = (rpacache_t*) r_zmalloc(sizeof(*cache));

	if (!cache)
		return NULL;
	for (i = 0; i < RPA_MCACHE_SIZE; i++) {
		cache->entry[i].records = r_array_create(sizeof(rparecord_t));
	}
	return cache;
}


void rpa_cache_destroy(rpacache_t *cache)
{
	rlong i;
	if (!cache)
		return;
	for (i = 0; i < RPA_MCACHE_SIZE; i++) {
		r_array_destroy(cache->entry[i].records);
	}
	r_free(cache);
}

void rpa_cache_invalidate(rpacache_t *cache)
{
	++cache->serial;
}


void rpa_cache_disable(rpacache_t *cache, rlong disable)
{
	cache->disalbled = disable;
}


void rpa_cache_set(rpacache_t *cache, rlong top, rlong ruleid, rlong ret, rarray_t *records, rlong startrec, rlong size)
{
	rlong i;
	rulong bucket = RPA_MCACHE_BUCKET(top, ruleid);
	rarray_t *cacherecords;

	if (cache->disalbled)
		return;

	if (ret > 0 && size > 128)
		return;
	cacherecords = cache->entry[bucket].records;
	cache->entry[bucket].ruleid = ruleid;
	cache->entry[bucket].top = top;
	cache->entry[bucket].ret = ret;
	cache->entry[bucket].startrec = startrec;
	cache->entry[bucket].recsize = size;
	cache->entry[bucket].serial = cache->serial;
	r_array_setlength(cacherecords, 0);
	for (i = 0; i < size; i++) {
		r_array_add(cacherecords, r_array_slot(records, startrec + i));
	}

}


rpacachedentry_t *rpa_cache_lookup(rpacache_t *cache, rlong top, rlong ruleid)
{
	rulong bucket = RPA_MCACHE_BUCKET(top, ruleid);
	rpacachedentry_t *entry = &cache->entry[bucket];

	if (cache->disalbled)
		return NULL;

	if (entry->serial == cache->serial && entry->ruleid == ruleid && entry->top == top) {
//		rparecord_t *prec = (rparecord_t *)r_array_slot(entry->records, 0);
//		r_printf("HIT the cache @ %ld,  top = %ld, ret = %ld, rulename = %s\n", bucket, entry->top, entry->ret, prec->rule);
		++cache->hit;
		return entry;
	}
	return NULL;
}
