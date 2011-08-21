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


static void rjs_op_not_unsigned(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1)
{
	ruword r;

	r = ~(RVM_REG_GETU(arg1));
	RVM_REG_SETU(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_UNSIGNED);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, r & RVM_SIGN_BIT);
}


static void rjs_op_not_castunsigned(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1)
{
	rvmreg_t uarg;

	RVM_REG_SETTYPE(&uarg, RVM_DTYPE_UNSIGNED);
	rjs_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_CAST, cpu, &uarg, arg1, &uarg);
	if (cpu->error)
		return;
	rjs_op_not_unsigned(cpu, opid, res, &uarg);
}


static void rjs_op_not_nan(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1)
{
	rvm_reg_cleanup(res);
	RVM_REG_SETTYPE(res, RVM_DTYPE_NAN);
}


void rjs_op_not_init(rjs_opmap_t *opmap)
{
	int i;

	rjs_opmap_add_unary_operator(opmap, RVM_OPID_NOT);
	for (i = 0; i < RVM_DTYPE_USER; i++) {
		rjs_opmap_set_unary_handler(opmap, RVM_OPID_NOT, rjs_op_not_nan, RVM_DTYPE_NAN);
		rjs_opmap_set_unary_handler(opmap, RVM_OPID_NOT, rjs_op_not_nan, RVM_DTYPE_UNDEF);
	}

	rjs_opmap_set_unary_handler(opmap, RVM_OPID_NOT, rjs_op_not_unsigned, RVM_DTYPE_UNSIGNED);
	rjs_opmap_set_unary_handler(opmap, RVM_OPID_NOT, rjs_op_not_unsigned, RVM_DTYPE_BOOLEAN);
	rjs_opmap_set_unary_handler(opmap, RVM_OPID_NOT, rjs_op_not_castunsigned, RVM_DTYPE_SIGNED);
	rjs_opmap_set_unary_handler(opmap, RVM_OPID_NOT, rjs_op_not_castunsigned, RVM_DTYPE_DOUBLE);
	rjs_opmap_set_unary_handler(opmap, RVM_OPID_NOT, rjs_op_not_castunsigned, RVM_DTYPE_STRING);
}
