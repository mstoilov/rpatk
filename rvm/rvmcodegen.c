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
	r_object_destroy((robject_t*)cg->code);
	r_free(cg);
}


void rvm_codegen_clear(rvm_codegen_t *cg)
{
	r_array_setlength(cg->code, 0);
	rvm_codemap_clear(cg->codemap);
}


rvm_asmins_t *rvm_codegen_getcode(rvm_codegen_t *cg, ruint index)
{
	return (rvm_asmins_t *)r_array_slot(cg->code, index);
}


rulong rvm_codegen_getcodesize(rvm_codegen_t *cg)
{
	return r_array_length(cg->code);
}

void rvm_codegen_setcodesize(rvm_codegen_t *cg, ruint size)
{
	r_array_setlength(cg->code, size);
}


ruint rvm_codegen_addins(rvm_codegen_t *cg, rvm_asmins_t ins)
{
	return r_array_add(cg->code, &ins);
}


ruint rvm_codegen_insertins(rvm_codegen_t *cg, ruint index, rvm_asmins_t ins)
{
	return r_array_insert(cg->code, index, &ins);
}


ruint rvm_codegen_replaceins(rvm_codegen_t *cg, ruint index, rvm_asmins_t ins)
{
	return r_array_replace(cg->code, index, &ins);

}


ruint rvm_codegen_funcstart(rvm_codegen_t *cg, const rchar* name, ruint namesize, ruint args)
{
	ruint start;
	rvm_codegen_addins(cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));
	start = rvm_codegen_addins(cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(FP)|BIT(SP)|BIT(LR)));
	rvm_codegen_addins(cg, rvm_asm(RVM_MOV, FP, SP, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_ADD, SP, SP, DA, args));
	rvm_codemap_addindex(cg->codemap, name, namesize, start);
	return start;
}


ruint rvm_codegen_funcstart_s(rvm_codegen_t *cg, const rchar* name, ruint args)
{
	return rvm_codegen_funcstart(cg, name, r_strlen(name), args);
}


ruint rvm_codegen_vargs_funcstart(rvm_codegen_t *cg, const rchar* name, ruint namesize)
{
	ruint start;
	rvm_codegen_addins(cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));
	start = rvm_codegen_addins(cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(FP)|BIT(SP)|BIT(LR)));
	rvm_codegen_addins(cg, rvm_asm(RVM_MOV, FP, SP, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_ADD, SP, SP, R0, 0));
	rvm_codemap_addindex(cg->codemap, name, namesize, start);
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
	rvm_codegen_addins(cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
}
