/*
 *  Regular Pattern Analyzer Toolkit (RPA/Tk)
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
#include "rlib/rstring.h"
#include "rvm/rvmreg.h"


void rjs_op_cat_string_string(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rstring_t *s1 = (rstring_t*)RVM_REG_GETP(arg1);
	rstring_t *s2 = (rstring_t*)RVM_REG_GETP(arg2);
	rstring_t *dst;

	dst = r_string_create_from_rstr(&s1->s);
	r_string_cat(dst, &s2->s);
	RVM_REG_SETP(res, dst);
	rvm_reg_setflag(res, RVM_DTYPE_STRING);
	rvm_reg_setflag(res, RVM_INFOBIT_ROBJECT);
}
