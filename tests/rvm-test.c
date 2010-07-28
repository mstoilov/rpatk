#include <stdio.h>
#include <stdlib.h>
#include "rvm.h"


static void test_swi_sub(rvm_cpu_t *cpu)
{
	rword res, op2 = RVM_GET_REGU(cpu, R0), op3 = RVM_GET_REGU(cpu, R1);

	res = op2 - op3;
	RVM_SET_REGU(cpu, R0, res);
}


static void test_swi_mul(rvm_cpu_t *cpu)
{
	rword res, op2 = RVM_GET_REGU(cpu, R0), op3 = RVM_GET_REGU(cpu, R1);

	res = op2 * op3;
	RVM_SET_REGU(cpu, R0, res);
}


int main(int argc, char *argv[])
{
	rvm_cpu_t *cpu;
	rvm_asmins_t code[1024];
	ruint off = 0;
	ruint nsub, nmul;

	cpu = rvm_cpu_create();
	nsub = rvm_cpu_switable_add(cpu, test_swi_sub);
	nmul = rvm_cpu_switable_add(cpu, test_swi_mul);

	code[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 1);
	code[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 2);
	code[off++] = rvm_asm(RVM_ADD, R2, R0, R1, 0);
	code[off++] = rvm_asm(RVM_SWI, DA, XX, XX, nmul);
	code[off++] = rvm_asm(RVM_SWI, DA, XX, XX, nsub);
	code[off++] = rvm_asm(RVM_EXT, XX, XX, XX, 0);
	rvm_cpu_exec_debug(cpu, code, 0);
	rvm_cpu_destroy(cpu);


	fprintf(stdout, "It works!\n");
	return 0;
}
