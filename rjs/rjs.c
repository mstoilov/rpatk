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

#include <stdarg.h>
#include "rlib/rmem.h"
#include "rlib/rmap.h"
#include "rjs/rjs.h"
#include "rvm/rvmcodegen.h"
#include "rvmoperator.h"
#include "rvmoperatorbin.h"
#include "rvmoperatorcast.h"
#include "rvmoperatornot.h"
#include "rvmoperatorlogicnot.h"


static void rjs_engine_initgp(rjs_engine_t *jse);
static void rjs_engine_print(rvmcpu_t *cpu, rvm_asmins_t *ins);
static void rjs_engine_dbgprint(rvmcpu_t *cpu, rvm_asmins_t *ins);
static void rjs_engine_object(rvmcpu_t *cpu, rvm_asmins_t *ins);
static void rjs_string_ltrim(rvmcpu_t *cpu, rvm_asmins_t *ins);

static rvm_switable_t rjsswitable[] = {
		{"print", rjs_engine_print},
		{"dbgprint", rjs_engine_dbgprint},
		{"Object", rjs_engine_object},
		{"Array", rjs_engine_object},
		{"string.ltrim", rjs_string_ltrim},
		{NULL, NULL},
};


static void rjs_op_cast(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_type_t type = (rvmreg_type_t)RVM_CPUREG_GETU(cpu, ins->op3);
	rvmreg_t tmp;

	RVM_REG_CLEAR(&tmp);
	RVM_REG_SETTYPE(&tmp, type);
	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_CAST, cpu, RVM_CPUREG_PTR(cpu, ins->op1), RVM_CPUREG_PTR(cpu, ins->op2), &tmp);
}


static void rjs_op_eadd(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_ADD, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rjs_op_esub(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_SUB, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rjs_op_eneg(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t zero;

	rvm_reg_setunsigned(&zero, 0);
	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_SUB, cpu, RVM_CPUREG_PTR(cpu, ins->op1), &zero, arg2);
}



static void rjs_op_emul(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_MUL, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rjs_op_ediv(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_DIV, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rjs_op_emod(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_MOD, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rjs_op_elsl(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_LSL, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rjs_op_elsr(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_LSR, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rjs_op_elsru(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_LSRU, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rjs_op_eand(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_AND, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rjs_op_eorr(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_OR, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rjs_op_exor(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_XOR, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rjs_op_enot(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);

	rjs_opmap_invoke_unary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_NOT, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2);
}


static void rjs_op_eland(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_LOGICAND, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rjs_op_elor(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_LOGICOR, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rjs_op_elnot(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);

	rjs_opmap_invoke_unary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_LOGICNOT, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2);
}


static void rjs_op_eeq(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_EQ, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rjs_op_enoteq(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_NOTEQ, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rjs_op_egreat(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_GREATER, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rjs_op_egreateq(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_GREATEREQ, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rjs_op_elesseq(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_LESSEQ, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rjs_op_eless(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_LESS, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rjs_op_ecmp(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_CMP, cpu, NULL, arg1, arg2);
}


static void rjs_op_ecmn(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_CMN, cpu, NULL, arg1, arg2);
}


static void rjs_op_stralloc(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rstring_t *s = r_string_create_strsize((const char*)RVM_CPUREG_GETP(cpu, ins->op2), (unsigned long)RVM_CPUREG_GETU(cpu, ins->op3));
	if (!s) {
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	}
	r_gc_add(cpu->gc, (robject_t*)s);
	rvm_reg_setstring(arg1, s);
}


static void rjs_op_mapalloc(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rmap_t *a = r_map_create(sizeof(rvmreg_t), 7);
	if (!a) {
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	}
	r_gc_add(cpu->gc, (robject_t*)a);
	rvm_reg_setjsobject(arg1, (robject_t*)a);
}


static long rjs_op_mapproplookupadd(rmap_t *map, rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	long index;
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	if (RVM_REG_GETTYPE(arg3) == RVM_DTYPE_SIGNED || RVM_REG_GETTYPE(arg3) == RVM_DTYPE_UNSIGNED) {
		index = r_map_lookup_l(map, -1, (long)RVM_REG_GETL(arg3));
		if (index < 0)
			index = r_map_gckey_add_l(map, cpu->gc, (long)RVM_REG_GETL(arg3), NULL);
	} else if (RVM_REG_GETTYPE(arg3) == RVM_DTYPE_DOUBLE) {
		index = r_map_lookup_d(map, -1, RVM_REG_GETD(arg3));
		if (index < 0)
			index = r_map_gckey_add_d(map, cpu->gc, RVM_REG_GETD(arg3), NULL);
	} else if (RVM_REG_GETTYPE(arg3) == RVM_DTYPE_STRING) {
		index = r_map_lookup(map, -1, ((rstring_t *)RVM_CPUREG_GETP(cpu, ins->op3))->s.str, ((rstring_t *)RVM_CPUREG_GETP(cpu, ins->op3))->s.size);
		if (index < 0)
			index = r_map_gckey_add(map, cpu->gc, ((rstring_t *)RVM_CPUREG_GETP(cpu, ins->op3))->s.str, ((rstring_t *)RVM_CPUREG_GETP(cpu, ins->op3))->s.size, NULL);
	} else if (RVM_REG_GETTYPE(arg3) == RVM_DTYPE_POINTER) {
		index = r_map_lookup(map, -1, (char*)RVM_CPUREG_GETP(cpu, ins->op3), (unsigned int)RVM_CPUREG_GETL(cpu, ins->op1));
		if (index < 0)
			index = r_map_gckey_add(map, cpu->gc, (char*)RVM_CPUREG_GETP(cpu, ins->op3), (unsigned int)RVM_CPUREG_GETL(cpu, ins->op1), NULL);
	} else {
		index = -1;
	}

	return index;
}


static void rjs_op_proplookupadd(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	long index = -1;
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rmap_t *map = (rmap_t*)RVM_REG_GETP(arg2);

	if (rvm_reg_gettype(arg2) == RVM_DTYPE_MAP) {
		index = rjs_op_mapproplookupadd(map, cpu, ins);
	} else {

	}
	RVM_REG_CLEAR(arg1);
	RVM_REG_SETTYPE(arg1, RVM_DTYPE_SIGNED);
	RVM_REG_SETL(arg1, index);
}


static long rjs_op_mapproplookup(rmap_t *map, rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	long index = -1;
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	if (RVM_REG_GETTYPE(arg3) == RVM_DTYPE_SIGNED || RVM_REG_GETTYPE(arg3) == RVM_DTYPE_UNSIGNED) {
		index = r_map_lookup_l(map, -1, (long)RVM_REG_GETL(arg3));
	} else if (RVM_REG_GETTYPE(arg3) == RVM_DTYPE_DOUBLE) {
		index = r_map_lookup_d(map, -1, RVM_REG_GETD(arg3));
	} else if (RVM_REG_GETTYPE(arg3) == RVM_DTYPE_STRING) {
		index = r_map_lookup(map, -1, ((rstring_t *)RVM_CPUREG_GETP(cpu, ins->op3))->s.str, ((rstring_t *)RVM_CPUREG_GETP(cpu, ins->op3))->s.size);
	} else if (RVM_REG_GETTYPE(arg3) == RVM_DTYPE_POINTER) {
		index = r_map_lookup(map, -1, (char*)RVM_CPUREG_GETP(cpu, ins->op3), (unsigned int)RVM_CPUREG_GETL(cpu, ins->op1));
	}
	return index;
}


static long rjs_op_stringproplookup(rstring_t *str, rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	long index = -1;
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	if (RVM_REG_GETTYPE(arg3) == RVM_DTYPE_SIGNED || RVM_REG_GETTYPE(arg3) == RVM_DTYPE_UNSIGNED) {
		index = RVM_REG_GETL(arg3);
	} else if (RVM_REG_GETTYPE(arg3) == RVM_DTYPE_DOUBLE) {
		index = (long)RVM_REG_GETD(arg3);
	} else if (RVM_REG_GETTYPE(arg3) == RVM_DTYPE_STRING) {
		index = r_strtol(((rstring_t *)RVM_CPUREG_GETP(cpu, ins->op3))->s.str, NULL, 10);
	}
	return index;
}


static void rjs_op_proplookup(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	long index = -1;
	rboolean globalprop = 0;
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);

	if (rvm_reg_gettype(arg2) == RVM_DTYPE_MAP) {
		rmap_t *map = (rmap_t*)RVM_REG_GETP(arg2);
		index = rjs_op_mapproplookup(map, cpu, ins);
		rvm_reg_setsigned(arg1, index);
	} else if (rvm_reg_gettype(arg2) == RVM_DTYPE_STRING) {
		rstring_t *str = (rstring_t*)RVM_REG_GETP(arg2);
		index = rjs_op_stringproplookup(str, cpu, ins);
		if (index < 0) {
			index = rjs_op_mapproplookup(RJS_CPU2JSE(cpu)->props[RVM_DTYPE_STRING], cpu, ins);
			if (index >= 0)
				globalprop = TRUE;
		}
		rvm_reg_setsigned(arg1, index);
		if (globalprop)
			RVM_REG_SETFLAG(arg1, RVM_INFOBIT_GLOBAL);
	}
}


static void rjs_op_propstr(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t tmp = rvm_reg_create_signed(0);
	rmap_t *a = NULL;
	rpointer value;
	long index;

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_CAST, cpu, &tmp, RVM_CPUREG_PTR(cpu, ins->op3), &tmp);
	index = (long)RVM_REG_GETL(&tmp);
	if (rvm_reg_gettype(arg2) != RVM_DTYPE_MAP) {

		return;
	}
	a = (rmap_t*)RVM_REG_GETP(arg2);
	value = r_map_value(a, index);
	if (!value)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	r_map_setvalue(a, index, arg1);
}


static void rjs_op_propldr(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t tmp = rvm_reg_create_signed(0);
	rpointer value;
	long index;

	index = (long)RVM_REG_GETL(arg1);
	rvm_reg_setundef(arg1);
	if (rvm_reg_gettype(arg2) == RVM_DTYPE_MAP) {
		rmap_t *a = (rmap_t*)RVM_REG_GETP(arg2);
		value = r_map_value(a, index);
		if (value) {
			*arg1 = *((rvmreg_t*)value);
		}
	} else if (rvm_reg_gettype(arg2) == RVM_DTYPE_STRING) {
		rstring_t *s = (rstring_t*)RVM_REG_GETP(arg2);
		if (index >= s->s.size) {
			rvm_reg_setundef(arg1);
		} else {
			if (1||RVM_REG_TSTFLAG(arg1, RVM_INFOBIT_GLOBAL)) {
				rmap_t *a = RJS_CPU2JSE(cpu)->props[RVM_DTYPE_STRING];
				value = r_map_value(a, index);
				if (value) {
					*arg1 = *((rvmreg_t*)value);
				}
			} else {
				rstring_t *allocstr = r_string_create_strsize(&s->s.str[index], 1);
				r_gc_add(cpu->gc, (robject_t*)allocstr);
				rvm_reg_setstring(arg1, allocstr);
			}
		}
	}
}


static void rjs_op_propkeyldr(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t tmp = rvm_reg_create_signed(0);
	rmap_t *map = NULL;
	rstring_t *key;
	long index;

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_CAST, cpu, &tmp, RVM_CPUREG_PTR(cpu, ins->op3), &tmp);
	index = (long)RVM_REG_GETL(&tmp);
	RVM_REG_CLEAR(arg1);
	RVM_REG_SETTYPE(arg1, RVM_DTYPE_UNDEF);
	if (rvm_reg_gettype(arg2) == RVM_DTYPE_MAP) {
		map = (rmap_t*)RVM_REG_GETP(arg2);
		key = r_map_key(map, index);
		if (key) {
			rvm_reg_setstring(arg1, key);
		}
	} else if (rvm_reg_gettype(arg2) == RVM_DTYPE_STRING) {

	}
}


static void rjs_op_propdel(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	int ret;
	long index;
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t tmp = rvm_reg_create_signed(0);
	rmap_t *a = NULL;

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_CAST, cpu, &tmp, RVM_CPUREG_PTR(cpu, ins->op3), &tmp);
	index = (long)RVM_REG_GETL(&tmp);
	rvm_reg_setboolean(arg1, 0);
	if (rvm_reg_gettype(arg2) == RVM_DTYPE_MAP) {
		a = (rmap_t*)RVM_REG_GETP(arg2);
		ret = r_map_delete(a, index);
		rvm_reg_setboolean(arg1, ret == 0 ? 1 : 0);
	}
}


static void rjs_op_propaddr(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t tmp = rvm_reg_create_signed(0);
	rmap_t *a;
	rpointer value;
	long index;

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_CAST, cpu, &tmp, RVM_CPUREG_PTR(cpu, ins->op3), &tmp);
	index = (long)RVM_REG_GETL(&tmp);
	if (rvm_reg_gettype(arg2) != RVM_DTYPE_MAP)
		RVM_ABORT(cpu, RVM_E_NOTOBJECT);
	a = (rmap_t*)RVM_REG_GETP(arg2);
	value = r_map_value(a, index);
	if (!value)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	RVM_REG_CLEAR(arg1);
	RVM_REG_SETTYPE(arg1, RVM_DTYPE_POINTER);
	RVM_REG_SETP(arg1, value);
}


static void rjs_op_propnext(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t tmp = rvm_reg_create_signed(0);
	rmap_t *map = NULL;
	long index = -1;

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_CAST, cpu, &tmp, RVM_CPUREG_PTR(cpu, ins->op3), &tmp);
	index = (long)RVM_REG_GETL(&tmp);
	if (rvm_reg_gettype(arg2) == RVM_DTYPE_MAP) {
		map = (rmap_t*)RVM_REG_GETP(arg2);
		if (index < 0)
			index = r_map_first(map);
		else
			index = r_map_next(map, index);
	}
	RVM_REG_CLEAR(arg1);
	RVM_REG_SETTYPE(arg1, RVM_DTYPE_SIGNED);
	RVM_REG_SETL(arg1, index);
}


static void rjs_op_propprev(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t tmp = rvm_reg_create_signed(0);
	rmap_t *map = NULL;
	long index = -1;

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_CAST, cpu, &tmp, RVM_CPUREG_PTR(cpu, ins->op3), &tmp);
	index = (long)RVM_REG_GETL(&tmp);
	if (rvm_reg_gettype(arg2) == RVM_DTYPE_MAP) {
		map = (rmap_t*)RVM_REG_GETP(arg2);
		if (index < 0)
			index = r_map_last(map);
		else
			index = r_map_prev(map, index);
	}
	RVM_REG_CLEAR(arg1);
	RVM_REG_SETTYPE(arg1, RVM_DTYPE_SIGNED);
	RVM_REG_SETL(arg1, index);
}


static void rjs_op_mapdel(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	int ret;
	long index;
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t tmp = rvm_reg_create_signed(0);
	rmap_t *a = NULL;

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_CAST, cpu, &tmp, RVM_CPUREG_PTR(cpu, ins->op3), &tmp);
	index = (long)RVM_REG_GETL(&tmp);
	if (rvm_reg_gettype(arg2) != RVM_DTYPE_MAP)
		RVM_ABORT(cpu, RVM_E_NOTOBJECT);
	a = (rmap_t*)RVM_REG_GETP(arg2);
	ret = r_map_delete(a, index);
	rvm_reg_setboolean(arg1, ret == 0 ? 1 : 0);
}


static void rjs_op_maplookup(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	long index;
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);
	rmap_t *a = (rmap_t*)RVM_REG_GETP(arg2);

	if (rvm_reg_gettype(arg2) != RVM_DTYPE_MAP) {
		RVM_ABORT(cpu, RVM_E_NOTOBJECT);
	}
	if (RVM_REG_GETTYPE(arg3) == RVM_DTYPE_SIGNED || RVM_REG_GETTYPE(arg3) == RVM_DTYPE_UNSIGNED) {
		index = r_map_lookup_l(a, -1, (long)RVM_REG_GETL(arg3));
	} else if (RVM_REG_GETTYPE(arg3) == RVM_DTYPE_DOUBLE) {
		index = r_map_lookup_d(a, -1, RVM_REG_GETD(arg3));
	} else if (RVM_REG_GETTYPE(arg3) == RVM_DTYPE_STRING) {
		index = r_map_lookup(a, -1, ((rstring_t *)RVM_CPUREG_GETP(cpu, ins->op3))->s.str, ((rstring_t *)RVM_CPUREG_GETP(cpu, ins->op3))->s.size);
	} else if (RVM_REG_GETTYPE(arg3) == RVM_DTYPE_POINTER) {
		index = r_map_lookup(a, -1, (char*)RVM_CPUREG_GETP(cpu, ins->op3), (unsigned int)RVM_CPUREG_GETL(cpu, ins->op1));
	} else {
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	}

	RVM_REG_CLEAR(arg1);
	RVM_REG_SETTYPE(arg1, RVM_DTYPE_SIGNED);
	RVM_REG_SETL(arg1, index);
}


static void rjs_op_mapadd(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	long index;
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);
	rmap_t *a = (rmap_t*)RVM_REG_GETP(arg2);

	if (rvm_reg_gettype(arg2) != RVM_DTYPE_MAP) {
		RVM_ABORT(cpu, RVM_E_NOTOBJECT);
	}
	if (RVM_REG_GETTYPE(arg3) == RVM_DTYPE_SIGNED || RVM_REG_GETTYPE(arg3) == RVM_DTYPE_UNSIGNED) {
		index = r_map_gckey_add_l(a, cpu->gc, (long)RVM_REG_GETL(arg3), NULL);
	} else if (RVM_REG_GETTYPE(arg3) == RVM_DTYPE_DOUBLE) {
		index = r_map_gckey_add_d(a, cpu->gc, RVM_REG_GETD(arg3), NULL);
	} else if (RVM_REG_GETTYPE(arg3) == RVM_DTYPE_STRING) {
		index = r_map_gckey_add(a, cpu->gc, ((rstring_t *)RVM_CPUREG_GETP(cpu, ins->op3))->s.str, ((rstring_t *)RVM_CPUREG_GETP(cpu, ins->op3))->s.size, NULL);
	} else if (RVM_REG_GETTYPE(arg3) == RVM_DTYPE_POINTER) {
		index = r_map_gckey_add(a, cpu->gc, (char*)RVM_CPUREG_GETP(cpu, ins->op3), (unsigned int)RVM_CPUREG_GETL(cpu, ins->op1), NULL);
	} else {
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	}

	RVM_REG_CLEAR(arg1);
	RVM_REG_SETTYPE(arg1, RVM_DTYPE_SIGNED);
	RVM_REG_SETL(arg1, index);
}


static void rjs_op_maplookupadd(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	long index;
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);
	rmap_t *a = (rmap_t*)RVM_REG_GETP(arg2);

	if (rvm_reg_gettype(arg2) != RVM_DTYPE_MAP) {
		RVM_ABORT(cpu, RVM_E_NOTOBJECT);
	}
	if (RVM_REG_GETTYPE(arg3) == RVM_DTYPE_SIGNED || RVM_REG_GETTYPE(arg3) == RVM_DTYPE_UNSIGNED) {
		index = r_map_lookup_l(a, -1, (long)RVM_REG_GETL(arg3));
		if (index < 0)
			index = r_map_gckey_add_l(a, cpu->gc, (long)RVM_REG_GETL(arg3), NULL);
	} else if (RVM_REG_GETTYPE(arg3) == RVM_DTYPE_DOUBLE) {
		index = r_map_lookup_d(a, -1, RVM_REG_GETD(arg3));
		if (index < 0)
			index = r_map_gckey_add_d(a, cpu->gc, RVM_REG_GETD(arg3), NULL);
	} else if (RVM_REG_GETTYPE(arg3) == RVM_DTYPE_STRING) {
		index = r_map_lookup(a, -1, ((rstring_t *)RVM_CPUREG_GETP(cpu, ins->op3))->s.str, ((rstring_t *)RVM_CPUREG_GETP(cpu, ins->op3))->s.size);
		if (index < 0)
			index = r_map_gckey_add(a, cpu->gc, ((rstring_t *)RVM_CPUREG_GETP(cpu, ins->op3))->s.str, ((rstring_t *)RVM_CPUREG_GETP(cpu, ins->op3))->s.size, NULL);
	} else if (RVM_REG_GETTYPE(arg3) == RVM_DTYPE_POINTER) {
		index = r_map_lookup(a, -1, (char*)RVM_CPUREG_GETP(cpu, ins->op3), (unsigned int)RVM_CPUREG_GETL(cpu, ins->op1));
		if (index < 0)
			index = r_map_gckey_add(a, cpu->gc, (char*)RVM_CPUREG_GETP(cpu, ins->op3), (unsigned int)RVM_CPUREG_GETL(cpu, ins->op1), NULL);
	} else {
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	}
	RVM_REG_CLEAR(arg1);
	RVM_REG_SETTYPE(arg1, RVM_DTYPE_SIGNED);
	RVM_REG_SETL(arg1, index);
}


static void rjs_op_mapaddr(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t tmp = rvm_reg_create_signed(0);
	rmap_t *a;
	rpointer value;
	long index;

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_CAST, cpu, &tmp, RVM_CPUREG_PTR(cpu, ins->op3), &tmp);
	index = (long)RVM_REG_GETL(&tmp);
	if (rvm_reg_gettype(arg2) != RVM_DTYPE_MAP)
		RVM_ABORT(cpu, RVM_E_NOTOBJECT);
	a = (rmap_t*)RVM_REG_GETP(arg2);
	value = r_map_value(a, index);
	if (!value)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	RVM_REG_CLEAR(arg1);
	RVM_REG_SETTYPE(arg1, RVM_DTYPE_POINTER);
	RVM_REG_SETP(arg1, value);
}


static void rjs_op_mapldr(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t tmp = rvm_reg_create_signed(0);
	rmap_t *a = NULL;
	rpointer value;
	long index;

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_CAST, cpu, &tmp, RVM_CPUREG_PTR(cpu, ins->op3), &tmp);
	index = (long)RVM_REG_GETL(&tmp);
	if (rvm_reg_gettype(arg2) != RVM_DTYPE_MAP)
		RVM_ABORT(cpu, RVM_E_NOTOBJECT);
	a = (rmap_t*)RVM_REG_GETP(arg2);
	value = r_map_value(a, index);
	if (!value) {
		RVM_REG_CLEAR(arg1);
		RVM_REG_SETTYPE(arg1, RVM_DTYPE_UNDEF);
	} else {
		*arg1 = *((rvmreg_t*)value);
	}
}


static void rjs_op_mapkeyldr(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t tmp = rvm_reg_create_signed(0);
	rmap_t *a = NULL;
	rstring_t *key;
	long index;

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_CAST, cpu, &tmp, RVM_CPUREG_PTR(cpu, ins->op3), &tmp);
	index = (long)RVM_REG_GETL(&tmp);
	if (rvm_reg_gettype(arg2) != RVM_DTYPE_MAP)
		RVM_ABORT(cpu, RVM_E_NOTOBJECT);
	a = (rmap_t*)RVM_REG_GETP(arg2);
	key = r_map_key(a, index);
	if (!key) {
		RVM_REG_CLEAR(arg1);
		RVM_REG_SETTYPE(arg1, RVM_DTYPE_UNDEF);
	} else {
		rvm_reg_setstring(arg1, key);
	}
}


static void rjs_op_mapnext(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t tmp = rvm_reg_create_signed(0);
	rmap_t *a = NULL;
	long index;

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_CAST, cpu, &tmp, RVM_CPUREG_PTR(cpu, ins->op3), &tmp);
	index = (long)RVM_REG_GETL(&tmp);
	if (rvm_reg_gettype(arg2) != RVM_DTYPE_MAP)
		RVM_ABORT(cpu, RVM_E_NOTOBJECT);
	a = (rmap_t*)RVM_REG_GETP(arg2);
	if (index < 0)
		index = r_map_first(a);
	else
		index = r_map_next(a, index);
	RVM_REG_CLEAR(arg1);
	RVM_REG_SETTYPE(arg1, RVM_DTYPE_SIGNED);
	RVM_REG_SETL(arg1, index);
}


static void rjs_op_mapprev(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t tmp = rvm_reg_create_signed(0);
	rmap_t *a = NULL;
	long index;

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_CAST, cpu, &tmp, RVM_CPUREG_PTR(cpu, ins->op3), &tmp);
	index = (long)RVM_REG_GETL(&tmp);
	if (rvm_reg_gettype(arg2) != RVM_DTYPE_MAP)
		RVM_ABORT(cpu, RVM_E_NOTOBJECT);
	a = (rmap_t*)RVM_REG_GETP(arg2);
	if (index < 0)
		index = r_map_last(a);
	else
		index = r_map_prev(a, index);
	RVM_REG_CLEAR(arg1);
	RVM_REG_SETTYPE(arg1, RVM_DTYPE_SIGNED);
	RVM_REG_SETL(arg1, index);
}


static void rjs_op_mapstr(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t tmp = rvm_reg_create_signed(0);
	rmap_t *a = NULL;
	rpointer value;
	long index;

	rjs_opmap_invoke_binary_handler(RJS_USERDATA2MAP(cpu->userdata2), RJS_OPID_CAST, cpu, &tmp, RVM_CPUREG_PTR(cpu, ins->op3), &tmp);
	index = (long)RVM_REG_GETL(&tmp);
	if (rvm_reg_gettype(arg2) != RVM_DTYPE_MAP)
		RVM_ABORT(cpu, RVM_E_NOTOBJECT);
	a = (rmap_t*)RVM_REG_GETP(arg2);
	value = r_map_value(a, index);
	if (!value)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	r_map_setvalue(a, index, arg1);
}


const char *rjs_version()
{
	return RJS_VERSION_STRING;
}


static void rjs_string_ltrim(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	const char *ptr, *list;
	unsigned long size;
	rvmreg_t *r = NULL, *l = NULL;
	rstring_t *src, *dest;

	r = (rvmreg_t *) RVM_CPUREG_PTR(cpu, TP);
	if (RJS_SWI_PARAMS(cpu) > 0) {
		l = (rvmreg_t *) RJS_SWI_PARAM(cpu, 1);
		if (rvm_reg_gettype(l) != RVM_DTYPE_STRING)
			RJS_SWI_ABORT(rjs_engine_get(cpu), NULL);
	}
	if (rvm_reg_gettype(r) != RVM_DTYPE_STRING)
		RJS_SWI_ABORT(rjs_engine_get(cpu), NULL);
	if (l)
		list = ((rstring_t *)RVM_REG_GETP(l))->s.str;
	else
		list = " \t\n\r\0";
	src = (rstring_t *)RVM_REG_GETP(r);
	ptr = src->s.str;
	size = src->s.size;
	while (size > 0) {
		if (!r_strchr(list, *ptr))
			break;
		size--;
		ptr++;
	}
	dest = r_string_create_strsize(ptr, size);
	r_gc_add(cpu->gc, (robject_t*)dest);
	rvm_reg_setstring(RVM_CPUREG_PTR(cpu, R0), dest);
}


rjs_engine_t *rjs_engine_create()
{
	rvmcpu_t *cpu;
	rvmreg_t *tp;
	rvmreg_t tmp;
	rjs_engine_t *jse = (rjs_engine_t *) r_zmalloc(sizeof(*jse));

	jse->pa = rjs_parser_create();
	jse->cpu = cpu = rvm_cpu_create_default();
	jse->co = rjs_compiler_create(jse->cpu);
	jse->cgs = r_array_create(sizeof(rvm_codegen_t*));
	jse->errors = r_array_create(sizeof(rjs_error_t));
	jse->cpu->userdata1 = jse;
	rvm_cpu_addswitable(jse->cpu, "rjsswitable", rjsswitable);
	if (!jse->pa || !jse->cpu || !jse->co || !jse->cgs)
		goto error;
	rjs_engine_initgp(jse);
	tp = rvm_cpu_alloc_global(jse->cpu);
	rvm_reg_setjsobject(tp, (robject_t *)r_map_create(sizeof(rvmreg_t), 7));
	r_gc_add(jse->cpu->gc, (robject_t*)RVM_REG_GETP(tp));
	rvm_cpu_setreg(jse->cpu, TP, tp);

	rvm_cpu_setophandler(cpu, RJS_CAST, "RJS_CAST", rjs_op_cast);
	rvm_cpu_setophandler(cpu, RJS_ENEG, "RJS_ENEG", rjs_op_eneg);
	rvm_cpu_setophandler(cpu, RJS_EADD, "RJS_EADD", rjs_op_eadd);
	rvm_cpu_setophandler(cpu, RJS_ESUB, "RJS_ESUB", rjs_op_esub);
	rvm_cpu_setophandler(cpu, RJS_EMUL, "RJS_EMUL", rjs_op_emul);
	rvm_cpu_setophandler(cpu, RJS_EDIV, "RJS_EDIV", rjs_op_ediv);
	rvm_cpu_setophandler(cpu, RJS_EMOD, "RJS_EMOD", rjs_op_emod);
	rvm_cpu_setophandler(cpu, RJS_ELSL, "RJS_ELSL", rjs_op_elsl);
	rvm_cpu_setophandler(cpu, RJS_ELSR, "RJS_ELSR", rjs_op_elsr);
	rvm_cpu_setophandler(cpu, RJS_ELSRU, "RJS_ELSRU", rjs_op_elsru);
	rvm_cpu_setophandler(cpu, RJS_EAND, "RJS_EAND", rjs_op_eand);
	rvm_cpu_setophandler(cpu, RJS_EORR, "RJS_EORR", rjs_op_eorr);
	rvm_cpu_setophandler(cpu, RJS_EXOR, "RJS_EXOR", rjs_op_exor);
	rvm_cpu_setophandler(cpu, RJS_ENOT, "RJS_ENOT", rjs_op_enot);
	rvm_cpu_setophandler(cpu, RJS_ELAND, "RJS_ELAND", rjs_op_eland);
	rvm_cpu_setophandler(cpu, RJS_ELOR, "RJS_ELOR", rjs_op_elor);
	rvm_cpu_setophandler(cpu, RJS_ELNOT, "RJS_ELNOT", rjs_op_elnot);
	rvm_cpu_setophandler(cpu, RJS_EEQ, "RJS_EEQ", rjs_op_eeq);
	rvm_cpu_setophandler(cpu, RJS_ENOTEQ, "RJS_ENOTEQ", rjs_op_enoteq);
	rvm_cpu_setophandler(cpu, RJS_EGREAT, "RJS_EGREAT", rjs_op_egreat);
	rvm_cpu_setophandler(cpu, RJS_EGREATEQ, "RJS_EGREATEQ", rjs_op_egreateq);
	rvm_cpu_setophandler(cpu, RJS_ELESS, "RJS_ELESS", rjs_op_eless);
	rvm_cpu_setophandler(cpu, RJS_ELESSEQ, "RJS_ELESSEQ", rjs_op_elesseq);
	rvm_cpu_setophandler(cpu, RJS_ECMP, "RJS_ECMP", rjs_op_ecmp);
	rvm_cpu_setophandler(cpu, RJS_ECMN, "RJS_ECMN", rjs_op_ecmn);
	rvm_cpu_setophandler(cpu, RJS_PROPLKUP, "RJS_PROPLKUP", rjs_op_proplookup);
	rvm_cpu_setophandler(cpu, RJS_PROPLKUPADD, "RJS_PROPLKUPADD", rjs_op_proplookupadd);
	rvm_cpu_setophandler(cpu, RJS_PROPLDR, "RJS_PROPLDR", rjs_op_propldr);
	rvm_cpu_setophandler(cpu, RJS_PROPSTR, "RJS_PROPSTR", rjs_op_propstr);
	rvm_cpu_setophandler(cpu, RJS_PROPADDR, "RJS_PROPADDR", rjs_op_propaddr);
	rvm_cpu_setophandler(cpu, RJS_PROPKEYLDR, "RJS_PROPKEYLDR", rjs_op_propkeyldr);
	rvm_cpu_setophandler(cpu, RJS_PROPDEL, "RJS_PROPDEL", rjs_op_propdel);
	rvm_cpu_setophandler(cpu, RJS_PROPNEXT, "RJS_PROPNEXT", rjs_op_propnext);
	rvm_cpu_setophandler(cpu, RJS_PROPPREV, "RJS_PROPPREV", rjs_op_propprev);
	rvm_cpu_setophandler(cpu, RJS_STRALLOC, "RJS_STRALLOC", rjs_op_stralloc);
	rvm_cpu_setophandler(cpu, RJS_MAPALLOC, "RJS_MAPALLOC", rjs_op_mapalloc);

	cpu->userdata2 = rjs_opmap_create();
	rjs_op_binary_init(RJS_USERDATA2MAP(cpu->userdata2));
	rjs_op_cast_init(RJS_USERDATA2MAP(cpu->userdata2));
	rjs_op_not_init(RJS_USERDATA2MAP(cpu->userdata2));
	rjs_op_logicnot_init(RJS_USERDATA2MAP(cpu->userdata2));


	jse->props[RVM_DTYPE_STRING] = r_map_create(sizeof(rvmreg_t), 3);
	tmp = rvm_reg_create_swi(rvm_cpu_swilookup_s(cpu, "rjsswitable", "string.ltrim"));
	r_map_add_s(jse->props[RVM_DTYPE_STRING], "ltrim", &tmp);
	r_gc_add(jse->cpu->gc, (robject_t*)jse->props[RVM_DTYPE_STRING]);

	return jse;
error:
	rjs_engine_destroy(jse);
	return NULL;
}


void rjs_engine_destroy(rjs_engine_t *jse)
{
	unsigned long i;

	if (jse) {
		if (jse->cgs) {
			for (i = 0; i < r_array_length(jse->cgs); i++) {
				rvm_codegen_destroy(r_array_index(jse->cgs, i, rvm_codegen_t*));
			}
		}
		r_array_destroy(jse->cgs);
		r_array_destroy(jse->errors);
		rjs_parser_destroy(jse->pa);
		rjs_opmap_destroy(RJS_USERDATA2MAP(jse->cpu->userdata2));
		rvm_cpu_destroy(jse->cpu);
		rjs_compiler_destroy(jse->co);
		r_free(jse);
	}
}


static void rjs_engine_addtypename(rjs_engine_t *jse, rmap_t *types, unsigned long type, const char *typename)
{
	rvmreg_t rs;
	rstring_t *s;

	s = r_string_create_from_ansistr(typename);
	r_gc_add(jse->cpu->gc, (robject_t*)s);
	rvm_reg_setstring(&rs, s);
	r_map_add_l(types, type, &rs);
}


static void rjs_engine_inittypes(rjs_engine_t *jse)
{
	rmap_t *gmap = (rmap_t *)RVM_CPUREG_GETP(jse->cpu, GP);
	rmap_t *types;
	rvmreg_t rt;

	types = r_map_create(sizeof(rvmreg_t), 3);
	r_gc_add(jse->cpu->gc, (robject_t*)types);
	rvm_reg_setjsobject(&rt, (robject_t *)types);
	r_map_add_l(gmap, RJS_GPKEY_TYPES, &rt);
	rjs_engine_addtypename(jse, types, RVM_DTYPE_UNDEF, "undefined");
	rjs_engine_addtypename(jse, types, RVM_DTYPE_BOOLEAN, "boolean");
	rjs_engine_addtypename(jse, types, RVM_DTYPE_DOUBLE, "number");
	rjs_engine_addtypename(jse, types, RVM_DTYPE_UNSIGNED, "number");
	rjs_engine_addtypename(jse, types, RVM_DTYPE_SIGNED, "number");
	rjs_engine_addtypename(jse, types, RVM_DTYPE_STRING, "string");
	rjs_engine_addtypename(jse, types, RVM_DTYPE_FUNCTION, "object");
	rjs_engine_addtypename(jse, types, RVM_DTYPE_NAN, "object");
	rjs_engine_addtypename(jse, types, RVM_DTYPE_MAP, "object");
	rjs_engine_addtypename(jse, types, RVM_DTYPE_POINTER, "object");
}


static void rjs_engine_initgp(rjs_engine_t *jse)
{
	rvmreg_t gp;
	rmap_t *gmap;

	rvm_reg_init(&gp);
	gmap = r_map_create(sizeof(rvmreg_t), 7);
	r_gc_add(jse->cpu->gc, (robject_t*)gmap);
	rvm_reg_setjsobject(RVM_CPUREG_PTR(jse->cpu, GP), (robject_t *)gmap);
	rjs_engine_inittypes(jse);
}


int rjs_engine_open(rjs_engine_t *jse)
{
	return 0;
}


int rjs_engine_addswitable(rjs_engine_t *jse, const char *tabname, rvm_switable_t *switalbe)
{
	return rvm_cpu_addswitable(jse->cpu, tabname, switalbe);
}


static int rjs_engine_parse(rjs_engine_t *jse, const char *script, unsigned long size, rarray_t *records, rjs_error_t *error)
{
	long res = 0;

	res = rjs_parser_exec(jse->pa, script, size, records, error);
	return res;
}


int rjs_engine_compile(rjs_engine_t *jse, const char *script, unsigned long size)
{
	rvm_codegen_t *topcg = NULL;
	rarray_t *records = rpa_records_create();
	rjs_error_t error;
	jse->co->debug = jse->debugcompile;

	r_memset(&error, 0, sizeof(error));
	if (rjs_engine_parse(jse, script, size, records, &error) < 0) {

		goto err;
	}

	topcg =  r_array_empty(jse->cgs) ? NULL : r_array_last(jse->cgs, rvm_codegen_t*);
	if (!topcg || (topcg->userdata & RJS_COMPILER_CODEGENKEEP)) {
		/*
		 * Keep this script codegen object. Allocate a new one for the
		 * next script.
		 */
		topcg = rvm_codegen_create();
		r_array_add(jse->cgs, &topcg);
	} else {
		rvm_codegen_clear(topcg);
	}
	topcg->userdata = 0;
	if (rjs_compiler_compile(jse->co, script, size, records, topcg, &error) < 0) {
		topcg->userdata = 0;
		goto err;
	}

	rpa_records_destroy(records);
	return 0;

err:
	r_array_add(jse->errors, &error);
	rpa_records_destroy(records);
	return -1;
}


int rjs_engine_dumpast(rjs_engine_t *jse, const char *script, unsigned long size)
{
	rjs_error_t error;
	rarray_t *records = rpa_records_create();

	if (rjs_engine_parse(jse, script, size, records, &error) < 0) {


		return -1;
	}

	if (records) {
		long i;
		for (i = 0; i < rpa_records_length(records); i++)
			rpa_record_dump(records, i);
	}

	rpa_records_destroy(records);
	return 0;
}


int rjs_engine_compile_s(rjs_engine_t *jse, const char *script)
{
	return rjs_engine_compile(jse, script, r_strlen(script));
}


int rjs_engine_close(rjs_engine_t *jse)
{

	return 0;
}


int rjs_engine_run(rjs_engine_t *jse)
{
	int res = 0;
	rvm_codegen_t *cg = r_array_empty(jse->cgs) ? NULL : r_array_last(jse->cgs, rvm_codegen_t*);

	if (!cg) {

		return -1;
	}
	if (jse->debugexec) {
		res = rvm_cpu_exec_debug(jse->cpu, rvm_codegen_getcode(cg, 0), 0);
	} else {
		res = rvm_cpu_exec(jse->cpu, rvm_codegen_getcode(cg, 0), 0);
	}

	if (jse->cpu->error == RVM_E_USERABORT) {
		rword idx = RVM_CPUREG_GETIP(jse->cpu, PC) - rvm_codegen_getcode(cg, 0);
		if (idx >= 0) {
			r_printf("Aborted at source index: %ld\n", rvm_codegen_getsource(cg, (unsigned long)idx));
		}
	}
	return res;
}


rvmreg_t * rjs_engine_exec(rjs_engine_t *jse, const char *script, unsigned long size)
{
	if (rjs_engine_compile(jse, script, size) < 0)
		return NULL;
	RVM_CPUREG_SETU(jse->cpu, FP, 0);
	RVM_CPUREG_SETU(jse->cpu, SP, 0);
	if (rjs_engine_run(jse) < 0)
		return NULL;
	return RVM_CPUREG_PTR(jse->cpu, R0);
}


rvmreg_t *rjs_engine_exec_s(rjs_engine_t *jse, const char *script)
{
	return rjs_engine_exec(jse, script, r_strlen(script));
}


static int rjs_compiler_argarray_setup(rjs_compiler_t *co)
{
	rvm_varmap_t *v;
	rvmreg_t count = rvm_reg_create_signed(0);
	rmap_t *a;

	v = rvm_scope_tiplookup_s(co->scope, "ARGS");
	if (!v) {
		return -1;
	}
	a = r_map_create(sizeof(rvmreg_t), 7);
	r_gc_add(co->cpu->gc, (robject_t*)a);
	r_map_add_s(a, "count", &count);
	rvm_reg_setjsobject((rvmreg_t*)v->data.ptr, (robject_t*)a);
	return 0;
}


static int rjs_compiler_addarg(rjs_compiler_t *co, rvmreg_t *arg)
{
	rvm_varmap_t *v;
	rmap_t *a;
	rvmreg_t *count;
	long index;

	v = rvm_scope_tiplookup_s(co->scope, "ARGS");
	if (!v) {
		return -1;
	}
	a = (rmap_t*)RVM_REG_GETP((rvmreg_t*)v->data.ptr);
	index = r_map_lookup_s(a, -1, "count");
	R_ASSERT(index >= 0);
	count = (rvmreg_t *)r_map_value(a, index);
	r_map_add_l(a, (long)RVM_REG_GETL(count), arg);
	rvm_reg_setsigned(count, RVM_REG_GETL(count) + 1);

	return 0;
}


rvmreg_t *rjs_engine_vexec(rjs_engine_t *jse, const char *script, unsigned long size, unsigned long nargs, va_list args)
{
	rvmreg_t arg;
	unsigned long i = 0;

	if (rjs_engine_compile(jse, script, size) < 0)
		return NULL;
	if (nargs > 0) {
		rjs_compiler_argarray_setup(jse->co);
		for (i = 0; i < nargs; i++) {
			arg = va_arg(args, rvmreg_t);
			rjs_compiler_addarg(jse->co, &arg);
		}
	}
	RVM_CPUREG_SETU(jse->cpu, FP, 0);
	RVM_CPUREG_SETU(jse->cpu, SP, 0);
	if (rjs_engine_run(jse) < 0)
		return NULL;
	return RVM_CPUREG_PTR(jse->cpu, R0);
}


rvmreg_t *rjs_engine_args_exec(rjs_engine_t *jse, const char *script, unsigned long size, unsigned long nargs, ...)
{
	rvmreg_t *ret;
	va_list args;
	va_start(args, nargs);
	ret = rjs_engine_vexec(jse, script, size, nargs, args);
	va_end(args);
	return ret;
}


rvmreg_t *rjs_engine_args_exec_s(rjs_engine_t *jse, const char *script, unsigned long nargs, ...)
{
	rvmreg_t *ret;
	va_list args;
	va_start(args, nargs);
	ret = rjs_engine_vexec(jse, script, r_strlen(script), nargs, args);
	va_end(args);
	return ret;
}


static void rjs_engine_print(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *r = (rvmreg_t *)RVM_STACK_ADDR(cpu->stack, RVM_CPUREG_GETU(cpu, FP) + 1);

	if (rvm_reg_gettype(r) == RVM_DTYPE_UNSIGNED)
		r_printf("%lu\n", RVM_REG_GETU(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_NAN)
		r_printf("NaN\n");
	else if (rvm_reg_gettype(r) == RVM_DTYPE_UNDEF)
		r_printf("undefined\n");
	else if (rvm_reg_gettype(r) == RVM_DTYPE_BOOLEAN)
		r_printf("%s\n", RVM_REG_GETU(r) ? "true" : "false");
	else if (rvm_reg_gettype(r) == RVM_DTYPE_POINTER)
		r_printf("%p\n", RVM_REG_GETP(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_SIGNED)
		r_printf("%ld\n", RVM_REG_GETL(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_DOUBLE)
		r_printf("%f\n", RVM_REG_GETD(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_STRING)
		r_printf("%s\n", ((rstring_t*) RVM_REG_GETP(r))->s.str);
	else if (rvm_reg_gettype(r) == RVM_DTYPE_STRPTR)
		r_printf("(STRPTR) %s\n", (RVM_REG_GETSTR(r)));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_MAP)
		r_printf("(object) %p\n",RVM_REG_GETP(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_HARRAY)
		r_printf("(hashed array) %p\n",RVM_REG_GETP(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_SWIID)
		r_printf("(function) %p\n",RVM_REG_GETP(r));
	else
		r_printf("%p\n", RVM_REG_GETP(r));
}


static void rjs_engine_dbgprint(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *r = (rvmreg_t *)RVM_STACK_ADDR(cpu->stack, RVM_CPUREG_GETU(cpu, FP) + 1);

	if (rvm_reg_gettype(r) == RVM_DTYPE_UNSIGNED)
		r_printf("(UNSIGNED) %lu\n", RVM_REG_GETU(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_NAN)
		r_printf("(NAN) NaN\n");
	else if (rvm_reg_gettype(r) == RVM_DTYPE_UNDEF)
		r_printf("(UNDEF) undefined\n");
	else if (rvm_reg_gettype(r) == RVM_DTYPE_BOOLEAN)
		r_printf("(BOOLEAN) %s\n", RVM_REG_GETU(r) ? "true" : "false");
	else if (rvm_reg_gettype(r) == RVM_DTYPE_POINTER)
		r_printf("(POINTER) %p\n", RVM_REG_GETP(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_SIGNED)
		r_printf("(LONG) %ld\n", RVM_REG_GETL(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_DOUBLE)
		r_printf("(DOUBLE) %f\n", RVM_REG_GETD(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_STRING)
		r_printf("(STRING) %s\n", ((rstring_t*) RVM_REG_GETP(r))->s.str);
	else if (rvm_reg_gettype(r) == RVM_DTYPE_STRPTR)
		r_printf("(STRPTR) %s\n", (RVM_REG_GETSTR(r)));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_MAP)
		r_printf("(object) %p\n",RVM_REG_GETP(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_HARRAY)
		r_printf("(hashed array) %p\n",RVM_REG_GETP(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_SWIID)
		r_printf("(swi function) %p\n",RVM_REG_GETP(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_FUNCTION)
		r_printf("(function) %p\n",RVM_REG_GETP(r));
	else
		r_printf("%p\n", RVM_REG_GETP(r));
}


static void rjs_engine_object(rvmcpu_t *cpu, rvm_asmins_t *ins)
{

}


void rjs_engine_abort(rjs_engine_t *jse, rjs_error_t *error)
{
	if (error) {
		r_array_add(jse->errors, error);
	}
	rvm_cpu_abort(jse->cpu);
}


rjs_engine_t *rjs_engine_get(rvmcpu_t *cpu)
{
	return (rjs_engine_t *)cpu->userdata1;
}
