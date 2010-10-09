#ifndef _RVMOPERATORMUL_H_
#define _RVMOPERATORMUL_H_

#include "rvmoperator.h"

void rvm_op_mul_long_long(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2);
void rvm_op_mul_double_long(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2);
void rvm_op_mul_long_double(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2);
void rvm_op_mul_double_double(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2);

#endif
