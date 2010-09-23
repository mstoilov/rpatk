#ifndef _RVMCODEMAP_H_
#define _RVMCODEMAP_H_

#include "rtypes.h"
#include "rvmcpu.h"
#include "rarray.h"
#include "rhash.h"
#include "rstring.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RVM_CODELABEL_INVALID ((rulong)-1)

typedef struct rvm_codelabel_s {
	rstring_t *name;
	rulong index;
} rvm_codelabel_t;


typedef struct rvm_codemap_s {
	rarray_t *labels;
	rhash_t *hash;
} rvm_codemap_t;


rvm_codemap_t *rvm_codemap_create();
void rvm_codemap_destroy(rvm_codemap_t *codemap);
void rvm_codemap_invalid_add(rvm_codemap_t *codemap, const rchar *name, ruint namesize);
void rvm_codemap_invalid_add_str(rvm_codemap_t *codemap, const rchar *name);
void rvm_codemap_add(rvm_codemap_t *codemap, const rchar *name, ruint namesize, rulong index);
void rvm_codemap_add_str(rvm_codemap_t *codemap, const rchar *name, rulong index);
rvm_codelabel_t *rvm_codemap_lookup(rvm_codemap_t *codemap, const rchar *name, ruint namesize);
rvm_codelabel_t *rvm_codemap_lookup_str(rvm_codemap_t *codemap, const rchar *name);


#ifdef __cplusplus
}
#endif

#endif
