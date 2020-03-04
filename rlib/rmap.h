/*
 *  Regular Pattern Analyzer Toolkit (RPA/Tk)
 *  Copyright (c) 2009-2012 Martin Stoilov
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
#include "rlib/robject.h"

#ifdef __cplusplus
extern "C" {
#endif


#define r_map_hashsize(__m__) ((size_t)((size_t)1 << (__m__)->nbits))
#define r_map_hashmask(__m__) (r_map_hashsize(__m__) - 1)

typedef struct rmap_s {
	robject_t obj;
	size_t nbits;
	size_t elt_size;
	rcarray_t *data;
	rlist_t *hash;
	rlist_t active;
	rlist_t inactive;
} rmap_t;


rmap_t *r_map_create(size_t elt_size, size_t nbits);
void r_map_destroy(rmap_t *array);
size_t r_map_lookup(rmap_t *map, const char *name, size_t namesize);
size_t r_map_lookup_s(rmap_t *map, const char *name);
size_t r_map_taillookup(rmap_t *map, const char *name, size_t namesize);
size_t r_map_taillookup_s(rmap_t *map, const char *name);
size_t r_map_lookup_d(rmap_t *map, double name);
size_t r_map_lookup_l(rmap_t *map, long name);
size_t r_map_add(rmap_t *map, const char *name, size_t namesize, rconstpointer pval);
size_t r_map_add_s(rmap_t *map, const char *name, rconstpointer pval);
size_t r_map_add_d(rmap_t *map, double name, rconstpointer pval);
size_t r_map_add_l(rmap_t *map, long name, rconstpointer pval);

size_t r_map_setvalue(rmap_t *map, size_t index, rconstpointer pval);
rstring_t *r_map_key(rmap_t *map, size_t index);
rpointer r_map_value(rmap_t *map, size_t index);
rboolean r_map_delete(rmap_t *map, size_t index);

size_t r_map_first(rmap_t *map);
size_t r_map_last(rmap_t *map);
size_t r_map_next(rmap_t *map, size_t current);
size_t r_map_prev(rmap_t *map, size_t current);


#ifdef __cplusplus
}
#endif

#endif /* _RMAP_H_ */
