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

#include "rvm/rvmoperator.h"
#include "rvm/rvmreg.h"
#include "rvm/rvmoperatornot.h"
#include "rvm/rvmoperatorbin.h"
#include "rvm/rvmoperatorlsr.h"
#include "rvm/rvmoperatorlsru.h"
#include "rvm/rvmoperatoreq.h"
#include "rvm/rvmoperatornoteq.h"
#include "rvm/rvmoperatorlogicor.h"
#include "rvm/rvmoperatorlogicand.h"
#include "rvm/rvmoperatorless.h"
#include "rvm/rvmoperatorlesseq.h"
#include "rvm/rvmoperatorgreater.h"
#include "rvm/rvmoperatorgreatereq.h"
#include "rvm/rvmoperatoror.h"
#include "rvm/rvmoperatorxor.h"
#include "rvm/rvmoperatorcast.h"
#include "rvm/rvmoperatorsub.h"
#include "rvm/rvmoperatoradd.h"
#include "rvm/rvmoperatormod.h"
#include "rvm/rvmoperatormul.h"
#include "rvm/rvmoperatorlsl.h"
#include "rvm/rvmoperatorcmn.h"
#include "rvm/rvmoperatorcmp.h"
#include "rvm/rvmoperatorand.h"
#include "rvm/rvmoperatorcat.h"
#include "rvm/rvmoperatordiv.h"

static rvm_binopmap_t binary_operations[RVM_OPID_LAST+1];

static void rvm_op_abort_unsigned(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rword op1, rword op2)
{
	RVM_ABORT(cpu, RVM_E_ILLEGAL);
}


static void rvm_op_abort_long(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rlong op1, rlong op2)
{
	RVM_ABORT(cpu, RVM_E_ILLEGAL);
}


static void rvm_op_abort_double(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rdouble op1, rdouble op2)
{
	RVM_ABORT(cpu, RVM_E_ILLEGAL);
}


inline void rvm_op_binary_unsigned(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rword op1, rword op2)
{
	binary_operations[opid].unsigned_binop_fun(cpu, opid, res, op1, op2);
}


inline void rvm_op_binary_long(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rlong op1, rlong op2)
{
	binary_operations[opid].long_binop_fun(cpu, opid, res, op1, op2);
}


inline void rvm_op_binary_double(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rdouble op1, rdouble op2)
{
	binary_operations[opid].double_binop_fun(cpu, opid, res, op1, op2);
}


static void rvm_op_binary_unsigned_unsigned(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvm_op_binary_unsigned(cpu, opid, res, RVM_REG_GETU(arg1), RVM_REG_GETU(arg2));
}


static void rvm_op_binary_long_long(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvm_op_binary_long(cpu, opid, res, RVM_REG_GETL(arg1), RVM_REG_GETL(arg2));
}


static void rvm_op_binary_double_long(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvm_op_binary_double(cpu, opid, res, RVM_REG_GETD(arg1), RVM_REG_GETL(arg2));
}


void rvm_op_binary_long_double(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvm_op_binary_double(cpu, opid, res, RVM_REG_GETL(arg1), RVM_REG_GETD(arg2));
}


static void rvm_op_binary_double_double(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvm_op_binary_double(cpu, opid, res, RVM_REG_GETD(arg1), RVM_REG_GETD(arg2));
}


static void rvm_op_binary_string_double(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvmreg_t s;

	if (rvm_reg_str2double(&s, arg1) < 0)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	rvm_op_binary_double(cpu, opid, res, RVM_REG_GETD(&s), RVM_REG_GETD(arg2));
}


static void rvm_op_binary_string_long(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvmreg_t s;

	if (rvm_reg_str2num(&s, arg1) < 0)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);

	if (rvm_reg_gettype(&s) == RVM_DTYPE_DOUBLE) {
		rvm_op_binary_double(cpu, opid, res, RVM_REG_GETD(&s), RVM_REG_GETL(arg2));
	} else {
		rvm_op_binary_long(cpu, opid, res, RVM_REG_GETL(&s), RVM_REG_GETL(arg2));
	}
}


static void rvm_op_binary_double_string(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvmreg_t s;

	if (rvm_reg_str2double(&s, arg2) < 0)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	rvm_op_binary_double(cpu, opid, res, RVM_REG_GETD(arg1), RVM_REG_GETD(&s));
}


static void rvm_op_binary_long_string(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvmreg_t s;

	if (rvm_reg_str2num(&s, arg2) < 0)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	if (rvm_reg_gettype(&s) == RVM_DTYPE_DOUBLE) {
		rvm_op_binary_double(cpu, opid, res, RVM_REG_GETL(arg1), RVM_REG_GETD(&s));
	} else {
		rvm_op_binary_long(cpu, opid, res, RVM_REG_GETL(arg1), RVM_REG_GETL(&s));
	}
}


static void rvm_op_binary_nan(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvm_reg_cleanup(res);
	RVM_REG_SETTYPE(res, RVM_DTYPE_NAN);
}


void rvm_op_binary_insert(rvm_opmap_t *opmap, rushort opid, rvm_binop_unsigned u, rvm_binop_long l, rvm_binop_double d)
{
	int i;

	binary_operations[opid].opid = opid;
	binary_operations[opid].unsigned_binop_fun = u;
	binary_operations[opid].long_binop_fun = l;
	binary_operations[opid].double_binop_fun = d;
	rvm_opmap_add_binary_operator(opmap, opid);

	for (i = 0; i < RVM_DTYPE_USER; i++) {
		rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_nan, RVM_DTYPE_NAN, i);
		rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_nan, RVM_DTYPE_UNDEF, i);
		rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_nan, i, RVM_DTYPE_NAN);
		rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_nan, i, RVM_DTYPE_UNDEF);
	}

	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_unsigned_unsigned, RVM_DTYPE_BOOLEAN, RVM_DTYPE_BOOLEAN);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_unsigned_unsigned, RVM_DTYPE_BOOLEAN, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_long_long, RVM_DTYPE_BOOLEAN, RVM_DTYPE_LONG);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_long_double, RVM_DTYPE_BOOLEAN, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_long_string, RVM_DTYPE_BOOLEAN, RVM_DTYPE_STRING);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_unsigned_unsigned, RVM_DTYPE_UNSIGNED, RVM_DTYPE_BOOLEAN);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_unsigned_unsigned, RVM_DTYPE_LONG, RVM_DTYPE_BOOLEAN);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_double_long, RVM_DTYPE_DOUBLE, RVM_DTYPE_BOOLEAN);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_string_long, RVM_DTYPE_STRING, RVM_DTYPE_BOOLEAN);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_unsigned_unsigned, RVM_DTYPE_UNSIGNED, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_double_double, RVM_DTYPE_DOUBLE, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_long_long, RVM_DTYPE_LONG, RVM_DTYPE_LONG);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_long_long, RVM_DTYPE_UNSIGNED, RVM_DTYPE_LONG);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_long_long, RVM_DTYPE_LONG, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_long_double, RVM_DTYPE_LONG, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_long_double, RVM_DTYPE_UNSIGNED, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_double_long, RVM_DTYPE_DOUBLE, RVM_DTYPE_LONG);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_double_long, RVM_DTYPE_DOUBLE, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_string_long, RVM_DTYPE_STRING, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_string_long, RVM_DTYPE_STRING, RVM_DTYPE_LONG);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_string_double, RVM_DTYPE_STRING, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_long_string, RVM_DTYPE_UNSIGNED, RVM_DTYPE_STRING);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_long_string, RVM_DTYPE_LONG, RVM_DTYPE_STRING);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_double_string, RVM_DTYPE_DOUBLE, RVM_DTYPE_STRING);
}


void rvm_op_binary_init(rvm_opmap_t *opmap)
{
	int i;

	for (i = 0; i < sizeof(binary_operations)/sizeof(binary_operations[0]); i++) {
		binary_operations[i].opid = RVM_OPID_NONE;
		binary_operations[i].unsigned_binop_fun = rvm_op_abort_unsigned;
		binary_operations[i].long_binop_fun = rvm_op_abort_long;
		binary_operations[i].double_binop_fun = rvm_op_abort_double;
	}

	rvm_op_binary_insert(opmap, RVM_OPID_ADD, rvm_op_add_unsigned, rvm_op_add_long, rvm_op_add_double);
	/*
	 * Overwrite RVM_OPID_ADD for string concatenation
	 */

	rvm_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rvm_op_concat_string_string, RVM_DTYPE_STRING, RVM_DTYPE_STRING);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rvm_op_concat_string_long, RVM_DTYPE_STRING, RVM_DTYPE_LONG);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rvm_op_concat_string_long, RVM_DTYPE_STRING, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rvm_op_concat_long_string, RVM_DTYPE_LONG, RVM_DTYPE_STRING);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rvm_op_concat_long_string, RVM_DTYPE_UNSIGNED, RVM_DTYPE_STRING);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rvm_op_concat_string_double, RVM_DTYPE_STRING, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rvm_op_concat_double_string, RVM_DTYPE_DOUBLE, RVM_DTYPE_STRING);


	rvm_op_binary_insert(opmap, RVM_OPID_SUB, rvm_op_sub_unsigned, rvm_op_sub_long, rvm_op_sub_double);
	rvm_op_binary_insert(opmap, RVM_OPID_MUL, rvm_op_mul_unsigned, rvm_op_mul_long, rvm_op_mul_double);
	rvm_op_binary_insert(opmap, RVM_OPID_DIV, rvm_op_div_unsigned, rvm_op_div_long, rvm_op_div_double);
	rvm_op_binary_insert(opmap, RVM_OPID_LSL, rvm_op_lsl_unsigned, rvm_op_lsl_long, rvm_op_lsl_double);
	rvm_op_binary_insert(opmap, RVM_OPID_LSR, rvm_op_lsr_unsigned, rvm_op_lsr_long, rvm_op_lsr_double);
	rvm_op_binary_insert(opmap, RVM_OPID_LSRU, rvm_op_lsru_unsigned, rvm_op_lsru_long, rvm_op_lsru_double);
	rvm_op_binary_insert(opmap, RVM_OPID_AND, rvm_op_and_unsigned, rvm_op_and_long, rvm_op_and_double);
	rvm_op_binary_insert(opmap, RVM_OPID_XOR, rvm_op_xor_unsigned, rvm_op_xor_long, rvm_op_xor_double);
	rvm_op_binary_insert(opmap, RVM_OPID_OR, rvm_op_or_unsigned, rvm_op_or_long, rvm_op_or_double);
	rvm_op_binary_insert(opmap, RVM_OPID_CMP, rvm_op_cmp_unsigned, rvm_op_cmp_long, rvm_op_cmp_double);
	rvm_op_binary_insert(opmap, RVM_OPID_CMN, rvm_op_cmn_unsigned, rvm_op_cmn_long, rvm_op_cmn_double);
	rvm_op_binary_insert(opmap, RVM_OPID_MOD, rvm_op_mod_unsigned, rvm_op_mod_long, rvm_op_mod_double);
	rvm_op_binary_insert(opmap, RVM_OPID_LOGICOR, rvm_op_logicor_unsigned, rvm_op_logicor_long, rvm_op_logicor_double);
	rvm_op_binary_insert(opmap, RVM_OPID_LOGICAND, rvm_op_logicand_unsigned, rvm_op_logicand_long, rvm_op_logicand_double);
	rvm_op_binary_insert(opmap, RVM_OPID_EQ, rvm_op_eq_unsigned, rvm_op_eq_long, rvm_op_eq_double);
	rvm_op_binary_insert(opmap, RVM_OPID_NOTEQ, rvm_op_noteq_unsigned, rvm_op_noteq_long, rvm_op_noteq_double);
	rvm_op_binary_insert(opmap, RVM_OPID_LESS, rvm_op_less_unsigned, rvm_op_less_long, rvm_op_less_double);
	rvm_op_binary_insert(opmap, RVM_OPID_LESSEQ, rvm_op_lesseq_unsigned, rvm_op_lesseq_long, rvm_op_lesseq_double);
	rvm_op_binary_insert(opmap, RVM_OPID_GREATER, rvm_op_greater_unsigned, rvm_op_greater_long, rvm_op_greater_double);
	rvm_op_binary_insert(opmap, RVM_OPID_GREATEREQ, rvm_op_greatereq_unsigned, rvm_op_greatereq_long, rvm_op_greatereq_double);
}
