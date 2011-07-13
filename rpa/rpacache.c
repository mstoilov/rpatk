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

#include "rpa/rpacache.h"
#include "rlib/rmem.h"
#include "rpa/rparecord.h"

#define RPA_MCACHE_BUCKET(_top_, _ruleid_) ( ( (((unsigned long)(_top_))<<3) ^ ((((unsigned long)(_ruleid_))>>5)) ) & RPA_MCACHE_MASK)


rpacache_t *rpa_cache_create()
{
	long i;
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
	long i;
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


void rpa_cache_disable(rpacache_t *cache, long disable)
{
	cache->disalbled = disable;
}


void rpa_cache_set(rpacache_t *cache, long top, long ruleid, long ret, rarray_t *records, long startrec, long size)
{
	long i;
	unsigned long bucket = RPA_MCACHE_BUCKET(top, ruleid);
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
	if (records) {
		for (i = 0; i < size; i++) {
			r_array_add(cacherecords, r_array_slot(records, startrec + i));
		}
	}
}


rpacachedentry_t *rpa_cache_lookup(rpacache_t *cache, long top, long ruleid)
{
	unsigned long bucket = RPA_MCACHE_BUCKET(top, ruleid);
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
