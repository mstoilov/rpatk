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

#ifndef _RVMOPERATOR_H_
#define _RVMOPERATOR_H_

#include "rtypes.h"
#include "rlib/rarray.h"
#include "rvm/rvmcpu.h"

#define RJS_UNARY_HANDLER(__t__) (__t__)
#define RJS_BINARY_HANDLER(__ft__, __st__) ((__ft__) * RVM_DTYPE_MAX + (__st__))
#define RJS_USERDATA2MAP(__ud__) ((rjs_opmap_t*)(__ud__))

enum {
	RJS_OPID_NONE = 0,
	RJS_OPID_ADD,
	RJS_OPID_SUB,
	RJS_OPID_MUL,
	RJS_OPID_DIV,
	RJS_OPID_CAT,
	RJS_OPID_CAST,
	RJS_OPID_LSL,
	RJS_OPID_LSR,
	RJS_OPID_LSRU,
	RJS_OPID_AND,
	RJS_OPID_XOR,
	RJS_OPID_OR,
	RJS_OPID_NOT,
	RJS_OPID_CMP,
	RJS_OPID_CMN,
	RJS_OPID_MOD,
	RJS_OPID_LOGICAND,
	RJS_OPID_LOGICOR,
	RJS_OPID_LOGICNOT,
	RJS_OPID_EQ,
	RJS_OPID_NOTEQ,
	RJS_OPID_LESS,
	RJS_OPID_LESSEQ,
	RJS_OPID_GREATER,
	RJS_OPID_GREATEREQ,

	RJS_OPID_USER0,
	RJS_OPID_USER1,
	RJS_OPID_USER2,
	RJS_OPID_USER3,
	RJS_OPID_USER4,
	RJS_OPID_USER5,
	RJS_OPID_USER6,
	RJS_OPID_USER7,
	RJS_OPID_USER8,
	RJS_OPID_USER9,
	RJS_OPID_USER10,
	RJS_OPID_USER11,
	RJS_OPID_USER12,
	RJS_OPID_USER13,
	RJS_OPID_USER14,
	RJS_OPID_USER15,
	RJS_OPID_LAST
};


/*
 * Important: the res pointer might be the same as one of the arguments, the operator must
 * be implemented to take care of such cases.
 */
typedef void (*rjs_unaryop_handler)(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg);
typedef void (*rjs_binaryop_handler)(rvmcpu_t *cpu, ruint16 opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2);

typedef union rjs_ophandler_s {
	rjs_unaryop_handler unary;
	rjs_binaryop_handler op;
} rjs_ophandler_t;

typedef struct rjs_opinfo_s {
	ruint16 opid;
	rboolean unary;
	rjs_ophandler_t *handlers;
} rjs_opinfo_t;

typedef struct rjs_opmap_s {
	rarray_t *operators;
} rjs_opmap_t;


rjs_opmap_t *rjs_opmap_create();
void rjs_opmap_destroy(rjs_opmap_t *opmap);
void rjs_opmap_add_binary_operator(rjs_opmap_t *opmap, ruint16 opid);
void rjs_opmap_add_unary_operator(rjs_opmap_t *opmap, ruint16 opid);
int rjs_opmap_set_binary_handler(rjs_opmap_t *opmap, ruint16 opid, rjs_binaryop_handler func, unsigned char firstType, unsigned char secondType);
int rjs_opmap_set_unary_handler(rjs_opmap_t *opmap, ruint16 opid, rjs_unaryop_handler func, unsigned char type);
void rjs_opmap_invoke_binary_handler(rjs_opmap_t *opmap, ruint16 opid, rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2);
void rjs_opmap_invoke_unary_handler(rjs_opmap_t *opmap, ruint16 opid, rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg);

#endif
