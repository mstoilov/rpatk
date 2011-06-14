#ifndef _RVMRELOCMAP_H_
#define _RVMRELOCMAP_H_

#include "rtypes.h"
#include "rarray.h"
#include "rhash.h"
#include "rstring.h"
#include "rvmcpu.h"
#include "rvmcodemap.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	RVM_RELOC_DEFAULT = 0,
	RVM_RELOC_JUMP,
	RVM_RELOC_BRANCH,
	RVM_RELOC_STRING,
	RVM_RELOC_BLOB,
} rvm_reloctype_t;


typedef enum {
	RVM_RELOC_CODE = 0,
	RVM_RELOC_DATA,
} rvm_reloctarget_t;


typedef struct rvm_relocrecord_s {
	rvm_reloctarget_t target;
	rvm_reloctype_t type;
	rulong offset;
	rulong label;
} rvm_relocrecord_t;


typedef struct rvm_relocmap_s {
	rarray_t *records;
} rvm_relocmap_t;


rvm_relocmap_t *rvm_relocmap_create();
void rvm_relocmap_destroy(rvm_relocmap_t *relocmap);
void rvm_relocmap_clear(rvm_relocmap_t *relocmap);
rlong rvm_relocmap_add(rvm_relocmap_t *relocmap, rvm_reloctarget_t target, rvm_reloctype_t type, rulong offset, rulong label);
rvm_relocrecord_t *rvm_relocmap_get(rvm_relocmap_t *relocmap, rulong index);
rulong rvm_relocmap_length(rvm_relocmap_t *relocmap);
rinteger rvm_relocmap_relocate(rvm_relocmap_t *relocmap, rvm_codemap_t *codemap, rvm_asmins_t *code, rvm_codelabel_t **err);


#ifdef __cplusplus
}
#endif

#endif
