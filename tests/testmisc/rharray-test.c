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
#include <stdlib.h>
#include "rvm/rvmcodegen.h"
#include "rvm/rvmreg.h"

#include "rlib/rstring.h"
#include "rlib/rmem.h"
#include "rvm/rvmcpu.h"



static void test_swi_print_r(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *r = RVM_CPUREG_PTR(cpu, ins->op1);

	if (rvm_reg_gettype(r) == RVM_DTYPE_SINGED)
		fprintf(stdout, "R%d = %ld\n", ins->op1, RVM_REG_GETL(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_DOUBLE)
		fprintf(stdout, "R%d = %5.2f\n", ins->op1, RVM_REG_GETD(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_STRING)
		fprintf(stdout, "R%d = %s\n", ins->op1, ((rstring_t*) RVM_REG_GETP(r))->s.str);
	else
		fprintf(stdout, "R%d = Unknown type\n", ins->op1);
}


static void test_swi_unref(rvmcpu_t *cpu, rvm_asmins_t *ins)
{

}


static rvm_switable_t switable[] = {
		{"unref", test_swi_unref},
		{"print", test_swi_print_r},
		{NULL, NULL},
};


int main(int argc, char *argv[])
{
	rvmreg_t ag, rh, rt, rh_copy, nareg;
	rharray_t *na, *nc;
	rvm_codegen_t *cg;
	rhash_node_t *node;
	rvmcpu_t *cpu;

	r_memset(&rh_copy, 0, sizeof(rh_copy));
	cpu = rvm_cpu_create_default();
	rvm_cpu_addswitable(cpu, "switable", switable);
	cg = rvm_codegen_create();

	ag = rvm_reg_create_double(4.55);
	rh = rvm_reg_create_string_ansi("Hello World");
	rt = rvm_reg_create_signed(55);

	na = r_harray_create_rvmreg();
	rvm_reg_setharray(&nareg, (robject_t*)na);
	r_harray_add_s(na, "again", &ag);
	r_harray_add_s(na, "hello", &rh);
	r_harray_add_s(na, "hellocopy", &rh_copy);
	r_harray_add_s(na, "there", NULL);
	r_harray_set(na, r_harray_lookup_s(na, "there"), &rt);
	nc = (rharray_t*)r_object_v_copy(&na->obj);

	fprintf(stdout, "lookup 'missing': %ld\n", r_harray_lookup_s(nc, "missing"));
	for (node = r_harray_nodelookup_s(nc, NULL, "again"); node; node = r_harray_nodelookup_s(nc, node, "again"))
		fprintf(stdout, "lookup 'again': %ld\n", r_hash_indexval(node));
	fprintf(stdout, "lookup 'hello': %ld\n", r_harray_lookup_s(nc, "hello"));
	fprintf(stdout, "lookup 'hellocopy': %ld\n", r_harray_lookup_s(nc, "hellocopy"));
	fprintf(stdout, "lookup 'there': %ld\n", r_harray_lookup_s(nc, "there"));

	/*
	 * Load the content of rh to R0
	 */
	rvm_codegen_addins(cg, rvm_asmp(RVM_LDRR, R0, DA, XX, &rh));

	/*
	 * Lookup the array member "again" and load the content to R1
	 */
	rvm_codegen_addins(cg, rvm_asmp(RVM_LDRR, R1, DA, XX, r_harray_get(nc, r_harray_lookup_s(nc, "again"))));

	rvm_codegen_addins(cg, rvm_asm(RVM_OPSWI(rvm_cpu_swilookup_s(cpu, "switable", "print")), R0, XX, XX, 0));	// print
	rvm_codegen_addins(cg, rvm_asm(RVM_OPSWI(rvm_cpu_swilookup_s(cpu, "switable", "print")), R1, XX, XX, 0));	// print

	/*
	 * Lookup the array member "there" and load the content to R2
	 */
	rvm_codegen_addins(cg, rvm_asmp(RVM_LDRR, R1, DA, XX, &nareg));
	rvm_codegen_addins(cg, rvm_asmp(RVM_MOV, R2, DA, XX, "where"));
	rvm_codegen_addins(cg, rvm_asm(RVM_MOV, R0, DA, XX, r_strlen("where")));
//	rvm_codegen_addins(cg, rvm_asm(RVM_KEYADD, R0, R1, R2, 0));
	rvm_codegen_addins(cg, rvm_asmd(RVM_STA, DA, R1, R0, 5.777));
	rvm_codegen_addins(cg, rvm_asm(RVM_MOV, R0, DA, XX, r_strlen("where")));
//	rvm_codegen_addins(cg, rvm_asm(RVM_KEYLOOKUP, R0, R1, R2, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_LDA, R0, R1, R0, 0));

	rvm_codegen_addins(cg, rvm_asm(RVM_MOV, R0, DA, XX, r_strlen("where")));
//	rvm_codegen_addins(cg, rvm_asm(RVM_KEYLOOKUPADD, R0, R1, R2, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_LDA, R0, R1, R0, 0));

	rvm_codegen_addins(cg, rvm_asm(RVM_OPSWI(rvm_cpu_swilookup_s(cpu, "switable", "print")), R0, XX, XX, 0));	// print
	rvm_codegen_addins(cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));

	rvm_cpu_exec_debug(cpu, rvm_codegen_getcode(cg, 0), 0);
	rvm_cpu_destroy(cpu);
	rvm_codegen_destroy(cg);


//	rvm_reg_cleanup(&rh_copy);
	r_object_destroy((robject_t*)na);
	r_object_destroy((robject_t*)nc);
	fprintf(stdout, "Max alloc mem: %ld\n", r_debug_get_maxmem());
	fprintf(stdout, "Leaked mem: %ld\n", r_debug_get_allocmem());
	return 0;
}
