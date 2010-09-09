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


static void rvm_codegen_addins(rvm_codegen_t *cg, rvm_asmins_t ins)
{
	r_array_add(cg->code, &ins);
}


void rvm_codegen_funcstart(rvm_codegen_t *cg, ruint args)
{
	rvm_codegen_addins(cg, rvm_asm(RVM_ADD, SP, FP, DA, args));

}


void rvm_codegen_funcend(rvm_codegen_t *cg)
{

}
