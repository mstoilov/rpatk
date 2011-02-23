#ifndef _RPACOMPILER_H_
#define _RPACOMPILER_H_

#include "rvmcodegen.h"
#include "rvmscope.h"
#include "rpavm.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rpa_ruledef_s {
	rlong labelidx;
	rlong emitidx;
	rlong endidx;
	rchar end[64];
} rpa_ruledef_t;

typedef struct rpa_compiler_s {
	rvm_codegen_t *cg;
	rpa_ruledef_t current;
	rboolean optimized;
	rvm_scope_t *scope;
	rulong fpoff;
} rpa_compiler_t;


rpa_compiler_t *rpa_compiler_create();
void rpa_compiler_destroy(rpa_compiler_t *co);
rint rpa_compiler_rule_begin(rpa_compiler_t *co, const rchar *name, ruint namesize);
rint rpa_compiler_rule_begin_s(rpa_compiler_t *co, const rchar *name);
rint rpa_compiler_rule_end(rpa_compiler_t *co);


#ifdef __cplusplus
}
#endif

#endif
