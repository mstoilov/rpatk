#include <stdio.h>
#include "common.h"
#include "rvmcodemap.h"
#include "rvmrelocmap.h"


	
int main(int argc, char *argv[])
{
	ruint ret = 0;
	ruint off = 0;
	rvm_asmins_t vmcode[256];
	rvmcpu_t *vm = rvm_cpu_create_default();
	rvm_codemap_t *codemap = rvm_codemap_create();
	rvm_relocmap_t *relocmap = rvm_relocmap_create();

	rvm_cpu_addswitable(vm, common_calltable);

	rvm_relocmap_add(relocmap, off, rvm_codemap_lookup_s(codemap, "l_main"));
	vmcode[off++]   = rvm_asmx(RVM_BX,   DA, XX, XX, rvm_codemap_label(codemap, rvm_codemap_lookup_s(codemap, "l_main")));

	/*
	 * R0 = R0 + R1
	 */
	rvm_codemap_addoffset_s(codemap, rvm_codemap_lookup_s(codemap, ".code"), "l_add2", off - 1);
	vmcode[off++] = rvm_asm(RVM_ADD, R0, R0, R1, 0);
	vmcode[off++] = rvm_asm(RVM_RET, XX, XX, XX, 0);


	/*
	 * R0 = R0 + R1 + R2
	 */
	rvm_codemap_addoffset_s(codemap, rvm_codemap_lookup_s(codemap, ".code"), "l_add3", off - 1);
	vmcode[off++] = rvm_asm(RVM_PUSHM,DA, XX, XX, BIT(R7)|BIT(R8)|BIT(SP)|BIT(LR));
	vmcode[off++] = rvm_asm(RVM_PUSH, R2, XX, XX, 0);
	rvm_relocmap_add(relocmap, off, rvm_codemap_lookup_s(codemap, "l_add2"));
	vmcode[off++] = rvm_asmx(RVM_BXL,   DA, XX, XX, rvm_codemap_label(codemap, rvm_codemap_lookup_s(codemap, "l_add2")));
	vmcode[off++] = rvm_asm(RVM_POP,  R1, XX, XX, 0);
	rvm_relocmap_add(relocmap, off, rvm_codemap_lookup_s(codemap, "l_add2"));
	vmcode[off++] = rvm_asmx(RVM_BXL,   DA, XX, XX, rvm_codemap_label(codemap, rvm_codemap_lookup_s(codemap, "l_add2")));
	vmcode[off++] = rvm_asm(RVM_POPM, DA, XX, XX, BIT(R7)|BIT(R8)|BIT(SP)|BIT(LR));
	vmcode[off++] = rvm_asm(RVM_RET,  XX, XX, XX, 0);
	/*
	 * We could directly restore the LR in the PC, so we will not need the RET instruction after POPM
	 *
	 * vmcode[off++] = rvm_asm(RVM_POPM, DA, XX, XX, BIT(R7)|BIT(R8)|BIT(SP)|BIT(PC));
	 *
	 */

	rvm_codemap_addoffset_s(codemap, rvm_codemap_lookup_s(codemap, ".code"), "l_main", off - 1);
	vmcode[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 1);
	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 2);
	rvm_relocmap_add(relocmap, off, rvm_codemap_lookup_s(codemap, "l_add2"));
	vmcode[off++] = rvm_asmx(RVM_BXL,  DA, XX, XX, rvm_codemap_label(codemap, rvm_codemap_lookup_s(codemap, "l_add2")));
	VMTEST_REG(vmcode, off, 0, 3, "BL/RET");
	vmcode[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 1);
	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 2);
	vmcode[off++] = rvm_asm(RVM_MOV, R2, DA, XX, 4);
	rvm_relocmap_add(relocmap, off, rvm_codemap_lookup_s(codemap, "l_add3"));
	vmcode[off++] = rvm_asmx(RVM_BXL,  DA, XX, XX, rvm_codemap_label(codemap, rvm_codemap_lookup_s(codemap, "l_add3")));
	VMTEST_REG(vmcode, off, 0, 7, "BL/RET");
	vmcode[off++] = rvm_asm(RVM_EXT, R0, XX, XX, 0);

	rvm_relocmap_relocate(relocmap, codemap, vmcode);

#ifdef EXECDEBUG
	ret = rvm_cpu_exec_debug(vm, vmcode, 0);
#else
	ret = rvm_cpu_exec(vm, vmcode, 0);
#endif
	rvm_cpu_destroy(vm);
	rvm_codemap_destroy(codemap);
	rvm_relocmap_destroy(relocmap);
	return 0;
}
