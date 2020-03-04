/*
 *  Regular Pattern Analyzer Toolkit (RPA/Tk)
 *  Copyright (c) 2009-2012 Martin Stoilov
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
#include <stdlib.h>
#include "common.h"


static void rvm_callback_two(rvmcpu_t *vm, rvm_asmins_t *ins)
{
	fprintf(stdout, "%s\n", __FUNCTION__);
}


static rvm_switable_t calltable[] = {
	{"rvm_callback_two", rvm_callback_two},
	{NULL, NULL},
};

	
int main(int argc, char *argv[])
{
	int ret = 0;
	long iter = 1;
	unsigned int off = 0;
	rvm_asmins_t vmcode[256];
	rvmcpu_t *vm = rvm_cpu_create_default();

	if (argc > 1) {
		iter = atol(argv[1]);
	}

	rvm_cpu_addswitable(vm, "calltable", calltable);
	rvm_cpu_addswitable(vm, "common_table", common_calltable);

	vmcode[off++] = rvm_asm(RVM_MOV, R4, DA, XX, 0);
	vmcode[off++] = rvm_asm(RVM_MOV, R5, DA, XX, iter);
	vmcode[off++] = rvm_asmd(RVM_MUL, R4, R4, DA, 2);
	vmcode[off++] = rvm_asmd(RVM_DIV, R4, R4, DA, 2);
	vmcode[off++] = rvm_asm(RVM_ADD, R4, R4, DA, 2);
	vmcode[off++] = rvm_asm(RVM_SUB, R4, R4, DA, 1);
	vmcode[off++] = rvm_asm(RVM_PUSH, R4, XX, XX, 0);
	vmcode[off++] = rvm_asm(RVM_POP, R4, XX, XX, 0);
	vmcode[off++] = rvm_asm(RVM_CMP, R4, R5, XX, 0);
	vmcode[off++] = rvm_asml(RVM_BLES, DA, XX, XX, -7);
	vmcode[off++] = rvm_asm(RVM_MOV, R0, R4, XX, 0);
	vmcode[off++] = rvm_asm(RVM_PRN, R0, XX, XX, 0);
	vmcode[off++] = rvm_asm(RVM_EXT, R0, XX, XX, 0);

	ret = rvm_cpu_exec(vm, vmcode, 0);
	fprintf(stdout, "R0 = %ld (%ld operations)\n", (unsigned long) RVM_CPUREG_GETU(vm, R0), (unsigned long)RVM_CPUREG_GETU(vm, R5));
	rvm_cpu_destroy(vm);
	return ret;
}
