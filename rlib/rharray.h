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


rharray_t *r_harray_create(ruint elt_size, r_array_destroyelt_fun destroy, r_array_copyelt_fun copy);
rharray_t *r_harray_copy(const rharray_t *array);
void r_harray_destroy(rharray_t *harray);
rint r_harray_add(rharray_t *harray, const rchar *name, ruint namesize, rconstpointer pval);
rint r_harray_stradd(rharray_t *harray, const rchar *name, rconstpointer pval);
rint r_harray_add(rharray_t *harray, const rchar *name, ruint namesize, rconstpointer pval);
rint r_harray_stradd(rharray_t *harray, const rchar *name, rconstpointer pval);
rint r_harray_set(rharray_t *harray, const rchar *name, ruint namesize, rconstpointer pval);
rint r_harray_strset(rharray_t *harray, const rchar *name, rconstpointer pval);
rlong r_harray_lookup_index(rharray_t *harray, const rchar *name, ruint namesize);
rlong r_harray_strlookup_index(rharray_t *harray, const rchar *name);
rpointer r_harray_lookup(rharray_t *harray, const rchar *name, ruint namesize);
rpointer r_harray_strlookup(rharray_t *harray, const rchar *name);


#ifdef __cplusplus
}
#endif

#endif
