#ifndef _RVMSCOPE_H_
#define _RVMSCOPE_H_

#include "rtypes.h"
#include "rarray.h"
#include "rhash.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct rvm_varmap_s {
	const rchar *name;
	rpointer *data;
} rvm_varmap_t;


typedef struct rvm_scope_s {
	rarray_t *names;
	rhash_t *nameshash;
	rarray_t *varstack;
	rarray_t *scopestack;
} rvm_scope_t;


rvm_scope_t *rvm_scope_create();
void rvm_scope_destroy(rvm_scope_t *scope);
void rvm_scope_addvar(rvm_scope_t *scope, const rchar* varname);




#ifdef __cplusplus
}
#endif

#endif
