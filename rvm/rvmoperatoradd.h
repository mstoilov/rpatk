#ifndef _RVMOPERATORADD_H_
#define _RVMOPERATORADD_H_

#include "rvmoperator.h"

void rvm_op_add_long_long(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2);
void rvm_op_add_double_long(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2);
void rvm_op_add_long_double(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2);
void rvm_op_add_double_double(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2);

#endif
