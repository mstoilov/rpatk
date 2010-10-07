#ifndef _RVMNAMEDARRAY_H_
#define _RVMNAMEDARRAY_H_

#include "rtypes.h"
#include "rvmcpu.h"
#include "rarray.h"
#include "rhash.h"
#include "rstring.h"
#include "rref.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rvm_namedmember_s {
	rstr_t *name;
	rvm_reg_t val;
} rvm_namedmember_t;


typedef struct rvm_namedarray_s {
	rref_t ref;
	rarray_t *members;
	rhash_t *hash;
} rvm_namedarray_t;


rvm_namedarray_t *rvm_namedarray_create();
rvm_namedarray_t *r_namedarray_copy(const rvm_namedarray_t *array);
void rvm_namedarray_destroy(rvm_namedarray_t *namedarray);
rint rvm_namedarray_add(rvm_namedarray_t *namedarray, const rchar *name, ruint namesize, const rvm_reg_t *pval);
rint rvm_namedarray_add_str(rvm_namedarray_t *namedarray, const rchar *name, const rvm_reg_t *pval);
rint rvm_namedarray_set(rvm_namedarray_t *namedarray, const rchar *name, ruint namesize, const rvm_reg_t *pval);
rint rvm_namedarray_set_str(rvm_namedarray_t *namedarray, const rchar *name, const rvm_reg_t *pval);
rlong rvm_namedarray_lookup(rvm_namedarray_t *namedarray, const rchar *name, ruint namesize);
rlong rvm_namedarray_lookup_str(rvm_namedarray_t *namedarray, const rchar *name);


#ifdef __cplusplus
}
#endif

#endif
