/*
 *  Regular Pattern Analyzer Toolkit (RPA/Tk)
 *  Copyright (c) 2009-2010 Martin Stoilov
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Martin Stoilov <martin@rpasearch.com>
 */

#include <stdio.h>
#include "common.h"
#include "rvm/rvmcodemap.h"
#include "rvm/rvmrelocmap.h"


	
int main(int argc, char *argv[])
{
	unsigned int ret = 0;
	unsigned int off = 0;
	rvm_asmins_t vmcode[256];
	rvm_codelabel_t *err = NULL;
	rvmcpu_t *vm = rvm_cpu_create_default();
	rvm_codemap_t *codemap = rvm_codemap_create();
	rvm_relocmap_t *relocmap = rvm_relocmap_create();

	rvm_cpu_addswitable(vm, "common_table", common_calltable);

	rvm_relocmap_add(relocmap, RVM_RELOC_CODE, RVM_RELOC_BRANCH, off, rvm_codemap_lookupadd_s(codemap, "l_main"));
	vmcode[off++]   = rvm_asm(RVM_B,   DA, XX, XX, 0);

	/*
	 * R0 = R0 + R1
	 */
	rvm_codemap_addoffset_s(codemap, "l_add2", rvm_codemap_lookupadd_s(codemap, ".code"), RVM_CODE2BYTE_OFFSET(off));
	vmcode[off++] = rvm_asm(RVM_ADD, R0, R0, R1, 0);
	vmcode[off++] = rvm_asm(RVM_RET, XX, XX, XX, 0);


	/*
	 * R0 = R0 + R1 + R2
	 */
	rvm_codemap_addoffset_s(codemap, "l_add3", rvm_codemap_lookupadd_s(codemap, ".code"), RVM_CODE2BYTE_OFFSET(off));
	vmcode[off++] = rvm_asm(RVM_PUSHM,DA, XX, XX, BIT(R7)|BIT(R8)|BIT(SP)|BIT(LR));
	vmcode[off++] = rvm_asm(RVM_PUSH, R2, XX, XX, 0);
	rvm_relocmap_add(relocmap, RVM_RELOC_CODE, RVM_RELOC_JUMP, off, rvm_codemap_lookupadd_s(codemap, "l_add2"));
	vmcode[off++] = rvm_asm(RVM_BXL,   DA, XX, XX, 0);
	vmcode[off++] = rvm_asm(RVM_POP,  R1, XX, XX, 0);
	rvm_relocmap_add(relocmap, RVM_RELOC_CODE, RVM_RELOC_JUMP, off, rvm_codemap_lookupadd_s(codemap, "l_add2"));
	vmcode[off++] = rvm_asm(RVM_BXL,   DA, XX, XX, 0);
	vmcode[off++] = rvm_asm(RVM_POPM, DA, XX, XX, BIT(R7)|BIT(R8)|BIT(SP)|BIT(LR));
	vmcode[off++] = rvm_asm(RVM_RET,  XX, XX, XX, 0);
	/*
	 * We could directly restore the LR in the PC, so we will not need the RET instruction after POPM
	 *
	 * vmcode[off++] = rvm_asm(RVM_POPM, DA, XX, XX, BIT(R7)|BIT(R8)|BIT(SP)|BIT(PC));
	 *
	 */

	rvm_codemap_addoffset_s(codemap, "l_main", rvm_codemap_lookupadd_s(codemap, ".code"), RVM_CODE2BYTE_OFFSET(off));
	vmcode[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 1);
	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 2);
	rvm_relocmap_add(relocmap, RVM_RELOC_CODE, RVM_RELOC_JUMP, off, rvm_codemap_lookupadd_s(codemap, "l_add2"));
	vmcode[off++] = rvm_asm(RVM_BXL,  DA, XX, XX, 0);
	VMTEST_REG(vmcode, off, 0, 3, "BL/RET");
	vmcode[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 1);
	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 2);
	vmcode[off++] = rvm_asm(RVM_MOV, R2, DA, XX, 4);
	rvm_relocmap_add(relocmap, RVM_RELOC_CODE, RVM_RELOC_JUMP, off, rvm_codemap_lookupadd_s(codemap, "l_add3"));
	vmcode[off++] = rvm_asm(RVM_BXL,  DA, XX, XX, 0);
	VMTEST_REG(vmcode, off, 0, 7, "BL/RET");
	vmcode[off++] = rvm_asm(RVM_EXT, R0, XX, XX, 0);

	rvm_codemap_addpointer_s(codemap, ".code", &vmcode[0]);
	if (rvm_relocmap_relocate(relocmap, codemap, vmcode, &err) < 0) {
		r_printf("Unresolved symbol: %s\n", err->name->str);
		goto end;
	}

#ifdef EXECDEBUG
	ret = rvm_cpu_exec_debug(vm, vmcode, 0);
#else
	ret = rvm_cpu_exec(vm, vmcode, 0);
#endif

end:
	rvm_cpu_destroy(vm);
	rvm_codemap_destroy(codemap);
	rvm_relocmap_destroy(relocmap);
	return 0;
}
