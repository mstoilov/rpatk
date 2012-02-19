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

#ifndef _RPACACHE_H_
#define _RPACACHE_H_

#include "rtypes.h"
#include "rlib/rarray.h"
#include "rpa/rparecord.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RPA_MCACHE_BITS 10
#define RPA_MCACHE_SIZE (1 << RPA_MCACHE_BITS)
#define RPA_MCACHE_MASK (RPA_MCACHE_SIZE - 1)


typedef struct rpacachedentry_s {
	long ruleid;
	long top;
	long ret;
	long startrec;
	long recsize;
	unsigned long serial;
	rarray_t *records;
} rpacachedentry_t;

typedef struct rpacache_s {
	rpacachedentry_t entry[RPA_MCACHE_SIZE];
	long hit;
	long disalbled;
	unsigned long serial;
} rpacache_t;


rpacache_t *rpa_cache_create();
void rpa_cache_destroy(rpacache_t *cache);
void rpa_cache_disable(rpacache_t *cache, long disable);
void rpa_cache_invalidate(rpacache_t *cache);
void rpa_cache_set(rpacache_t *cache, long top, long ruleid, long ret, rarray_t *records, long startrec, long size);
rpacachedentry_t *rpa_cache_lookup(rpacache_t *cache, long top, long ruleid);
#ifdef __cplusplus
}
#endif

#endif
