#include "rmem.h"
#include "rvmoperator.h"


rvm_opmap_t *rvm_opmap_create()
{
	rvm_opmap_t *opmap;

	opmap = (rvm_opmap_t*)r_malloc(sizeof(*opmap));
	if (!opmap)
		return NULL;
	r_memset(opmap, 0, sizeof(*opmap));
	opmap->operators = r_array_create(sizeof(rvm_opinfo_t));
	return opmap;
}


void rvm_opmap_destroy(rvm_opmap_t *opmap)
{
	int i;
	rvm_opinfo_t *opinfo;


	if (!opmap)
		return;
	for (i = 0; i < opmap->operators->len; i++) {
		opinfo = ((rvm_opinfo_t*)r_array_slot(opmap->operators, i));
		r_free(opinfo->handlers);
	}
	r_array_destroy(opmap->operators);
	r_free(opmap);
}


void rvm_opmap_add_binary_operator(rvm_opmap_t *opmap, rushort opid)
{
	rvm_opinfo_t opinfo;

	r_memset(&opinfo, 0, sizeof(opinfo));
	opinfo.opid = opid;
	opinfo.handlers = r_malloc(RVM_DTYPE_MAX * RVM_DTYPE_MAX * sizeof(rvm_ophandler_t));
	r_array_replace(opmap->operators, opid, &opinfo);
}


void rvm_opmap_add_unary_operator(rvm_opmap_t *opmap, rushort opid)
{
	rvm_opinfo_t opinfo;

	r_memset(&opinfo, 0, sizeof(opinfo));
	opinfo.opid = opid;
	opinfo.unary = TRUE;
	opinfo.handlers = r_malloc(RVM_DTYPE_MAX * sizeof(rvm_ophandler_t));
	r_array_replace(opmap->operators, opid, &opinfo);
}


rint rvm_opmap_set_binary_handler(rvm_opmap_t *opmap, rushort opid, rvm_binaryop_handler func, ruchar firstType, ruchar secondType)
{
	rvm_ophandler_t *h;
	ruint index = RVM_OP_HANDLER(firstType, secondType);
	rvm_opinfo_t *opinfo = ((rvm_opinfo_t*)r_array_slot(opmap->operators, opid));
	if (!opinfo->handlers)
		return -1;
	h = &opinfo->handlers[index];
	h->op = func;
	return 0;
}


rint rvm_opmap_set_unary_handler(rvm_opmap_t *opmap, rushort opid, rvm_unaryop_handler func, ruchar type)
{
	rvm_ophandler_t *h;
	rvm_opinfo_t *opinfo = ((rvm_opinfo_t*)r_array_slot(opmap->operators, opid));
	if (!opinfo->handlers)
		return -1;
	h = &opinfo->handlers[RVM_UNARY_HANDLER(type)];
	h->unary = func;
	return 0;
}


void rvm_opmap_invoke_binary_handler(rvm_opmap_t *opmap, rushort opid, rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvm_ophandler_t *h;
	rvm_opinfo_t *opinfo;
	ruint index = RVM_OP_HANDLER(RVM_REG_GETTYPE(arg1), RVM_REG_GETTYPE(arg2));

	if (opid >= opmap->operators->len)
		goto error;
	opinfo = ((rvm_opinfo_t*)r_array_slot(opmap->operators, opid));
	h = &opinfo->handlers[index];
	if (h->op == NULL)
		goto error;
	h->op(cpu, res, arg1, arg2);
	return;

error:
	RVM_ABORT(cpu, RVM_E_ILLEGAL);
}


void rvm_opmap_invoke_unary_handler(rvm_opmap_t *opmap, rushort opid, rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg)
{
	rvm_ophandler_t *h;
	rvm_opinfo_t *opinfo;
	ruint index = RVM_UNARY_HANDLER(RVM_REG_GETTYPE(arg));

	if (opid >= opmap->operators->len)
		goto error;
	opinfo = ((rvm_opinfo_t*)r_array_slot(opmap->operators, opid));
	h = &opinfo->handlers[index];
	if (h->unary == NULL)
		goto error;
	h->unary(cpu, res, arg);
	return;

error:
	RVM_ABORT(cpu, RVM_E_ILLEGAL);
}
