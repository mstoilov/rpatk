#ifndef _RVMREG_H_
#define _RVMREG_H_

#include "rvmcpu.h"
#include "rarray.h"
#include "rharray.h"

#ifdef __cplusplus
extern "C" {
#endif

rarray_t *r_array_create_rvmreg();
rharray_t *r_harray_create_rvmreg();

rvmreg_t rvm_reg_create_string_ansi(const rchar *s);
rvmreg_t rvm_reg_create_string(const rstr_t *s);
rvmreg_t rvm_reg_create_array();
rvmreg_t rvm_reg_create_harray();
rvmreg_t rvm_reg_create_double(rdouble d);
rvmreg_t rvm_reg_create_long(rlong l);


#ifdef __cplusplus
}
#endif

#endif
