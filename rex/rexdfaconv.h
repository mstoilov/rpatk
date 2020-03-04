/*
 *  Regular Pattern Analyzer Toolkit (RPA/Tk)
 *  Copyright (c) 2009-2012 Martin Stoilov
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

#ifndef _REXDFACONV_H_
#define _REXDFACONV_H_

#include "rtypes.h"
#include "rlib/robject.h"
#include "rlib/rarray.h"
#include "rlib/rhash.h"
#include "rexstate.h"
#include "rexdb.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct rexdfaconv_s {
	rarray_t *stack;
	rarray_t *setT;
	rarray_t *setU;
	rarray_t *setV;
	rarray_t *trans;
	rarray_t *temptrans;
	rarray_t *temptrans1;
	rarray_t *ranges;
	rarray_t *marked;
	rhash_t *hash;
} rexdfaconv_t;


rexdfaconv_t *rex_dfaconv_create();
void rex_dfaconv_destroy(rexdfaconv_t *co);

/**
 * @brief Convert rexdb_t of type REXDB_TYPE_NFA into rexdb_t of type REXDB_TYPE_DFA.
 *
 * @param co Pointer to rexdfaconv_t object.
 * @param nfa The NFA that will be converted to DFA
 * @param start the UID of the start state.
 * @return Pointer to rexdb_t object of type REXDB_TYPE_DFA.
 */
rexdb_t *rex_dfaconv_run(rexdfaconv_t *co, rexdb_t *nfa, size_t start);


#ifdef __cplusplus
}
#endif
#endif
