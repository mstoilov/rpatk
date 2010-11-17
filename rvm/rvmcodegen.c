#include "rmem.h"
#include "rvmcodegen.h"
#include "rvmcpu.h"


rvm_codegen_t *rvm_codegen_create()
{
	rvm_codegen_t *cg;

	cg = (rvm_codegen_t *)r_malloc(sizeof(*cg));
	if (!cg)
		return (NULL);
	r_memset(cg, 0, sizeof(*cg));
	cg->code = r_array_create(sizeof(rvm_asmins_t));
	cg->codemap = rvm_codemap_create();
	return cg;
}


void rvm_codegen_destroy(rvm_codegen_t *cg)
{
	rvm_codemap_destroy(cg->codemap);
	r_array_destroy(cg->code);
	r_free(cg);
}


rvm_asmins_t *rvm_codegen_getcode(rvm_codegen_t *cg, ruint index)
{
	return (rvm_asmins_t *)r_array_slot(cg->code, index);
}


rulong rvm_codegen_getcodesize(rvm_codegen_t *cg)
{
	return r_array_size(cg->code);
}


ruint rvm_codegen_addins(rvm_codegen_t *cg, rvm_asmins_t ins)
{
	return r_array_add(cg->code, &ins);
}


ruint rvm_codegen_insertins(rvm_codegen_t *cg, ruint index, rvm_asmins_t ins)
{
	return r_array_insert(cg->code, index, &ins);
}


ruint rvm_codegen_funcstart(rvm_codegen_t *cg, const rchar* name, ruint namesize, ruint args)
{
	ruint start = rvm_codegen_addins(cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(R0)|BIT(FP)|BIT(SP)|BIT(LR)));
	rvm_codegen_addins(cg, rvm_asm(RVM_MOV, FP, SP, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_ADD, SP, SP, DA, args));
	rvm_codemap_add(cg->codemap, name, namesize, start);
	return start;
}


ruint rvm_codegen_funcstart_s(rvm_codegen_t *cg, const rchar* name, ruint args)
{
	return rvm_codegen_funcstart(cg, name, r_strlen(name), args);
}


ruint rvm_codegen_vargs_funcstart(rvm_codegen_t *cg, const rchar* name, ruint namesize)
{
	ruint start = rvm_codegen_addins(cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(R0)|BIT(FP)|BIT(SP)|BIT(LR)));
	rvm_codegen_addins(cg, rvm_asm(RVM_MOV, FP, SP, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_ADD, SP, SP, R0, 0));
	rvm_codemap_add(cg->codemap, name, namesize, start);
	return start;
}


ruint rvm_codegen_vargs_funcstart_s(rvm_codegen_t *cg, const rchar* name)
{
	return rvm_codegen_vargs_funcstart(cg, name, r_strlen(name));
}


void rvm_codegen_funcend(rvm_codegen_t *cg)
{
	rvm_codegen_addins(cg, rvm_asm(RVM_MOV, SP, FP, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(FP)|BIT(SP)|BIT(LR)));
	rvm_codegen_addins(cg, rvm_asm(RVM_RET, XX, XX, XX, 0));
}
