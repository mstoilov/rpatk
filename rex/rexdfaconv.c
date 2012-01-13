#include "rlib/rmem.h"
#include "rexdfaconv.h"
#include "rextransition.h"


rexdfaconv_t *rex_dfaconv_create()
{
	rexdfaconv_t *co;

	co = (rexdfaconv_t *)r_malloc(sizeof(*co));
	r_memset(co, 0, sizeof(*co));
	co->stack = r_array_create(sizeof(unsigned long));
	co->setT = r_array_create(sizeof(unsigned long));
	co->setU = r_array_create(sizeof(unsigned long));
	co->setV = r_array_create(sizeof(unsigned long));
	co->trans = r_array_create(sizeof(rex_transition_t));
	return co;
}


void rex_dfaconv_destroy(rexdfaconv_t *co)
{
	if (co) {
		r_array_destroy(co->stack);
		r_array_destroy(co->setT);
		r_array_destroy(co->setU);
		r_array_destroy(co->setV);
		r_array_destroy(co->trans);
	}
	r_free(co);
}


static void rex_dfaconv_addsubstate(rarray_t *set, unsigned long uid)
{
	unsigned long substate;
	long min, max, mid;

	min = 0;
	max = min + r_array_length(set);
	while (max > min) {
		mid = (min + max)/2;
		substate = rex_subset_index(set, mid);
		if (uid == substate) {
			/*
			 * We already have it in the subset.
			 */
			return;
		} else if (uid >= substate) {
			min = mid + 1;
		} else {
			max = mid;
		}
	}
	rex_subset_insert(set, min, uid);
}


static long rex_dfaconv_setsubstates(rexstate_t *state, rexdb_t *nfa, rarray_t *set)
{
	long i;
	unsigned long u;
	rexstate_t *s;

	rex_subset_clear(state->subset);
	for (i = 0; i < r_array_length(set); i++) {
		u = rex_subset_index(set, i);
		rex_dfaconv_addsubstate(state->subset, u);
		if (rex_db_getstate(nfa, u)->type == REX_STATETYPE_ACCEPT)
			state->type = REX_STATETYPE_ACCEPT;
	}
	return rex_subset_length(state->subset);
}


static void rex_dfaconv_eclosure(rexdfaconv_t *co, rexdb_t *nfa, const rarray_t *src, rarray_t *dest)
{
	long i, j;
	unsigned long uid;
	rexstate_t *s;
	rex_transition_t *t;

	/*
	 * Empty the dest and temp stack(setT).
	 */
	rex_subset_clear(dest);
	rex_subset_clear(co->setT);
	for (i = 0; i < r_array_length(src); i++) {
		uid = r_array_index(src, i, unsigned long);
		rex_dfaconv_addsubstate(co->setT, uid);
		rex_dfaconv_addsubstate(dest, uid);
	}
	while (!r_array_empty(co->setT)) {
		uid = r_array_pop(co->setT, unsigned long);
		s = rex_db_getstate(nfa, uid);
		for (j = 0; j < r_array_length(s->etrans); j++) {
			t = (rex_transition_t *)r_array_slot(s->etrans, j);
			rex_dfaconv_addsubstate(co->setT, t->dstuid);
			rex_dfaconv_addsubstate(dest, t->dstuid);
		}
	}
}


static void rex_dfaconv_getsubsettransitions(rexdfaconv_t *co, rexdb_t *nfa, rarray_t *states, rarray_t *trans)
{
	long i, j;
	long uid;
	rexstate_t *s;
	rex_transition_t *t;

	r_array_setlength(trans, 0);
	for (i = 0; i < r_array_length(states); i++) {
		uid = rex_subset_index(states, i);
		s = rex_db_getstate(nfa, uid);
		for (j = 0; j < r_array_length(s->trans); j++) {
			t = (rex_transition_t *)r_array_slot(s->trans, j);
			r_array_add(trans, t);
		}
	}
}


static void rex_dfaconv_move(rexdfaconv_t *co, rexdb_t *nfa, const rarray_t *states, unsigned long c, rarray_t *moveset)
{
	long i, j;
	long uid;
	rexstate_t *s;
	rex_transition_t *t;

	rex_subset_clear(moveset);
	for (i = 0; i < r_array_length(states); i++) {
		uid = rex_subset_index(states, i);
		s = rex_db_getstate(nfa, uid);
		for (j = 0; j < r_array_length(s->trans); j++) {
			t = (rex_transition_t *)r_array_slot(s->trans, j);
			if (t->type == REX_TRANSITION_INPUT && t->lowin == c) {
				rex_dfaconv_addsubstate(moveset, t->dstuid);
			} else if (t->type == REX_TRANSITION_RANGE && t->lowin <= c && t->highin >= c) {
				rex_dfaconv_addsubstate(moveset, t->dstuid);
			}
		}
	}
}


rexdb_t *rex_dfaconv_run(rexdfaconv_t *co, rexdb_t *nfa, unsigned long start)
{
	long i, j;
	rexdb_t *dfa = rex_db_create(REXDB_TYPE_DFA);
	rexstate_t *s, *nextstate;
	rex_transition_t *t;
	long uid = rex_db_createstate(dfa, REX_STATETYPE_START);

	s = rex_db_getstate(dfa, uid);
	rex_subset_clear(co->setU);
	rex_subset_push(co->setU, start);
	rex_dfaconv_eclosure(co, nfa, co->setU, s->subset);

	for (i = 0; i < r_array_length(dfa->states); i++) {
		s = r_array_index(dfa->states, i, rexstate_t*);
		rex_dfaconv_getsubsettransitions(co, nfa, s->subset, co->trans);
		for (j = 0; j < r_array_length(co->trans); j++) {
			t = (rex_transition_t *)r_array_slot(co->trans, j);
			if (t->type != REX_TRANSITION_INPUT && t->type != REX_TRANSITION_RANGE) {
				/*
				 * Error. This should never happen!
				 */
				continue;
			}
			rex_dfaconv_move(co, nfa, s->subset, t->lowin, co->setU);
			if (!rex_subset_length(co->setU))
				continue;
			rex_dfaconv_eclosure(co, nfa, co->setU, co->setV);
			if ((uid = rex_db_findstate(dfa, co->setV)) < 0) {
				uid = rex_db_createstate(dfa, REX_STATETYPE_NONE);
				nextstate = rex_db_getstate(dfa, uid);
				rex_dfaconv_setsubstates(nextstate, nfa, co->setV);
			}
			if (t->type == REX_TRANSITION_RANGE)
				rex_state_addrangetransition(s, t->lowin, t->highin, uid);
			else
				rex_state_addtransition(s, t->lowin, uid);
		}
		rex_state_normalizetransitions(s);
	}
	return dfa;
}
