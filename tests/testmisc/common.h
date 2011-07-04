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

#include "rvm/rvmcpu.h"
#include <stdio.h>

#define EXECDEBUG 1


#define VMTEST_REG(code, index, reg, val, msg) \
	do { code[index++] = rvm_asm(RVM_MOV, R8, DA, XX, reg); \
		 code[index++] = rvm_asm(RVM_MOV, R9, DA, XX, val); \
		 code[index++] = rvm_asm(RVM_PRN, reg, XX, XX, 0); \
		 code[index++] = rvm_asmp(RVM_MOV, R10, DA, XX, msg); \
		 code[index++] = rvm_asm(RVM_OPSWI(rvm_cpu_swilookup_s(vm, "common_table", "rvm_vmtest_check_reg")), XX, XX, R0, 0); \
		 code[index++] = rvm_asm(RVM_NOP, XX, XX, XX, 0); } while (0)

#define VMTEST_REGL(code, index, reg, val, msg) \
	do { code[index++] = rvm_asm(RVM_MOV, R8, DA, XX, reg); \
		 code[index++] = rvm_asml(RVM_MOV, R9, DA, XX, val); \
		 code[index++] = rvm_asm(RVM_PRN, reg, XX, XX, 0); \
		 code[index++] = rvm_asmp(RVM_MOV, R10, DA, XX, msg); \
		 code[index++] = rvm_asm(RVM_OPSWI(rvm_cpu_swilookup_s(vm, "common_table", "rvm_vmtest_check_reg")), XX, XX, R0, 0); \
		 code[index++] = rvm_asm(RVM_NOP, XX, XX, XX, 0); } while (0)

#define VMTEST_REGD(code, index, reg, val, msg) \
	do { code[index++] = rvm_asm(RVM_MOV, R8, DA, XX, reg); \
		 code[index++] = rvm_asmd(RVM_MOV, R9, DA, XX, val); \
		 code[index++] = rvm_asm(RVM_PRN, reg, XX, XX, 0); \
		 code[index++] = rvm_asmp(RVM_MOV, R10, DA, XX, msg); \
		 code[index++] = rvm_asm(RVM_OPSWI(rvm_cpu_swilookup_s(vm, "common_table", "rvm_vmtest_check_reg")), XX, XX, R0, 0); \
		 code[index++] = rvm_asm(RVM_NOP, XX, XX, XX, 0); } while (0)


#define VMTEST_REGP(code, index, reg, pval, msg) \
	do { code[index++] = rvm_asm(RVM_MOV, R8, DA, XX, reg); \
		 code[index++] = rvm_asmp(RVM_LDRR, R9, DA, XX, pval); \
		 code[index++] = rvm_asm(RVM_PRN, R9, XX, XX, 0); \
		 code[index++] = rvm_asmp(RVM_MOV, R10, DA, XX, msg); \
		 code[index++] = rvm_asm(RVM_OPSWI(rvm_cpu_swilookup_s(vm, "common_table", "rvm_vmtest_check_reg")), XX, XX, R0, 0); \
		 code[index++] = rvm_asm(RVM_NOP, XX, XX, XX, 0); } while (0)


#define VMTEST_STATUS(code, index, val, msg) \
	do { code[index++] = rvm_asm(RVM_MOV, R9, DA, XX, val); \
		 code[index++] = rvm_asmp(RVM_MOV, R10, DA, XX, msg); \
		 code[index++] = rvm_asm(RVM_OPSWI(rvm_cpu_swilookup_s(vm, "common_table", "rvm_vmtest_check_status")), XX, XX, R0, 0); \
		 code[index++] = rvm_asm(RVM_NOP, XX, XX, XX, 0); } while (0)


/*
 * R10 - (const char*) msg
 * R9 - argument (to compare)
 * R8 - (0 : 7) Register to check
 */
static void rvm_vmtest_check_reg(rvmcpu_t *vm, rvm_asmins_t *ins)
{
	fprintf(stdout, "%s: %s\n", (const char*)RVM_CPUREG_GETU(vm, 10), RVM_CPUREG_GETU(vm, RVM_CPUREG_GETU(vm, 8)) == RVM_CPUREG_GETU(vm, 9) ? "PASSED" : "FAILED");
}


/*
 * R10 - (const char*) msg
 * R9 - argument (to compare the status register)
 */
static void rvm_vmtest_check_status(rvmcpu_t *vm, rvm_asmins_t *ins)
{
	fprintf(stdout, "%s: %s\n", (const char*)RVM_CPUREG_GETU(vm, 10), vm->status == RVM_CPUREG_GETU(vm, 9) ? "PASSED" : "FAILED");
}


static rvm_switable_t common_calltable[] = {
	{"rvm_vmtest_check_reg", rvm_vmtest_check_reg},
	{"rvm_vmtest_check_status", rvm_vmtest_check_status},
	{NULL, NULL},
};
