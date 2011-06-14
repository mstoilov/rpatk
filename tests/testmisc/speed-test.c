#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "rvmoperator.h"


static void rvm_eadd(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_ADD, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
//	fprintf(stdout, "%s %ld\n", __FUNCTION__, RVM_CPUREG_GETU(cpu, ins->op1));
}


static void rvm_callback_two(rvmcpu_t *vm, rvm_asmins_t *ins)
{
	fprintf(stdout, "%s\n", __FUNCTION__);
}


static rvm_switable_t calltable[] = {
	{"rvm_callback_two", rvm_callback_two},
	{"rvm_eadd", rvm_eadd},
	{NULL, NULL},
};

	
int main(int argc, char *argv[])
{
	rlong i, iter = 1;
	ruinteger ret = 0;
	ruinteger off = 0;
	rvm_asmins_t vmcode[256];
	rvmcpu_t *vm = rvm_cpu_create_default();

	if (argc > 1) {
		iter = atol(argv[1]);
	}

	rvm_cpu_addswitable(vm, "calltable", calltable);
	rvm_cpu_addswitable(vm, "common_table", common_calltable);

	vmcode[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 0);
	for (i = 0; i < 25; i++)
		vmcode[off++] = rvm_asm(RVM_NOP, XX, XX, XX, 0);
	for (i = 0; i < 15; i++)
		vmcode[off++] = rvm_asm(RVM_NOP, XX, XX, XX, BIT(R5));
	for (i = 0; i < 15; i++)
		vmcode[off++] = rvm_asm(RVM_NOP, XX, XX, XX, BIT(R5));
	for (i = 0; i < 5; i++)
		vmcode[off++] = rvm_asm(RVM_NOP, XX, XX, XX, 0);
	for (i = 0; i < 5; i++)
		vmcode[off++] = rvm_asm(RVM_NOP, XX, XX, XX, 0);

	vmcode[off++] = rvm_asm(RVM_ADD, R0, R0, DA, 1);
	vmcode[off++] = rvm_asm(RVM_CMP, R0, DA, XX, iter);
	vmcode[off++] = rvm_asm(RVM_BLES, DA, XX, XX, -45);
	vmcode[off++] = rvm_asm(RVM_PRN, R0, XX, XX, 0);
	vmcode[off++] = rvm_asm(RVM_EXT, R0, XX, XX, 0);


	ret = rvm_cpu_exec(vm, vmcode, 0);
	fprintf(stdout, "R0 = %ld (%ld operations)\n", (unsigned long) RVM_CPUREG_GETU(vm, R0), (unsigned long)RVM_CPUREG_GETU(vm, R5));
	rvm_cpu_destroy(vm);
	return 0;
}
