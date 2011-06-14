#include <stdio.h>
#include "common.h"


	
int main(int argc, char *argv[])
{
	ruinteger ret = 0;
	ruinteger off = 0;
	rvm_asmins_t vmcode[256];
	rvmcpu_t *vm = rvm_cpu_create_default();
	
	rvm_cpu_addswitable(vm, "common_table", common_calltable);

	vmcode[off++] = rvm_asm(RVM_B,   DA, XX, XX, 3);
	vmcode[off++] = rvm_asm(RVM_ADD, R0, R0, R1, 0);
	vmcode[off++] = rvm_asm(RVM_RET, XX, XX, XX, 0);
	vmcode[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 1);
	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 2);
	vmcode[off++] = rvm_asm(RVM_BL,  DA, XX, XX, -4);
	vmcode[off++] = rvm_asm(RVM_NOP, XX, XX, XX, 0);
	VMTEST_REG(vmcode, off, 0, 3, "BL/RET");
	vmcode[off++] = rvm_asm(RVM_EXT, R0, XX, XX, 0);
#ifdef EXECDEBUG
	ret = rvm_cpu_exec_debug(vm, vmcode, 0);
#else
	ret = rvm_cpu_exec(vm, vmcode, 0);
#endif
	rvm_cpu_destroy(vm);
	return 0;
}
