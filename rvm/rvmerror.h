#ifndef _RVMERROR_H_
#define _RVMERROR_H_

#define RVM_E_NONE 0

#include "rtypes.h"

typedef ruinteger rvm_error_t;

#define RVM_ERROR_BITS 16
#define RVM_ERROR_MASK ((1 << RVM_ERROR_BITS) - 1)

#define rvm_make_error(__m__, __c__) ((rvm_error_t)(((__m__) << RVM_ERROR_BITS) | ((__c__) & RVM_ERROR_MASK)))
#define rvm_set_error(__p__, __m__, __c__) do { (*(__p__) = rvm_make_error(__m__, __c__)); } while (0)
#define rvm_error_code(__e__) ((__e__) & RVM_ERROR_MASK)
#define rvm_error_module(__e__) ((__e__) >> RVM_ERROR_BITS)

#endif
