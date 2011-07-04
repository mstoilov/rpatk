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
	ruinteger ret = 0;
	ruinteger off = 0;
	rvm_asmins_t vmcode[256];
	rvmcpu_t *vm = rvm_cpu_create_default();
	
	rvm_cpu_addswitable(vm, "common_table", common_calltable);
	vmcode[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 0);
	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 0);
	vmcode[off++] = rvm_asm(RVM_MOV, R2, DA, XX, 1);
	vmcode[off++] = rvm_asm(RVM_MOV, R3, DA, XX, 2);
	vmcode[off++] = rvm_asm(RVM_MOV, R4, DA, XX, -3);

	vmcode[off++] = rvm_asm(RVM_ORR, R0, R1, R1, 0);
	VMTEST_REG(vmcode, off, 0, 0, "ORR (0, 0)");
	vmcode[off++] = rvm_asm(RVM_ORR, R0, R1, R2, 0);
	VMTEST_REG(vmcode, off, 0, 1, "ORR (0, 1)");
	vmcode[off++] = rvm_asm(RVM_ORR, R0, R2, R3, 0);
	VMTEST_REG(vmcode, off, 0, 3, "ORR (1, 2)");


	vmcode[off++] = rvm_asm(RVM_XOR, R0, R1, R1, 0);
	VMTEST_REG(vmcode, off, 0, 0, "EOR (0, 0)");
	vmcode[off++] = rvm_asm(RVM_XOR, R0, R1, R2, 0);
	VMTEST_REG(vmcode, off, 0, 1, "EOR (0, 1)");
	vmcode[off++] = rvm_asm(RVM_XOR, R0, R1, R3, 0);
	VMTEST_REG(vmcode, off, 0, 2, "EOR (0, 2)");
	vmcode[off++] = rvm_asm(RVM_XOR, R0, R2, R4, 0);
	VMTEST_REG(vmcode, off, 0, -4, "EOR (1, -3)");

	vmcode[off++] = rvm_asm(RVM_AND, R0, R1, R1, 0);
	VMTEST_REG(vmcode, off, 0, 0, "AND (0, 0)");
	vmcode[off++] = rvm_asm(RVM_AND, R0, R1, R2, 0);
	VMTEST_REG(vmcode, off, 0, 0, "AND (0, 1)");
	vmcode[off++] = rvm_asm(RVM_AND, R0, R1, R3, 0);
	VMTEST_REG(vmcode, off, 0, 0, "AND (1, 2)");
	vmcode[off++] = rvm_asm(RVM_AND, R0, R2, R4, 0);
	VMTEST_REG(vmcode, off, 0, 1, "AND (1, -3)");
	vmcode[off++] = rvm_asm(RVM_NOT, R0, R2, XX, 0);
	VMTEST_REG(vmcode, off, 0, -2, "NOT 1");


	vmcode[off++] = rvm_asm(RVM_EXT, R0, XX, XX, 0);
#ifdef EXECDEBUG
	ret = rvm_cpu_exec_debug(vm, vmcode, 0);
#else
	ret = rvm_cpu_exec(vm, vmcode, 0);
#endif
	
	rvm_cpu_destroy(vm);
	return 0;
}
