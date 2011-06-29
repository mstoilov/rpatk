#ifndef _RVMCODEGEN_H_
#define _RVMCODEGEN_H_

#include "rtypes.h"
#include "rvm/rvmerror.h"
#include "rlib/rarray.h"
#include "rlib/rhash.h"
#include "rvm/rvmcpu.h"
#include "rvm/rvmcodemap.h"
#include "rvm/rvmrelocmap.h"

#ifdef __cplusplus
extern "C" {
#endif


#define RVM_CODEGEN_FUNCINITOFFSET 3
#define RVM_CODEGEN_E_NONE 0


typedef struct rvm_codegen_s {
	rarray_t *code;
	rarray_t *data;
	ruinteger codeoff;
	rarray_t *sourceidx;
	rvm_codemap_t *codemap;
	rvm_relocmap_t *relocmap;
	rsize_t cursrcidx;
	rulong userdata;
} rvm_codegen_t;


rvm_codegen_t *rvm_codegen_create();
void rvm_codegen_destroy(rvm_codegen_t *cg);
void rvm_codegen_setsource(rvm_codegen_t *cg, rsize_t srcidx);
rlong rvm_codegen_getsource(rvm_codegen_t *cg, rsize_t codeidx);
ruinteger rvm_codegen_funcstart(rvm_codegen_t *cg, const rchar* name, ruinteger namesize, ruinteger args);
ruinteger rvm_codegen_funcstart_s(rvm_codegen_t *cg, const rchar* name, ruinteger args);
ruinteger rvm_codegen_vargs_funcstart(rvm_codegen_t *cg, const rchar* name, ruinteger namesize);
ruinteger rvm_codegen_vargs_funcstart_s(rvm_codegen_t *cg, const rchar* name);
void rvm_codegen_funcend(rvm_codegen_t *cg);
rsize_t rvm_codegen_addins(rvm_codegen_t *cg, rvm_asmins_t ins);
rsize_t rvm_codegen_addlabelins(rvm_codegen_t *cg, const rchar* name, ruinteger namesize, rvm_asmins_t ins);
rsize_t rvm_codegen_addlabelins_s(rvm_codegen_t *cg, const rchar* name, rvm_asmins_t ins);
rsize_t rvm_codegen_index_addrelocins(rvm_codegen_t *cg, rvm_reloctype_t type, rulong index, rvm_asmins_t ins);
rsize_t rvm_codegen_addrelocins(rvm_codegen_t *cg, rvm_reloctype_t type, const rchar* name, ruinteger namesize, rvm_asmins_t ins);
rsize_t rvm_codegen_addrelocins_s(rvm_codegen_t *cg, rvm_reloctype_t type, const rchar* name, rvm_asmins_t ins);
rsize_t rvm_codegen_insertins(rvm_codegen_t *cg, ruinteger index, rvm_asmins_t ins);
rsize_t rvm_codegen_replaceins(rvm_codegen_t *cg, ruinteger index, rvm_asmins_t ins);
rvm_asmins_t *rvm_codegen_getcode(rvm_codegen_t *cg, ruinteger index);
rsize_t rvm_codegen_getcodesize(rvm_codegen_t *cg);
void rvm_codegen_setcodesize(rvm_codegen_t *cg, ruinteger size);
void rvm_codegen_clear(rvm_codegen_t *cg);
rinteger rvm_codegen_relocate(rvm_codegen_t *cg, rvm_codelabel_t **err);
rlong rvm_codegen_validlabel(rvm_codegen_t *cg, rlong index);
rlong rvm_codegen_redefinelabel(rvm_codegen_t *cg, rlong index, rulong offset);
rlong rvm_codegen_redefinelabel_default(rvm_codegen_t *cg, rlong index);
rlong rvm_codegen_redefinepointer(rvm_codegen_t *cg, rlong index, rpointer data);
rlong rvm_codegen_addlabel(rvm_codegen_t *cg, const rchar* name, ruinteger namesize, rulong offset);
rlong rvm_codegen_addlabel_s(rvm_codegen_t *cg, const rchar* name, rulong offset);
rlong rvm_codegen_addlabel_default(rvm_codegen_t *cg, const rchar* name, ruinteger namesize);
rlong rvm_codegen_addlabel_default_s(rvm_codegen_t *cg, const rchar* name);
rlong rvm_codegen_invalid_addlabel(rvm_codegen_t *cg, const rchar* name, ruinteger namesize);
rlong rvm_codegen_invalid_addlabel_s(rvm_codegen_t *cg, const rchar* name);
rlong rvm_codegen_adddata(rvm_codegen_t *cg, const rchar *name, ruinteger namesize, rconstpointer data, rsize_t size);
rlong rvm_codegen_adddata_s(rvm_codegen_t *cg, const rchar *name, rconstpointer data, rsize_t size);
rlong rvm_codegen_addstring(rvm_codegen_t *cg, const rchar *name, ruinteger namesize, const rchar* data);
rlong rvm_codegen_addstring_s(rvm_codegen_t *cg, const rchar *name, const rchar* data);
rlong rvm_codegen_add_numlabel_s(rvm_codegen_t *cg, const rchar *alphaname, rlong numname);
rlong rvm_codegen_invalid_add_numlabel_s(rvm_codegen_t *cg, const rchar *alphaname, rlong numname);

#ifdef __cplusplus
}
#endif

#endif

