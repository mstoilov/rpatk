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


void rjs_op_lesseq_unsigned(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, ruword op1, ruword op2)
{
	ruword r;

	r = (op1 <= op2) ? 1 : 0;
	RVM_REG_SETU(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_BOOLEAN);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
}


void rjs_op_lesseq_signed(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, rword op1, rword op2)
{
	ruword r;

	r = (op1 <= op2) ? 1 : 0;
	RVM_REG_SETU(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_BOOLEAN);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
}


void rjs_op_lesseq_double(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, double op1, double op2)
{
	ruword r;

	r = (op1 <= op2) ? 1 : 0;
	RVM_REG_SETU(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_BOOLEAN);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
}


void rjs_op_lesseq_string_string(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	ruword r;
	rstring_t *s1 = (rstring_t *)RVM_REG_GETP(arg1);
	rstring_t *s2 = (rstring_t *)RVM_REG_GETP(arg2);

	r = (r_strncmp(s1->s.str, s2->s.str, R_MIN(s1->s.size, s2->s.size)) <= 0) ? 1 : 0;
	rvm_reg_setboolean(res, r);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
}

