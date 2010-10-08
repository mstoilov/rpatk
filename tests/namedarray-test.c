#include <stdio.h>
#include <stdlib.h>
#include "rvmcodegen.h"
#include "rvmnamedarray.h"
#include "rharray.h"

#include "rstring.h"
#include "rmem.h"
#include "rvmcpu.h"



static void test_swi_print_r(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	if (rvm_reg_gettype(RVM_CPUREG_PTR(cpu, ins->op2)) == RVM_DTYPE_LONG)
		fprintf(stdout, "R%d = %ld\n", ins->op2, RVM_CPUREG_GETL(cpu, ins->op2));
	else if (rvm_reg_gettype(RVM_CPUREG_PTR(cpu, ins->op2)) == RVM_DTYPE_DOUBLE)
		fprintf(stdout, "R%d = %5.2f\n", ins->op2, RVM_CPUREG_GETD(cpu, ins->op2));
	else if (rvm_reg_gettype(RVM_CPUREG_PTR(cpu, ins->op2)) == RVM_DTYPE_STRING)
		fprintf(stdout, "R%d = %s\n", ins->op2, ((rstring_t*) RVM_CPUREG_GETP(cpu, ins->op2))->s.str);
	else
		fprintf(stdout, "R%d = Unknown type\n", ins->op2);
}


static void test_swi_unref(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	if (rvm_reg_flagtst(RVM_CPUREG_PTR(cpu, ins->op2), RVM_INFOBIT_REFOBJECT))
		r_ref_dec((rref_t*)RVM_CPUREG_GETP(cpu, ins->op2));
}


static rvm_switable_t switable[] = {
		{"unref", test_swi_unref},
		{"print", test_swi_print_r},
		{NULL, NULL},
};


int main(int argc, char *argv[])
{
	rvm_reg_t ag, rh, rt;
	rharray_t *na, *nc;
	rvm_codegen_t *cg;
	rvm_cpu_t *cpu;

	cpu = rvm_cpu_create();
	rvm_cpu_switable_add(cpu, switable);
	cg = rvm_codegen_create();

	RVM_REG_SETD(&ag, 4.55);
	RVM_REG_SETSTR(&rh, r_string_create_from_ansistr("Hello World"));
	RVM_REG_SETSTR(&rt, r_string_create_from_ansistr(", there"));


	na = r_harray_create(sizeof(rvm_reg_t), NULL, NULL);
	r_harray_stradd(na, "again", &ag);
	r_harray_stradd(na, "hello", &rh);
	r_harray_stradd(na, "there", &rt);
	nc = (rharray_t*)r_ref_copy(&na->ref);

	fprintf(stdout, "lookup 'again': %ld\n", r_harray_strlookup_index(nc, "again"));
	fprintf(stdout, "lookup 'hello': %ld\n", r_harray_strlookup_index(nc, "hello"));
	fprintf(stdout, "lookup 'there': %ld\n", r_harray_strlookup_index(nc, "there"));

	rvm_codegen_addins(cg, rvm_asmp(RVM_LDRR, R0, DA, XX, &rh));
	rvm_codegen_addins(cg, rvm_asmp(RVM_LDRR, R1, DA, XX, r_harray_strlookup(nc, "again")));
	rvm_codegen_addins(cg, rvm_asmp(RVM_LDRR, R2, DA, XX, r_harray_strlookup(nc, "there")));
	rvm_codegen_addins(cg, rvm_asm(RVM_SWI, DA, R0, XX, rvm_cpu_getswi(cpu, "print")));	// print
	rvm_codegen_addins(cg, rvm_asm(RVM_SWI, DA, R1, XX, rvm_cpu_getswi(cpu, "print")));	// print
	rvm_codegen_addins(cg, rvm_asm(RVM_SWI, DA, R2, XX, rvm_cpu_getswi(cpu, "print")));	// print
	rvm_codegen_addins(cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));

	rvm_relocate(rvm_codegen_getcode(cg, 0), rvm_codegen_getcodesize(cg));
	rvm_cpu_exec_debug(cpu, rvm_codegen_getcode(cg, 0), 0);
	rvm_cpu_destroy(cpu);
	rvm_codegen_destroy(cg);


	r_ref_dec((rref_t*)na);
	r_ref_dec((rref_t*)nc);
	fprintf(stdout, "Max alloc mem: %ld\n", r_debug_get_maxmem());
	fprintf(stdout, "Leaked mem: %ld\n", r_debug_get_allocmem());
	return 0;
}
