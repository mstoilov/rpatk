#ifndef _RHARRAY_H_
#define _RHARRAY_H_

#include "rtypes.h"
#include "rarray.h"
#include "rcarray.h"
#include "rhash.h"
#include "rstring.h"
#include "robject.h"

#ifdef __cplusplus
extern "C" {
#endif


#define r_harray_index(__harray__, __index__, __type__) r_carray_index((__harray__)->members, __index__, __type__)
#define r_harray_length(__harray__) r_carray_length((__harray__)->members)
#define r_harray_index(__harray__, __index__, __type__) r_carray_index((__harray__)->members, __index__, __type__)
#define r_harray_slot(__harray__, __index__) r_carray_slot((__harray__)->members, __index__)

typedef struct rharray_s {
	robject_t obj;
	rcarray_t *members;
	rarray_t *names;
	rhash_t *hash;
} rharray_t;


rharray_t *r_harray_create(ruinteger elt_size);
void r_harray_destroy(rharray_t *array);
robject_t *r_harray_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy, ruinteger elt_size);
rlong r_harray_add(rharray_t *harray, const rchar *name, rsize_t namesize, rconstpointer pval);
rlong r_harray_add_s(rharray_t *harray, const rchar *name, rconstpointer pval);
rlong r_harray_replace(rharray_t *harray, const rchar *name, rsize_t namesize, rconstpointer pval);
rlong r_harray_replace_s(rharray_t *harray, const rchar *name, rconstpointer pval);
rlong r_harray_lookup(rharray_t *harray, const rchar *name, rsize_t namesize);
rlong r_harray_lookup_s(rharray_t *harray, const rchar *name);
rlong r_harray_taillookup(rharray_t *harray, const rchar *name, rsize_t namesize);
rlong r_harray_taillookup_s(rharray_t *harray, const rchar *name);
rhash_node_t* r_harray_nodelookup(rharray_t *harray, rhash_node_t *cur, const rchar *name, rsize_t namesize);
rhash_node_t* r_harray_nodelookup_s(rharray_t *harray, rhash_node_t *cur, const rchar *name);
rhash_node_t* r_harray_nodetaillookup(rharray_t *harray, rhash_node_t *cur, const rchar *name, rsize_t namesize);
rhash_node_t* r_harray_nodetaillookup_s(rharray_t *harray, rhash_node_t *cur, const rchar *name);
rpointer r_harray_get(rharray_t *harray, rsize_t index);
rinteger r_harray_set(rharray_t *harray, rlong index, rconstpointer pval);

robject_t *r_harray_copy(const robject_t *obj);
void r_harray_cleanup(robject_t *obj);

#ifdef __cplusplus
}
#endif

#endif
