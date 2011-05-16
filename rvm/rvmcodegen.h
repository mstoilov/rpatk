#ifndef _RVMCODEGEN_H_
#define _RVMCODEGEN_H_

#include "rtypes.h"
#include "rvmerror.h"
#include "rarray.h"
#include "rhash.h"
#include "rvmcpu.h"
#include "rvmcodemap.h"
#include "rvmrelocmap.h"

#ifdef __cplusplus
extern "C" {
#endif


#define RVM_CODEGEN_FUNCINITOFFSET 3
#define RVM_CODEGEN_E_NONE 0


typedef struct rvm_codegen_s {
	rarray_t *code;
	rarray_t *data;
	ruint codeoff;
	rvm_codemap_t *codemap;
	rvm_relocmap_t *relocmap;
	rulong userdata;
} rvm_codegen_t;


rvm_codegen_t *rvm_codegen_create();
void rvm_codegen_destroy(rvm_codegen_t *cg);
ruint rvm_codegen_funcstart(rvm_codegen_t *cg, const rchar* name, ruint namesize, ruint args);
ruint rvm_codegen_funcstart_s(rvm_codegen_t *cg, const rchar* name, ruint args);
ruint rvm_codegen_vargs_funcstart(rvm_codegen_t *cg, const rchar* name, ruint namesize);
ruint rvm_codegen_vargs_funcstart_s(rvm_codegen_t *cg, const rchar* name);
void rvm_codegen_funcend(rvm_codegen_t *cg);
ruint rvm_codegen_addins(rvm_codegen_t *cg, rvm_asmins_t ins);
ruint rvm_codegen_addlabelins(rvm_codegen_t *cg, const rchar* name, ruint namesize, rvm_asmins_t ins);
ruint rvm_codegen_addlabelins_s(rvm_codegen_t *cg, const rchar* name, rvm_asmins_t ins);
ruint rvm_codegen_index_addrelocins(rvm_codegen_t *cg, rvm_reloctype_t type, rulong index, rvm_asmins_t ins);
ruint rvm_codegen_addrelocins(rvm_codegen_t *cg, rvm_reloctype_t type, const rchar* name, ruint namesize, rvm_asmins_t ins);
ruint rvm_codegen_addrelocins_s(rvm_codegen_t *cg, rvm_reloctype_t type, const rchar* name, rvm_asmins_t ins);
ruint rvm_codegen_insertins(rvm_codegen_t *cg, ruint index, rvm_asmins_t ins);
ruint rvm_codegen_replaceins(rvm_codegen_t *cg, ruint index, rvm_asmins_t ins);
rvm_asmins_t *rvm_codegen_getcode(rvm_codegen_t *cg, ruint index);
rulong rvm_codegen_getcodesize(rvm_codegen_t *cg);
void rvm_codegen_setcodesize(rvm_codegen_t *cg, ruint size);
void rvm_codegen_clear(rvm_codegen_t *cg);
rint rvm_codegen_relocate(rvm_codegen_t *cg, rvm_codelabel_t **err);
rlong rvm_codegen_validlabel(rvm_codegen_t *cg, rlong index);
rlong rvm_codegen_redefinelabel(rvm_codegen_t *cg, rlong index, rulong offset);
rlong rvm_codegen_redefinelabel_default(rvm_codegen_t *cg, rlong index);
rlong rvm_codegen_redefinepointer(rvm_codegen_t *cg, rlong index, rpointer data);
rlong rvm_codegen_addlabel(rvm_codegen_t *cg, const rchar* name, ruint namesize, rulong offset);
rlong rvm_codegen_addlabel_s(rvm_codegen_t *cg, const rchar* name, rulong offset);
rlong rvm_codegen_addlabel_default(rvm_codegen_t *cg, const rchar* name, ruint namesize);
rlong rvm_codegen_addlabel_default_s(rvm_codegen_t *cg, const rchar* name);
rlong rvm_codegen_invalid_addlabel(rvm_codegen_t *cg, const rchar* name, ruint namesize);
rlong rvm_codegen_invalid_addlabel_s(rvm_codegen_t *cg, const rchar* name);
rlong rvm_codegen_adddata(rvm_codegen_t *cg, const rchar *name, ruint namesize, rconstpointer data, rsize_t size);
rlong rvm_codegen_adddata_s(rvm_codegen_t *cg, const rchar *name, rconstpointer data, rsize_t size);
rlong rvm_codegen_addstring(rvm_codegen_t *cg, const rchar *name, ruint namesize, const rchar* data);
rlong rvm_codegen_addstring_s(rvm_codegen_t *cg, const rchar *name, const rchar* data);
rlong rvm_codegen_add_numlabel_s(rvm_codegen_t *cg, const rchar *alphaname, rlong numname);
rlong rvm_codegen_invalid_add_numlabel_s(rvm_codegen_t *cg, const rchar *alphaname, rlong numname);

#ifdef __cplusplus
}
#endif

#endif

