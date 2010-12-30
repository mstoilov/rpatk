#include <stdio.h>
#include <stdlib.h>
#include "rvmcpu.h"
#include "rvmreg.h"
#include "rvmcodegen.h"
#include "rvmcodemap.h"


static void test_swi_print_r(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *r = RVM_CPUREG_PTR(cpu, ins->op1);

	if (rvm_reg_gettype(r) == RVM_DTYPE_WORD)
		fprintf(stdout, "R%d = %ld\n", ins->op1, RVM_REG_GETL(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_LONG)
		fprintf(stdout, "R%d = %ld\n", ins->op1, RVM_REG_GETL(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_DOUBLE)
		fprintf(stdout, "R%d = %5.2f\n", ins->op1, RVM_REG_GETD(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_STRING)
		fprintf(stdout, "R%d = %s\n", ins->op1, ((rstring_t*) RVM_REG_GETP(r))->s.str);
	else
		fprintf(stdout, "R%d = Unknown type\n", ins->op1);
}


static rvm_switable_t switable[] = {
		{"print", test_swi_print_r},
		{NULL, NULL},
};


int main(int argc, char *argv[])
{
	rvmcpu_t *cpu;
	rvm_codegen_t *cg;
	ruint ntable;

	cg = rvm_codegen_create();
	cpu = rvm_cpu_create();
	ntable = rvm_cpu_addswitable(cpu, switable);

	rvm_codemap_invalid_add_s(cg->codemap, "add2");

	rvm_codegen_addins(cg, rvm_asm(RVM_MOV, R0, DA, XX, 7));
	rvm_codegen_addins(cg, rvm_asm(RVM_STS, R0, SP, DA, 1 + RVM_CODEGEN_FUNCINITOFFSET));
	rvm_codegen_addins(cg, rvm_asm(RVM_MOV, R0, DA, XX, 8));
	rvm_codegen_addins(cg, rvm_asm(RVM_STS, R0, SP, DA, 2 + RVM_CODEGEN_FUNCINITOFFSET));
	rvm_codegen_addins(cg, rvm_asmx(RVM_BL,  DA, XX, XX, &rvm_codemap_lookup_s(cg->codemap, "add2")->index));
	rvm_codegen_addins(cg, rvm_asm(RVM_OPSWI(rvm_cpu_getswi_s(cpu, "print")), R0, XX, XX, 0));

	rvm_codegen_addins(cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));

	rvm_codegen_funcstart_s(cg, "add2", 2);
	rvm_codegen_addins(cg, rvm_asm(RVM_LDS, R0, FP, DA, 1));
	rvm_codegen_addins(cg, rvm_asm(RVM_LDS, R1, FP, DA, 2));
	rvm_codegen_addins(cg, rvm_asm(RVM_ADD, R0, R0, R1, 0));
	rvm_codegen_funcend(cg);

	rvm_codegen_addins(cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));

	rvm_relocate(rvm_codegen_getcode(cg, 0), rvm_codegen_getcodesize(cg));


	rvm_asm_dump(rvm_codegen_getcode(cg, 0), rvm_codegen_getcodesize(cg));

	rvm_cpu_exec_debug(cpu, rvm_codegen_getcode(cg, 0), 0);
	rvm_cpu_destroy(cpu);
	rvm_codegen_destroy(cg);


	fprintf(stdout, "It works!\n");
	return 0;
}
