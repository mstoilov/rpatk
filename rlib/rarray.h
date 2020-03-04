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
	size_t len;
	size_t alloc_size;
	size_t elt_size;
};

/*
 * Return the array allocated elements.
 */
#define r_array_size(__array__) (__array__)->alloc_size

/*
 * Return the array elements number
 */
#define r_array_length(__array__) (__array__)->len

/*
 * Return 1 if the array is empty, otherwise return 0
 */
#define r_array_empty(__array__) ((r_array_length(__array__)) ? 0 : 1)

/*
 * Return the last element
 */
#define r_array_last(__array__, __type__) (r_array_index(__array__, (__array__)->len - 1, __type__))

/*
 * Return pointer to the memory location for index
 */
#define r_array_slot(__array__, __index__) ((rpointer)(((ruint8*)(__array__)->data) + (__array__)->elt_size * (__index__)))

/*
 * Return data element at index
 */
#define r_array_index(__array__, __index__, __type__) *((__type__*)r_array_slot(__array__, __index__))

/*
 * Return pointer to the last element or NULL if the array is empty
 */
#define r_array_lastslot(__array__) (r_array_length(__array__) ? r_array_slot(__array__, r_array_length(__array__) - 1) : NULL)

/*
 * Use the array as stack with Push/Pop operations
 */
#define r_array_push(__array__, __val__, __type__) do {__type__ __v__ = (__type__)__val__; r_array_add(__array__, &__v__); } while(0)
#define r_array_pop(__array__, __type__) (r_array_index(__array__, (__array__)->len ? --(__array__)->len : 0, __type__))
#define r_array_sort(__a__, __t__) \
	do { \
		__t__ __vi__, __vj__; \
		size_t __i__, __j__; \
		for (__i__ = 0; __i__ < r_array_length(__a__); __i__++) { \
			for (__j__ = __i__ + 1; __j__ < r_array_length(__a__); __j__++) { \
				__vi__ = r_array_index(__a__, __i__, __t__); \
				__vj__ = r_array_index(__a__, __j__, __t__); \
				if (__vj__ < __vi__) {\
					r_array_replace(__a__, __i__, &__vj__); \
					r_array_replace(__a__, __j__, &__vi__); \
				} \
			} \
		} \
	} while (0)

robject_t *r_array_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy, size_t elt_size);
rarray_t *r_array_create(size_t elt_size);
void r_array_destroy(rarray_t *array);
size_t r_array_add(rarray_t *array, rconstpointer data);
int r_array_move(rarray_t *array, size_t dest, size_t src, size_t size);
size_t r_array_removelast(rarray_t *array);
size_t r_array_insert(rarray_t *array, size_t index, rconstpointer data);
size_t r_array_replace(rarray_t *array, size_t index, rconstpointer data);
size_t r_array_setlength(rarray_t *array, size_t len);
void r_array_expand(rarray_t *array, size_t len);
void *r_array_slot_expand(rarray_t *array, size_t index);
void r_array_delete(rarray_t *array, size_t index);

/*
 * Virtual methods implementation
 */
void r_array_cleanup(robject_t *obj);
robject_t *r_array_copy(const robject_t *obj);

#ifdef __cplusplus
}
#endif

#endif
