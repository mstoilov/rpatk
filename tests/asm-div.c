#include <stdio.h>
#include "common.h"


	
int main(int argc, char *argv[])
{
	ruint ret = 0;
	ruint off = 0;
	rvm_asmins_t vmcode[256];
	rvmcpu_t *vm = rvm_cpu_create();
	
	rvm_cpu_addswitable(vm, common_calltable);
	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 3);
	vmcode[off++] = rvm_asm(RVM_MOV, R2, DA, XX, 1);
	vmcode[off++] = rvm_asm(RVM_DIV, R0, R1, R2, 0);
	VMTEST_REG(vmcode, off, 0, 3, "DIV(3, 1)");

	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 5);
	vmcode[off++] = rvm_asm(RVM_MOV, R2, DA, XX, 2);
	vmcode[off++] = rvm_asm(RVM_DIVS, R0, R1, R2, 0);
	VMTEST_REG(vmcode, off, 0, 2, "DIVS(5, 2)");
	VMTEST_STATUS(vmcode, off, 0, "DIVS STATUS");

	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 0);
	vmcode[off++] = rvm_asm(RVM_MOV, R2, DA, XX, 2);
	vmcode[off++] = rvm_asm(RVM_DIVS, R0, R1, R2, 0);
	VMTEST_REG(vmcode, off, 0, 0, "DIVS(0, 2)");
	VMTEST_STATUS(vmcode, off, RVM_STATUS_Z, "DIVS STATUS");

	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 5);
	vmcode[off++] = rvm_asm(RVM_MOV, R2, DA, XX, -2);
	vmcode[off++] = rvm_asm(RVM_DVS, R0, R1, R2, 0);
	VMTEST_REG(vmcode, off, 0, -2, "DVS(5, -2)");
	VMTEST_STATUS(vmcode, off, RVM_STATUS_N, "DVS STATUS");

	vmcode[off++] = rvm_asm(RVM_EXT, R0, XX, XX, 0);
#ifdef EXECDEBUG
	ret = rvm_cpu_exec_debug(vm, vmcode, 0);
#else
	ret = rvm_cpu_exec(vm, vmcode, 0);
#endif
	rvm_cpu_destroy(vm);
	return 0;
}
