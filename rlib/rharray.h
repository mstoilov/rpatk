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

#ifndef _RHARRAY_H_
#define _RHARRAY_H_

#include "rtypes.h"
#include "rlib/rarray.h"
#include "rlib/rcarray.h"
#include "rlib/rhash.h"
#include "rlib/rstring.h"
#include "rlib/robject.h"

#ifdef __cplusplus
extern "C" {
#endif


#define r_harray_index(__harray__, __index__, __type__) r_carray_index((__harray__)->members, __index__, __type__)
#define r_harray_length(__harray__) r_carray_length((__harray__)->members)
#define r_harray_index(__harray__, __index__, __type__) r_carray_index((__harray__)->members, __index__, __type__)
#define r_harray_slot(__harray__, __index__) r_carray_slot((__harray__)->members, __index__)

typedef struct rharray_s {
	robject_t obj;
	rcarray_t *members;
	rarray_t *names;
	rhash_t *hash;
} rharray_t;


rharray_t *r_harray_create(unsigned int elt_size);
void r_harray_destroy(rharray_t *array);
robject_t *r_harray_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy, unsigned int elt_size);
long r_harray_add(rharray_t *harray, const char *name, unsigned int namesize, rconstpointer pval);
long r_harray_add_s(rharray_t *harray, const char *name, rconstpointer pval);
long r_harray_replace(rharray_t *harray, const char *name, unsigned int namesize, rconstpointer pval);
long r_harray_replace_s(rharray_t *harray, const char *name, rconstpointer pval);
long r_harray_lookup(rharray_t *harray, const char *name, unsigned int namesize);
long r_harray_lookup_s(rharray_t *harray, const char *name);
long r_harray_taillookup(rharray_t *harray, const char *name, unsigned int namesize);
long r_harray_taillookup_s(rharray_t *harray, const char *name);
rhash_node_t* r_harray_nodelookup(rharray_t *harray, rhash_node_t *cur, const char *name, unsigned int namesize);
rhash_node_t* r_harray_nodelookup_s(rharray_t *harray, rhash_node_t *cur, const char *name);
rhash_node_t* r_harray_nodetaillookup(rharray_t *harray, rhash_node_t *cur, const char *name, unsigned int namesize);
rhash_node_t* r_harray_nodetaillookup_s(rharray_t *harray, rhash_node_t *cur, const char *name);
rpointer r_harray_get(rharray_t *harray, unsigned long index);
int r_harray_set(rharray_t *harray, long index, rconstpointer pval);

robject_t *r_harray_copy(const robject_t *obj);
void r_harray_cleanup(robject_t *obj);

#ifdef __cplusplus
}
#endif

#endif
