#include "rpacache.h"
#include "rmem.h"
#include "rparecord.h"

#define RPA_MCACHE_BUCKET(_top_, _ruleid_) ( ( (((rulong)(_top_))<<7) ^ ((((rulong)(_ruleid_))>>4)) ) & RPA_MCACHE_MASK)


rpacache_t *rpa_cache_create()
{
	rpacache_t *cache = (rpacache_t*) r_zmalloc(sizeof(*cache));

	if (!cache)
		return NULL;
	return cache;
}


void rpa_cache_destroy(rpacache_t *cache)
{
	if (!cache)
		return;
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


void rpa_cache_set(rpacache_t *cache, rlong top, rlong ruleid, rlong ret, rlong startrec, rlong endrec)
{
	rulong bucket = RPA_MCACHE_BUCKET(top, ruleid);

	if (ret <= 0 || cache->disalbled)
		return;
	cache->entry[bucket].ruleid = ruleid;
	cache->entry[bucket].top = top;
	cache->entry[bucket].ret = ret;
	cache->entry[bucket].startrec = startrec;
	cache->entry[bucket].endrec = endrec;
	cache->entry[bucket].serial = cache->serial;

}


rpacachedentry_t *rpa_cache_lookup(rpacache_t *cache, rlong top, rlong ruleid)
{
	rulong bucket = RPA_MCACHE_BUCKET(top, ruleid);
	rpacachedentry_t *entry = &cache->entry[bucket];

	if (entry->serial == cache->serial && entry->ruleid == ruleid && entry->top == top) {
//		rparecord_t *prec = (rparecord_t *)r_array_slot(entry->records, 0);
//		r_printf("HIT the cache @ %ld,  top = %ld, ret = %ld, rulename = %s\n", bucket, entry->top, entry->ret, prec->rule);
		++cache->hit;
		return entry;
	}
	return NULL;
}
