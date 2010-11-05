#ifndef _RHARRAY_H_
#define _RHARRAY_H_

#include "rtypes.h"
#include "rarray.h"
#include "rhash.h"
#include "rstring.h"
#include "robject.h"

#ifdef __cplusplus
extern "C" {
#endif



typedef struct rharray_s {
	robject_t obj;
	rarray_t *members;
	rarray_t *names;
	rhash_t *hash;
} rharray_t;


rharray_t *r_harray_create(ruint elt_size);
rharray_t *r_harray_copy(const rharray_t *array);
void r_harray_destroy(rharray_t *harray);
rint r_harray_add(rharray_t *harray, const rchar *name, ruint namesize, rconstpointer pval);
rint r_harray_add_s(rharray_t *harray, const rchar *name, rconstpointer pval);
rlong r_harray_lookup(rharray_t *harray, const rchar *name, ruint namesize);
rlong r_harray_lookup_s(rharray_t *harray, const rchar *name);
rhash_node_t* r_harray_nodelookup(rharray_t *harray, rhash_node_t *cur, const rchar *name, ruint namesize);
rhash_node_t* r_harray_nodelookup_s(rharray_t *harray, rhash_node_t *cur, const rchar *name);

rpointer r_harray_get(rharray_t *harray, rulong index);
rint r_harray_set(rharray_t *harray, rlong index, rconstpointer pval);


#ifdef __cplusplus
}
#endif

#endif
