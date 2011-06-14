#ifndef _RMAP_H_
#define _RMAP_H_

#include "rcarray.h"
#include "rhash.h"
#include "rlist.h"
#include "rstring.h"
#include "rgc.h"
#include "robject.h"

#ifdef __cplusplus
extern "C" {
#endif


#define r_map_hashsize(__m__) (1 << (__m__)->nbits)
#define r_map_hashmask(__m__) (r_map_hashsize(__m__) - 1)

typedef struct rmap_s {
	robject_t obj;
	ruinteger nbits;
	ruinteger elt_size;
	rcarray_t *data;
	rlist_t *hash;
	rlist_t active;
	rlist_t inactive;
} rmap_t;


rmap_t *r_map_create(ruinteger elt_size, ruinteger nbits);
void r_map_destroy(rmap_t *array);
rlong r_map_lookup(rmap_t *map, rlong current, const rchar *name, rsize_t namesize);
rlong r_map_lookup_s(rmap_t *map, rlong current, const rchar *name);
rlong r_map_taillookup(rmap_t *map, rlong current, const rchar *name, rsize_t namesize);
rlong r_map_taillookup_s(rmap_t *map, rlong current, const rchar *name);
rlong r_map_lookup_d(rmap_t *map, rlong current, double name);
rlong r_map_lookup_l(rmap_t *map, rlong current, long name);
rlong r_map_add(rmap_t *map, const rchar *name, rsize_t namesize, rconstpointer pval);
rlong r_map_add_s(rmap_t *map, const rchar *name, rconstpointer pval);
rlong r_map_add_d(rmap_t *map, double name, rconstpointer pval);
rlong r_map_add_l(rmap_t *map, long name, rconstpointer pval);

/*
 * The following functions allow the created keys (rstring_t objects) to be added to
 * GC list and not being destroyed by the rmap_t, but leave it to the users of rmap_t
 * to decide when to destroy those keys. These is useful for scripting languages with
 * GC memory management. Another possibility would be to get the key as a rstrit_t* and
 * make rmap_t completely get out of the memory management business.
 */
rlong r_map_gckey_add(rmap_t *map, rgc_t* gc, const rchar *name, rsize_t namesize, rconstpointer pval);
rlong r_map_gckey_add_s(rmap_t *map, rgc_t* gc, const rchar *name, rconstpointer pval);
rlong r_map_gckey_add_d(rmap_t *map, rgc_t* gc, double name, rconstpointer pval);
rlong r_map_gckey_add_l(rmap_t *map, rgc_t* gc, long name, rconstpointer pval);
rlong r_map_setvalue(rmap_t *map, rlong index, rconstpointer pval);
rstring_t *r_map_key(rmap_t *map, rulong index);
rpointer r_map_value(rmap_t *map, rulong index);
rinteger r_map_delete(rmap_t *map, rulong index);

rlong r_map_first(rmap_t *map);
rlong r_map_last(rmap_t *map);
rlong r_map_next(rmap_t *map, rlong current);
rlong r_map_prev(rmap_t *map, rlong current);


#ifdef __cplusplus
}
#endif

#endif /* _RMAP_H_ */
