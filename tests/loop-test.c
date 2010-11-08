#include <stdio.h>
#include "common.h"


	
int main(int argc, char *argv[])
{
	ruint ret = 0;
	ruint off = 0;
	rvm_asmins_t vmcode[256];
	rvmcpu_t *vm = rvm_cpu_create();
	
	rvmcpu_switable_add(vm, common_calltable);

	vmcode[off++] = rvm_asm(RVM_MOV, R4, DA, XX, 0);
	vmcode[off++] = rvm_asm(RVM_MOV, R5, DA, XX, 1000*1000 * 200);
	vmcode[off++] = rvm_asm(RVM_ADD, R4, R4, DA, 1);
	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 1);
	vmcode[off++] = rvm_asm(RVM_MOV, R2, DA, XX, 2);
	vmcode[off++] = rvm_asm(RVM_ADD, R0, R1, R2, 0);
	vmcode[off++] = rvm_asm(RVM_CMP, R4, R5, XX, 0);
	vmcode[off++] = rvm_asml(RVM_BLES, DA, XX, XX, -5);
	vmcode[off++] = rvm_asm(RVM_EXT, R0, XX, XX, 0);


	rvm_relocate(vmcode, off);
	ret = rvm_cpu_exec(vm, vmcode, 0);
	fprintf(stdout, "R0 = %ld (%ld operations)\n", (unsigned long) RVM_CPUREG_GETU(vm, R0), (unsigned long)RVM_CPUREG_GETU(vm, R5));
	rvm_cpu_destroy(vm);
	return 0;
}
