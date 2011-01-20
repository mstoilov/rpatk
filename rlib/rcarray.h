#ifndef _RCARRAY_H_
#define _RCARRAY_H_

#include "rtypes.h"
#include "robject.h"
#include "rarray.h"

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
	ruint alloc_size;
	ruint len;
	ruint elt_size;
	r_carray_callback oncleanup;
	r_carray_callback oncopy;
	rpointer *userdata;
};

#define r_carray_size(__carray__) (__carray__)->alloc_size
#define r_carray_length(__carray__) (__carray__)->len
#define r_carray_empty(__carray__) ((r_carray_length(__carray__)) ? 0 : 1)
#define r_carray_slot(__carray__, __index__)(((rchar*)r_array_index((__carray__)->array, (__index__) >> R_CARRAY_CHUNKBITS, rpointer)) + ((__index__) & R_CARRAY_CHUNKMASK) * (__carray__)->elt_size)
#define r_carray_index(__carray__, __index__, __type__) *((__type__*)r_carray_slot(__carray__, __index__))

robject_t *r_carray_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy, ruint elt_size);
rcarray_t *r_carray_create(ruint elt_size);
rint r_carray_replace(rcarray_t *carray, ruint index, rconstpointer data);
rint r_carray_add(rcarray_t *carray, rconstpointer data);
void r_carray_setlength(rcarray_t *carray, ruint len);
void r_carray_inclength(rcarray_t *carray);
void r_carray_inclength(rcarray_t *carray);
void r_carray_checkexpand(rcarray_t *carray, ruint size);
rpointer r_carray_slot_expand(rcarray_t *carray, ruint index);

/*
 * Virtual methods implementation
 */
void r_carray_cleanup(robject_t *obj);
robject_t *r_carray_copy(const robject_t *obj);

#ifdef __cplusplus
}
#endif

#endif
