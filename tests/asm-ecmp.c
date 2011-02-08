#include <stdio.h>
#include "common.h"
#include "rvmreg.h"

	
int main(int argc, char *argv[])
{
	ruint ret = 0;
	ruint off = 0;
	rvmreg_t d1 = rvm_reg_create_double(1.0);
	rvmreg_t d1s = rvm_reg_create_string_ansi("2.0");
	rvm_asmins_t vmcode[256];
	rvmcpu_t *vm = rvm_cpu_create_default();
	
	rvm_cpu_addswitable(vm, common_calltable);

	vmcode[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 8);
	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 1);
	vmcode[off++] = rvm_asm(RVM_ELSL, R2, R0, R1, 0);
	VMTEST_REG(vmcode, off, 2, 16, "ELSL");
	vmcode[off++] = rvm_asm(RVM_ELSR, R2, R0, R1, 0);
	VMTEST_REG(vmcode, off, 2, 4, "ELSR");
	vmcode[off++] = rvm_asm(RVM_EAND, R2, R0, R1, 0);
	VMTEST_REG(vmcode, off, 2, 0, "EAND");
	vmcode[off++] = rvm_asm(RVM_EORR, R2, R0, R1, 0);
	VMTEST_REG(vmcode, off, 2, 9, "EORR");
	vmcode[off++] = rvm_asm(RVM_EXOR, R2, R0, R1, 0);
	VMTEST_REG(vmcode, off, 2, 9, "EXOR");
	vmcode[off++] = rvm_asm(RVM_ENOT, R2, R0, XX, 0);
	VMTEST_REG(vmcode, off, 2, -9UL, "ENOT");
	vmcode[off++] = rvm_asm(RVM_ENOT, R2, R1, XX, 0);
	VMTEST_REG(vmcode, off, 2, -2UL, "ENOT");

	vmcode[off++] = rvm_asm(RVM_ECMP, R0, R1, XX, 0);
	VMTEST_STATUS(vmcode, off, 0, "CMP(8, 1)");

	vmcode[off++] = rvm_asm(RVM_ECMP, R1, R0, XX, 0);
	VMTEST_STATUS(vmcode, off, RVM_STATUS_N | RVM_STATUS_C, "CMP(1, 8)");

	vmcode[off++] = rvm_asm(RVM_ECMP, R1, R1, XX, 0);
	VMTEST_STATUS(vmcode, off, RVM_STATUS_Z, "CMP(1, 1)");

	vmcode[off++] = rvm_asm(RVM_ECMN, R0, R1, XX, 0);
	vmcode[off++] = rvm_asm(RVM_ECMN, R1, R0, XX, 0);
	vmcode[off++] = rvm_asm(RVM_ECMN, R1, R1, XX, 0);

	vmcode[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 8);
	vmcode[off++] = rvm_asmp(RVM_LDRR, R1, DA, XX, &d1);
	vmcode[off++] = rvm_asm(RVM_EAND, R2, R0, R1, 0);
	vmcode[off++] = rvm_asm(RVM_EORR, R2, R0, R1, 0);
	vmcode[off++] = rvm_asm(RVM_EXOR, R2, R0, R1, 0);
	vmcode[off++] = rvm_asm(RVM_ENOT, R2, R1, XX, 0);
	vmcode[off++] = rvm_asm(RVM_ELSL, R2, R0, R1, 0);
	vmcode[off++] = rvm_asm(RVM_ELSR, R2, R0, R1, 0);

	vmcode[off++] = rvm_asm(RVM_ECMP, R0, R1, XX, 0);
	VMTEST_STATUS(vmcode, off, 0, "CMP(8, 1)");

	vmcode[off++] = rvm_asm(RVM_ECMP, R1, R0, XX, 0);
	VMTEST_STATUS(vmcode, off, RVM_STATUS_N | RVM_STATUS_C, "CMP(1, 8)");

	vmcode[off++] = rvm_asm(RVM_ECMP, R1, R1, XX, 0);
	VMTEST_STATUS(vmcode, off, RVM_STATUS_Z, "CMP(1, 1)");

	vmcode[off++] = rvm_asm(RVM_ECMN, R0, R1, XX, 0);
	vmcode[off++] = rvm_asm(RVM_ECMN, R1, R0, XX, 0);
	vmcode[off++] = rvm_asm(RVM_ECMN, R1, R1, XX, 0);


	vmcode[off++] = rvm_asm(RVM_EXT, R0, XX, XX, 0);
#ifdef EXECDEBUG
	ret = rvm_cpu_exec_debug(vm, vmcode, 0);
#else
	ret = rvm_cpu_exec(vm, vmcode, 0);
#endif
	rvm_cpu_destroy(vm);
	rvm_reg_cleanup(&d1s);
	return 0;
}
