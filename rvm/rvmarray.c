#include "rvmarray.h"


static void r_array_oncopy_rvmreg(rarray_t *array)
{
	ruint index;
	ruint info;
	rvmreg_t *r;


	for (index = 0; index < array->len; index++) {
		r = (rvmreg_t *)r_array_slot(array, index);
		info = rvm_reg_getinfo(r);
		if (rvm_reg_flagtst(r, RVM_INFOBIT_REFOBJECT))
			RVM_REG_SETPVAL(r, r_ref_copy(RVM_REG_GETP(r)), info);
	}
}


static void r_array_ondestroy_rvmreg(rarray_t *array)
{
	ruint index;
	rvmreg_t *r;


	for (index = 0; index < array->len; index++) {
		r = (rvmreg_t *)r_array_slot(array, index);
		if (rvm_reg_flagtst(r, RVM_INFOBIT_REFOBJECT)) {
			r_ref_dec(RVM_REG_GETP(r));
			RVM_REG_CLEAR(r);
		}
	}
}


rarray_t *r_array_create_rvmreg()
{
	rarray_t *array = r_array_create(sizeof(rvmreg_t));
	if (array) {
		array->oncopy = r_array_oncopy_rvmreg;
		array->ondestroy = r_array_ondestroy_rvmreg;
	}
	return array;
}


rharray_t *r_harray_create_rvmreg()
{
	rharray_t *harray = r_harray_create(sizeof(rvmreg_t));
	if (harray) {
		harray->members->oncopy = r_array_oncopy_rvmreg;
		harray->members->ondestroy = r_array_ondestroy_rvmreg;
	}
	return harray;
}


rvmreg_t rvm_reg_create_string_ansi(const rchar *s)
{
	rvmreg_t r;
	RVM_REG_SETSTR(&r, r_string_create_from_ansistr(s));
	return r;
}


rvmreg_t rvm_reg_create_string(const rstr_t *s)
{
	rvmreg_t r;
	RVM_REG_SETSTR(&r, r_string_create_from_rstr(s));
	return r;
}


rvmreg_t rvm_reg_create_array()
{
	rvmreg_t r;
	RVM_REG_SETARRAY(&r, r_array_create_rvmreg());
	return r;
}


rvmreg_t rvm_reg_create_harray()
{
	rvmreg_t r;
	RVM_REG_SETHARRAY(&r, r_harray_create_rvmreg());
	return r;
}


rvmreg_t rvm_reg_create_double(rdouble d)
{
	rvmreg_t r;
	RVM_REG_SETD(&r, d);
	return r;
}


rvmreg_t rvm_reg_create_long(rlong l)
{
	rvmreg_t r;
	RVM_REG_SETL(&r, l);
	return r;
}
