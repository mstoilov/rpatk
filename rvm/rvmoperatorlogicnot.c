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


static void rvm_op_logicnot_unsigned(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1)
{
	ruword r;

	r = (RVM_REG_GETU(arg1)) ? 0 : 1;
	RVM_REG_SETU(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_BOOLEAN);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
}


static void rvm_op_logicnot_signed(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1)
{
	ruword r;

	r = (RVM_REG_GETL(arg1)) ? 0 : 1;
	RVM_REG_SETU(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_BOOLEAN);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
}


static void rvm_op_logicnot_double(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1)
{
	ruword r;

	r = (RVM_REG_GETD(arg1)) ? 0 : 1;
	RVM_REG_SETU(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_BOOLEAN);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
}


static void rvm_op_logicnot_string(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1)
{
	ruword r;

	r = (r_string_empty(RVM_REG_GETP(arg1))) ? 1 : 0;
	RVM_REG_SETU(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_BOOLEAN);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
}


static void rvm_op_logicnot_nan(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1)
{
	rvm_reg_cleanup(res);
	RVM_REG_SETTYPE(res, RVM_DTYPE_NAN);
}


void rvm_op_logicnot_init(rvm_opmap_t *opmap)
{
	int i;
	rvm_opmap_add_unary_operator(opmap, RVM_OPID_LOGICNOT);
	for (i = 0; i < RVM_DTYPE_USER; i++) {
		rvm_opmap_set_unary_handler(opmap, RVM_OPID_LOGICNOT, rvm_op_logicnot_nan, RVM_DTYPE_NAN);
		rvm_opmap_set_unary_handler(opmap, RVM_OPID_LOGICNOT, rvm_op_logicnot_nan, RVM_DTYPE_UNDEF);
	}
	rvm_opmap_set_unary_handler(opmap, RVM_OPID_LOGICNOT, rvm_op_logicnot_unsigned, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_unary_handler(opmap, RVM_OPID_LOGICNOT, rvm_op_logicnot_unsigned, RVM_DTYPE_BOOLEAN);
	rvm_opmap_set_unary_handler(opmap, RVM_OPID_LOGICNOT, rvm_op_logicnot_unsigned, RVM_DTYPE_ARRAY);
	rvm_opmap_set_unary_handler(opmap, RVM_OPID_LOGICNOT, rvm_op_logicnot_unsigned, RVM_DTYPE_HARRAY);
	rvm_opmap_set_unary_handler(opmap, RVM_OPID_LOGICNOT, rvm_op_logicnot_signed, RVM_DTYPE_SIGNED);
	rvm_opmap_set_unary_handler(opmap, RVM_OPID_LOGICNOT, rvm_op_logicnot_double, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_unary_handler(opmap, RVM_OPID_LOGICNOT, rvm_op_logicnot_string, RVM_DTYPE_STRING);
}
