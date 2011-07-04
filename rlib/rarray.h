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

#ifndef _RARRAY_H_
#define _RARRAY_H_

#include "rtypes.h"
#include "rlib/robject.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct rarray_s rarray_t;
typedef void (*r_array_callback)(rarray_t *array);

struct rarray_s {
	robject_t obj;
	rpointer *data;
	rsize_t len;
	rsize_t alloc_size;
	rsize_t elt_size;
	r_array_callback oncleanup;
	r_array_callback oncopy;
	rpointer *user;
};


#define r_array_size(__array__) ((__array__)->alloc_size)
#define r_array_length(__array__) ((__array__)->len)
#define r_array_empty(__array__) ((r_array_length(__array__)) ? 0 : 1)
#define r_array_last(__array__, __type__) (r_array_index(__array__, (__array__)->len - 1, __type__))
#define r_array_inclast(__array__, __type__) (*((__type__*)(r_array_lastslot(__array__))) += 1)
#define r_array_declast(__array__, __type__) (*((__type__*)(r_array_lastslot(__array__))) -= 1)

/* #define r_array_index(__array__, __index__, __type__) (((__type__*)(void*)(__array__)->data)[__index__]) */
#define r_array_slot(__array__, __index__) (((ruint8*)(__array__)->data) + (__array__)->elt_size * (__index__))
#define r_array_index(__array__, __index__, __type__) *((__type__*)r_array_slot(__array__, __index__))
#define r_array_lastslot(__array__) (r_array_length(__array__) ? r_array_slot(__array__, r_array_length(__array__) - 1) : NULL)
#define r_array_push(__array__, __val__, __type__) do {__type__ __v__ = (__type__)__val__; r_array_add(__array__, &__v__); } while(0)
#define r_array_pop(__array__, __type__) (r_array_index(__array__, (__array__)->len ? --(__array__)->len : 0, __type__))

robject_t *r_array_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy, rsize_t elt_size);
rarray_t *r_array_create(rsize_t elt_size);
void r_array_destroy(rarray_t *array);
rsize_t r_array_add(rarray_t *array, rconstpointer data);
rinteger r_array_move(rarray_t *array, rlong dest, rlong src, rlong size);
rsize_t r_array_removelast(rarray_t *array);
rsize_t r_array_insert(rarray_t *array, rsize_t index, rconstpointer data);
rsize_t r_array_replace(rarray_t *array, rsize_t index, rconstpointer data);
rsize_t r_array_setlength(rarray_t *array, rsize_t len);
rsize_t r_array_expand(rarray_t *array, rsize_t len);
void *r_array_slot_expand(rarray_t *array, rsize_t index);
void r_array_delete(rarray_t *array, rsize_t index);

/*
 * Virtual methods implementation
 */
void r_array_cleanup(robject_t *obj);
robject_t *r_array_copy(const robject_t *obj);

#ifdef __cplusplus
}
#endif

#endif
