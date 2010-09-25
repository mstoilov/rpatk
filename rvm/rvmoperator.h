#ifndef _RVMOPERATOR_H_
#define _RVMOPERATOR_H_

#include "rtypes.h"
#include "rarray.h"
#include "rvmcpu.h"

#define RVM_UNARY_HANDLER(__t__) (__t__)
#define RVM_OP_HANDLER(__ft__, __st__) ((__ft__) * RVM_DTYPE_MAX + (__st__))


#define RVM_OPID_NONE 0
#define RVM_OPID_ADD 1

/*
 * Important: the res pointer might be the same as one of the arguments, the operator must
 * be implemented to take care of such cases.
 */
typedef void (*rvm_unaryop_handler)(rvm_cpu_t *cpu, rvm_reg_t *res, const rvm_reg_t *arg);
typedef void (*rvm_op_handler)(rvm_cpu_t *cpu, rvm_reg_t *res, const rvm_reg_t *arg1, const rvm_reg_t *arg2);

typedef union rvm_ophandler_s {
	rvm_unaryop_handler unary;
	rvm_op_handler op;
} rvm_ophandler_t;

typedef struct rvm_opinfo_s {
	rushort opid;
	rboolean unary;
	rvm_ophandler_t *handlers;
} rvm_opinfo_t;

typedef struct rvm_opmap_s {
	rarray_t *operators;
} rvm_opmap_t;


rvm_opmap_t *rvm_opmap_create();
void rvm_opmap_destroy(rvm_opmap_t *opmap);
void rvm_opmap_add_operator(rvm_opmap_t *opmap, rushort opid);
void rvm_opmap_add_unary_operator(rvm_opmap_t *opmap, rushort opid);
rint rvm_opmap_set_handler(rvm_opmap_t *opmap, rushort opid, rvm_op_handler func, ruchar firstType, ruchar secondType);
rint rvm_opmap_set_unary_handler(rvm_opmap_t *opmap, rushort opid, rvm_unaryop_handler func, ruchar type);
void rvm_opmap_invoke_handler(rvm_opmap_t *opmap, rushort opid, rvm_cpu_t *cpu, rvm_reg_t *res, const rvm_reg_t *arg1, const rvm_reg_t *arg2);
void rvm_opmap_invoke_unary_handler(rvm_opmap_t *opmap, rushort opid, rvm_cpu_t *cpu, rvm_reg_t *res, const rvm_reg_t *arg);

#endif
