#include "rvmarray.h"


static void rvmreg_array_oncopy(rarray_t *array)
{
	ruint index;
	ruint info;
	rvm_reg_t *r;


	for (index = 0; index < array->len; index++) {
		r = (rvm_reg_t *)r_array_slot(array, index);
		info = rvm_reg_getinfo(r);
		if (rvm_reg_flagtst(r, RVM_INFOBIT_REFOBJECT))
			RVM_REG_SETPVAL(r, r_ref_copy(RVM_REG_GETP(r)), info);
	}
}


static void rvmreg_array_ondestroy(rarray_t *array)
{
	ruint index;
	rvm_reg_t *r;


	for (index = 0; index < array->len; index++) {
		r = (rvm_reg_t *)r_array_slot(array, index);
		if (rvm_reg_flagtst(r, RVM_INFOBIT_REFOBJECT)) {
			r_ref_dec(RVM_REG_GETP(r));
			RVM_REG_CLEAR(r);
		}
	}
}


rarray_t *rvmreg_array_create()
{
	rarray_t *array = r_array_create(sizeof(rvm_reg_t));
	if (array) {
		array->oncopy = rvmreg_array_oncopy;
		array->ondestroy = rvmreg_array_ondestroy;
	}
	return array;
}


rharray_t *rvmreg_harray_create()
{
	rharray_t *harray = r_harray_create(sizeof(rvm_reg_t));
	if (harray) {
		harray->members->oncopy = rvmreg_array_oncopy;
		harray->members->ondestroy = rvmreg_array_ondestroy;
	}
	return harray;
}
