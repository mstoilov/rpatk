/*
 *  Regular Pattern Analyzer (RPA)
 *  Copyright (c) 2009-2010 Martin Stoilov
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Martin Stoilov <martin@rpasearch.com>
 */

#include "rlib/rmem.h"
#include "rvm/rvmoperator.h"


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
	for (i = 0; i < r_array_length(opmap->operators); i++) {
		opinfo = ((rvm_opinfo_t*)r_array_slot(opmap->operators, i));
		if (opinfo->opid)
			r_free(opinfo->handlers);
	}
	r_object_destroy((robject_t*)opmap->operators);
	r_free(opmap);
}


void rvm_opmap_add_binary_operator(rvm_opmap_t *opmap, unsigned short opid)
{
	rvm_opinfo_t opinfo;

	r_memset(&opinfo, 0, sizeof(opinfo));
	opinfo.opid = opid;
	opinfo.handlers = r_zmalloc(RVM_DTYPE_MAX * RVM_DTYPE_MAX * sizeof(rvm_ophandler_t));
	r_array_replace(opmap->operators, opid, &opinfo);
}


void rvm_opmap_add_unary_operator(rvm_opmap_t *opmap, unsigned short opid)
{
	rvm_opinfo_t opinfo;

	r_memset(&opinfo, 0, sizeof(opinfo));
	opinfo.opid = opid;
	opinfo.unary = TRUE;
	opinfo.handlers = r_malloc(RVM_DTYPE_MAX * sizeof(rvm_ophandler_t));
	r_array_replace(opmap->operators, opid, &opinfo);
}


int rvm_opmap_set_binary_handler(rvm_opmap_t *opmap, unsigned short opid, rvm_binaryop_handler func, unsigned char firstType, unsigned char secondType)
{
	rvm_ophandler_t *h;
	unsigned int index = RVM_OP_HANDLER(firstType, secondType);
	rvm_opinfo_t *opinfo = ((rvm_opinfo_t*)r_array_slot(opmap->operators, opid));
	if (!opinfo->handlers)
		return -1;
	h = &opinfo->handlers[index];
	h->op = func;
	return 0;
}


int rvm_opmap_set_unary_handler(rvm_opmap_t *opmap, unsigned short opid, rvm_unaryop_handler func, unsigned char type)
{
	rvm_ophandler_t *h;
	rvm_opinfo_t *opinfo = ((rvm_opinfo_t*)r_array_slot(opmap->operators, opid));
	if (!opinfo->handlers)
		return -1;
	h = &opinfo->handlers[RVM_UNARY_HANDLER(type)];
	h->unary = func;
	return 0;
}


void rvm_opmap_invoke_binary_handler(rvm_opmap_t *opmap, unsigned short opid, rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvm_ophandler_t *h;
	rvm_opinfo_t *opinfo;
	unsigned int index;
	rstring_t tempstr1, tempstr2;
	rvmreg_t temparg1, temparg2;

	/*
	 * if any of the arg1 or arg2 are of type RVM_DTYPE_STRPTR (static string) We need to convert them to
	 * rstring_t;
	 */
	if (RVM_REG_GETTYPE(arg1) == RVM_DTYPE_STRPTR) {
		r_memset(&tempstr1, 0, sizeof(tempstr1));
		tempstr1.s.str = RVM_REG_GETSTR(arg1);
		tempstr1.s.size = RVM_REG_GETSIZE(arg1);
		RVM_REG_SETP(&temparg1, &tempstr1);
		RVM_REG_SETTYPE(&temparg1, RVM_DTYPE_STRING);
		arg1 = &temparg1;
	}

	if (RVM_REG_GETTYPE(arg2) == RVM_DTYPE_STRPTR) {
		r_memset(&tempstr2, 0, sizeof(tempstr2));
		tempstr2.s.str = RVM_REG_GETSTR(arg2);
		tempstr2.s.size = RVM_REG_GETSIZE(arg2);
		RVM_REG_SETP(&temparg2, &tempstr2);
		RVM_REG_SETTYPE(&temparg2, RVM_DTYPE_STRING);
		arg2 = &temparg2;
	}


	index = RVM_OP_HANDLER(RVM_REG_GETTYPE(arg1), RVM_REG_GETTYPE(arg2));
	if (opid >= r_array_length(opmap->operators))
		goto error;
	opinfo = ((rvm_opinfo_t*)r_array_slot(opmap->operators, opid));
	h = &opinfo->handlers[index];
	if (h->op == NULL)
		goto error;
	h->op(cpu, opid, res, arg1, arg2);
	return;

error:
	RVM_ABORT(cpu, RVM_E_ILLEGAL);
}


void rvm_opmap_invoke_unary_handler(rvm_opmap_t *opmap, unsigned short opid, rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg)
{
	rvm_ophandler_t *h;
	rvm_opinfo_t *opinfo;
	unsigned int index;
	rstring_t tempstr;
	rvmreg_t temparg;

	if (RVM_REG_GETTYPE(arg) == RVM_DTYPE_STRPTR) {
		r_memset(&tempstr, 0, sizeof(tempstr));
		tempstr.s.str = RVM_REG_GETSTR(arg);
		tempstr.s.size = RVM_REG_GETSIZE(arg);
		RVM_REG_SETP(&temparg, &tempstr);
		RVM_REG_SETTYPE(&temparg, RVM_DTYPE_STRING);
		arg = &temparg;
	}

	index = RVM_UNARY_HANDLER(RVM_REG_GETTYPE(arg));
	if (opid >= r_array_length(opmap->operators))
		goto error;
	opinfo = ((rvm_opinfo_t*)r_array_slot(opmap->operators, opid));
	h = &opinfo->handlers[index];
	if (h->unary == NULL)
		goto error;
	h->unary(cpu, opid, res, arg);
	return;

error:
	RVM_ABORT(cpu, RVM_E_ILLEGAL);
}
