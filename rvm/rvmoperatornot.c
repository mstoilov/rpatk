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


static void rvm_op_not_unsigned(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1)
{
	ruword r;

	r = ~(RVM_REG_GETU(arg1));
	RVM_REG_SETU(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_UNSIGNED);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, r & RVM_SIGN_BIT);
}


static void rvm_op_not_castunsigned(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1)
{
	rvmreg_t uarg;

	RVM_REG_SETTYPE(&uarg, RVM_DTYPE_UNSIGNED);
	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_CAST, cpu, &uarg, arg1, &uarg);
	if (cpu->error)
		return;
	rvm_op_not_unsigned(cpu, opid, res, &uarg);
}


static void rvm_op_not_nan(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1)
{
	rvm_reg_cleanup(res);
	RVM_REG_SETTYPE(res, RVM_DTYPE_NAN);
}


void rvm_op_not_init(rvm_opmap_t *opmap)
{
	int i;

	rvm_opmap_add_unary_operator(opmap, RVM_OPID_NOT);
	for (i = 0; i < RVM_DTYPE_USER; i++) {
		rvm_opmap_set_unary_handler(opmap, RVM_OPID_NOT, rvm_op_not_nan, RVM_DTYPE_NAN);
		rvm_opmap_set_unary_handler(opmap, RVM_OPID_NOT, rvm_op_not_nan, RVM_DTYPE_UNDEF);
	}

	rvm_opmap_set_unary_handler(opmap, RVM_OPID_NOT, rvm_op_not_unsigned, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_unary_handler(opmap, RVM_OPID_NOT, rvm_op_not_unsigned, RVM_DTYPE_BOOLEAN);
	rvm_opmap_set_unary_handler(opmap, RVM_OPID_NOT, rvm_op_not_castunsigned, RVM_DTYPE_SINGED);
	rvm_opmap_set_unary_handler(opmap, RVM_OPID_NOT, rvm_op_not_castunsigned, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_unary_handler(opmap, RVM_OPID_NOT, rvm_op_not_castunsigned, RVM_DTYPE_STRING);
}
