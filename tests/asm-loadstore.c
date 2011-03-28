#include <stdio.h>
#include "common.h"

	
int main(int argc, char *argv[])
{
	ruint ret = 0;
	rword s =  -2;
	rword d =  0;
	ruint off = 0;
	rvm_asmins_t vmcode[256];
	rvmcpu_t *vm = rvm_cpu_create_default();

	rvm_cpu_addswitable(vm, "common_table", common_calltable);
	vmcode[off++] = rvm_asmp(RVM_LDR, R0, DA, XX, &s);
	VMTEST_REG(vmcode, off, 0, -2, "LDR");
	vmcode[off++] = rvm_asmp(RVM_LDRB, R0, DA, XX, &s);
	VMTEST_REG(vmcode, off, 0, 0xFE, "LDRB");
	vmcode[off++] = rvm_asmp(RVM_LDRH, R0, DA, XX, &s);
	VMTEST_REG(vmcode, off, 0, 0xFFFE, "LDRH");
	
	vmcode[off++] = rvm_asmp(RVM_MOV, R5, DA, XX, &d);
	vmcode[off++] = rvm_asm(RVM_STR, DA, R5, XX, 0);
	vmcode[off++] = rvm_asmp(RVM_LDR, R0, DA, XX, &s);
	vmcode[off++] = rvm_asm(RVM_STR, R0, R5, XX, 0);
	vmcode[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 0);
	vmcode[off++] = rvm_asm(RVM_LDR, R0, R5, XX, 0);
	VMTEST_REG(vmcode, off, 0, -2, "LDR");

	vmcode[off++] = rvm_asm(RVM_STR, DA, R5, XX, 0);
	vmcode[off++] = rvm_asmp(RVM_LDR, R0, DA, XX, &s);
	vmcode[off++] = rvm_asm(RVM_STRB, R0, R5, XX, 0);
	vmcode[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 0);
	vmcode[off++] = rvm_asm(RVM_LDR, R0, R5, XX, 0);
	VMTEST_REG(vmcode, off, 0, 0xFE, "LDRB");

	vmcode[off++] = rvm_asm(RVM_STR, DA, R5, XX, 0);
	vmcode[off++] = rvm_asmp(RVM_LDR, R0, DA, XX, &s);
	vmcode[off++] = rvm_asm(RVM_STRH, R0, R5, XX, 0);
	vmcode[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 0);
	vmcode[off++] = rvm_asm(RVM_LDR, R0, R5, XX, 0);
	VMTEST_REG(vmcode, off, 0, 0xFFFE, "LDRH");

	vmcode[off++] = rvm_asm(RVM_EXT, R0, XX, XX, 0);
#ifdef EXECDEBUG
	ret = rvm_cpu_exec_debug(vm, vmcode, 0);
#else
	ret = rvm_cpu_exec(vm, vmcode, 0);
#endif
	
	rvm_cpu_destroy(vm);
	return 0;
}
