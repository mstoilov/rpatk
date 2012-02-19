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

/**
 * @file rex/rexdfa.h
 * @brief Definition of DFA interface
 *
 *
 * <h2>Synopsis</h2>
 * This file defines the data structures and the macros for the DFA implementation.
 */

#ifndef _REXDFA_H_
#define _REXDFA_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef REX_USERDATA_TYPE
typedef unsigned long rexuserdata_t;
#else
typedef REX_USERDATA_TYPE rexuserdata_t;
#endif

#ifndef REX_UWORD_TYPE
typedef unsigned long rexuword_t;
#else
typedef REX_UWORD_TYPE rexuword_t;
#endif

#ifndef REX_UINT_TYPE
typedef unsigned int rexuint_t;
#else
typedef REX_UINT_TYPE rexuint_t;
#endif


#ifndef REX_CHAR_TYPE
typedef unsigned int rexchar_t;
#else
typedef REX_CHAR_TYPE rexchar_t;
#endif
#define REX_CHAR_MAX ((rexchar_t)-1)
#define REX_CHAR_MIN ((rexchar_t)0)

/**
 * Definition of state types.
 */
typedef enum {
	REX_STATETYPE_NONE = 0,			/**< There is nothing interesting about this state */
	REX_STATETYPE_START = 1,		/**< This state is marked as starting point for the automaton */
	REX_STATETYPE_ACCEPT = 2,		/**< This type indicates that one or more regular expressions compiled in the automaton matched. */
	REX_STATETYPE_DEAD = 3,			/**< The automaton is in the dead state(all transitions lead back to the same state) */
} rex_statetype_t;


#define REX_DFA_DEADSTATE (0)		/**< DFA Dead State ID, In rexdfa_t object the state at offset 0 is always the dead state  */
#define REX_DFA_STARTSTATE (1)		/**< DFA Start State ID, In rexdfa_t object the start state is always at offset 1  */

#define REX_DFA_STATE(__dfa__, __nstate__)							(&(__dfa__)->states[__nstate__])
#define REX_DFA_TRANSITION(__dfa__, __nstate__, __ntrans__)			(&(__dfa__)->trans[(REX_DFA_STATE(__dfa__, __nstate__)->trans) + (__ntrans__)])
#define REX_DFA_SUBSTATE(__dfa__, __nstate__, __nsubstate__)		((__dfa__)->substates ? &(__dfa__)->substates[REX_DFA_STATE(__dfa__, __nstate__)->substates + (__nsubstate__)] : ((rexdfss_t*)0))
#define REX_DFA_ACCSUBSTATE(__dfa__, __nstate__, __naccsubstate__)	((__dfa__)->accsubstates ? &(__dfa__)->accsubstates[REX_DFA_STATE(__dfa__, __nstate__)->accsubstates + (__naccsubstate__)] : ((rexdfss_t*)0))
#define REX_DFA_NEXT(__dfa__, __nstate__, __input__) \
		({ \
			rexdft_t *t; \
			rexuint_t mid, min = 0, max = REX_DFA_STATE(__dfa__, __nstate__)->ntrans; \
			while (max > min) { \
				mid = (min + max)/2; \
				t = REX_DFA_TRANSITION(__dfa__, nstate, mid); \
				if ((__input__) >= t->lowin) { \
					min = mid + 1; \
				} else { \
					max = mid; \
				} \
			} \
			t = REX_DFA_TRANSITION(__dfa__, __nstate__, min-1); \
			(t->state); \
		})



/**
 * Define DFA sub-state.
 */
typedef struct rexdfss_s {
	rexuint_t type;					/**< This is the original NFA state type(substate of a DFA state). */
	rexuint_t uid;					/**< Unique ID of the NFA state(substate of a DFA state). */
	rexuserdata_t userdata;			/**< If this is an accepting sub-state, this parameter has the value specified in the @ref rex_db_addexpression call.
									 	 This parameter is used to track which regular expression is matching, when the DFA gets to accepting state. */
} rexdfss_t;


/**
 * Define DFA transition.
 */
typedef struct rexdft_s {
	rexchar_t lowin;				/**< Low input boundary */
	rexchar_t highin;				/**< High input boundary */
	rexuint_t state;				/**< New state to go to if the input is within the boundary */
} rexdft_t;


/**
 * State definition
 */
typedef struct rexdfs_s {
	rexuint_t type;					/**< Type of DFA state. */
	rexuint_t trans;				/**< The offset of the first transition for this state in the dfa->trans array. */
	rexuint_t ntrans;				/**< Total number of transitions. */
	rexuint_t accsubstates;			/**< The offset of the first accepting sub-state for this state in the dfa->accsubstates array. */
	rexuint_t naccsubstates;		/**< Total number of accepting sub-states. */
	rexuint_t substates;			/**< The offset of the first sub-state for this state in the dfa->substates array. */
	rexuint_t nsubstates;			/**< Total number of sub-states. */
} rexdfs_t;


/**
 * Define DFA.
 */
typedef struct rexdfa_s {
	rexuint_t nstates;				/**< Total number of states. */
	rexdfs_t *states;				/**< Array of states */
	rexuint_t ntrans;				/**< Total number of transitions */
	rexdft_t *trans;				/**< Array of transitions, all states transitions are recorded here. Each state keeps the offset of its first transition it this array */
	rexuint_t naccsubstates;		/**< Total number of accepting substates */
	rexdfss_t *accsubstates;		/**< Array of accepting sub-states, all states accepting sub-states are recorded here. */
	rexuint_t nsubstates;			/**< Total number of substates */
	rexdfss_t *substates;			/**< Array of sub-states, all states sub-states are recorded here. */
	rexuword_t reserved[64];
} rexdfa_t;


rexdfa_t *rex_dfa_create(rexuint_t nstates, rexuint_t ntrans, rexuint_t naccsubstates, rexuint_t nsubstates);
void rex_dfa_destroy(rexdfa_t *dfa);
void rex_dfa_dumpstate(rexdfa_t *dfa, rexuint_t nstate);
rexdfs_t *rex_dfa_state(rexdfa_t *dfa, rexuint_t nstate);
rexdft_t *rex_dfa_transition(rexdfa_t *dfa, rexuint_t nstate, rexuint_t ntransition);
rexdfss_t *rex_dfa_substate(rexdfa_t *dfa, rexuint_t nstate, rexuint_t nsubstate);
rexdfss_t *rex_dfa_accsubstate(rexdfa_t *dfa, rexuint_t nstate, rexuint_t naccsubstate);
rexuint_t rex_dfa_next(rexdfa_t *dfa, rexuint_t nstate, rexchar_t input);

#ifdef __cplusplus
}
#endif


#endif /* _REXDFA_H_ */
