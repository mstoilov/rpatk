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
	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 0);
	vmcode[off++] = rvm_asm(RVM_MOV, R2, DA, XX, 1);
	vmcode[off++] = rvm_asm(RVM_MOV, R3, DA, XX, 2);
	vmcode[off++] = rvm_asm(RVM_MOV, R4, DA, XX, -3);

	vmcode[off++] = rvm_asm(RVM_ORR, R0, R1, R1, 0);
	VMTEST_REG(vmcode, off, 0, 0, "ORR (0, 0)");
	vmcode[off++] = rvm_asm(RVM_ORR, R0, R1, R2, 0);
	VMTEST_REG(vmcode, off, 0, 1, "ORR (0, 1)");
	vmcode[off++] = rvm_asm(RVM_ORR, R0, R2, R3, 0);
	VMTEST_REG(vmcode, off, 0, 3, "ORR (1, 2)");


	vmcode[off++] = rvm_asm(RVM_EOR, R0, R1, R1, 0);
	VMTEST_REG(vmcode, off, 0, 0, "EOR (0, 0)");
	vmcode[off++] = rvm_asm(RVM_EOR, R0, R1, R2, 0);
	VMTEST_REG(vmcode, off, 0, 1, "EOR (0, 1)");
	vmcode[off++] = rvm_asm(RVM_EOR, R0, R1, R3, 0);
	VMTEST_REG(vmcode, off, 0, 2, "EOR (0, 2)");
	vmcode[off++] = rvm_asm(RVM_EOR, R0, R2, R4, 0);
	VMTEST_REG(vmcode, off, 0, -4, "EOR (1, -3)");

	vmcode[off++] = rvm_asm(RVM_AND, R0, R1, R1, 0);
	VMTEST_REG(vmcode, off, 0, 0, "AND (0, 0)");
	vmcode[off++] = rvm_asm(RVM_AND, R0, R1, R2, 0);
	VMTEST_REG(vmcode, off, 0, 0, "AND (0, 1)");
	vmcode[off++] = rvm_asm(RVM_AND, R0, R1, R3, 0);
	VMTEST_REG(vmcode, off, 0, 0, "AND (1, 2)");
	vmcode[off++] = rvm_asm(RVM_AND, R0, R2, R4, 0);
	VMTEST_REG(vmcode, off, 0, 1, "AND (1, -3)");
	vmcode[off++] = rvm_asm(RVM_NOT, R0, R2, XX, 0);
	VMTEST_REG(vmcode, off, 0, -2, "NOT 1");


	vmcode[off++] = rvm_asm(RVM_EXT, R0, XX, XX, 0);
#ifdef EXECDEBUG
	ret = rvm_cpu_exec_debug(vm, vmcode, 0);
#else
	ret = rvm_cpu_exec(vm, vmcode, 0);
#endif
	
	rvm_cpu_destroy(vm);
	return 0;
}
