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


void rvm_op_cast_static_static(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	*res = *arg1;
}


void rvm_op_cast_double_unsigned(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	ruword r = (ruword)RVM_REG_GETD(arg1);

	RVM_REG_CLEAR(res);
	RVM_REG_SETTYPE(res, RVM_DTYPE_UNSIGNED);
	RVM_REG_SETL(res, r);
}

void rvm_op_cast_double_signed(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	long r = (long)RVM_REG_GETD(arg1);

	RVM_REG_CLEAR(res);
	RVM_REG_SETTYPE(res, RVM_DTYPE_SIGNED);
	RVM_REG_SETL(res, r);
}


void rvm_op_cast_signed_unsigned(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	ruword r = (ruword)RVM_REG_GETL(arg1);

	RVM_REG_CLEAR(res);
	RVM_REG_SETTYPE(res, RVM_DTYPE_UNSIGNED);
	RVM_REG_SETU(res, r);
}


void rvm_op_cast_signed_double(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	double r = (double)RVM_REG_GETL(arg1);

	RVM_REG_CLEAR(res);
	RVM_REG_SETTYPE(res, RVM_DTYPE_DOUBLE);
	RVM_REG_SETL(res, r);
}


void rvm_op_cast_signed_boolean(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	ruword r = (ruword)RVM_REG_GETL(arg1);

	RVM_REG_CLEAR(res);
	RVM_REG_SETTYPE(res, RVM_DTYPE_BOOLEAN);
	RVM_REG_SETL(res, r ? 1 : 0);
}


void rvm_op_cast_double_boolean(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	ruword r = (ruword)RVM_REG_GETD(arg1);

	RVM_REG_CLEAR(res);
	RVM_REG_SETTYPE(res, RVM_DTYPE_BOOLEAN);
	RVM_REG_SETL(res, r ? 1 : 0);
}


void rvm_op_cast_pointer_boolean(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rpointer r = (rpointer)RVM_REG_GETP(arg1);

	RVM_REG_CLEAR(res);
	RVM_REG_SETTYPE(res, RVM_DTYPE_BOOLEAN);
	RVM_REG_SETL(res, r ? 1 : 0);
}


void rvm_op_cast_unsigned_signed(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	long r = (long)RVM_REG_GETU(arg1);

	RVM_REG_CLEAR(res);
	RVM_REG_SETTYPE(res, RVM_DTYPE_SIGNED);
	RVM_REG_SETL(res, r);
}


void rvm_op_cast_unsigned_double(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	double r = (double)RVM_REG_GETU(arg1);

	RVM_REG_CLEAR(res);
	RVM_REG_SETTYPE(res, RVM_DTYPE_DOUBLE);
	RVM_REG_SETD(res, r);
}


void rvm_op_cast_string_unsigned(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvmreg_t s;

	if (rvm_reg_str2signed(&s, arg1) < 0)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	rvm_reg_setunsigned(res, RVM_REG_GETL(&s));
}


void rvm_op_cast_string_signed(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvmreg_t s;

	if (rvm_reg_str2signed(&s, arg1) < 0)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	rvm_reg_setsigned(res, RVM_REG_GETL(&s));
}


void rvm_op_cast_string_double(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rvmreg_t s;

	if (rvm_reg_str2double(&s, arg1) < 0)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	rvm_reg_setdouble(res, RVM_REG_GETD(&s));
}


void rvm_op_cast_init(rvm_opmap_t *opmap)
{
	rvm_opmap_add_binary_operator(opmap, RVM_OPID_CAST);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_static_static, RVM_DTYPE_UNSIGNED, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_static_static, RVM_DTYPE_DOUBLE, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_static_static, RVM_DTYPE_SIGNED, RVM_DTYPE_SIGNED);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_static_static, RVM_DTYPE_STRING, RVM_DTYPE_STRING);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_static_static, RVM_DTYPE_BOOLEAN, RVM_DTYPE_BOOLEAN);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_signed_boolean, RVM_DTYPE_SIGNED, RVM_DTYPE_BOOLEAN);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_signed_boolean, RVM_DTYPE_UNSIGNED, RVM_DTYPE_BOOLEAN);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_double_boolean, RVM_DTYPE_DOUBLE, RVM_DTYPE_BOOLEAN);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_pointer_boolean, RVM_DTYPE_STRING, RVM_DTYPE_BOOLEAN);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_pointer_boolean, RVM_DTYPE_ARRAY, RVM_DTYPE_BOOLEAN);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_pointer_boolean, RVM_DTYPE_HARRAY, RVM_DTYPE_BOOLEAN);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_unsigned_signed, RVM_DTYPE_BOOLEAN, RVM_DTYPE_SIGNED);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_unsigned_double, RVM_DTYPE_BOOLEAN, RVM_DTYPE_DOUBLE);

	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_signed_double, RVM_DTYPE_SIGNED, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_signed_unsigned, RVM_DTYPE_SIGNED, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_double_signed, RVM_DTYPE_DOUBLE, RVM_DTYPE_SIGNED);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_double_unsigned, RVM_DTYPE_DOUBLE, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_unsigned_signed, RVM_DTYPE_UNSIGNED, RVM_DTYPE_SIGNED);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_unsigned_double, RVM_DTYPE_UNSIGNED, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_string_unsigned, RVM_DTYPE_STRING, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_string_signed, RVM_DTYPE_STRING, RVM_DTYPE_SIGNED);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_CAST, rvm_op_cast_string_double, RVM_DTYPE_STRING, RVM_DTYPE_DOUBLE);

}


