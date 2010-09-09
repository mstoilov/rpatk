#include "rmem.h"
#include "rvmcodegen.h"
#include "rvm.h"


rvm_codegen_t *rvm_codegen_create()
{
	rvm_codegen_t *cg;

	cg = (rvm_codegen_t *)r_malloc(sizeof(*cg));
	if (!cg)
		return (NULL);
	r_memset(cg, 0, sizeof(*cg));
	cg->code = r_array_create(sizeof(rvm_asmins_t));
	return cg;
}


void rvm_codegen_destroy(rvm_codegen_t *cg)
{
	r_array_destroy(cg->code);
	r_free(cg);
}


rvm_asmins_t *rvm_codegen_getcode(rvm_codegen_t *cg)
{
	return (rvm_asmins_t *)r_array_slot(cg->code, 0);
}


rulong rvm_codegen_getcodesize(rvm_codegen_t *cg)
{
	return r_array_size(cg->code);
}


void rvm_codegen_addins(rvm_codegen_t *cg, rvm_asmins_t ins)
{
	r_array_add(cg->code, &ins);
}


void rvm_codegen_funcstart(rvm_codegen_t *cg, ruint args)
{
	rvm_codegen_addins(cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(FP)|BIT(SP)|BIT(LR)));
	rvm_codegen_addins(cg, rvm_asm(RVM_MOV, FP, SP, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_ADD, SP, SP, DA, args));
}


void rvm_codegen_funcend(rvm_codegen_t *cg)
{
	rvm_codegen_addins(cg, rvm_asm(RVM_MOV, SP, FP, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(FP)|BIT(SP)|BIT(LR)));
	rvm_codegen_addins(cg, rvm_asm(RVM_RET, XX, XX, XX, 0));
}
