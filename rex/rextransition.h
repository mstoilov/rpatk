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


/**
 * @file rextransition.h
 * @brief Definitions related to NFA/DFA transitions
 *
 *
 * <h2>Synopsis</h2>
 * The following definitions are implementation of NFA/DFA transitions.
 */


#ifndef _REXTRANSITION_H_
#define _REXTRANSITION_H_

#include "rlib/rarray.h"
#include "rex/rexdef.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
	REX_TRANSITION_INPUT = 0,
	REX_TRANSITION_EMPTY = 1
} rex_transitiontype_t;


/*
 * Definition of transition structure. The transition type will
 * determine which members are valid.
 *
 */
typedef struct rex_transition_s {
	unsigned int type;
	rexchar_t lowin;
	rexchar_t highin;
	size_t srcuid;
	size_t dstuid;
} rex_transition_t;


rex_transition_t *rex_transitions_add(rarray_t *trans, rexchar_t c1, rexchar_t c2, size_t srcuid, size_t dstuid);
rex_transition_t *rex_transitions_add_e(rarray_t *etrans, size_t srcuid, size_t dstuid);
void rex_transitions_normalize(rarray_t *trans);
void rex_transitions_negative(rarray_t *dtrans, rarray_t *strans, size_t srcuid, size_t dstuid);
rex_transition_t *rex_transitions_find(rarray_t *trans, rexchar_t c);
void rex_transitions_renamedestination(rarray_t *trans, size_t dstold, size_t dstnew);
void rex_transitions_dump(rarray_t *trans);

#ifdef __cplusplus
}
#endif

#endif
