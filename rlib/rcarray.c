#include "rcarray.h"
#include "rmem.h"

#define MIN_CARRAY_LEN 2



static void r_carray_checkexpand(rcarray_t *carray, ruint index);


static rpointer r_carray_allocate_chunk(ruint elt_size)
{
	return r_zmalloc(R_CARRAY_CHUNKSIZE * elt_size);
}


static rpointer r_carray_get_chunk(const rcarray_t *carray, ruint nchunk)
{
	return r_array_index(carray->array, nchunk, rpointer);
}


static void r_carray_add_chunks(rcarray_t *carray, ruint nchunks)
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


robject_t *r_carray_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy, ruint elt_size)
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


rcarray_t *r_carray_create(ruint elt_size)
{
	rcarray_t *carray;
	carray = (rcarray_t*)r_object_create(sizeof(*carray));
	r_carray_init((robject_t*)carray, R_OBJECT_CARRAY, r_carray_cleanup, r_carray_copy, elt_size);
	return carray;
}


robject_t *r_carray_copy(const robject_t *obj)
{
	ruint i;
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


rint r_carray_replace(rcarray_t *carray, ruint index, rconstpointer data)
{
	if (data)
		r_memcpy(r_carray_slot_expand(carray, index), data, carray->elt_size);
	else
		r_memset(r_carray_slot_expand(carray, index), 0, carray->elt_size);
	return index;
}


rint r_carray_add(rcarray_t *carray, rconstpointer data)
{
	ruint index = r_carray_length(carray);
	return r_carray_replace(carray, index, data);
}


void r_carray_setlength(rcarray_t *carray, ruint len)
{
	r_carray_checkexpand(carray, len);
	carray->len = len;
}


static void r_carray_checkexpand(rcarray_t *carray, ruint size)
{
	ruint chunks;

	if (r_carray_size(carray) < size) {
		chunks = (size - r_carray_size(carray) + R_CARRAY_CHUNKSIZE) / R_CARRAY_CHUNKSIZE;
		r_carray_add_chunks(carray, chunks);
	}
}


void *r_carray_slot_expand(rcarray_t *carray, ruint index)
{
	r_carray_checkexpand(carray, index+1);
	return (void*) r_carray_slot(carray, index);
}

rpointer r_carray_slot_notused(rcarray_t *carray, ruint index)
{
	ruint nchunk = index >> R_CARRAY_CHUNKBITS;
	ruint offset = index & R_CARRAY_CHUNKMASK;
	rpointer chunk = r_array_index(carray->array, nchunk, rpointer);

	rpointer v = (rpointer)(((rchar*)chunk) + (offset * carray->elt_size));
	return v;
}
