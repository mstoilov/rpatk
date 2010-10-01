#include <stdio.h>
#include <stdlib.h>
#include "rstring.h"
#include "rmem.h"
#include "rvmcpu.h"
#include "rvmoperator.h"
#include "rvmoperatoradd.h"
#include "rvmoperatorsub.h"
#include "rvmoperatormul.h"
#include "rvmoperatordiv.h"


typedef struct rvm_testctx_s {
	rvm_opmap_t *opmap;
} rvm_testctx_t;


static void test_swi_print_r(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	if (rvm_reg_gettype(RVM_REG_PTR(cpu, ins->op2)) == RVM_DTYPE_LONG)
		fprintf(stdout, "R%d = %ld\n", ins->op2, RVM_GET_REGL(cpu, ins->op2));
	else if (rvm_reg_gettype(RVM_REG_PTR(cpu, ins->op2)) == RVM_DTYPE_DOUBLE)
		fprintf(stdout, "R%d = %5.2f\n", ins->op2, RVM_GET_REGD(cpu, ins->op2));
	else if (rvm_reg_gettype(RVM_REG_PTR(cpu, ins->op2)) == RVM_DTYPE_STRING)
		fprintf(stdout, "R%d = %s\n", ins->op2, ((rstring_t*) RVM_GET_REGP(cpu, ins->op2))->s.str);
	else
		fprintf(stdout, "Unknown type\n");
}


static void test_swi_strinit(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	char *p = (char*) RVM_GET_REGP(cpu, ins->op2);
	long size = RVM_GET_REGL(cpu, ins->op3);
	rstr_t str = {p, size};
	RVM_SET_REGP(cpu, R0, (void*)r_string_create_from_rstr(&str));
	rvm_reg_flagset(RVM_REG_PTR(cpu, R0), RVM_INFOBIT_REFOBJECT);
	rvm_reg_settype(RVM_REG_PTR(cpu, R0), RVM_DTYPE_STRING);
}


static void test_swi_unref(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	if (rvm_reg_flagtst(RVM_REG_PTR(cpu, ins->op2), RVM_INFOBIT_REFOBJECT))
		r_ref_dec((rref_t*)RVM_GET_REGP(cpu, ins->op2));
}


static rvm_switable_t switable[] = {
		{"str_init", test_swi_strinit},
		{"unref", test_swi_unref},
		{"print", test_swi_print_r},
		{NULL, NULL},
};


int main(int argc, char *argv[])
{
	rchar *hello = "Hello, World there";
	rvm_testctx_t ctx;
	rvm_cpu_t *cpu;
	rvm_opmap_t *opmap;
	rvm_asmins_t code[1024];
	ruint off = 0;
	ruint ntable;

	ctx.opmap = opmap = rvm_opmap_create();
	cpu = rvm_cpu_create();
	cpu->userdata = &ctx;

	rvm_opmap_add_binary_operator(opmap, RVM_OPID_ADD);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rvm_op_add_double_double, RVM_DTYPE_DOUBLE, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rvm_op_add_long_double, RVM_DTYPE_LONG, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rvm_op_add_double_long, RVM_DTYPE_DOUBLE, RVM_DTYPE_LONG);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rvm_op_add_long_long, RVM_DTYPE_LONG, RVM_DTYPE_LONG);

	rvm_opmap_add_binary_operator(opmap, RVM_OPID_SUB);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_SUB, rvm_op_sub_double_double, RVM_DTYPE_DOUBLE, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_SUB, rvm_op_sub_long_double, RVM_DTYPE_LONG, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_SUB, rvm_op_sub_double_long, RVM_DTYPE_DOUBLE, RVM_DTYPE_LONG);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_SUB, rvm_op_sub_long_long, RVM_DTYPE_LONG, RVM_DTYPE_LONG);

	rvm_opmap_add_binary_operator(opmap, RVM_OPID_MUL);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_MUL, rvm_op_mul_double_double, RVM_DTYPE_DOUBLE, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_MUL, rvm_op_mul_long_double, RVM_DTYPE_LONG, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_MUL, rvm_op_mul_double_long, RVM_DTYPE_DOUBLE, RVM_DTYPE_LONG);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_MUL, rvm_op_mul_long_long, RVM_DTYPE_LONG, RVM_DTYPE_LONG);

	rvm_opmap_add_binary_operator(opmap, RVM_OPID_DIV);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_DIV, rvm_op_div_double_double, RVM_DTYPE_DOUBLE, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_DIV, rvm_op_div_long_double, RVM_DTYPE_LONG, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_DIV, rvm_op_div_double_long, RVM_DTYPE_DOUBLE, RVM_DTYPE_LONG);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_DIV, rvm_op_div_long_long, RVM_DTYPE_LONG, RVM_DTYPE_LONG);

	ntable = rvm_cpu_switable_add(cpu, switable);
	code[off++] = rvm_asmp(RVM_MOV, R1, DA, XX, hello);
	code[off++] = rvm_asml(RVM_MOV, R2, DA, XX, r_strlen(hello));
	code[off++] = rvm_asm(RVM_SWI, DA, R1, R2, rvm_cpu_getswi(cpu, "str_init"));
	code[off++] = rvm_asm(RVM_SWI, DA, R0, XX, rvm_cpu_getswi(cpu, "print"));	// print
	code[off++] = rvm_asm(RVM_SWI, DA, R0, XX, rvm_cpu_getswi(cpu, "unref"));	// unref

	code[off++] = rvm_asm(RVM_EXT, XX, XX, XX, 0);



	rvm_cpu_exec_debug(cpu, code, 0);
	rvm_cpu_destroy(cpu);
	rvm_opmap_destroy(opmap);

	fprintf(stdout, "Max alloc mem: %ld\n", r_debug_get_maxmem());
	fprintf(stdout, "Leaked mem: %ld\n", r_debug_get_allocmem());
	return 0;
}
