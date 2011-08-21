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

#include "rvmoperator.h"
#include "rvm/rvmreg.h"
#include "rvmoperatornot.h"
#include "rvmoperatorbin.h"
#include "rvmoperatorlsr.h"
#include "rvmoperatorlsru.h"
#include "rvmoperatoreq.h"
#include "rvmoperatornoteq.h"
#include "rvmoperatorlogicor.h"
#include "rvmoperatorlogicand.h"
#include "rvmoperatorless.h"
#include "rvmoperatorlesseq.h"
#include "rvmoperatorgreater.h"
#include "rvmoperatorgreatereq.h"
#include "rvmoperatoror.h"
#include "rvmoperatorxor.h"
#include "rvmoperatorcast.h"
#include "rvmoperatorsub.h"
#include "rvmoperatoradd.h"
#include "rvmoperatormod.h"
#include "rvmoperatormul.h"
#include "rvmoperatorlsl.h"
#include "rvmoperatorcmn.h"
#include "rvmoperatorcmp.h"
#include "rvmoperatorand.h"
#include "rvmoperatorcat.h"
#include "rvmoperatordiv.h"

static rvm_binopmap_t binary_operations[RVM_OPID_LAST+1];

static void rjs_op_abort_unsigned(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, ruword op1, ruword op2)
{
	RVM_ABORT(cpu, RVM_E_ILLEGAL);
}


static void rjs_op_abort_signed(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, rword op1, rword op2)
{
	RVM_ABORT(cpu, RVM_E_ILLEGAL);
}


static void rjs_op_abort_double(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, double op1, double op2)
{
	RVM_ABORT(cpu, RVM_E_ILLEGAL);
}


static void rjs_op_binary_unsigned(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, ruword op1, ruword op2)
{
	binary_operations[opid].unsigned_binop_fun(cpu, opid, res, op1, op2);
}


static void rjs_op_binary_signed(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, rword op1, rword op2)
{
	binary_operations[opid].long_binop_fun(cpu, opid, res, op1, op2);
}


static void rjs_op_binary_double(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, double op1, double op2)
{
	binary_operations[opid].double_binop_fun(cpu, opid, res, op1, op2);
}


static void rjs_op_binary_unsigned_unsigned(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rjs_op_binary_unsigned(cpu, opid, res, RVM_REG_GETU(arg1), RVM_REG_GETU(arg2));
}


static void rjs_op_binary_signed_signed(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rjs_op_binary_signed(cpu, opid, res, RVM_REG_GETL(arg1), RVM_REG_GETL(arg2));
}


static void rjs_op_binary_double_signed(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rjs_op_binary_double(cpu, opid, res, RVM_REG_GETD(arg1), (double)RVM_REG_GETL(arg2));
}


void rjs_op_binary_signed_double(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rjs_op_binary_double(cpu, opid, res, (double)RVM_REG_GETL(arg1), RVM_REG_GETD(arg2));
}


static void rjs_op_binary_double_double(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rjs_op_binary_double(cpu, opid, res, RVM_REG_GETD(arg1), RVM_REG_GETD(arg2));
}


static void rjs_op_binary_string_double(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvmreg_t s;

	if (rvm_reg_str2double(&s, arg1) < 0)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	rjs_op_binary_double(cpu, opid, res, RVM_REG_GETD(&s), RVM_REG_GETD(arg2));
}


static void rjs_op_binary_string_signed(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvmreg_t s;

	if (rvm_reg_str2num(&s, arg1) < 0)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);

	if (rvm_reg_gettype(&s) == RVM_DTYPE_DOUBLE) {
		rjs_op_binary_double(cpu, opid, res, RVM_REG_GETD(&s), (double)RVM_REG_GETL(arg2));
	} else {
		rjs_op_binary_signed(cpu, opid, res, RVM_REG_GETL(&s), RVM_REG_GETL(arg2));
	}
}


static void rjs_op_binary_double_string(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvmreg_t s;

	if (rvm_reg_str2double(&s, arg2) < 0)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	rjs_op_binary_double(cpu, opid, res, RVM_REG_GETD(arg1), RVM_REG_GETD(&s));
}


static void rjs_op_binary_signed_string(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvmreg_t s;

	if (rvm_reg_str2num(&s, arg2) < 0)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	if (rvm_reg_gettype(&s) == RVM_DTYPE_DOUBLE) {
		rjs_op_binary_double(cpu, opid, res, (double)RVM_REG_GETL(arg1), RVM_REG_GETD(&s));
	} else {
		rjs_op_binary_signed(cpu, opid, res, RVM_REG_GETL(arg1), RVM_REG_GETL(&s));
	}
}


static void rjs_op_binary_nan(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvm_reg_cleanup(res);
	RVM_REG_SETTYPE(res, RVM_DTYPE_NAN);
}


void rjs_op_binary_insert(rjs_opmap_t *opmap, ruint16 opid, rvm_binop_unsigned u, rvm_binop_signed l, rvm_binop_double d)
{
	int i;

	binary_operations[opid].opid = opid;
	binary_operations[opid].unsigned_binop_fun = u;
	binary_operations[opid].long_binop_fun = l;
	binary_operations[opid].double_binop_fun = d;
	rjs_opmap_add_binary_operator(opmap, opid);

	for (i = 0; i < RVM_DTYPE_USER; i++) {
		rjs_opmap_set_binary_handler(opmap, opid, rjs_op_binary_nan, RVM_DTYPE_NAN, i);
		rjs_opmap_set_binary_handler(opmap, opid, rjs_op_binary_nan, RVM_DTYPE_UNDEF, i);
		rjs_opmap_set_binary_handler(opmap, opid, rjs_op_binary_nan, i, RVM_DTYPE_NAN);
		rjs_opmap_set_binary_handler(opmap, opid, rjs_op_binary_nan, i, RVM_DTYPE_UNDEF);
	}

	rjs_opmap_set_binary_handler(opmap, opid, rjs_op_binary_unsigned_unsigned, RVM_DTYPE_BOOLEAN, RVM_DTYPE_BOOLEAN);
	rjs_opmap_set_binary_handler(opmap, opid, rjs_op_binary_unsigned_unsigned, RVM_DTYPE_BOOLEAN, RVM_DTYPE_UNSIGNED);
	rjs_opmap_set_binary_handler(opmap, opid, rjs_op_binary_signed_signed, RVM_DTYPE_BOOLEAN, RVM_DTYPE_SIGNED);
	rjs_opmap_set_binary_handler(opmap, opid, rjs_op_binary_signed_double, RVM_DTYPE_BOOLEAN, RVM_DTYPE_DOUBLE);
	rjs_opmap_set_binary_handler(opmap, opid, rjs_op_binary_signed_string, RVM_DTYPE_BOOLEAN, RVM_DTYPE_STRING);
	rjs_opmap_set_binary_handler(opmap, opid, rjs_op_binary_unsigned_unsigned, RVM_DTYPE_UNSIGNED, RVM_DTYPE_BOOLEAN);
	rjs_opmap_set_binary_handler(opmap, opid, rjs_op_binary_unsigned_unsigned, RVM_DTYPE_SIGNED, RVM_DTYPE_BOOLEAN);
	rjs_opmap_set_binary_handler(opmap, opid, rjs_op_binary_double_signed, RVM_DTYPE_DOUBLE, RVM_DTYPE_BOOLEAN);
	rjs_opmap_set_binary_handler(opmap, opid, rjs_op_binary_string_signed, RVM_DTYPE_STRING, RVM_DTYPE_BOOLEAN);
	rjs_opmap_set_binary_handler(opmap, opid, rjs_op_binary_unsigned_unsigned, RVM_DTYPE_UNSIGNED, RVM_DTYPE_UNSIGNED);
	rjs_opmap_set_binary_handler(opmap, opid, rjs_op_binary_double_double, RVM_DTYPE_DOUBLE, RVM_DTYPE_DOUBLE);
	rjs_opmap_set_binary_handler(opmap, opid, rjs_op_binary_signed_signed, RVM_DTYPE_SIGNED, RVM_DTYPE_SIGNED);
	rjs_opmap_set_binary_handler(opmap, opid, rjs_op_binary_signed_signed, RVM_DTYPE_UNSIGNED, RVM_DTYPE_SIGNED);
	rjs_opmap_set_binary_handler(opmap, opid, rjs_op_binary_signed_signed, RVM_DTYPE_SIGNED, RVM_DTYPE_UNSIGNED);
	rjs_opmap_set_binary_handler(opmap, opid, rjs_op_binary_signed_double, RVM_DTYPE_SIGNED, RVM_DTYPE_DOUBLE);
	rjs_opmap_set_binary_handler(opmap, opid, rjs_op_binary_signed_double, RVM_DTYPE_UNSIGNED, RVM_DTYPE_DOUBLE);
	rjs_opmap_set_binary_handler(opmap, opid, rjs_op_binary_double_signed, RVM_DTYPE_DOUBLE, RVM_DTYPE_SIGNED);
	rjs_opmap_set_binary_handler(opmap, opid, rjs_op_binary_double_signed, RVM_DTYPE_DOUBLE, RVM_DTYPE_UNSIGNED);
	rjs_opmap_set_binary_handler(opmap, opid, rjs_op_binary_string_signed, RVM_DTYPE_STRING, RVM_DTYPE_UNSIGNED);
	rjs_opmap_set_binary_handler(opmap, opid, rjs_op_binary_string_signed, RVM_DTYPE_STRING, RVM_DTYPE_SIGNED);
	rjs_opmap_set_binary_handler(opmap, opid, rjs_op_binary_string_double, RVM_DTYPE_STRING, RVM_DTYPE_DOUBLE);
	rjs_opmap_set_binary_handler(opmap, opid, rjs_op_binary_signed_string, RVM_DTYPE_UNSIGNED, RVM_DTYPE_STRING);
	rjs_opmap_set_binary_handler(opmap, opid, rjs_op_binary_signed_string, RVM_DTYPE_SIGNED, RVM_DTYPE_STRING);
	rjs_opmap_set_binary_handler(opmap, opid, rjs_op_binary_double_string, RVM_DTYPE_DOUBLE, RVM_DTYPE_STRING);
}


void rjs_op_binary_init(rjs_opmap_t *opmap)
{
	int i;

	for (i = 0; i < sizeof(binary_operations)/sizeof(binary_operations[0]); i++) {
		binary_operations[i].opid = RVM_OPID_NONE;
		binary_operations[i].unsigned_binop_fun = rjs_op_abort_unsigned;
		binary_operations[i].long_binop_fun = rjs_op_abort_signed;
		binary_operations[i].double_binop_fun = rjs_op_abort_double;
	}

	rjs_op_binary_insert(opmap, RVM_OPID_SUB, rjs_op_sub_unsigned, rjs_op_sub_signed, rjs_op_sub_double);
	rjs_op_binary_insert(opmap, RVM_OPID_MUL, rjs_op_mul_unsigned, rjs_op_mul_signed, rjs_op_mul_double);
	rjs_op_binary_insert(opmap, RVM_OPID_DIV, rjs_op_div_unsigned, rjs_op_div_signed, rjs_op_div_double);
	rjs_op_binary_insert(opmap, RVM_OPID_LSL, rjs_op_lsl_unsigned, rjs_op_lsl_signed, rjs_op_lsl_double);
	rjs_op_binary_insert(opmap, RVM_OPID_LSR, rjs_op_lsr_unsigned, rjs_op_lsr_signed, rjs_op_lsr_double);
	rjs_op_binary_insert(opmap, RVM_OPID_LSRU, rjs_op_lsru_unsigned, rjs_op_lsru_signed, rjs_op_lsru_double);
	rjs_op_binary_insert(opmap, RVM_OPID_AND, rjs_op_and_unsigned, rjs_op_and_signed, rjs_op_and_double);
	rjs_op_binary_insert(opmap, RVM_OPID_XOR, rjs_op_xor_unsigned, rjs_op_xor_signed, rjs_op_xor_double);
	rjs_op_binary_insert(opmap, RVM_OPID_OR, rjs_op_or_unsigned, rjs_op_or_signed, rjs_op_or_double);
	rjs_op_binary_insert(opmap, RVM_OPID_CMP, rjs_op_cmp_unsigned, rjs_op_cmp_signed, rjs_op_cmp_double);
	rjs_op_binary_insert(opmap, RVM_OPID_CMN, rjs_op_cmn_unsigned, rjs_op_cmn_signed, rjs_op_cmn_double);
	rjs_op_binary_insert(opmap, RVM_OPID_MOD, rjs_op_mod_unsigned, rjs_op_mod_signed, rjs_op_mod_double);
	rjs_op_binary_insert(opmap, RVM_OPID_LOGICOR, rjs_op_logicor_unsigned, rjs_op_logicor_signed, rjs_op_logicor_double);
	rjs_op_binary_insert(opmap, RVM_OPID_LOGICAND, rjs_op_logicand_unsigned, rjs_op_logicand_signed, rjs_op_logicand_double);

	rjs_op_binary_insert(opmap, RVM_OPID_ADD, rjs_op_add_unsigned, rjs_op_add_signed, rjs_op_add_double);
	/*
	 * Overwrite RVM_OPID_ADD for string concatenation
	 */
	rjs_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rjs_op_concat_string_string, RVM_DTYPE_STRING, RVM_DTYPE_STRING);
	rjs_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rjs_op_concat_string_signed, RVM_DTYPE_STRING, RVM_DTYPE_SIGNED);
	rjs_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rjs_op_concat_string_signed, RVM_DTYPE_STRING, RVM_DTYPE_UNSIGNED);
	rjs_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rjs_op_concat_signed_string, RVM_DTYPE_SIGNED, RVM_DTYPE_STRING);
	rjs_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rjs_op_concat_signed_string, RVM_DTYPE_UNSIGNED, RVM_DTYPE_STRING);
	rjs_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rjs_op_concat_string_double, RVM_DTYPE_STRING, RVM_DTYPE_DOUBLE);
	rjs_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rjs_op_concat_double_string, RVM_DTYPE_DOUBLE, RVM_DTYPE_STRING);

	rjs_op_binary_insert(opmap, RVM_OPID_EQ, rjs_op_eq_unsigned, rjs_op_eq_signed, rjs_op_eq_double);
	rjs_opmap_set_binary_handler(opmap, RVM_OPID_EQ, rjs_op_eq_string_string, RVM_DTYPE_STRING, RVM_DTYPE_STRING);

	rjs_op_binary_insert(opmap, RVM_OPID_NOTEQ, rjs_op_noteq_unsigned, rjs_op_noteq_signed, rjs_op_noteq_double);
	rjs_opmap_set_binary_handler(opmap, RVM_OPID_NOTEQ, rjs_op_noteq_string_string, RVM_DTYPE_STRING, RVM_DTYPE_STRING);

	rjs_op_binary_insert(opmap, RVM_OPID_LESS, rjs_op_less_unsigned, rjs_op_less_signed, rjs_op_less_double);
	rjs_opmap_set_binary_handler(opmap, RVM_OPID_LESS, rjs_op_less_string_string, RVM_DTYPE_STRING, RVM_DTYPE_STRING);

	rjs_op_binary_insert(opmap, RVM_OPID_LESSEQ, rjs_op_lesseq_unsigned, rjs_op_lesseq_signed, rjs_op_lesseq_double);
	rjs_opmap_set_binary_handler(opmap, RVM_OPID_LESSEQ, rjs_op_lesseq_string_string, RVM_DTYPE_STRING, RVM_DTYPE_STRING);

	rjs_op_binary_insert(opmap, RVM_OPID_GREATER, rjs_op_greater_unsigned, rjs_op_greater_signed, rjs_op_greater_double);
	rjs_opmap_set_binary_handler(opmap, RVM_OPID_GREATER, rjs_op_greater_string_string, RVM_DTYPE_STRING, RVM_DTYPE_STRING);

	rjs_op_binary_insert(opmap, RVM_OPID_GREATEREQ, rjs_op_greatereq_unsigned, rjs_op_greatereq_signed, rjs_op_greatereq_double);
	rjs_opmap_set_binary_handler(opmap, RVM_OPID_GREATEREQ, rjs_op_greatereq_string_string, RVM_DTYPE_STRING, RVM_DTYPE_STRING);

}
