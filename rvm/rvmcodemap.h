#ifndef _RVMCODEMAP_H_
#define _RVMCODEMAP_H_

#include "rtypes.h"
#include "rvm.h"
#include "rarray.h"
#include "rhash.h"
#include "rstring.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct rvm_codelabel_s {
	rstring_t *name;
	ruint index;
} rvm_codelabel_t;


typedef struct rvm_codemap_s {
	rarray_t *labels;
	rhash_t *hash;
} rvm_codemap_t;


rvm_codemap_t *rvm_codemap_create();
void rvm_codemap_destroy(rvm_codemap_t *codemap);
void rvm_codemap_add(rvm_codemap_t *codemap, const rchar *name, ruint namesize, ruint index);
void rvm_codemap_add_str(rvm_codemap_t *codemap, const rchar *name, ruint index);
rvm_codelabel_t *rvm_codemap_lookup(rvm_codemap_t *codemap, const rchar *name, ruint namesize);
rvm_codelabel_t *rvm_codemap_lookup_str(rvm_codemap_t *codemap, const rchar *name);


#ifdef __cplusplus
}
#endif

#endif
