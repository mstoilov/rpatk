#ifndef _RVMSCOPE_H_
#define _RVMSCOPE_H_

#include "rtypes.h"
#include "rarray.h"
#include "rhash.h"


#ifdef __cplusplus
extern "C" {
#endif

#define VARMAP_DATATYPE_OFFSET 0
#define VARMAP_DATATYPE_PTR 1


typedef struct rvm_varmap_s {
	const rchar *name;
	union {
		rpointer ptr;
		ruint32 offset;
	} data;
	ruchar datatype;
} rvm_varmap_t;


typedef struct rvm_scope_s {
	rarray_t *names;
	rhash_t *nameshash;
	rarray_t *varstack;
	rarray_t *scopestack;
} rvm_scope_t;


rvm_scope_t *rvm_scope_create();
void rvm_scope_destroy(rvm_scope_t *scope);
rchar *rvm_scope_addname(rvm_scope_t *scope, const rchar *name, ruint namesize);
rchar *rvm_scope_addstrname(rvm_scope_t *scope, const rchar *name);
void rvm_scope_addoffset(rvm_scope_t *scope, const rchar *name, ruint namesize, ruint32 off);
void rvm_scope_addpointer(rvm_scope_t *scope, const rchar *name, ruint namesize, rpointer ptr);
void rvm_scope_push(rvm_scope_t* scope);
void rvm_scope_pop(rvm_scope_t* scope);
ruint rvm_scope_count(rvm_scope_t* scope);
ruint rvm_scope_numentries(rvm_scope_t *scope);
rvm_varmap_t *rvm_scope_lookup(rvm_scope_t *scope, const rchar *name, ruint namesize);
rvm_varmap_t *rvm_scope_lookup_s(rvm_scope_t *scope, const rchar *name);
rvm_varmap_t *rvm_scope_tiplookup(rvm_scope_t *scope, const rchar *name, ruint namesize);
rvm_varmap_t *rvm_scope_tiplookup_s(rvm_scope_t *scope, const rchar *name);


#ifdef __cplusplus
}
#endif

#endif
