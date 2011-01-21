#include "rvmoperator.h"
#include "rvmreg.h"


static void rvm_op_logicnot_unsigned(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1)
{
	rword r;

	r = (RVM_REG_GETU(arg1)) ? 0 : 1;
	RVM_REG_SETU(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_BOOLEAN);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
}


static void rvm_op_logicnot_long(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1)
{
	rword r;

	r = (RVM_REG_GETL(arg1)) ? 0 : 1;
	RVM_REG_SETU(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_BOOLEAN);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
}


static void rvm_op_logicnot_double(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1)
{
	rword r;

	r = (RVM_REG_GETD(arg1)) ? 0 : 1;
	RVM_REG_SETU(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_BOOLEAN);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
}


static void rvm_op_logicnot_string(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1)
{
	rword r;

	r = (r_string_empty(RVM_REG_GETP(arg1))) ? 1 : 0;
	RVM_REG_SETU(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_BOOLEAN);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
}


static void rvm_op_logicnot_nan(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1)
{
	rvm_reg_cleanup(res);
	RVM_REG_SETTYPE(res, RVM_DTYPE_NAN);
}


void rvm_op_logicnot_init(rvm_opmap_t *opmap)
{
	int i;
	rvm_opmap_add_unary_operator(opmap, RVM_OPID_LOGICNOT);
	for (i = 0; i < RVM_DTYPE_USER; i++) {
		rvm_opmap_set_unary_handler(opmap, RVM_OPID_LOGICNOT, rvm_op_logicnot_nan, RVM_DTYPE_NAN);
		rvm_opmap_set_unary_handler(opmap, RVM_OPID_LOGICNOT, rvm_op_logicnot_nan, RVM_DTYPE_UNDEF);
	}
	rvm_opmap_set_unary_handler(opmap, RVM_OPID_LOGICNOT, rvm_op_logicnot_unsigned, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_unary_handler(opmap, RVM_OPID_LOGICNOT, rvm_op_logicnot_unsigned, RVM_DTYPE_BOOLEAN);
	rvm_opmap_set_unary_handler(opmap, RVM_OPID_LOGICNOT, rvm_op_logicnot_unsigned, RVM_DTYPE_ARRAY);
	rvm_opmap_set_unary_handler(opmap, RVM_OPID_LOGICNOT, rvm_op_logicnot_unsigned, RVM_DTYPE_HARRAY);
	rvm_opmap_set_unary_handler(opmap, RVM_OPID_LOGICNOT, rvm_op_logicnot_long, RVM_DTYPE_LONG);
	rvm_opmap_set_unary_handler(opmap, RVM_OPID_LOGICNOT, rvm_op_logicnot_double, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_unary_handler(opmap, RVM_OPID_LOGICNOT, rvm_op_logicnot_string, RVM_DTYPE_STRING);
}
