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

typedef enum {
	REX_STATETYPE_NONE = 0,
	REX_STATETYPE_START = 1,
	REX_STATETYPE_ACCEPT = 2,
	REX_STATETYPE_DEAD = 3,
} rex_statetype_t;

#define REX_DFA_DEADSTATE (0)
#define REX_DFA_STARTSTATE (1)

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


/*
 * Sub-state info definition
 */
typedef struct rexdfss_s {
	rexuint_t type;
	rexuint_t uid;
	rexuserdata_t userdata;
} rexdfss_t;


/*
 * Transition definition
 */
typedef struct rexdft_s {
	rexchar_t lowin;
	rexchar_t highin;
	rexuint_t state;
} rexdft_t;


/*
 * State definition
 */
typedef struct rexdfs_s {
	rexuint_t type;
	rexuint_t trans;
	rexuint_t ntrans;
	rexuint_t accsubstates;
	rexuint_t naccsubstates;
	rexuint_t substates;
	rexuint_t nsubstates;
} rexdfs_t;


/*
 * Automata definition
 */
typedef struct rexdfa_s {
	rexuint_t nstates;
	rexdfs_t *states;
	rexuint_t ntrans;
	rexdft_t *trans;
	rexuint_t naccsubstates;
	rexdfss_t *accsubstates;
	rexuint_t nsubstates;
	rexdfss_t *substates;
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
