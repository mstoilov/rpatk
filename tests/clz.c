#include <stdio.h>
#include "common.h"


	
int main(int argc, char *argv[])
{
	ruint ret = 0;
	ruint off = 0;
	rvm_asmins_t vmcode[256];
	rvm_cpu_t *vm = rvm_cpu_create();
	
	rvm_cpu_switable_add(vm, common_calltable);
	vmcode[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 0);
	vmcode[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 0);
	vmcode[off++] = rvm_asm(RVM_CLZ, R0, R0, XX, 0);
	VMTEST_REG(vmcode, off, 0, sizeof * 8, "CLZ 0");

	vmcode[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 0);
	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 1 << 0);
	vmcode[off++] = rvm_asm(RVM_CLZ, R0, R1, XX, 0);
	VMTEST_REG(vmcode, off, 0, sizeof * 8 - 1, "CLZ 1");

	vmcode[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 0);
	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 1 << 1);
	vmcode[off++] = rvm_asm(RVM_CLZ, R0, R1, XX, 0);
	VMTEST_REG(vmcode, off, 0, sizeof * 8 - 2, "CLZ 2");


	vmcode[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 0);
	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 1 << 31);
	vmcode[off++] = rvm_asm(RVM_CLZ, R0, R1, XX, 0);
	VMTEST_REG(vmcode, off, 0, sizeof * 8 - 32, "CLZ (1<<31)");

	vmcode[off++] = rvm_asm(RVM_EXT, R0, XX, XX, 0);
#ifdef EXECDEBUG
	ret = rvm_cpu_exec_debug(vm, vmcode, 0);
#else
	ret = rvm_cpu_exec(vm, vmcode, 0);
#endif
	
	rvm_cpu_destroy(vm);
	return 0;
}
