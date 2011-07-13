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

#ifndef __RVMOPERATOREQ_H_
#define __RVMOPERATOREQ_H_

#include "rvm/rvmoperator.h"
#include "rvm/rvmreg.h"

#ifdef __cplusplus
extern "C" {
#endif

void rvm_op_eq_unsigned(rvmcpu_t *cpu, unsigned short opid, rvmreg_t *res, rword op1, rword op2);
void rvm_op_eq_long(rvmcpu_t *cpu, unsigned short opid, rvmreg_t *res, long op1, long op2);
void rvm_op_eq_double(rvmcpu_t *cpu, unsigned short opid, rvmreg_t *res, double op1, double op2);

#ifdef __cplusplus
}
#endif

#endif
