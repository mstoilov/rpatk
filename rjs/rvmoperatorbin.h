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

#ifndef __RVMOPERATORBIN_H_
#define __RVMOPERATORBIN_H_

#include "rvmoperator.h"
#include "rvm/rvmreg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*rvm_binop_unsigned)(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, ruword op1, ruword op2);
typedef void (*rvm_binop_signed)(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, rword op1, rword op2);
typedef void (*rvm_binop_double)(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, double op1, double op2);


typedef struct rvm_binopmap_s {
	ruint16 opid;
	rvm_binop_unsigned unsigned_binop_fun;
	rvm_binop_signed long_binop_fun;
	rvm_binop_double double_binop_fun;
} rvm_binopmap_t;

void rjs_op_binary_insert(rjs_opmap_t *opmap, ruint16 opid, rvm_binop_unsigned u, rvm_binop_signed l, rvm_binop_double d);
void rjs_op_binary_init(rjs_opmap_t *opmap);

#ifdef __cplusplus
}
#endif

#endif
