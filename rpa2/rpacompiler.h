#ifndef _RPACOMPILER_H_
#define _RPACOMPILER_H_

#include "rvmcodegen.h"
#include "rvmscope.h"
#include "rpavm.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RPA_RULENAME_MAXSIZE 256

/*
 * This should be renamed to rpa_expdef_s
 */
typedef struct rpa_ruledef_s {
	rulong branch;
	rulong start;
	rlong startidx;
	rlong dataidx;
	rlong endidx;
	rlong loopidx;
	rlong successidx;
	rlong failidx;
	rlong againidx;
	ruint ruleuid;
	ruint flags;
} rpa_ruledef_t;


#define RPA_COMPILER_CURRENTEXP(__co__) ((rpa_ruledef_t*)r_array_lastslot((__co__)->expressions))

/*
 * The rpa_rulepref_t user preferences are compiled into
 * rpa_ruledata_t and access in runtime.
 */
typedef struct rpa_rulepref_s {
	rlong ruleuid;
	rulong flags;
} rpa_rulepref_t;

typedef struct rpa_compiler_s {
	rvm_codegen_t *cg;
	rharray_t *ruleprefs;
	rarray_t *expressions;
	rboolean optimized;
	rvm_scope_t *scope;
	rulong fpoff;
	rpair_t currange;
} rpa_compiler_t;


rpa_compiler_t *rpa_compiler_create();
void rpa_compiler_destroy(rpa_compiler_t *co);
rlong rpa_compiler_addblob(rpa_compiler_t *co, rlong ruleid, rlong ruleuid, rulong flags, const rchar *name, rulong namesize);
rlong rpa_compiler_addblob_s(rpa_compiler_t *co, rlong ruleid, rlong ruleuid, rulong flags, const rchar *name);

rint rpa_compiler_loop_begin(rpa_compiler_t *co, const rchar *name, ruint namesize);
rint rpa_compiler_loop_begin_s(rpa_compiler_t *co, const rchar *name);
rint rpa_compiler_loop_end(rpa_compiler_t *co);

rint rpa_compiler_rule_begin(rpa_compiler_t *co, const rchar *name, ruint namesize);
rint rpa_compiler_rule_begin_s(rpa_compiler_t *co, const rchar *name);
rint rpa_compiler_rule_end(rpa_compiler_t *co);

rint rpa_compiler_inlinerule_begin(rpa_compiler_t *co, const rchar *name, ruint namesize, ruint flags);
rint rpa_compiler_inlinerule_begin_s(rpa_compiler_t *co, const rchar *name, ruint flags);
rint rpa_compiler_inlinerule_end(rpa_compiler_t *co);

rint rpa_compiler_exp_begin(rpa_compiler_t *co, ruint flags);
rint rpa_compiler_exp_end(rpa_compiler_t *co);

rint rpa_compiler_altexp_begin(rpa_compiler_t *co, ruint flags);
rint rpa_compiler_altexp_end(rpa_compiler_t *co);

rint rpa_compiler_branch_begin(rpa_compiler_t *co, ruint flags);
rint rpa_compiler_branch_end(rpa_compiler_t *co);

rint rpa_compiler_nonloopybranch_begin(rpa_compiler_t *co, ruint flags);
rint rpa_compiler_nonloopybranch_end(rpa_compiler_t *co);

rint rpa_compiler_class_begin(rpa_compiler_t *co, ruint flags);
rint rpa_compiler_class_end(rpa_compiler_t *co);

rint rpa_compiler_notexp_begin(rpa_compiler_t *co, ruint flags);
rint rpa_compiler_notexp_end(rpa_compiler_t *co);

void rpa_compiler_index_reference(rpa_compiler_t *co, rulong index, ruint qflag);
void rpa_compiler_index_reference_nan(rpa_compiler_t *co, rulong index);
void rpa_compiler_index_reference_opt(rpa_compiler_t *co, rulong index);
void rpa_compiler_index_reference_mul(rpa_compiler_t *co, rulong index);
void rpa_compiler_index_reference_mop(rpa_compiler_t *co, rulong index);
void rpa_compiler_reference(rpa_compiler_t *co, const rchar *name, rsize_t namesize, ruint qflag);
void rpa_compiler_reference_nan(rpa_compiler_t *co, const rchar *name, rsize_t namesize);
void rpa_compiler_reference_opt(rpa_compiler_t *co, const rchar *name, rsize_t namesize);
void rpa_compiler_reference_mul(rpa_compiler_t *co, const rchar *name, rsize_t namesize);
void rpa_compiler_reference_mop(rpa_compiler_t *co, const rchar *name, rsize_t namesize);
void rpa_compiler_reference_s(rpa_compiler_t *co, const rchar *name, ruint qflag);
void rpa_compiler_reference_nan_s(rpa_compiler_t *co, const rchar *name);
void rpa_compiler_reference_opt_s(rpa_compiler_t *co, const rchar *name);
void rpa_compiler_reference_mul_s(rpa_compiler_t *co, const rchar *name);
void rpa_compiler_reference_mop_s(rpa_compiler_t *co, const rchar *name);

void rpa_compiler_rulepref_set_ruleuid(rpa_compiler_t *co, const rchar *name, ruint namesize, rlong ruleuid);
void rpa_compiler_rulepref_set_ruleuid_s(rpa_compiler_t *co, const rchar *name, rlong ruleuid);
void rpa_compiler_rulepref_set_flag(rpa_compiler_t *co, const rchar *name, ruint namesize, rulong flag);
void rpa_compiler_rulepref_set_flag_s(rpa_compiler_t *co, const rchar *name, rulong flag);
void rpa_compiler_rulepref_clear_flag(rpa_compiler_t *co, const rchar *name, ruint namesize, rulong flag);
void rpa_compiler_rulepref_clear_flag_s(rpa_compiler_t *co, const rchar *name, rulong flag);
void rpa_compiler_rulepref_set_ruleuid_flags(rpa_compiler_t *co, const rchar *name, ruint namesize, rlong ruleuid, rulong flags);
void rpa_compiler_rulepref_clear_flag(rpa_compiler_t *co, const rchar *name, ruint namesize, rulong flag);
void rpa_compiler_rulepref_set_ruleuid_flags_s(rpa_compiler_t *co, const rchar *name, rlong ruleuid, rulong flags);
#ifdef __cplusplus
}
#endif

#endif
