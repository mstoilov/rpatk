#include <stdio.h>
#include <stdlib.h>
#include "rvmcodegen.h"
#include "rstring.h"
#include "rmem.h"
#include "rvmcpu.h"
#include "rvmreg.h"
#include "rvmoperator.h"


typedef struct rvm_testctx_s {
	rvm_opmap_t *opmap;
} rvm_testctx_t;


static void test_swi_cat(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvm_testctx_t *ctx = (rvm_testctx_t *)cpu->userdata1;
	rvm_opmap_invoke_binary_handler(ctx->opmap, RVM_OPID_CAT, cpu, RVM_CPUREG_PTR(cpu, ins->op1), RVM_CPUREG_PTR(cpu, ins->op2), RVM_CPUREG_PTR(cpu, ins->op3));
}


static void test_swi_print_r(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	if (rvm_reg_gettype(RVM_CPUREG_PTR(cpu, ins->op1)) == RVM_DTYPE_LONG)
		fprintf(stdout, "R%d = %ld\n", ins->op1, RVM_CPUREG_GETL(cpu, ins->op1));
	else if (rvm_reg_gettype(RVM_CPUREG_PTR(cpu, ins->op1)) == RVM_DTYPE_DOUBLE)
		fprintf(stdout, "R%d = %5.2f\n", ins->op1, RVM_CPUREG_GETD(cpu, ins->op1));
	else if (rvm_reg_gettype(RVM_CPUREG_PTR(cpu, ins->op1)) == RVM_DTYPE_STRING)
		fprintf(stdout, "R%d = %s\n", ins->op1, ((rstring_t*) RVM_CPUREG_GETP(cpu, ins->op1))->s.str);
	else
		fprintf(stdout, "Unknown type\n");
}


static void test_swi_strinit(rvmcpu_t *cpu, rvm_asmins_t *ins)
{

}


static void test_swi_unref(rvmcpu_t *cpu, rvm_asmins_t *ins)
{

}


static void test_swi_strtodouble(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rstring_t *s = (rstring_t*)RVM_CPUREG_GETP(cpu, ins->op1);
	double d = strtod(s->s.str, NULL);
	RVM_CPUREG_SETD(cpu, R0, d);
}


static rvm_switable_t switable[] = {
		{"str_init", test_swi_strinit},
		{"str_to_double", test_swi_strtodouble},
		{"unref", test_swi_unref},
		{"cat", test_swi_cat},
		{"print", test_swi_print_r},
		{NULL, NULL},
};


int main(int argc, char *argv[])
{
	rchar *hello = "Hello World";
	rchar *there = ", right there";

	rvm_testctx_t ctx;
	rvm_codegen_t *cg;
	rvm_codelabel_t *err;
	rvmcpu_t *cpu;
	rvm_opmap_t *opmap;
	ruint ntable;

	ctx.opmap = opmap = rvm_opmap_create();
	cpu = rvm_cpu_create_default();
	cpu->userdata1 = &ctx;
	cg = rvm_codegen_create();


	rvm_codemap_invalid_add_s(cg->codemap, "str_init");
	rvm_codemap_invalid_add_s(cg->codemap, "str_to_double");
	ntable = rvm_cpu_addswitable(cpu, switable);

	rvm_codegen_addins(cg, rvm_asmp(RVM_MOV, R0, DA, XX, hello));
	rvm_codegen_addins(cg, rvm_asm(RVM_STS, R0, SP, DA, 1 + RVM_CODEGEN_FUNCINITOFFSET));
	rvm_codegen_addins(cg, rvm_asml(RVM_MOV, R0, DA, XX, r_strlen(hello)));
	rvm_codegen_addins(cg, rvm_asm(RVM_STS, R0, SP, DA, 2 + RVM_CODEGEN_FUNCINITOFFSET));
	rvm_codegen_addrelocins_s(cg, RVM_RELOC_BRANCH, "str_init", rvm_asm(RVM_BL,  DA, XX, XX, 0));
	rvm_codegen_addins(cg, rvm_asmp(RVM_MOV, R7, R0, XX, 0));

	rvm_codegen_addins(cg, rvm_asmp(RVM_MOV, R0, DA, XX, there));
	rvm_codegen_addins(cg, rvm_asm(RVM_STS, R0, SP, DA, 1 + RVM_CODEGEN_FUNCINITOFFSET));
	rvm_codegen_addins(cg, rvm_asml(RVM_MOV, R0, DA, XX, r_strlen(there)));
	rvm_codegen_addins(cg, rvm_asm(RVM_STS, R0, SP, DA, 2 + RVM_CODEGEN_FUNCINITOFFSET));
	rvm_codegen_addrelocins_s(cg, RVM_RELOC_BRANCH, "str_init", rvm_asm(RVM_BL,  DA, XX, XX, 0));
	rvm_codegen_addins(cg, rvm_asmp(RVM_MOV, R8, R0, XX, 0));

	rvm_codegen_addins(cg, rvm_asmp(RVM_MOV, R0, R7, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_OPSWI(rvm_cpu_getswi_s(cpu, "cat")), R0, R0, R8, 0));
	rvm_codegen_addins(cg, rvm_asmp(RVM_MOV, R7, R0, XX, 0));

	rvm_codegen_addins(cg, rvm_asm(RVM_OPSWI(rvm_cpu_getswi_s(cpu, "print")), R7, XX, XX, 0));	// print
	rvm_codegen_addins(cg, rvm_asm(RVM_OPSWI(rvm_cpu_getswi_s(cpu, "print")), R8, XX, XX, 0));	// print
	rvm_codegen_addins(cg, rvm_asm(RVM_OPSWI(rvm_cpu_getswi_s(cpu, "unref")), R7, XX, XX, 0));	// unref
	rvm_codegen_addins(cg, rvm_asm(RVM_OPSWI(rvm_cpu_getswi_s(cpu, "unref")), R8, XX, XX, 0));	// unref
	rvm_codegen_addins(cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));

	rvm_codegen_funcstart_s(cg, "str_init", 2);
	rvm_codegen_addins(cg, rvm_asm(RVM_LDS, R0, FP, DA, 1));
	rvm_codegen_addins(cg, rvm_asm(RVM_LDS, R1, FP, DA, 2));
	rvm_codegen_addins(cg, rvm_asm(RVM_OPSWI(rvm_cpu_getswi_s(cpu, "str_init")), R0, R1, XX, 0));
	rvm_codegen_funcend(cg);

	rvm_codegen_funcstart_s(cg, "str_to_double", 1);
	rvm_codegen_addins(cg, rvm_asm(RVM_LDS, R0, FP, DA, 1));
	rvm_codegen_addins(cg, rvm_asm(RVM_OPSWI(rvm_cpu_getswi_s(cpu, "str_to_double")), R0, R0, XX, 0));
	rvm_codegen_funcend(cg);

	if (rvm_codegen_relocate(cg, &err) < 0) {
		r_printf("Unresolved symbol: %s\n", err->name->str);
		goto end;
	}

	rvm_cpu_exec_debug(cpu, rvm_codegen_getcode(cg, 0), 0);

end:
	rvm_cpu_destroy(cpu);
	rvm_opmap_destroy(opmap);
	rvm_codegen_destroy(cg);

	fprintf(stdout, "Max alloc mem: %ld\n", r_debug_get_maxmem());
	fprintf(stdout, "Leaked mem: %ld\n", r_debug_get_allocmem());
	return 0;
}
