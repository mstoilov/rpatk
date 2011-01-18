#include <stdio.h>
#include "common.h"


	
int main(int argc, char *argv[])
{
	ruint ret = 0;
	ruint off = 0;
	rvm_asmins_t vmcode[256];
	rvmcpu_t *vm = rvm_cpu_create_default();
	
	rvm_cpu_addswitable(vm, common_calltable);
	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 3);
	vmcode[off++] = rvm_asm(RVM_MOV, R2, DA, XX, 1);
	vmcode[off++] = rvm_asm(RVM_MUL, R0, R1, R2, 0);
	VMTEST_REG(vmcode, off, 0, 3, "MUL(3, 1)");
	VMTEST_STATUS(vmcode, off, 0, "MUL STATUS");

	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 3);
	vmcode[off++] = rvm_asm(RVM_MOV, R2, DA, XX, 1);
	vmcode[off++] = rvm_asm(RVM_MULS, R0, R1, R2, 0);
	VMTEST_REG(vmcode, off, 0, 3, "MULS(3, 1)");
	VMTEST_STATUS(vmcode, off, 0, "MULS STATUS");

	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, -3);
	vmcode[off++] = rvm_asm(RVM_MOV, R2, DA, XX, -1);
	vmcode[off++] = rvm_asm(RVM_MULS, R0, R1, R2, 0);
	VMTEST_REG(vmcode, off, 0, 3, "MULS(-3, -1)");
	VMTEST_STATUS(vmcode, off, RVM_STATUS_C, "MULS STATUS");

	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, -3);
	vmcode[off++] = rvm_asm(RVM_MOV, R2, DA, XX, 2);
	vmcode[off++] = rvm_asm(RVM_MLS, R0, R1, R2, 0);
	VMTEST_REG(vmcode, off, 0, -6, "MLS(-3, 2)");
	VMTEST_STATUS(vmcode, off, RVM_STATUS_N, "MLS STATUS");


	vmcode[off++] = rvm_asm(RVM_EXT, R0, XX, XX, 0);
#ifdef EXECDEBUG
	ret = rvm_cpu_exec_debug(vm, vmcode, 0);
#else
	ret = rvm_cpu_exec(vm, vmcode, 0);
#endif
	rvm_cpu_destroy(vm);
	return 0;
}
