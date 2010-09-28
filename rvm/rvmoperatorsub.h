#ifndef _RVMOPERATORSUB_H_
#define _RVMOPERATORSUB_H_

#include "rvmoperator.h"

void rvm_op_sub_long_long(rvm_cpu_t *cpu, rvm_reg_t *res, const rvm_reg_t *arg1, const rvm_reg_t *arg2);
void rvm_op_sub_double_long(rvm_cpu_t *cpu, rvm_reg_t *res, const rvm_reg_t *arg1, const rvm_reg_t *arg2);
void rvm_op_sub_long_double(rvm_cpu_t *cpu, rvm_reg_t *res, const rvm_reg_t *arg1, const rvm_reg_t *arg2);
void rvm_op_sub_double_double(rvm_cpu_t *cpu, rvm_reg_t *res, const rvm_reg_t *arg1, const rvm_reg_t *arg2);

#endif
