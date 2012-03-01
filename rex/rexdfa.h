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
#ifdef WIN32
typedef size_t rexuserdata_t;
#else
typedef unsigned long rexuserdata_t;
#endif
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
	unsigned int hbytes;
	unsigned int hbits;
	unsigned int hsize;
	unsigned char *bits;
	rexuword_t reserved[64];
} rexdfa_t;


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


/**
 * @def REX_DFA_STATE(__dfa__, __nstate__)
 *
 * Get a pointer to @ref rexdfa_t state.
 * @param __dfa__ Pointer to @ref rexdfa_t object
 * @param __nstate__ State ID returned from @ref REX_DFA_NEXT or @ref REX_DFA_DEADSTATE, @ref REX_DFA_STARTSTATE
 * @return Pointer to @ref rexdfa_t
 */
#define REX_DFA_STATE(__dfa__, __nstate__)							(&(__dfa__)->states[__nstate__])

/**
 * @def REX_DFA_TRANSITION(__dfa__, __nstate__, __ntrans__)
 * Get a pointer to @ref rexdft_t transition. This macro is used internally to find
 * a transition to the next state.
 *
 * @param __dfa__ Pointer to @ref rexdfa_t object
 * @param __nstate__ State ID returned from @ref REX_DFA_NEXT or @ref REX_DFA_DEADSTATE, @ref REX_DFA_STARTSTATE
 * @param __ntrans__ Transition offset in the array of transitions for the specified state. This parameter
 * must be from 0 to rexdfs_t::ntrans - 1.
 * @return Pointer to @ref rexdft_t transition
 */
#define REX_DFA_TRANSITION(__dfa__, __nstate__, __ntrans__)			(&(__dfa__)->trans[(REX_DFA_STATE(__dfa__, __nstate__)->trans) + (__ntrans__)])

/**
 * @def REX_DFA_SUBSTATE(__dfa__, __nstate__, __nsubstate__)
 * Get a pointer to @ref rexdfss_t sub-state. This macro would only work if the DFA
 * is generated with its NFA sub-states.
 *
 * @param __dfa__ Pointer to @ref rexdfa_t object
 * @param __nstate__ State ID returned from @ref REX_DFA_NEXT or @ref REX_DFA_STARTSTATE
 * @param __nsubstate__ Sub-state offset in the array of sub-states for the specified state. This parameter
 * must from 0 to rexdfs_t::nsubstates - 1.
 * @return Pointer to @ref rexdfss_t substate.
 */
#define REX_DFA_SUBSTATE(__dfa__, __nstate__, __nsubstate__)		((__dfa__)->substates ? &(__dfa__)->substates[REX_DFA_STATE(__dfa__, __nstate__)->substates + (__nsubstate__)] : ((rexdfss_t*)0))

/**
 * @def REX_DFA_ACCSUBSTATE(__dfa__, __nstate__, __naccsubstate__)
 * Get a pointer to @ref rexdfss_t accepting sub-state.
 *
 * @param __dfa__ Pointer to @ref rexdfa_t object
 * @param __nstate__ State ID returned from @ref REX_DFA_NEXT or @ref REX_DFA_STARTSTATE
 * @param __naccsubstate__ Sub-state offset in the array of accepting sub-states for the specified state. This parameter
 * must be from 0 to rexdfs_t::naccsubstates - 1.
 * @return Pointer to @ref rexdfss_t accepting substate.
 */
#define REX_DFA_ACCSUBSTATE(__dfa__, __nstate__, __naccsubstate__)	((__dfa__)->accsubstates ? &(__dfa__)->accsubstates[REX_DFA_STATE(__dfa__, __nstate__)->accsubstates + (__naccsubstate__)] : ((rexdfss_t*)0))


/**
 * @def REX_DFA_NEXT(__dfa__, __nstate__, __input__, __nextptr__)
 *
 * Get the next state ID in the DFA for the specified input. The macro will
 * search through the transitions of the current state to find the next
 * state of the DFA for the specified input. The next state will be assigned
 * to *(__nextptr__).
 *
 * @param __dfa__ Pointer to @ref rexdfa_t object
 * @param __nstate__ Current state of the DFA.
 * @param __input__ Current input
 * @param __nextptr__ Output the next state in here
 * @return The next state of the DFA for the specified input is written in __nextptr__
 */
#define REX_DFA_NEXT(__dfa__, __nstate__, __input__, __nextptr__) \
		do { \
			rexdft_t *t; \
			rexuint_t mid, min = 0, max = REX_DFA_STATE(__dfa__, __nstate__)->ntrans; \
			while (max > min) { \
				mid = (min + max)/2; \
				t = REX_DFA_TRANSITION(__dfa__, __nstate__, mid); \
				if ((__input__) >= t->lowin) { \
					min = mid + 1; \
				} else { \
					max = mid; \
				} \
			} \
			t = REX_DFA_TRANSITION(__dfa__, __nstate__, min-1); \
			*(__nextptr__) = t->state; \
		} while (0)


#define REX_DFA_HASHBITS(__bytes__, __bitsperbyte__) ((__bytes__) * (__bitsperbyte__))
#define REX_DFA_HASHSIZE(__bytes__, __bitsperbyte__) (1 << REX_DFA_HASHBITS(__bytes__, __bitsperbyte__))
#define REX_DFA_HASHMASK(__bytes__, __bitsperbyte__) (REX_DFA_HASHSIZE(__bytes__, __bitsperbyte__) - 1)

#define REX_BITARRAY_BYTE(__arr__, __entry__) (((unsigned char*)__arr__)[((unsigned int)(__entry__))>>3])
#define REX_BITARRAY_GET(__arr__, __entry__) ((((unsigned char*)__arr__)[((unsigned int)(__entry__))>>3] & (1 << (((unsigned int)(__entry__)) & 0x7))) ? 1 : 0)
#define REX_BITARRAY_SET(__arr__, __entry__) do { ((unsigned char*)__arr__)[((unsigned int)(__entry__))>>3] |= (1 << (((unsigned int)(__entry__)) & 0x7)); } while (0)
#define REX_BITARRAY_CLR(__arr__, __entry__) do { ((unsigned char*)__arr__)[((unsigned int)(__entry__))>>3] &= ~(1 << (((unsigned int)(__entry__)) & 0x7)); } while (0)


rexdfa_t *rex_dfa_create(rexuint_t nstates, rexuint_t ntrans, rexuint_t naccsubstates, rexuint_t nsubstates);
void rex_dfa_destroy(rexdfa_t *dfa);
int rex_dfa_hash(rexdfa_t *dfa, unsigned int hbytes, unsigned int hbits);
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
