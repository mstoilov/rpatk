#ifndef _RMAP_H_
#define _RMAP_H_

#include "rcarray.h"
#include "rhash.h"
#include "rlist.h"
#include "rstring.h"
#include "robject.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct rmap_s {
	robject_t obj;
	rhash_t *hash;
	rcarray_t *members;
	rlist_t active;
	rlist_t inactive;
} rmap_t;


rmap_t *r_map_create(ruint elt_size);
void r_map_destroy(rmap_t *array);


#ifdef __cplusplus
}
#endif

#endif /* _RMAP_H_ */
