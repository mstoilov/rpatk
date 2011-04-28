#include "rmem.h"
#include "rjscompiler.h"


void rjs_compiler_debughead(rjs_compiler_t *co, rarray_t *records, rlong rec, rparecord_t *prec)
{
	if (co->debug) {
		rpa_record_dump(records, rec);

	}
}


void rjs_compiler_debugtail(rjs_compiler_t *co, rarray_t *records, rlong rec, rparecord_t *prec)
{
	if (co->debug) {

	}

}


rint rjs_compiler_rh_program(rjs_compiler_t *co, rarray_t *records, rlong rec, rparecord_t *prec)
{
	rjs_compiler_debughead(co, records, rec, prec);

	rjs_compiler_debugtail(co, records, rec, prec);
	return 0;
}


rint rjs_compiler_rh_expression(rjs_compiler_t *co, rarray_t *records, rlong rec, rparecord_t *prec)
{
	rjs_compiler_debughead(co, records, rec, prec);

	rjs_compiler_debugtail(co, records, rec, prec);
	return 0;
}


rint rjs_compiler_rh_lefthandsideexpression(rjs_compiler_t *co, rarray_t *records, rlong rec, rparecord_t *prec)
{
	rjs_compiler_debughead(co, records, rec, prec);

	rjs_compiler_debugtail(co, records, rec, prec);
	return 0;
}


rjs_compiler_t *rjs_compiler_create(rvmcpu_t *cpu)
{
	rjs_compiler_t *co = (rjs_compiler_t *) r_zmalloc(sizeof(*co));

	co->cg = rvm_codegen_create();
	co->coexp = r_array_create(sizeof(rjs_coexp_t));
	co->cpu = cpu;
	r_memset(co->handlers, 0, sizeof(co->handlers));

	co->handlers[RJS_PROGRAM] = rjs_compiler_rh_program;
	co->handlers[RJS_EXPRESSION] = rjs_compiler_rh_expression;
	co->handlers[RJS_LEFTHANDSIDEEXPRESSION] = rjs_compiler_rh_lefthandsideexpression;

	return co;
}


void rjs_compiler_destroy(rjs_compiler_t *co)
{
	if (co) {
		rvm_codegen_destroy(co->cg);
		r_array_destroy(co->coexp);
		co->cpu = NULL;
		r_free(co);
	}
}


rint rjs_compiler_compile(rjs_compiler_t *co, rarray_t *records)
{
	rlong i;
	rparecord_t *prec;

	if (!co || !records) {

		return -1;
	}


	for (i = 0; i < r_array_length(records); i++) {
		prec = (rparecord_t *)r_array_slot(records, i);
		if (prec->ruleuid >= 0 && prec->ruleuid < RJS_COMPILER_NHANDLERS && co->handlers[prec->ruleuid]) {
			co->handlers[prec->ruleuid](co, records, i, prec);
		}
	}

	return 0;
}
