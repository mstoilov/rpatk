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


typedef struct rvm_relocrecord_s {
	rulong offset;
	rulong label;
} rvm_relocrecord_t;


typedef struct rvm_relocmap_s {
	rarray_t *records;
} rvm_relocmap_t;


rvm_relocmap_t *rvm_relocmap_create();
void rvm_relocmap_destroy(rvm_relocmap_t *relocmap);
void rvm_relocmap_clear(rvm_relocmap_t *relocmap);
rvm_relocrecord_t *rvm_relocmap_add(rvm_relocmap_t *relocmap, rulong offset, rulong label);
rvm_relocrecord_t *rvm_relocmap_get(rvm_relocmap_t *relocmap, rulong index);
rulong rvm_relocmap_length(rvm_relocmap_t *relocmap);
rlong rvm_relocmap_relocate(rvm_relocmap_t *relocmap, rvm_codemap_t *codemap, rvm_asmins_t *code);


#ifdef __cplusplus
}
#endif

#endif
