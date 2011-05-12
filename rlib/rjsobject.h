#ifndef _RJSOBJECT_H_
#define _RJSOBJECT_H_

#include "rtypes.h"
#include "robject.h"
#include "rcarray.h"
#include "rharray.h"

/*
 * This class targets the JavaScript Object implementation. Although I think it is pretty useful
 * collection primitive that is worth staying in RLIB. In the future I might either rename it or
 * move it to RJS.
 */

#ifdef __cplusplus
extern "C" {
#endif


typedef struct rjs_object_s rjs_object_t;


struct rjs_object_s {
	robject_t obj;
	rcarray_t *narray;
	rharray_t *harray;
};

robject_t *rjs_object_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy, ruint elt_size);
rjs_object_t *rjs_object_create();

/*
 * Virtual methods implementation
 */
void rjs_object_cleanup(robject_t *obj);
robject_t *rjs_object_copy(const robject_t *obj);

#ifdef __cplusplus
}
#endif

#endif
