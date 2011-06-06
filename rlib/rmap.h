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


#define r_map_hashsize(__m__) (1 << (__m__)->nbits)
#define r_map_hashmask(__m__) (r_map_hashsize(__m__) - 1)

typedef struct rmap_s {
	robject_t obj;
	ruint nbits;
	rcarray_t *data;
	rarray_t *nodes;
	rlist_t *hash;
	rlist_t active;
	rlist_t inactive;
} rmap_t;


rmap_t *r_map_create(ruint elt_size, ruint nbits);
void r_map_destroy(rmap_t *array);
rlong r_map_add(rmap_t *map, const rchar *name, rsize_t namesize, rconstpointer pval);
rlong r_map_add_s(rmap_t *map, const rchar *name, rconstpointer pval);
rlong r_map_replace(rmap_t *map, const rchar *name, rsize_t namesize, rconstpointer pval);
rlong r_map_replace_s(rmap_t *map, const rchar *name, rconstpointer pval);
const rchar *r_map_key(rmap_t *map, rsize_t index);
rpointer r_map_value(rmap_t *map, rsize_t index);
rint r_map_delete(rmap_t *map, rsize_t index);


#ifdef __cplusplus
}
#endif

#endif /* _RMAP_H_ */
