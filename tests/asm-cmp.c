#include <stdio.h>
#include "common.h"

	
int main(int argc, char *argv[])
{
	ruint ret = 0;
	ruint off = 0;
	rvm_asmins_t vmcode[256];
	rvmcpu_t *vm = rvm_cpu_create();
	
	rvm_cpu_addswitable(vm, common_calltable);
	vmcode[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 0);
	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 1);
	vmcode[off++] = rvm_asm(RVM_MOV, R2, DA, XX, 2);
	vmcode[off++] = rvm_asm(RVM_MOV, R3, DA, XX, -3);

	vmcode[off++] = rvm_asm(RVM_CMP, R0, DA, XX, 0);
	VMTEST_STATUS(vmcode, off, RVM_STATUS_Z, "CMP (0, 0)");
	vmcode[off++] = rvm_asm(RVM_CMP, R1, R0, XX, 0);
	VMTEST_STATUS(vmcode, off, 0, "CMP (1, 0)");
	vmcode[off++] = rvm_asm(RVM_CMP, R1, R2, XX, 0);
	VMTEST_STATUS(vmcode, off, RVM_STATUS_N | RVM_STATUS_C, "CMP (1, 2)");
	vmcode[off++] = rvm_asm(RVM_CMP, R3, R2, XX, 0);
	VMTEST_STATUS(vmcode, off, RVM_STATUS_N | RVM_STATUS_V, "CMP (-3, 2)");
	vmcode[off++] = rvm_asm(RVM_CMP, R2, R3, XX, 0);
	VMTEST_STATUS(vmcode, off, RVM_STATUS_C | RVM_STATUS_V, "CMP (2, -3)");

	vmcode[off++] = rvm_asm(RVM_CMN, R0, DA, XX, 0);
	VMTEST_STATUS(vmcode, off, RVM_STATUS_Z, "CMN (0, 0)");
	vmcode[off++] = rvm_asm(RVM_CMN, R1, R0, XX, 0);
	VMTEST_STATUS(vmcode, off, 0, "CMN (1, 0)");
	vmcode[off++] = rvm_asm(RVM_CMN, R1, R2, XX, 0);
	VMTEST_STATUS(vmcode, off, 0, "CMN (1, 2)");
	vmcode[off++] = rvm_asm(RVM_CMN, R3, R2, XX, 0);
	VMTEST_STATUS(vmcode, off, RVM_STATUS_N, "CMN (-3, 2)");
	vmcode[off++] = rvm_asm(RVM_CMN, R2, R3, XX, 0);
	VMTEST_STATUS(vmcode, off, RVM_STATUS_N, "CMN (2, -3)");

	vmcode[off++] = rvm_asm(RVM_EXT, R0, XX, XX, 0);
#ifdef EXECDEBUG
	ret = rvm_cpu_exec_debug(vm, vmcode, 0);
#else
	ret = rvm_cpu_exec(vm, vmcode, 0);
#endif
	
	rvm_cpu_destroy(vm);
	return 0;
}
