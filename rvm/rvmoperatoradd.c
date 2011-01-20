#include "rvmoperator.h"
#include "rvmreg.h"


void rvm_op_add_unsigned(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rword op1, rword op2)
{
	rword r;
	r = op1 + op2;
	RVM_REG_SETU(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_UNSIGNED);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_C, r < op1 || r < op2);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, r & RVM_SIGN_BIT);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_V, (op1 & RVM_SIGN_BIT) == (op2 & RVM_SIGN_BIT) && (r & RVM_SIGN_BIT) != (op1 & RVM_SIGN_BIT));
}


void rvm_op_add_long(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rlong op1, rlong op2)
{
	rlong r;
	r = op1 + op2;
	RVM_REG_SETL(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_LONG);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, r & RVM_SIGN_BIT);
}

void rvm_op_add_double(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rdouble op1, rdouble op2)
{
	rdouble r;
	r = op1 + op2;
	RVM_REG_SETD(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_DOUBLE);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, r < 0.0);
}


void rvm_op_concat_string_string(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rstring_t *s1 = (rstring_t *)RVM_REG_GETP(arg1);
	rstring_t *s2 = (rstring_t *)RVM_REG_GETP(arg2);
	rstring_t *d = r_string_create_from_rstr(&s1->s);
	r_string_cat(d, &s2->s);

	rvm_gc_add(cpu->gc, (robject_t*)d);
	rvm_reg_setstring(res, d);
}


void rvm_op_concat_long_string(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rstring_t *s1 = r_string_create_from_long(RVM_REG_GETL(arg1));
	rstring_t *s2 = (rstring_t *)RVM_REG_GETP(arg2);
	rstring_t *d = r_string_create_from_rstr(&s1->s);
	r_string_cat(d, &s2->s);
	r_object_destroy((robject_t*)s1);

	rvm_gc_add(cpu->gc, (robject_t*)d);
	rvm_reg_setstring(res, d);
}


void rvm_op_concat_string_long(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rstring_t *s1 = (rstring_t *)RVM_REG_GETP(arg1);
	rstring_t *s2 = r_string_create_from_long(RVM_REG_GETL(arg2));
	rstring_t *d = r_string_create_from_rstr(&s1->s);
	r_string_cat(d, &s2->s);
	r_object_destroy((robject_t*)s2);

	rvm_gc_add(cpu->gc, (robject_t*)d);
	rvm_reg_setstring(res, d);
}


void rvm_op_concat_double_string(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rstring_t *s1 = r_string_create_from_double(RVM_REG_GETD(arg1));
	rstring_t *s2 = (rstring_t *)RVM_REG_GETP(arg2);
	rstring_t *d = r_string_create_from_rstr(&s1->s);
	r_string_cat(d, &s2->s);
	r_object_destroy((robject_t*)s1);

	rvm_gc_add(cpu->gc, (robject_t*)d);
	rvm_reg_setstring(res, d);
}


void rvm_op_concat_string_double(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rstring_t *s1 = (rstring_t *)RVM_REG_GETP(arg1);
	rstring_t *s2 = r_string_create_from_long(RVM_REG_GETD(arg2));
	rstring_t *d = r_string_create_from_rstr(&s1->s);
	r_string_cat(d, &s2->s);
	r_object_destroy((robject_t*)s2);

	rvm_gc_add(cpu->gc, (robject_t*)d);
	rvm_reg_setstring(res, d);
}
