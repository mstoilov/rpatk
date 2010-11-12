#ifndef _RVMOPERATOR_H_
#define _RVMOPERATOR_H_

#include "rtypes.h"
#include "rarray.h"
#include "rvmcpu.h"

#define RVM_UNARY_HANDLER(__t__) (__t__)
#define RVM_OP_HANDLER(__ft__, __st__) ((__ft__) * RVM_DTYPE_MAX + (__st__))


#define RVM_OPID_NONE 0
#define RVM_OPID_ADD 1
#define RVM_OPID_SUB 2
#define RVM_OPID_MUL 3
#define RVM_OPID_DIV 4
#define RVM_OPID_CAT 5
#define RVM_OPID_CAST 6
#define RVM_OPID_LSL 7
#define RVM_OPID_LSR 8

/*
 * Important: the res pointer might be the same as one of the arguments, the operator must
 * be implemented to take care of such cases.
 */
typedef void (*rvm_unaryop_handler)(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg);
typedef void (*rvm_binaryop_handler)(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2);

typedef union rvm_ophandler_s {
	rvm_unaryop_handler unary;
	rvm_binaryop_handler op;
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
void rvm_opmap_add_binary_operator(rvm_opmap_t *opmap, rushort opid);
void rvm_opmap_add_unary_operator(rvm_opmap_t *opmap, rushort opid);
rint rvm_opmap_set_binary_handler(rvm_opmap_t *opmap, rushort opid, rvm_binaryop_handler func, ruchar firstType, ruchar secondType);
rint rvm_opmap_set_unary_handler(rvm_opmap_t *opmap, rushort opid, rvm_unaryop_handler func, ruchar type);
void rvm_opmap_invoke_binary_handler(rvm_opmap_t *opmap, rushort opid, rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2);
void rvm_opmap_invoke_unary_handler(rvm_opmap_t *opmap, rushort opid, rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg);


void rvm_op_lsl_init(rvm_opmap_t *opmap);
void rvm_op_lsr_init(rvm_opmap_t *opmap);
#endif
