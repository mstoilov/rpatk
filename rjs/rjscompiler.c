#include "rmem.h"
#include "rjscompiler.h"


rjs_compiler_t *rjs_compiler_create()
{
	rjs_compiler_t *co = (rjs_compiler_t *) r_zmalloc(sizeof(*co));

	co->cg = rvm_codegen_create();
	return co;
}


void rjs_compiler_destroy(rjs_compiler_t *co)
{
	if (co) {
		rvm_codegen_destroy(co->cg);
		r_free(co);
	}
}
