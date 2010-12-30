#include <stdio.h>
#include "common.h"


	
int main(int argc, char *argv[])
{
	ruint ret = 0;
	ruint off = 0;
	rvm_asmins_t vmcode[256];
	rvmcpu_t *vm = rvm_cpu_create();
	
	rvm_cpu_addswitable(vm, common_calltable);
	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 1);
	vmcode[off++] = rvm_asm(RVM_LSL, R0, R1, DA, 3);
	vmcode[off++] = rvm_asm(RVM_LSR, R0, R0, DA, 3);
	vmcode[off++] = rvm_asm(RVM_ROR, R0, R0, DA, 1);
	vmcode[off++] = rvm_asm(RVM_ROR, R0, R0, DA, sizeof(rword) * 8 - 1);
	VMTEST_REG(vmcode, off, 0, 1, "SHIFT");

	vmcode[off++] = rvm_asm(RVM_EXT, R0, XX, XX, 0);
#ifdef EXECDEBUG
	ret = rvm_cpu_exec_debug(vm, vmcode, 0);
#else
	ret = rvm_cpu_exec(vm, vmcode, 0);
#endif
	
	rvm_cpu_destroy(vm);
	return 0;
}
