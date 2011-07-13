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

#include "rlib/rcarray.h"
#include "rlib/rmem.h"


static rpointer r_carray_allocate_chunk(rsize_t elt_size)
{
	return r_zmalloc(R_CARRAY_CHUNKSIZE * elt_size);
}


static rpointer r_carray_get_chunk(const rcarray_t *carray, rsize_t nchunk)
{
	return r_array_index(carray->array, nchunk, rpointer);
}


static void r_carray_add_chunks(rcarray_t *carray, rsize_t nchunks)
{
	rpointer chunk;

	while (nchunks) {
		chunk = r_carray_allocate_chunk(carray->elt_size);
		r_array_add(carray->array, &chunk);
		carray->alloc_size += R_CARRAY_CHUNKSIZE;
		--nchunks;
	}
}


void r_carray_cleanup(robject_t *obj)
{
	int i;
	rcarray_t *carray = (rcarray_t *)obj;
	if (carray->oncleanup)
		carray->oncleanup(carray);
	for (i = 0; i < r_array_length(carray->array); i++)
		r_free(r_carray_get_chunk(carray, i));
	r_object_destroy((robject_t*)carray->array);
	r_object_cleanup((robject_t*)carray);
}


robject_t *r_carray_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy, rsize_t elt_size)
{
	rcarray_t *carray = (rcarray_t*)obj;

	r_object_init(obj, type, cleanup, copy);
	carray->elt_size = elt_size;
	carray->array = r_array_create(sizeof(rpointer));
	carray->alloc_size = 0;
	carray->len = 0;
	r_carray_add_chunks(carray, 1);
	return obj;
}


rcarray_t *r_carray_create(rsize_t elt_size)
{
	rcarray_t *carray;
	carray = (rcarray_t*)r_object_create(sizeof(*carray));
	r_carray_init((robject_t*)carray, R_OBJECT_CARRAY, r_carray_cleanup, r_carray_copy, elt_size);
	return carray;
}


void r_carray_destroy(rcarray_t *array)
{
	r_object_destroy((robject_t*)array);
}


robject_t *r_carray_copy(const robject_t *obj)
{
	rsize_t i;
	rcarray_t *dst;
	const rcarray_t *carray = (const rcarray_t *)obj;

	if (!carray)
		return NULL;
	dst = r_carray_create(carray->elt_size);
	if (!dst)
		return NULL;
	r_carray_add_chunks(dst, r_array_length(carray->array));
	for (i = 0; i < r_array_length(carray->array); i++)
		r_memcpy(r_carray_get_chunk(dst, i), r_carray_get_chunk(carray, i), R_CARRAY_CHUNKSIZE * carray->elt_size);
	dst->len = carray->len;
	dst->oncopy = carray->oncopy;
	dst->oncleanup = carray->oncleanup;
	if (dst->oncopy)
		dst->oncopy(dst);
	return (robject_t *)dst;
}


int r_carray_replace(rcarray_t *carray, rsize_t index, rconstpointer data)
{
	if (data)
		r_memcpy(r_carray_slot_expand(carray, index), data, carray->elt_size);
	else
		r_memset(r_carray_slot_expand(carray, index), 0, carray->elt_size);
	return index;
}


int r_carray_add(rcarray_t *carray, rconstpointer data)
{
	rsize_t index = r_carray_length(carray);
	r_carray_inclength(carray);
	return r_carray_replace(carray, index, data);
}


void r_carray_setlength(rcarray_t *carray, rsize_t len)
{
	r_carray_checkexpand(carray, len);
	r_carray_length(carray) = len;
}


void r_carray_inclength(rcarray_t *carray)
{
	r_carray_checkexpand(carray, r_carray_length(carray) + 1);
	r_carray_length(carray) += 1;
}


void r_carray_declength(rcarray_t *carray)
{
	if (r_carray_length(carray))
		r_carray_length(carray) -= 1;
}


void r_carray_checkexpand(rcarray_t *carray, rsize_t size)
{
	rsize_t chunks;

	if (r_carray_size(carray) < size) {
		chunks = (size - r_carray_size(carray) + R_CARRAY_CHUNKSIZE) / R_CARRAY_CHUNKSIZE;
		r_carray_add_chunks(carray, chunks);
	}
}


rpointer r_carray_slot_expand(rcarray_t *carray, rsize_t index)
{
	r_carray_checkexpand(carray, index+1);
	return (void*) r_carray_slot(carray, index);
}

rpointer r_carray_slot_notused(rcarray_t *carray, rsize_t index)
{
	rsize_t nchunk = index >> R_CARRAY_CHUNKBITS;
	rsize_t offset = index & R_CARRAY_CHUNKMASK;
	rpointer chunk = r_array_index(carray->array, nchunk, rpointer);

	rpointer v = (rpointer)(((char*)chunk) + (offset * carray->elt_size));
	return v;
}
