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

static void rvm_op_abort_unsigned(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, ruword op1, ruword op2)
{
	RVM_ABORT(cpu, RVM_E_ILLEGAL);
}


static void rvm_op_abort_signed(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, rword op1, rword op2)
{
	RVM_ABORT(cpu, RVM_E_ILLEGAL);
}


static void rvm_op_abort_double(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, double op1, double op2)
{
	RVM_ABORT(cpu, RVM_E_ILLEGAL);
}


static void rvm_op_binary_unsigned(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, ruword op1, ruword op2)
{
	binary_operations[opid].unsigned_binop_fun(cpu, opid, res, op1, op2);
}


static void rvm_op_binary_signed(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, rword op1, rword op2)
{
	binary_operations[opid].long_binop_fun(cpu, opid, res, op1, op2);
}


static void rvm_op_binary_double(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, double op1, double op2)
{
	binary_operations[opid].double_binop_fun(cpu, opid, res, op1, op2);
}


static void rvm_op_binary_unsigned_unsigned(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvm_op_binary_unsigned(cpu, opid, res, RVM_REG_GETU(arg1), RVM_REG_GETU(arg2));
}


static void rvm_op_binary_signed_signed(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvm_op_binary_signed(cpu, opid, res, RVM_REG_GETL(arg1), RVM_REG_GETL(arg2));
}


static void rvm_op_binary_double_signed(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvm_op_binary_double(cpu, opid, res, RVM_REG_GETD(arg1), (double)RVM_REG_GETL(arg2));
}


void rvm_op_binary_signed_double(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvm_op_binary_double(cpu, opid, res, (double)RVM_REG_GETL(arg1), RVM_REG_GETD(arg2));
}


static void rvm_op_binary_double_double(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvm_op_binary_double(cpu, opid, res, RVM_REG_GETD(arg1), RVM_REG_GETD(arg2));
}


static void rvm_op_binary_string_double(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvmreg_t s;

	if (rvm_reg_str2double(&s, arg1) < 0)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	rvm_op_binary_double(cpu, opid, res, RVM_REG_GETD(&s), RVM_REG_GETD(arg2));
}


static void rvm_op_binary_string_signed(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvmreg_t s;

	if (rvm_reg_str2num(&s, arg1) < 0)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);

	if (rvm_reg_gettype(&s) == RVM_DTYPE_DOUBLE) {
		rvm_op_binary_double(cpu, opid, res, RVM_REG_GETD(&s), (double)RVM_REG_GETL(arg2));
	} else {
		rvm_op_binary_signed(cpu, opid, res, RVM_REG_GETL(&s), RVM_REG_GETL(arg2));
	}
}


static void rvm_op_binary_double_string(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvmreg_t s;

	if (rvm_reg_str2double(&s, arg2) < 0)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	rvm_op_binary_double(cpu, opid, res, RVM_REG_GETD(arg1), RVM_REG_GETD(&s));
}


static void rvm_op_binary_signed_string(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvmreg_t s;

	if (rvm_reg_str2num(&s, arg2) < 0)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	if (rvm_reg_gettype(&s) == RVM_DTYPE_DOUBLE) {
		rvm_op_binary_double(cpu, opid, res, (double)RVM_REG_GETL(arg1), RVM_REG_GETD(&s));
	} else {
		rvm_op_binary_signed(cpu, opid, res, RVM_REG_GETL(arg1), RVM_REG_GETL(&s));
	}
}


static void rvm_op_binary_nan(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvm_reg_cleanup(res);
	RVM_REG_SETTYPE(res, RVM_DTYPE_NAN);
}


void rvm_op_binary_insert(rvm_opmap_t *opmap, ruint16 opid, rvm_binop_unsigned u, rvm_binop_signed l, rvm_binop_double d)
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
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_signed_signed, RVM_DTYPE_BOOLEAN, RVM_DTYPE_SIGNED);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_signed_double, RVM_DTYPE_BOOLEAN, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_signed_string, RVM_DTYPE_BOOLEAN, RVM_DTYPE_STRING);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_unsigned_unsigned, RVM_DTYPE_UNSIGNED, RVM_DTYPE_BOOLEAN);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_unsigned_unsigned, RVM_DTYPE_SIGNED, RVM_DTYPE_BOOLEAN);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_double_signed, RVM_DTYPE_DOUBLE, RVM_DTYPE_BOOLEAN);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_string_signed, RVM_DTYPE_STRING, RVM_DTYPE_BOOLEAN);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_unsigned_unsigned, RVM_DTYPE_UNSIGNED, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_double_double, RVM_DTYPE_DOUBLE, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_signed_signed, RVM_DTYPE_SIGNED, RVM_DTYPE_SIGNED);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_signed_signed, RVM_DTYPE_UNSIGNED, RVM_DTYPE_SIGNED);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_signed_signed, RVM_DTYPE_SIGNED, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_signed_double, RVM_DTYPE_SIGNED, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_signed_double, RVM_DTYPE_UNSIGNED, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_double_signed, RVM_DTYPE_DOUBLE, RVM_DTYPE_SIGNED);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_double_signed, RVM_DTYPE_DOUBLE, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_string_signed, RVM_DTYPE_STRING, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_string_signed, RVM_DTYPE_STRING, RVM_DTYPE_SIGNED);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_string_double, RVM_DTYPE_STRING, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_signed_string, RVM_DTYPE_UNSIGNED, RVM_DTYPE_STRING);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_signed_string, RVM_DTYPE_SIGNED, RVM_DTYPE_STRING);
	rvm_opmap_set_binary_handler(opmap, opid, rvm_op_binary_double_string, RVM_DTYPE_DOUBLE, RVM_DTYPE_STRING);
}


void rvm_op_binary_init(rvm_opmap_t *opmap)
{
	int i;

	for (i = 0; i < sizeof(binary_operations)/sizeof(binary_operations[0]); i++) {
		binary_operations[i].opid = RVM_OPID_NONE;
		binary_operations[i].unsigned_binop_fun = rvm_op_abort_unsigned;
		binary_operations[i].long_binop_fun = rvm_op_abort_signed;
		binary_operations[i].double_binop_fun = rvm_op_abort_double;
	}

	rvm_op_binary_insert(opmap, RVM_OPID_SUB, rvm_op_sub_unsigned, rvm_op_sub_signed, rvm_op_sub_double);
	rvm_op_binary_insert(opmap, RVM_OPID_MUL, rvm_op_mul_unsigned, rvm_op_mul_signed, rvm_op_mul_double);
	rvm_op_binary_insert(opmap, RVM_OPID_DIV, rvm_op_div_unsigned, rvm_op_div_signed, rvm_op_div_double);
	rvm_op_binary_insert(opmap, RVM_OPID_LSL, rvm_op_lsl_unsigned, rvm_op_lsl_signed, rvm_op_lsl_double);
	rvm_op_binary_insert(opmap, RVM_OPID_LSR, rvm_op_lsr_unsigned, rvm_op_lsr_signed, rvm_op_lsr_double);
	rvm_op_binary_insert(opmap, RVM_OPID_LSRU, rvm_op_lsru_unsigned, rvm_op_lsru_signed, rvm_op_lsru_double);
	rvm_op_binary_insert(opmap, RVM_OPID_AND, rvm_op_and_unsigned, rvm_op_and_signed, rvm_op_and_double);
	rvm_op_binary_insert(opmap, RVM_OPID_XOR, rvm_op_xor_unsigned, rvm_op_xor_signed, rvm_op_xor_double);
	rvm_op_binary_insert(opmap, RVM_OPID_OR, rvm_op_or_unsigned, rvm_op_or_signed, rvm_op_or_double);
	rvm_op_binary_insert(opmap, RVM_OPID_CMP, rvm_op_cmp_unsigned, rvm_op_cmp_signed, rvm_op_cmp_double);
	rvm_op_binary_insert(opmap, RVM_OPID_CMN, rvm_op_cmn_unsigned, rvm_op_cmn_signed, rvm_op_cmn_double);
	rvm_op_binary_insert(opmap, RVM_OPID_MOD, rvm_op_mod_unsigned, rvm_op_mod_signed, rvm_op_mod_double);
	rvm_op_binary_insert(opmap, RVM_OPID_LOGICOR, rvm_op_logicor_unsigned, rvm_op_logicor_signed, rvm_op_logicor_double);
	rvm_op_binary_insert(opmap, RVM_OPID_LOGICAND, rvm_op_logicand_unsigned, rvm_op_logicand_signed, rvm_op_logicand_double);

	rvm_op_binary_insert(opmap, RVM_OPID_ADD, rvm_op_add_unsigned, rvm_op_add_signed, rvm_op_add_double);
	/*
	 * Overwrite RVM_OPID_ADD for string concatenation
	 */
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rvm_op_concat_string_string, RVM_DTYPE_STRING, RVM_DTYPE_STRING);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rvm_op_concat_string_signed, RVM_DTYPE_STRING, RVM_DTYPE_SIGNED);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rvm_op_concat_string_signed, RVM_DTYPE_STRING, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rvm_op_concat_signed_string, RVM_DTYPE_SIGNED, RVM_DTYPE_STRING);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rvm_op_concat_signed_string, RVM_DTYPE_UNSIGNED, RVM_DTYPE_STRING);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rvm_op_concat_string_double, RVM_DTYPE_STRING, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rvm_op_concat_double_string, RVM_DTYPE_DOUBLE, RVM_DTYPE_STRING);

	rvm_op_binary_insert(opmap, RVM_OPID_EQ, rvm_op_eq_unsigned, rvm_op_eq_signed, rvm_op_eq_double);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_EQ, rvm_op_eq_string_string, RVM_DTYPE_STRING, RVM_DTYPE_STRING);

	rvm_op_binary_insert(opmap, RVM_OPID_NOTEQ, rvm_op_noteq_unsigned, rvm_op_noteq_signed, rvm_op_noteq_double);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_NOTEQ, rvm_op_noteq_string_string, RVM_DTYPE_STRING, RVM_DTYPE_STRING);

	rvm_op_binary_insert(opmap, RVM_OPID_LESS, rvm_op_less_unsigned, rvm_op_less_signed, rvm_op_less_double);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_LESS, rvm_op_less_string_string, RVM_DTYPE_STRING, RVM_DTYPE_STRING);

	rvm_op_binary_insert(opmap, RVM_OPID_LESSEQ, rvm_op_lesseq_unsigned, rvm_op_lesseq_signed, rvm_op_lesseq_double);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_LESSEQ, rvm_op_lesseq_string_string, RVM_DTYPE_STRING, RVM_DTYPE_STRING);

	rvm_op_binary_insert(opmap, RVM_OPID_GREATER, rvm_op_greater_unsigned, rvm_op_greater_signed, rvm_op_greater_double);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_GREATER, rvm_op_greater_string_string, RVM_DTYPE_STRING, RVM_DTYPE_STRING);

	rvm_op_binary_insert(opmap, RVM_OPID_GREATEREQ, rvm_op_greatereq_unsigned, rvm_op_greatereq_signed, rvm_op_greatereq_double);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_GREATEREQ, rvm_op_greatereq_string_string, RVM_DTYPE_STRING, RVM_DTYPE_STRING);

}
