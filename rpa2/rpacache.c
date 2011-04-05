#include "rpacache.h"
#include "rmem.h"
#include "rparecord.h"

#define RPA_MCACHE_BUCKET(_top_, _ruleid_) ( ( ((rulong)(_top_)) ^ ((rulong)(_ruleid_)) ) & RPA_MCACHE_MASK)


rpacache_t *rpa_cache_create()
{
	rint i;
	rpacache_t *cache = (rpacache_t*) r_zmalloc(sizeof(*cache));

	if (!cache)
		return NULL;
	for (i = 0; i < RPA_MCACHE_SIZE; i++) {
		cache->entry[i].records = r_array_create(sizeof(rparecord_t));
		if (!cache->entry[i].records) {
			rpa_cache_destroy(cache);
			return NULL;
		}
	}

	return cache;
}


void rpa_cache_destroy(rpacache_t *cache)
{
	rint i;

	if (!cache)
		return;
	for (i = 0; i < RPA_MCACHE_SIZE; i++) {
		if (cache->entry[i].records) {
			r_array_destroy(cache->entry[i].records);
		}
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


void rpa_cache_set(rpacache_t *cache, rlong top, rlong ruleid, rlong ret, rparecord_t* records, rsize_t nrecords)
{
	rlong i;
	rulong bucket = RPA_MCACHE_BUCKET(top, ruleid);

	if (ret <= 0 || cache->disalbled)
		return;
//	r_printf("Set the cache @ %ld for: top = %ld, ret = %d, nrecors = %ld, rulename = %s\n", bucket, top, ret, nrecords, records->rule);
	cache->entry[bucket].ruleid = ruleid;
	cache->entry[bucket].top = top;
	cache->entry[bucket].ret = ret;
	cache->entry[bucket].serial = cache->serial;
	r_array_setlength(cache->entry[bucket].records, 0);
	for (i = 0; i < nrecords; i++)
		r_array_add(cache->entry[bucket].records, &records[i]);
}


rpacachedentry_t *rpa_cache_lookup(rpacache_t *cache, rlong top, rlong ruleid)
{
	rulong bucket = RPA_MCACHE_BUCKET(top, ruleid);
	rpacachedentry_t *entry = &cache->entry[bucket];

	if (entry->serial == cache->serial && entry->ruleid == ruleid && entry->top == top) {
		rparecord_t *prec = (rparecord_t *)r_array_slot(entry->records, 0);
//		r_printf("HIT the cache @ %ld,  top = %ld, ret = %ld, rulename = %s\n", bucket, entry->top, entry->ret, prec->rule);
		++cache->hit;
		return entry;
	}
	return NULL;
}
