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
	rstr_t *name;
	rulong index;
	rulong size; // Optional, used for function declarations
} rvm_codelabel_t;


typedef struct rvm_loopblock_s {
	rulong begin;
	rulong size;
} rvm_loopblock_t;


typedef struct rvm_codemap_s {
	rarray_t *blocks;
	rarray_t *labels;
	rhash_t *hash;
} rvm_codemap_t;


rvm_codemap_t *rvm_codemap_create();
void rvm_codemap_destroy(rvm_codemap_t *codemap);
void rvm_codemap_clear(rvm_codemap_t *codemap);
void rvm_codemap_invalid_add(rvm_codemap_t *codemap, const rchar *name, ruint namesize);
void rvm_codemap_invalid_add_s(rvm_codemap_t *codemap, const rchar *name);
void rvm_codemap_add(rvm_codemap_t *codemap, const rchar *name, ruint namesize, rulong index);
void rvm_codemap_add_s(rvm_codemap_t *codemap, const rchar *name, rulong index);
rvm_codelabel_t *rvm_codemap_lookup(rvm_codemap_t *codemap, const rchar *name, ruint namesize);
rvm_codelabel_t *rvm_codemap_lookup_s(rvm_codemap_t *codemap, const rchar *name);
rvm_codelabel_t *rvm_codemap_lastlabel(rvm_codemap_t *codemap);

void rvm_codemap_pushloopblock(rvm_codemap_t *codemap, rulong begin, rulong size);
void rvm_codemap_poploopblock(rvm_codemap_t *codemap);
rvm_loopblock_t *rvm_codemap_currentloopblock(rvm_codemap_t *codemap);

#ifdef __cplusplus
}
#endif

#endif
