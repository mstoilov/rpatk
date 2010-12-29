#include "rvmoperator.h"
#include "rstring.h"
#include "rvmreg.h"


void rvm_op_cat_string_string(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rstring_t *s1 = (rstring_t*)RVM_REG_GETP(arg1);
	rstring_t *s2 = (rstring_t*)RVM_REG_GETP(arg2);
	rstring_t *dst;

	dst = r_string_create_from_rstr(&s1->s);
	r_string_cat(dst, &s2->s);
	RVM_REG_SETP(res, dst);
	rvm_reg_setflag(res, RVM_DTYPE_STRING);
	rvm_reg_setflag(res, RVM_INFOBIT_ROBJECT);
}
