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

#ifndef _REXDFA_H_
#define _REXDFA_H_

#include "rex/rexdef.h"

#ifdef __cplusplus
extern "C" {
#endif


#define REX_DFA_DEADSTATE (0)
#define REX_DFA_STARTSTATE (1)

/*
 * Sub-state info definition
 */
typedef struct rexdfss_s {
	unsigned int type;
	unsigned long uid;
	rexuserdata_t userdata;
} rexdfss_t;


/*
 * Transition definition
 */
typedef struct rexdft_s {
	rexchar_t lowin;
	rexchar_t highin;
	unsigned long state;
} rexdft_t;


/*
 * State definition
 */
typedef struct rexdfs_s {
	unsigned long type;
	unsigned long trans;
	unsigned long ntrans;
	unsigned long substates;
	unsigned long nsubstates;
	unsigned long accsubstates;
	unsigned long naccsubstates;
} rexdfs_t;


/*
 * Automata definition
 */
typedef struct rexdfa_s {
	unsigned long nstates;
	rexdfs_t *states;
	unsigned long ntrans;
	rexdft_t *trans;
	unsigned long nsubstates;
	rexdfss_t *substates;
	unsigned long naccsubstates;
	rexdfss_t *accsubstates;
} rexdfa_t;


rexdfa_t *rex_dfa_create(unsigned long nstates, unsigned long ntrans, unsigned long nsubsets, unsigned long naccsubsets);
void rex_dfa_destroy(rexdfa_t *dfa);
void rex_dfa_dumpstate(rexdfa_t *dfa, unsigned long nstate);
rexdfs_t *rex_dfa_state(rexdfa_t *dfa, unsigned long nstate);
rexdft_t *rex_dfa_transition(rexdfa_t *dfa, unsigned long nstate, unsigned long ntransition);
rexdfss_t *rex_dfa_substate(rexdfa_t *dfa, unsigned long nstate, unsigned long nsubstate);
rexdfss_t *rex_dfa_accsubstate(rexdfa_t *dfa, unsigned long nstate, unsigned long naccsubstate);
long rex_dfa_next(rexdfa_t *dfa, unsigned long nstate, rexchar_t input);

#ifdef __cplusplus
}
#endif


#endif /* _REXDFA_H_ */
