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

#ifndef _REXDFACONV_H_
#define _REXDFACONV_H_

#include "rtypes.h"
#include "rlib/robject.h"
#include "rlib/rarray.h"
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
} rexdfaconv_t;


rexdfaconv_t *rex_dfaconv_create();
void rex_dfaconv_destroy(rexdfaconv_t *co);
rexdb_t *rex_dfaconv_run(rexdfaconv_t *co, rexdb_t *nfa, unsigned long start);


#ifdef __cplusplus
}
#endif
#endif
