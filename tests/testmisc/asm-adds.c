#include <stdio.h>
#include "common.h"


	
int main(int argc, char *argv[])
{
	ruint ret = 0;
	ruint off = 0;
	rvm_asmins_t vmcode[256];
	rvmcpu_t *vm = rvm_cpu_create_default();
	
	rvm_cpu_addswitable(vm, "common_table", common_calltable);
	vmcode[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 0);
	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, -2);
	vmcode[off++] = rvm_asm(RVM_MOV, R2, DA, XX, -3);
	vmcode[off++] = rvm_asm(RVM_MOV, R3, DA, XX, 1);
	vmcode[off++] = rvm_asm(RVM_MOV, R4, DA, XX, (rword)0x7FFFEEDD00FFEEDD);
	vmcode[off++] = rvm_asm(RVM_MOV, R5, DA, XX, 3);

	vmcode[off++] = rvm_asm(RVM_ADDS, R0, R0, R1, 0);	/* N */
	VMTEST_REG(vmcode, off, 0, -2, "ADDS (0, -2)");
	VMTEST_STATUS(vmcode, off, RVM_STATUS_N, "ADDS STATUS");
	
	vmcode[off++] = rvm_asm(RVM_ADDS, R0, R1, R2, 0);	/* C N */
	VMTEST_REG(vmcode, off, 0, -5, "ADDS (-2, -3)");
	VMTEST_STATUS(vmcode, off, RVM_STATUS_N|RVM_STATUS_C, "ADDS STATUS");
	
	vmcode[off++] = rvm_asm(RVM_ADDS, R0, R2, R5, 0);	/* C Z */
	VMTEST_REG(vmcode, off, 0, 0, "ADDS (-3, 3)");
	VMTEST_STATUS(vmcode, off, RVM_STATUS_Z|RVM_STATUS_C, "ADDS STATUS");
	
	vmcode[off++] = rvm_asm(RVM_ADDS, R0, R3, R5, 0);
	VMTEST_REG(vmcode, off, 0, 4, "ADDS (1, 3)");
	VMTEST_STATUS(vmcode, off, 0, "ADDS STATUS");
	
	vmcode[off++] = rvm_asm(RVM_ADDS, R0, R4, R4, 0);   /* V N */
	VMTEST_STATUS(vmcode, off, RVM_STATUS_V|RVM_STATUS_N, "ADDS STATUS");
	
	vmcode[off++] = rvm_asm(RVM_EXT, R0, XX, XX, 0);
#ifdef EXECDEBUG
	ret = rvm_cpu_exec_debug(vm, vmcode, 0);
#else
	ret = rvm_cpu_exec(vm, vmcode, 0);
#endif
	rvm_cpu_destroy(vm);
	return 0;
}
