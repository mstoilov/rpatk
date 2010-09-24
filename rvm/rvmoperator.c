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


void rvm_opmap_add_operator(rvm_opmap_t *opmap, rushort opid)
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


rint rvm_opmap_set_handler(rvm_opmap_t *opmap, rushort opid, rvm_op_handler func, ruchar firstType, ruchar secondType)
{
	rvm_ophandler_t *h;
	rvm_opinfo_t *opinfo = ((rvm_opinfo_t*)r_array_slot(opmap->operators, opid));
	if (!opinfo->handlers)
		return -1;
	h = &opinfo->handlers[firstType * RVM_DTYPE_MAX + secondType];
	h->op = func;
	return 0;
}


rint rvm_opmap_set_unary_handler(rvm_opmap_t *opmap, rushort opid, rvm_unaryop_handler func, ruchar type)
{
	rvm_ophandler_t *h;
	rvm_opinfo_t *opinfo = ((rvm_opinfo_t*)r_array_slot(opmap->operators, opid));
	if (!opinfo->handlers)
		return -1;
	h = &opinfo->handlers[type];
	h->unary = func;
	return 0;
}


void rvm_opmap_invoke_handler(rvm_opmap_t *opmap, rushort opid, rvm_cpu_t *cpu, rvm_reg_t *res, const rvm_reg_t *arg1, const rvm_reg_t *arg2)
{


}


void rvm_opmap_invoke_unary_handler(rvm_opmap_t *opmap, rushort opid, rvm_cpu_t *cpu, rvm_reg_t *res, const rvm_reg_t *arg)
{

}
