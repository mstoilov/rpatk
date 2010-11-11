#include "rvmoperatorsub.h"
#include "rvmreg.h"


void rvm_op_sub_unsigned_unsigned(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rword r = RVM_REG_GETU(arg1) - RVM_REG_GETU(arg2);
	RVM_REG_SETU(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_UNSIGNED);
}


void rvm_op_sub_long_long(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rlong r = RVM_REG_GETL(arg1) - RVM_REG_GETL(arg2);
	RVM_REG_SETL(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_LONG);
}


void rvm_op_sub_double_long(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rdouble r = RVM_REG_GETD(arg1) - (rdouble) RVM_REG_GETL(arg2);
	RVM_REG_SETD(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_DOUBLE);
}


void rvm_op_sub_long_double(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rdouble r = (rdouble) RVM_REG_GETL(arg1) - RVM_REG_GETD(arg2);
	RVM_REG_SETD(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_DOUBLE);
}


void rvm_op_sub_double_double(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rdouble r = RVM_REG_GETD(arg1) - RVM_REG_GETD(arg2);
	RVM_REG_SETD(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_DOUBLE);
}


void rvm_op_sub_string_double(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rdouble r = r_strtod(R_STRING2PTR(RVM_REG_GETP(arg1)), NULL) - RVM_REG_GETD(arg2);
	RVM_REG_SETL(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_DOUBLE);
}


void rvm_op_sub_string_long(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rchar *dptr, *lptr;
	rdouble d = r_strtod(R_STRING2PTR(RVM_REG_GETP(arg1)), &dptr);
	rlong l = r_strtol(R_STRING2PTR(RVM_REG_GETP(arg1)), &lptr, 10);

	if (dptr > lptr) {
		RVM_REG_SETD(res, d - RVM_REG_GETL(arg2));
		RVM_REG_SETTYPE(res, RVM_DTYPE_DOUBLE);
	} else {
		RVM_REG_SETD(res, l - RVM_REG_GETL(arg2));
		RVM_REG_SETTYPE(res, RVM_DTYPE_LONG);
	}
}


void rvm_op_sub_double_string(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rdouble r = RVM_REG_GETD(arg1) - r_strtod(R_STRING2PTR(RVM_REG_GETP(arg2)), NULL);
	RVM_REG_SETL(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_DOUBLE);
}


void rvm_op_sub_long_string(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rchar *dptr, *lptr;
	rdouble d = r_strtod(R_STRING2PTR(RVM_REG_GETP(arg2)), &dptr);
	rlong l = r_strtol(R_STRING2PTR(RVM_REG_GETP(arg2)), &lptr, 10);

	if (dptr > lptr) {
		RVM_REG_SETD(res, RVM_REG_GETL(arg1) - d);
		RVM_REG_SETTYPE(res, RVM_DTYPE_DOUBLE);
	} else {
		RVM_REG_SETD(res, RVM_REG_GETL(arg1) - l);
		RVM_REG_SETTYPE(res, RVM_DTYPE_LONG);
	}
}


void rvm_op_sub_init(rvm_opmap_t *opmap)
{
	rvm_opmap_add_binary_operator(opmap, RVM_OPID_SUB);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_SUB, rvm_op_sub_unsigned_unsigned, RVM_DTYPE_UNSIGNED, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_SUB, rvm_op_sub_long_long, RVM_DTYPE_LONG, RVM_DTYPE_LONG);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_SUB, rvm_op_sub_long_long, RVM_DTYPE_UNSIGNED, RVM_DTYPE_LONG);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_SUB, rvm_op_sub_long_long, RVM_DTYPE_LONG, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_SUB, rvm_op_sub_long_double, RVM_DTYPE_LONG, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_SUB, rvm_op_sub_long_double, RVM_DTYPE_UNSIGNED, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_SUB, rvm_op_sub_double_long, RVM_DTYPE_DOUBLE, RVM_DTYPE_LONG);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_SUB, rvm_op_sub_double_long, RVM_DTYPE_DOUBLE, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_SUB, rvm_op_sub_string_long, RVM_DTYPE_STRING, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_SUB, rvm_op_sub_string_long, RVM_DTYPE_STRING, RVM_DTYPE_LONG);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_SUB, rvm_op_sub_string_double, RVM_DTYPE_STRING, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_SUB, rvm_op_sub_long_string, RVM_DTYPE_UNSIGNED, RVM_DTYPE_STRING);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_SUB, rvm_op_sub_long_string, RVM_DTYPE_LONG, RVM_DTYPE_STRING);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_SUB, rvm_op_sub_double_string, RVM_DTYPE_DOUBLE, RVM_DTYPE_STRING);
}
