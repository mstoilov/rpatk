/*
 *  Regular Pattern Analyzer (RPA)
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

	
int main(int argc, char *argv[])
{
	unsigned int ret = 0;
	ruword s =  -2;
	ruword d =  0;
	unsigned int off = 0;
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
