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

#ifndef _RMAP_H_
#define _RMAP_H_

#include "rlib/rcarray.h"
#include "rlib/rhash.h"
#include "rlib/rlist.h"
#include "rlib/rstring.h"
#include "rlib/rgc.h"
#include "rlib/robject.h"

#ifdef __cplusplus
extern "C" {
#endif


#define r_map_hashsize(__m__) ((unsigned long)((unsigned long)1 << (__m__)->nbits))
#define r_map_hashmask(__m__) (r_map_hashsize(__m__) - 1)

typedef struct rmap_s {
	robject_t obj;
	unsigned long nbits;
	unsigned long elt_size;
	rcarray_t *data;
	rlist_t *hash;
	rlist_t active;
	rlist_t inactive;
} rmap_t;


rmap_t *r_map_create(unsigned int elt_size, unsigned int nbits);
void r_map_destroy(rmap_t *array);
long r_map_lookup(rmap_t *map, long current, const char *name, unsigned int namesize);
long r_map_lookup_s(rmap_t *map, long current, const char *name);
long r_map_taillookup(rmap_t *map, long current, const char *name, unsigned int namesize);
long r_map_taillookup_s(rmap_t *map, long current, const char *name);
long r_map_lookup_d(rmap_t *map, long current, double name);
long r_map_lookup_l(rmap_t *map, long current, long name);
long r_map_add(rmap_t *map, const char *name, unsigned int namesize, rconstpointer pval);
long r_map_add_s(rmap_t *map, const char *name, rconstpointer pval);
long r_map_add_d(rmap_t *map, double name, rconstpointer pval);
long r_map_add_l(rmap_t *map, long name, rconstpointer pval);

/*
 * The following functions allow the created keys (rstring_t objects) to be added to
 * GC list and not being destroyed by the rmap_t, but leave it to the users of rmap_t
 * to decide when to destroy those keys. These is useful for scripting languages with
 * GC memory management. Another possibility would be to get the key as a rstrit_t* and
 * make rmap_t completely get out of the memory management business.
 */
long r_map_gckey_add(rmap_t *map, rgc_t* gc, const char *name, unsigned int namesize, rconstpointer pval);
long r_map_gckey_add_s(rmap_t *map, rgc_t* gc, const char *name, rconstpointer pval);
long r_map_gckey_add_d(rmap_t *map, rgc_t* gc, double name, rconstpointer pval);
long r_map_gckey_add_l(rmap_t *map, rgc_t* gc, long name, rconstpointer pval);
long r_map_setvalue(rmap_t *map, long index, rconstpointer pval);
rstring_t *r_map_key(rmap_t *map, unsigned long index);
rpointer r_map_value(rmap_t *map, unsigned long index);
int r_map_delete(rmap_t *map, unsigned long index);

long r_map_first(rmap_t *map);
long r_map_last(rmap_t *map);
long r_map_next(rmap_t *map, long current);
long r_map_prev(rmap_t *map, long current);


#ifdef __cplusplus
}
#endif

#endif /* _RMAP_H_ */
