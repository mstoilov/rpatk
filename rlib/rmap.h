#ifndef _RMAP_H_
#define _RMAP_H_

#include "rcarray.h"
#include "rhash.h"
#include "rstring.h"
#include "robject.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct rmap_s {
	robject_t obj;
	rcarray_t *members;
	rarray_t *names;
	rhash_t *hash;
} rmap_t;


rmap_t *r_map_create(ruint elt_size);
void r_map_destroy(rmap_t *array);


#ifdef __cplusplus
}
#endif

#endif /* _RMAP_H_ */
