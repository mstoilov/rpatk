#include "rvmoperatormul.h"
#include "rvmreg.h"


static void rvm_op_mul_unsigned(rvmcpu_t *cpu, rvmreg_t *res, rword op1, rword op2)
{
	rword r;

	r = op1 * op2;
	RVM_REG_SETU(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_UNSIGNED);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_C, op1 && (r / op1) != op2);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, r & RVM_SIGN_BIT);
}


static void rvm_op_mul_long(rvmcpu_t *cpu, rvmreg_t *res, rlong op1, rlong op2)
{
	rlong r;

	r = op1 * op2;
	RVM_REG_SETL(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_LONG);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, r & RVM_SIGN_BIT);
}


static void rvm_op_mul_double(rvmcpu_t *cpu, rvmreg_t *res, rdouble op1, rdouble op2)
{
	rdouble r;

	r = op1 * op2;
	RVM_REG_SETD(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_DOUBLE);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, r < 0.0);
}


static void rvm_op_mul_unsigned_unsigned(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvm_op_mul_unsigned(cpu, res, RVM_REG_GETU(arg1), RVM_REG_GETU(arg2));
}


static void rvm_op_mul_long_long(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvm_op_mul_long(cpu, res, RVM_REG_GETL(arg1), RVM_REG_GETL(arg2));
}


static void rvm_op_mul_double_long(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvm_op_mul_double(cpu, res, RVM_REG_GETD(arg1), RVM_REG_GETL(arg2));
}


void rvm_op_mul_long_double(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvm_op_mul_double(cpu, res, RVM_REG_GETL(arg1), RVM_REG_GETD(arg2));
}


static void rvm_op_mul_double_double(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvm_op_mul_double(cpu, res, RVM_REG_GETD(arg1), RVM_REG_GETD(arg2));
}


static void rvm_op_mul_string_double(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvmreg_t s;

	if (rvm_reg_str2double(&s, arg1) < 0)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	rvm_op_mul_double(cpu, res, RVM_REG_GETD(&s), RVM_REG_GETD(arg2));
}


static void rvm_op_mul_string_long(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvmreg_t s;

	if (rvm_reg_str2num(&s, arg1) < 0)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);

	if (rvm_reg_gettype(&s) == RVM_DTYPE_DOUBLE) {
		rvm_op_mul_double(cpu, res, RVM_REG_GETD(&s), RVM_REG_GETL(arg2));
	} else {
		rvm_op_mul_long(cpu, res, RVM_REG_GETL(&s), RVM_REG_GETL(arg2));
	}
}


static void rvm_op_mul_double_string(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvmreg_t s;

	if (rvm_reg_str2double(&s, arg2) < 0)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	rvm_op_mul_double(cpu, res, RVM_REG_GETD(arg1), RVM_REG_GETD(&s));
}


static void rvm_op_mul_long_string(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvmreg_t s;

	if (rvm_reg_str2num(&s, arg2) < 0)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	if (rvm_reg_gettype(&s) == RVM_DTYPE_DOUBLE) {
		rvm_op_mul_double(cpu, res, RVM_REG_GETL(arg1), RVM_REG_GETD(&s));
	} else {
		rvm_op_mul_long(cpu, res, RVM_REG_GETL(arg1), RVM_REG_GETL(&s));
	}
}


void rvm_op_mul_init(rvm_opmap_t *opmap)
{
	rvm_opmap_add_binary_operator(opmap, RVM_OPID_MUL);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_MUL, rvm_op_mul_unsigned_unsigned, RVM_DTYPE_UNSIGNED, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_MUL, rvm_op_mul_double_double, RVM_DTYPE_DOUBLE, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_MUL, rvm_op_mul_long_long, RVM_DTYPE_LONG, RVM_DTYPE_LONG);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_MUL, rvm_op_mul_long_long, RVM_DTYPE_UNSIGNED, RVM_DTYPE_LONG);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_MUL, rvm_op_mul_long_long, RVM_DTYPE_LONG, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_MUL, rvm_op_mul_long_double, RVM_DTYPE_LONG, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_MUL, rvm_op_mul_long_double, RVM_DTYPE_UNSIGNED, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_MUL, rvm_op_mul_double_long, RVM_DTYPE_DOUBLE, RVM_DTYPE_LONG);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_MUL, rvm_op_mul_double_long, RVM_DTYPE_DOUBLE, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_MUL, rvm_op_mul_string_long, RVM_DTYPE_STRING, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_MUL, rvm_op_mul_string_long, RVM_DTYPE_STRING, RVM_DTYPE_LONG);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_MUL, rvm_op_mul_string_double, RVM_DTYPE_STRING, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_MUL, rvm_op_mul_long_string, RVM_DTYPE_UNSIGNED, RVM_DTYPE_STRING);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_MUL, rvm_op_mul_long_string, RVM_DTYPE_LONG, RVM_DTYPE_STRING);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_MUL, rvm_op_mul_double_string, RVM_DTYPE_DOUBLE, RVM_DTYPE_STRING);
}
