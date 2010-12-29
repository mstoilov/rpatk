#include <stdio.h>
#include "common.h"

	
int main(int argc, char *argv[])
{
	ruint i, ret = 0;
	ruint off = 0;
	rvm_asmins_t vmcode[1024];
	rvmcpu_t *vm = rvm_cpu_create();

	rvmcpu_switable_add(vm, common_calltable);
	vmcode[off++] = rvm_asm(RVM_PUSH, DA, XX, XX, 1);
	vmcode[off++] = rvm_asm(RVM_PUSH, DA, XX, XX, 2);
	vmcode[off++] = rvm_asm(RVM_POP, R1, XX, XX, 0);
	vmcode[off++] = rvm_asm(RVM_POP, R0, XX, XX, 0);
	vmcode[off++] = rvm_asm(RVM_ADD, R0, R1, R0, 0);
	VMTEST_REG(vmcode, off, 0, 3, "PUSH/POP");

	vmcode[off++] = rvm_asm(RVM_MOV, FP, SP, XX, 0);
	vmcode[off++] = rvm_asm(RVM_PUSH, DA, XX, XX, 1);
	vmcode[off++] = rvm_asm(RVM_PUSH, DA, XX, XX, 2);
	vmcode[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 3);
	vmcode[off++] = rvm_asm(RVM_STS, R0, FP, DA, 1);
	vmcode[off++] = rvm_asm(RVM_LDS, R2, FP, DA, 2);
	vmcode[off++] = rvm_asm(RVM_LDS, R3, FP, DA, 1);
	vmcode[off++] = rvm_asm(RVM_POP, R1, XX, XX, 0);
	vmcode[off++] = rvm_asm(RVM_POP, R0, XX, XX, 0);
	vmcode[off++] = rvm_asm(RVM_ADD, R0, R1, R0, 0);
	vmcode[off++] = rvm_asm(RVM_ADD, R2, R2, R3, 0);
	VMTEST_REG(vmcode, off, 0, 5, "PUSH/POP");
	VMTEST_REG(vmcode, off, 2, 5, "STS/LDS");

	vmcode[off++] = rvm_asm(RVM_MOV, FP, DA, XX, 100);
	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 1);
	vmcode[off++] = rvm_asm(RVM_MOV, R2, DA, XX, 2);
	vmcode[off++] = rvm_asm(RVM_STS, R1, FP, DA, 1);
	vmcode[off++] = rvm_asm(RVM_STS, R2, FP, DA, 2);
	vmcode[off++] = rvm_asm(RVM_LDS, R2, FP, DA, 1);
	vmcode[off++] = rvm_asm(RVM_LDS, R3, FP, DA, 2);
	vmcode[off++] = rvm_asm(RVM_ADD, R2, R2, R3, 0);
	VMTEST_REG(vmcode, off, 2, 3, "STS/LDS");
	
	vmcode[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 0);
	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 1);
	vmcode[off++] = rvm_asm(RVM_MOV, R2, DA, XX, 2);
	vmcode[off++] = rvm_asm(RVM_MOV, R5, DA, XX, 5);
	vmcode[off++] = rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(0)|BIT(1)|BIT(2)|BIT(5)|BIT(SP));
	vmcode[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 0);
	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 0);
	vmcode[off++] = rvm_asm(RVM_MOV, R2, DA, XX, 0);
	vmcode[off++] = rvm_asm(RVM_MOV, R5, DA, XX, 0);
	vmcode[off++] = rvm_asm(RVM_POPM, DA, XX, XX, BIT(0)|BIT(1)|BIT(2)|BIT(5)|BIT(SP));
	VMTEST_REG(vmcode, off, 5, 5, "PUSHM/POPM");
	VMTEST_REG(vmcode, off, 1, 1, "PUSHM/POPM");

	for (i = 0; i < 512; i++)
		vmcode[off++] = rvm_asm(RVM_PUSH, DA, XX, XX, i);

	vmcode[off++] = rvm_asm(RVM_PUSH, DA, XX, XX, 1);
	vmcode[off++] = rvm_asm(RVM_PUSH, DA, XX, XX, 2);
	vmcode[off++] = rvm_asm(RVM_POP, R1, XX, XX, 0);
	vmcode[off++] = rvm_asm(RVM_POP, R0, XX, XX, 0);
	vmcode[off++] = rvm_asm(RVM_ADD, R0, R1, R0, 0);
	VMTEST_REG(vmcode, off, 0, 3, "PUSH/POP");

	
	
	vmcode[off++] = rvm_asm(RVM_EXT, R0, XX, XX, 0);
#ifdef EXECDEBUG
	ret = rvm_cpu_exec_debug(vm, vmcode, 0);
#else
	ret = rvm_cpu_exec(vm, vmcode, 0);
#endif
	
	rvm_cpu_destroy(vm);
	return 0;
}
