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
#include "rlib/rstring.h"
#include "rvm/rvmreg.h"
#include "rvm/rvmcpu.h"
#include "common.h"

static void test_swi_print_r(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	ruint8 R = ins->op1;
	rvmreg_t *r = RVM_CPUREG_PTR(cpu, R);

	if (rvm_reg_gettype(r) == RVM_DTYPE_WORD)
		fprintf(stdout, "R%d = %ld\n", R, RVM_REG_GETL(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_LONG)
		fprintf(stdout, "R%d = %ld\n", R, RVM_REG_GETL(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_DOUBLE)
		fprintf(stdout, "R%d = %5.2f\n", R, RVM_REG_GETD(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_STRING)
		fprintf(stdout, "R%d = %s\n", R, ((rstring_t*) RVM_REG_GETP(r))->s.str);
	else
		fprintf(stdout, "R%d = Unknown type\n", R);
}


static rvm_switable_t switable[] = {
		{"print", test_swi_print_r},
		{NULL, NULL},
};

	
int main(int argc, char *argv[])
{
	ruinteger ret = 0;
	ruinteger off = 0;
	rvmreg_t d1 = rvm_reg_create_double(1.0);
	rvmreg_t d2 = rvm_reg_create_double(-1.0);
	rvmreg_t ds = rvm_reg_create_string_ansi("-3.5785");
	rvm_asmins_t vmcode[256];
	rvmcpu_t *vm = rvm_cpu_create_default();
	
	rvm_cpu_addswitable(vm, "common_table", common_calltable);
	rvm_cpu_addswitable(vm, "switable", switable);
	vmcode[off++] = rvm_asmp(RVM_LDRR, R0, DA, XX, &d1);
	vmcode[off++] = rvm_asm(RVM_TYPE, R1, R0, XX, 0);
	vmcode[off++] = rvm_asm(RVM_OPSWI(rvm_cpu_swilookup_s(vm, "switable", "print")), R1, XX, XX, 0);
	vmcode[off++] = rvm_asm(RVM_CAST, R0, R0, DA, RVM_DTYPE_UNSIGNED);
	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 2);
	vmcode[off++] = rvm_asm(RVM_ADD, R0, R1, R0, 0);
	vmcode[off++] = rvm_asm(RVM_OPSWI(rvm_cpu_swilookup_s(vm, "switable", "print")), R0, XX, XX, 0);
	VMTEST_REG(vmcode, off, 0, 3, "CAST");
	VMTEST_STATUS(vmcode, off, 0, "CAST STATUS");

	vmcode[off++] = rvm_asmp(RVM_LDRR, R0, DA, XX, &d2);
	vmcode[off++] = rvm_asm(RVM_TYPE, R1, R0, XX, 0);
	vmcode[off++] = rvm_asm(RVM_OPSWI(rvm_cpu_swilookup_s(vm, "switable", "print")), R1, XX, XX, 0);
	vmcode[off++] = rvm_asm(RVM_CAST, R0, R0, DA, RVM_DTYPE_LONG);
	vmcode[off++] = rvm_asm(RVM_OPSWI(rvm_cpu_swilookup_s(vm, "switable", "print")), R0, XX, XX, 0);
	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 2);
	vmcode[off++] = rvm_asm(RVM_ADD, R0, R1, R0, 0);
	vmcode[off++] = rvm_asm(RVM_OPSWI(rvm_cpu_swilookup_s(vm, "switable", "print")), R0, XX, XX, 0);
	VMTEST_REG(vmcode, off, 0, 1, "CAST");
	VMTEST_STATUS(vmcode, off, 0, "CAST STATUS");

	vmcode[off++] = rvm_asmp(RVM_LDRR, R1, DA, XX, &ds);
	vmcode[off++] = rvm_asmp(RVM_PRN, R1, XX, XX, 0);
	vmcode[off++] = rvm_asm(RVM_CAST, R0, R1, DA, RVM_DTYPE_UNSIGNED);
	vmcode[off++] = rvm_asmp(RVM_PRN, R0, XX, XX, 0);
	vmcode[off++] = rvm_asm(RVM_CAST, R0, R1, DA, RVM_DTYPE_LONG);
	vmcode[off++] = rvm_asmp(RVM_PRN, R0, XX, XX, 0);
	vmcode[off++] = rvm_asm(RVM_CAST, R0, R1, DA, RVM_DTYPE_DOUBLE);
	vmcode[off++] = rvm_asmp(RVM_PRN, R0, XX, XX, 0);

	vmcode[off++] = rvm_asm(RVM_EXT, R0, XX, XX, 0);
#ifdef EXECDEBUG
	ret = rvm_cpu_exec_debug(vm, vmcode, 0);
#else
	ret = rvm_cpu_exec(vm, vmcode, 0);
#endif
	rvm_cpu_destroy(vm);
	return 0;
}
