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
 * @file rex/rexdb.h
 * @brief Public interface for creating regular expressions.
 *
 *
 * <h2>Synopsis</h2>
 * The following APIs are used to create an automata object, based on regular expressions.
 */

#ifndef _REXDB_H_
#define _REXDB_H_

#include "rtypes.h"
#include "rlib/robject.h"
#include "rlib/rarray.h"
#include "rex/rexstate.h"
#include "rex/rexdfa.h"


#ifdef __cplusplus
extern "C" {
#endif

#define R_OBJECT_REXDB 35

/**
 * @example js-tokenizer.c
 * This is an example how to use rexdb_t and rexdfa_t
 */

/**
 * Define rexdb_t type. It could either REXDB_TYPE_NFA or REXDB_TYPE_DFA
 */
typedef enum {
	REXDB_TYPE_NFA = 0,			/**< The automaton is NFA, empty transitions are allowed. */
	REXDB_TYPE_DFA = 1,			/**< The automaton is DFA, there are no empty transitions. */
} rexdb_type_t;

struct rexcompiler_s;

/**
 * Define the rexdb_t type. This structure is used to create and manage the states
 * of the finite automata (NFA or DFA depending on the type). If the automaton is DFA
 * the sub-states member will contain information about the NFA states that produced
 * this DFA.
 */
typedef struct rexdb_s {
	robject_t obj;				/**< Base class */
	rexdb_type_t type;			/**< Automata type NFA or DFA */
	struct rexcompiler_s *co;	/**< Pointer to a compiler object used for the parsing of regular expression string and turning it into NFA states. */
	rarray_t *states;			/**< Array holding all the states of the automata */
	rarray_t *substates;		/**< Used only if the type is REXDB_TYPE_DFA. */
	rexstate_t *start;
} rexdb_t;


/**
 * Create a new empty object of type rexdb_t
 *
 * @param type This is REXDB_TYPE_NFA or REXDB_TYPE_DFA
 * @return Empty automata object. You should never need
 * to create an object of type REXDB_TYPE_DFA directly with
 * this function, instead use @ref rex_db_createdfa.
 *
 */
rexdb_t *rex_db_create(rexdb_type_t type);

/**
 * Create a new DFA object of type rexdb_t, constructed from the
 * states of the NFA, passed as parameter.
 *
 * @param nfa This is REXDB_TYPE_NFA type automata object used to construct the DFA.
 * @param start Start state of the NFA.
 * @return DFA object.
 */
rexdb_t *rex_db_createdfa(rexdb_t *nfa, unsigned long start);

/**
 * This function is used to destroy @ref rexdb_t objects, created with
 * @ref rex_db_create or @ref rex_db_createdfa.
 */
void rex_db_destroy(rexdb_t *rexdb);

/**
 * This function is use to add new regular expression to the NFA.
 * All expression added with this create a union.
 * @param nfa NFA object.
 * @param prev This is the previous start state of the automata, returned from a previous call to this function.
 * 				If this is the first call to this function prev is ignored.
 * @param str Regular expression string.
 * @param size The size of the string to be parsed.
 * @param userdata The value of this parameter is stored in the accepting state of the NFA(which also becomes
 * a sub-state in an accepting DFA state). You can use this value to identify which of the many regular expressions
 * compiled into the automaton is actually matching. A DFA state can have multiple sub-states, this means it can have
 * multiple accepting sub-states(multiple regular expressions matched). You can examine the values of the userdata
 * for these states to find out which are the underlying regular expressions.
 *
 * @return New starting state for the automaton.
 */
long rex_db_addexpression(rexdb_t *nfa, unsigned long prev, const char *str, unsigned int size, rexuserdata_t userdata);

/**
 * This functions is the same as @ref rex_db_addexpression, but it assumes the str parameter is 0 terminated string.
 */
long rex_db_addexpression_s(rexdb_t *nfa, unsigned long prev, const char *str, rexuserdata_t userdata);


robject_t *rex_db_init(robject_t *obj, unsigned int objtype, r_object_cleanupfun cleanup, rexdb_type_t type);
long rex_db_createstate(rexdb_t *rexdb, rex_statetype_t type);
long rex_db_insertstate(rexdb_t *rexdb, rexstate_t *s);
long rex_db_findstate(rexdb_t *rexdb, const rarray_t *subset);
rexstate_t *rex_db_getstate(rexdb_t *rexdb, long uid);
rexsubstate_t *rex_db_getsubstate(rexdb_t *rexdb, unsigned long uid);
rex_transition_t * rex_db_addrangetrasition(rexdb_t *rexdb, rexchar_t c1, rexchar_t c2, unsigned long srcuid, unsigned long dstuid);
rex_transition_t * rex_db_addtrasition_e(rexdb_t *rexdb, unsigned long srcuid, unsigned long dstuid);
void rex_db_dumpstate(rexdb_t *rexdb, unsigned long uid);
long rex_db_numtransitions(rexdb_t *rexdb);
long rex_db_numstates(rexdb_t *rexdb);
long rex_db_numsubstates(rexdb_t *rexdb);
long rex_db_numaccsubstates(rexdb_t *rexdb);
const char *rex_db_version();

/**
 * @brief Convert @ref rexdb_t of type REXDB_TYPE_DFA to @ref rexdfa_t object.
 *
 * This function is used to generate rexdfa_t object from a rexdb_t. The rexdfa_t
 * has a smaller memory footprint and is easier to use. Also, you have the option
 * to eliminate the sub-states because they tend to take a lot of memory and are
 * pretty much useless once once the DFA is constructed.
 *
 * @param db Pointer to the rexdb_t DFA to be converted.
 * @param withsubstates Supported values are:
 * 			- 0 No sub-states information will be written.
 * 			- 1 Sub-states information from the underlying NFA will be written.
 * 	@return Pointer to rexdfa_t object or NULL on error. If the functions succeeds, the return
 * 	object must be destroyed with @ref rex_dfa_destroy when not needed any more.
 */
rexdfa_t *rex_db_todfa(rexdb_t *db, int withsubstates);


/*
 * Virtual methods implementation
 */
void rex_db_cleanup(robject_t *obj);


/**
 * @example js-tokenizer.c
 */


#ifdef __cplusplus
}
#endif
#endif
