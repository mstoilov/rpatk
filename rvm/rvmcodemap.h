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

typedef struct rvm_loopblock_s {
	rulong begin;
	rulong size;
} rvm_loopblock_t;


typedef struct rvm_codelabel_s {
	enum {
		RVM_CODELABEL_INDEX = 0,
		RVM_CODELABEL_POINTER,
		RVM_CODELABEL_INVALID,
	} type;
	rulong base;
	union {
		rulong index;
		rpointer ptr;
	} loc;
	rstr_t *name;
	rulong size; // Optional, used for function declarations
} rvm_codelabel_t;


typedef struct rvm_codemap_s {
	rarray_t *blocks;
	rarray_t *labels;
	rhash_t *hash;
} rvm_codemap_t;


rvm_codemap_t *rvm_codemap_create();
void rvm_codemap_destroy(rvm_codemap_t *codemap);
void rvm_codemap_clear(rvm_codemap_t *codemap);
rlong rvm_codemap_invalid_add(rvm_codemap_t *codemap, const rchar *name, ruint namesize);
rlong rvm_codemap_invalid_add_s(rvm_codemap_t *codemap, const rchar *name);
rlong rvm_codemap_addoffset(rvm_codemap_t *codemap, rulong base, const rchar *name, ruint namesize, rulong offset);
rlong rvm_codemap_addoffset_s(rvm_codemap_t *codemap, rulong base, const rchar *name, rulong offset);
rlong rvm_codemap_addpointer(rvm_codemap_t *codemap, const rchar *name, ruint namesize, rvm_asmins_t *ptr);
rlong rvm_codemap_addpointer_s(rvm_codemap_t *codemap, const rchar *name, rvm_asmins_t *ptr);
rlong rvm_codemap_lookup(rvm_codemap_t *codemap, const rchar *name, ruint namesize);
rlong rvm_codemap_lookup_s(rvm_codemap_t *codemap, const rchar *name);
rlong rvm_codemap_lastlabel(rvm_codemap_t *codemap);
rvm_codelabel_t *rvm_codemap_label(rvm_codemap_t *codemap, rlong index);


#ifdef __cplusplus
}
#endif

#endif
