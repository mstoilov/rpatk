#ifndef _RHARRAY_H_
#define _RHARRAY_H_

#include "rtypes.h"
#include "rarray.h"
#include "rhash.h"
#include "rstring.h"
#include "rref.h"

#ifdef __cplusplus
extern "C" {
#endif



typedef struct rharray_s {
	rref_t ref;
	rarray_t *members;
	rarray_t *names;
	rhash_t *hash;
} rharray_t;


rharray_t *r_harray_create(ruint elt_size);
rharray_t *r_harray_copy(const rharray_t *array);
void r_harray_destroy(rharray_t *harray);
rint r_harray_add(rharray_t *harray, const rchar *name, ruint namesize, rconstpointer pval);
rint r_harray_add_s(rharray_t *harray, const rchar *name, rconstpointer pval);
rlong r_harray_lookupindex(rharray_t *harray, const rchar *name, ruint namesize);
rlong r_harray_lookupindex_s(rharray_t *harray, const rchar *name);
rpointer r_harray_lookup(rharray_t *harray, const rchar *name, ruint namesize);
rpointer r_harray_lookup_s(rharray_t *harray, const rchar *name);
rint r_harray_set(rharray_t *harray, rlong index, rconstpointer pval);


#ifdef __cplusplus
}
#endif

#endif
