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

#ifndef __RVMOPERATORNOTEQ_H_
#define __RVMOPERATORNOTEQ_H_

#include "rvmoperator.h"
#include "rvm/rvmreg.h"

#ifdef __cplusplus
extern "C" {
#endif

void rjs_op_noteq_unsigned(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, ruword op1, ruword op2);
void rjs_op_noteq_signed(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, rword op1, rword op2);
void rjs_op_noteq_double(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, double op1, double op2);
void rjs_op_noteq_string_string(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2);

#ifdef __cplusplus
}
#endif

#endif
