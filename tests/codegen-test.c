#include <stdio.h>
#include <stdlib.h>
#include "rvm.h"
#include "rvmcodegen.h"
#include "rvmcodemap.h"


static void test_swi_print_r0(rvm_cpu_t *cpu)
{
	rword res = RVM_GET_REGU(cpu, R0);
	fprintf(stdout, "R0 = %d\n", (int)res);
}

static rvm_switable_t switable[] = {
		{"print", test_swi_print_r0},
		{NULL, NULL},
};


int main(int argc, char *argv[])
{
	rvm_cpu_t *cpu;
	rvm_codegen_t *cg;
	rvm_codelabel_t *label;
	ruint ntable;

	cg = rvm_codegen_create();
	cpu = rvm_cpu_create();
	ntable = rvm_cpu_switable_add(cpu, switable);

	rvm_codegen_addins(cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));

	rvm_codegen_addins(cg, rvm_asm(RVM_MOV, R0, DA, XX, 7));
	rvm_codegen_addins(cg, rvm_asm(RVM_STS, R0, SP, DA, 1 + RVM_CODEGEN_FUNCINITOFFSET));
	rvm_codegen_addins(cg, rvm_asm(RVM_MOV, R0, DA, XX, 8));
	rvm_codegen_addins(cg, rvm_asm(RVM_STS, R0, SP, DA, 2 + RVM_CODEGEN_FUNCINITOFFSET));
	rvm_codegen_addins(cg, rvm_asm(RVM_MOV, R0, DA, XX, 9));
	rvm_codegen_addins(cg, rvm_asm(RVM_STS, R0, SP, DA, 3 + RVM_CODEGEN_FUNCINITOFFSET));
	rvm_codegen_addins(cg, rvm_asm(RVM_BL,  DA, XX, XX, 15));
	rvm_codegen_addins(cg, rvm_asm(RVM_SWI, DA, XX, XX, RVM_SWI_ID(ntable, 0)));
	rvm_codegen_addins(cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));

	rvm_codegen_funcstart_str(cg, "add2", 2);
	rvm_codegen_addins(cg, rvm_asm(RVM_LDS, R0, FP, DA, 1));
	rvm_codegen_addins(cg, rvm_asm(RVM_LDS, R1, FP, DA, 2));
	rvm_codegen_addins(cg, rvm_asm(RVM_ADD, R0, R0, R1, 0));
	rvm_codegen_funcend(cg);

	rvm_codegen_addins(cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));

	rvm_codegen_funcstart_str(cg, "add3", 3);
	rvm_codegen_addins(cg, rvm_asm(RVM_LDS, R0, FP, DA, 1));
	rvm_codegen_addins(cg, rvm_asm(RVM_LDS, R1, FP, DA, 2));
	rvm_codegen_addins(cg, rvm_asm(RVM_STS, R0, SP, DA, 1 + RVM_CODEGEN_FUNCINITOFFSET));
	rvm_codegen_addins(cg, rvm_asm(RVM_STS, R1, SP, DA, 2 + RVM_CODEGEN_FUNCINITOFFSET));
//	rvm_codegen_addins(cg, rvm_asm(RVM_BL,  DA, XX, XX, 8));
	label = rvm_codemap_lookup_str(cg->codemap, "add2");
	rvm_codegen_addins(cg, rvm_asmr(RVM_BL,  DA, XX, XX, &label->ins));


	rvm_codegen_addins(cg, rvm_asm(RVM_LDS, R1, FP, DA, 3));
	rvm_codegen_addins(cg, rvm_asm(RVM_STS, R0, SP, DA, 1 + RVM_CODEGEN_FUNCINITOFFSET));
	rvm_codegen_addins(cg, rvm_asm(RVM_STS, R1, SP, DA, 2 + RVM_CODEGEN_FUNCINITOFFSET));
//	rvm_codegen_addins(cg, rvm_asm(RVM_BL,  DA, XX, XX, 5));
	label = rvm_codemap_lookup_str(cg->codemap, "add2");
	rvm_codegen_addins(cg, rvm_asmr(RVM_BL,  DA, XX, XX, &label->ins));

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
