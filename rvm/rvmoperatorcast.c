#include "rvmoperator.h"
#include "rvmreg.h"


void rvm_op_cast_static_static(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	*res = *arg1;
}


void rvm_op_cast_double_unsigned(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rword r = (rword)RVM_REG_GETD(arg1);

	RVM_REG_CLEAR(res);
	RVM_REG_SETTYPE(res, RVM_DTYPE_UNSIGNED);
	RVM_REG_SETL(res, r);
}

void rvm_op_cast_double_long(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rlong r = (rlong)RVM_REG_GETD(arg1);

	RVM_REG_CLEAR(res);
	RVM_REG_SETTYPE(res, RVM_DTYPE_LONG);
	RVM_REG_SETL(res, r);
}


void rvm_op_cast_long_unsigned(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rword r = (rword)RVM_REG_GETL(arg1);

	RVM_REG_CLEAR(res);
	RVM_REG_SETTYPE(res, RVM_DTYPE_UNSIGNED);
	RVM_REG_SETU(res, r);
}


void rvm_op_cast_long_double(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rdouble r = (rdouble)RVM_REG_GETL(arg1);

	RVM_REG_CLEAR(res);
	RVM_REG_SETTYPE(res, RVM_DTYPE_DOUBLE);
	RVM_REG_SETL(res, r);
}


void rvm_op_cast_long_boolean(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rword r = (rword)RVM_REG_GETL(arg1);

	RVM_REG_CLEAR(res);
	RVM_REG_SETTYPE(res, RVM_DTYPE_BOOLEAN);
	RVM_REG_SETL(res, r ? 1 : 0);
}


void rvm_op_cast_double_boolean(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rword r = (rword)RVM_REG_GETD(arg1);

	RVM_REG_CLEAR(res);
	RVM_REG_SETTYPE(res, RVM_DTYPE_BOOLEAN);
	RVM_REG_SETL(res, r ? 1 : 0);
}


void rvm_op_cast_pointer_boolean(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rpointer r = (rpointer)RVM_REG_GETP(arg1);

	RVM_REG_CLEAR(res);
	RVM_REG_SETTYPE(res, RVM_DTYPE_BOOLEAN);
	RVM_REG_SETL(res, r ? 1 : 0);
}


void rvm_op_cast_unsigned_long(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rlong r = (rlong)RVM_REG_GETU(arg1);

	RVM_REG_CLEAR(res);
	RVM_REG_SETTYPE(res, RVM_DTYPE_LONG);
	RVM_REG_SETL(res, r);
}


void rvm_op_cast_unsigned_double(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rdouble r = (rdouble)RVM_REG_GETU(arg1);

	RVM_REG_CLEAR(res);
	RVM_REG_SETTYPE(res, RVM_DTYPE_DOUBLE);
	RVM_REG_SETD(res, r);
}


void rvm_op_cast_string_unsigned(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvmreg_t s;

	if (rvm_reg_str2long(&s, arg1) < 0)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	rvm_reg_setunsigned(res, RVM_REG_GETL(&s));
}


void rvm_op_cast_string_long(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvmreg_t s;

	if (rvm_reg_str2long(&s, arg1) < 0)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	rvm_reg_setlong(res, RVM_REG_GETL(&s));
}


void rvm_op_cast_string_double(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvmreg_t s;

	if (rvm_reg_str2double(&s, arg1) < 0)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	rvm_reg_setdouble(res, RVM_REG_GETD(&s));
}


void rvm_op_cast_init(rvm_opmap_t *opmap)
{
	rvm_opmap_add_binary_operator(opmap, RVM_OPID_CAST);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_static_static, RVM_DTYPE_UNSIGNED, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_static_static, RVM_DTYPE_DOUBLE, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_static_static, RVM_DTYPE_LONG, RVM_DTYPE_LONG);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_static_static, RVM_DTYPE_STRING, RVM_DTYPE_STRING);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_static_static, RVM_DTYPE_BOOLEAN, RVM_DTYPE_BOOLEAN);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_long_boolean, RVM_DTYPE_LONG, RVM_DTYPE_BOOLEAN);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_long_boolean, RVM_DTYPE_UNSIGNED, RVM_DTYPE_BOOLEAN);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_double_boolean, RVM_DTYPE_DOUBLE, RVM_DTYPE_BOOLEAN);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_pointer_boolean, RVM_DTYPE_STRING, RVM_DTYPE_BOOLEAN);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_pointer_boolean, RVM_DTYPE_ARRAY, RVM_DTYPE_BOOLEAN);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_pointer_boolean, RVM_DTYPE_HARRAY, RVM_DTYPE_BOOLEAN);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_unsigned_long, RVM_DTYPE_BOOLEAN, RVM_DTYPE_LONG);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_unsigned_double, RVM_DTYPE_BOOLEAN, RVM_DTYPE_DOUBLE);

	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_long_double, RVM_DTYPE_LONG, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_long_unsigned, RVM_DTYPE_LONG, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_double_long, RVM_DTYPE_DOUBLE, RVM_DTYPE_LONG);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_double_unsigned, RVM_DTYPE_DOUBLE, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_unsigned_long, RVM_DTYPE_UNSIGNED, RVM_DTYPE_LONG);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_unsigned_double, RVM_DTYPE_UNSIGNED, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_string_unsigned, RVM_DTYPE_STRING, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_string_long, RVM_DTYPE_STRING, RVM_DTYPE_LONG);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_string_double, RVM_DTYPE_STRING, RVM_DTYPE_DOUBLE);

}


