#ifndef _RVMOPERATORMUL_H_
#define _RVMOPERATORMUL_H_

#include "rvmoperator.h"

void rvm_op_mul_long_long(rvm_cpu_t *cpu, rvm_reg_t *res, const rvm_reg_t *arg1, const rvm_reg_t *arg2);
void rvm_op_mul_double_long(rvm_cpu_t *cpu, rvm_reg_t *res, const rvm_reg_t *arg1, const rvm_reg_t *arg2);
void rvm_op_mul_long_double(rvm_cpu_t *cpu, rvm_reg_t *res, const rvm_reg_t *arg1, const rvm_reg_t *arg2);
void rvm_op_mul_double_double(rvm_cpu_t *cpu, rvm_reg_t *res, const rvm_reg_t *arg1, const rvm_reg_t *arg2);

#endif
