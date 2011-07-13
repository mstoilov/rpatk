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

#ifndef _RCARRAY_H_
#define _RCARRAY_H_

#include "rtypes.h"
#include "rlib/robject.h"
#include "rlib/rarray.h"

#ifdef __cplusplus
extern "C" {
#endif


#define R_CARRAY_CHUNKBITS 4
#define R_CARRAY_CHUNKSIZE (1 << R_CARRAY_CHUNKBITS)
#define R_CARRAY_CHUNKMASK (R_CARRAY_CHUNKSIZE - 1)

typedef struct rcarray_s rcarray_t;
typedef void (*r_carray_callback)(rcarray_t *carray);


struct rcarray_s {
	robject_t obj;
	rarray_t *array;
	rsize_t alloc_size;
	rsize_t len;
	rsize_t elt_size;
	r_carray_callback oncleanup;
	r_carray_callback oncopy;
	rpointer *userdata;
};

#define r_carray_size(__carray__) (__carray__)->alloc_size
#define r_carray_length(__carray__) (__carray__)->len
#define r_carray_empty(__carray__) ((r_carray_length(__carray__)) ? 0 : 1)
#define r_carray_slot(__carray__, __index__)(((char*)r_array_index((__carray__)->array, (__index__) >> R_CARRAY_CHUNKBITS, rpointer)) + ((__index__) & R_CARRAY_CHUNKMASK) * (__carray__)->elt_size)
#define r_carray_index(__carray__, __index__, __type__) *((__type__*)r_carray_slot(__carray__, __index__))

robject_t *r_carray_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy, rsize_t elt_size);
rcarray_t *r_carray_create(rsize_t elt_size);
void r_carray_destroy(rcarray_t *array);
int r_carray_replace(rcarray_t *carray, rsize_t index, rconstpointer data);
int r_carray_add(rcarray_t *carray, rconstpointer data);
void r_carray_setlength(rcarray_t *carray, rsize_t len);
void r_carray_inclength(rcarray_t *carray);
void r_carray_inclength(rcarray_t *carray);
void r_carray_checkexpand(rcarray_t *carray, rsize_t size);
rpointer r_carray_slot_expand(rcarray_t *carray, rsize_t index);

/*
 * Virtual methods implementation
 */
void r_carray_cleanup(robject_t *obj);
robject_t *r_carray_copy(const robject_t *obj);

#ifdef __cplusplus
}
#endif

#endif
