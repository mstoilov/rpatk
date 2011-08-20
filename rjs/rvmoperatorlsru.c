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


void rvm_op_lsru_unsigned(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, ruword op1, ruword op2)
{
	ruword r;

	r = op1 >> op2;
	RVM_REG_SETU(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_UNSIGNED);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, r & RVM_SIGN_BIT);
}


void rvm_op_lsru_signed(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, rword op1, rword op2)
{
	rvm_op_lsru_unsigned(cpu, opid, res, op1, op2);
}


void rvm_op_lsru_double(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, double op1, double op2)
{
	rvm_op_lsru_unsigned(cpu, opid, res, (ruword)((long)op1), (ruword)((long)op2));
}

