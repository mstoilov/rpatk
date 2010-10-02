#include <stdio.h>
#include <stdlib.h>
#include "rvmcodegen.h"
#include "rstring.h"
#include "rmem.h"
#include "rvmcpu.h"
#include "rvmoperator.h"
#include "rvmoperatoradd.h"
#include "rvmoperatorsub.h"
#include "rvmoperatormul.h"
#include "rvmoperatordiv.h"
#include "rvmoperatorcat.h"


typedef struct rvm_testctx_s {
	rvm_opmap_t *opmap;
} rvm_testctx_t;


static void test_swi_cat(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	rvm_testctx_t *ctx = (rvm_testctx_t *)cpu->userdata;
	rvm_opmap_invoke_binary_handler(ctx->opmap, RVM_OPID_CAT, cpu, RVM_REG_PTR(cpu, R0), RVM_REG_PTR(cpu, ins->op2), RVM_REG_PTR(cpu, ins->op3));
}


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
		{"cat", test_swi_cat},
		{"print", test_swi_print_r},
		{NULL, NULL},
};


int main(int argc, char *argv[])
{
	rchar *hello = "Hello World";
	rchar *there = ", right there";

	rvm_testctx_t ctx;
	rvm_codegen_t *cg;
	rvm_cpu_t *cpu;
	rvm_opmap_t *opmap;
	ruint ntable;

	ctx.opmap = opmap = rvm_opmap_create();
	cpu = rvm_cpu_create();
	cpu->userdata = &ctx;
	cg = rvm_codegen_create();

	rvm_opmap_add_binary_operator(opmap, RVM_OPID_CAT);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAT, rvm_op_cat_string_string, RVM_DTYPE_STRING, RVM_DTYPE_STRING);

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


	rvm_codemap_invalid_add_str(cg->codemap, "str_init");
	ntable = rvm_cpu_switable_add(cpu, switable);

	rvm_codegen_addins(cg, rvm_asmp(RVM_MOV, R0, DA, XX, hello));
	rvm_codegen_addins(cg, rvm_asm(RVM_STS, R0, SP, DA, 1 + RVM_CODEGEN_FUNCINITOFFSET));
	rvm_codegen_addins(cg, rvm_asml(RVM_MOV, R0, DA, XX, r_strlen(hello)));
	rvm_codegen_addins(cg, rvm_asm(RVM_STS, R0, SP, DA, 2 + RVM_CODEGEN_FUNCINITOFFSET));
	rvm_codegen_addins(cg, rvm_asmx(RVM_BL,  DA, XX, XX, &rvm_codemap_lookup_str(cg->codemap, "str_init")->index));
	rvm_codegen_addins(cg, rvm_asmp(RVM_MOV, R7, R0, XX, 0));

	rvm_codegen_addins(cg, rvm_asmp(RVM_MOV, R0, DA, XX, there));
	rvm_codegen_addins(cg, rvm_asm(RVM_STS, R0, SP, DA, 1 + RVM_CODEGEN_FUNCINITOFFSET));
	rvm_codegen_addins(cg, rvm_asml(RVM_MOV, R0, DA, XX, r_strlen(there)));
	rvm_codegen_addins(cg, rvm_asm(RVM_STS, R0, SP, DA, 2 + RVM_CODEGEN_FUNCINITOFFSET));
	rvm_codegen_addins(cg, rvm_asmx(RVM_BL,  DA, XX, XX, &rvm_codemap_lookup_str(cg->codemap, "str_init")->index));
	rvm_codegen_addins(cg, rvm_asmp(RVM_MOV, R8, R0, XX, 0));

	rvm_codegen_addins(cg, rvm_asmp(RVM_MOV, R0, R7, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_SWI, DA, R0, R8, rvm_cpu_getswi(cpu, "cat")));
	rvm_codegen_addins(cg, rvm_asmp(RVM_MOV, R7, R0, XX, 0));

	rvm_codegen_addins(cg, rvm_asm(RVM_SWI, DA, R7, XX, rvm_cpu_getswi(cpu, "print")));	// print
	rvm_codegen_addins(cg, rvm_asm(RVM_SWI, DA, R8, XX, rvm_cpu_getswi(cpu, "print")));	// print
	rvm_codegen_addins(cg, rvm_asm(RVM_SWI, DA, R7, XX, rvm_cpu_getswi(cpu, "unref")));	// unref
	rvm_codegen_addins(cg, rvm_asm(RVM_SWI, DA, R8, XX, rvm_cpu_getswi(cpu, "unref")));	// unref
	rvm_codegen_addins(cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));

	rvm_codegen_funcstart_str(cg, "str_init", 2);
	rvm_codegen_addins(cg, rvm_asm(RVM_LDS, R0, FP, DA, 1));
	rvm_codegen_addins(cg, rvm_asm(RVM_LDS, R1, FP, DA, 2));
	rvm_codegen_addins(cg, rvm_asm(RVM_SWI, DA, R0, R1, rvm_cpu_getswi(cpu, "str_init")));
	rvm_codegen_funcend(cg);

	rvm_relocate(rvm_codegen_getcode(cg, 0), rvm_codegen_getcodesize(cg));
	rvm_cpu_exec_debug(cpu, rvm_codegen_getcode(cg, 0), 0);
	rvm_cpu_destroy(cpu);
	rvm_opmap_destroy(opmap);
	rvm_codegen_destroy(cg);

	fprintf(stdout, "Max alloc mem: %ld\n", r_debug_get_maxmem());
	fprintf(stdout, "Leaked mem: %ld\n", r_debug_get_allocmem());
	return 0;
}
