#include "rlib/rmem.h"
#include "rexdfaconv.h"
#include "rextransition.h"


rexdfaconv_t *rex_dfaconv_create()
{
	rexdfaconv_t *co;

	co = (rexdfaconv_t *)r_malloc(sizeof(*co));
	r_memset(co, 0, sizeof(*co));
	co->setT = r_array_create(sizeof(rexsubstate_t));
	co->setU = r_array_create(sizeof(rexsubstate_t));
	co->setV = r_array_create(sizeof(rexsubstate_t));
	co->trans = r_array_create(sizeof(rex_transition_t));
	co->temptrans = r_array_create(sizeof(rex_transition_t));
	return co;
}


void rex_dfaconv_destroy(rexdfaconv_t *co)
{
	if (co) {
		r_array_destroy(co->setT);
		r_array_destroy(co->setU);
		r_array_destroy(co->setV);
		r_array_destroy(co->trans);
		r_array_destroy(co->temptrans);
	}
	r_free(co);
}


static long rex_dfaconv_setsubstates(rexstate_t *state, rexdb_t *nfa, rarray_t *set)
{
	long i;
	unsigned long u;
	rexstate_t *s;

	rex_subset_clear(state->subset);
	for (i = 0; i < r_array_length(set); i++) {
		u = rex_subset_slot(set, i)->ss_uid;
		s = rex_db_getstate(nfa, u);
		rex_state_addsubstate(state, s);
		if (s->type == REX_STATETYPE_ACCEPT)
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
		uid = rex_subset_slot(src, i)->ss_uid;
		rex_subset_addnewsubstate(co->setT, uid, REX_STATETYPE_NONE, NULL);
		rex_subset_addnewsubstate(dest, uid, REX_STATETYPE_NONE, NULL);
	}
	while (!r_array_empty(co->setT)) {
		rexsubstate_t subst = rex_subset_pop(co->setT);
		uid = subst.ss_uid;
		s = rex_db_getstate(nfa, uid);
		for (j = 0; j < r_array_length(s->etrans); j++) {
			t = (rex_transition_t *)r_array_slot(s->etrans, j);
			rex_subset_addnewsubstate(co->setT, t->dstuid, REX_STATETYPE_NONE, NULL);
			rex_subset_addnewsubstate(dest, t->dstuid, REX_STATETYPE_NONE, NULL);
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
		uid = rex_subset_slot(states, i)->ss_uid;
		s = rex_db_getstate(nfa, uid);
		for (j = 0; j < r_array_length(s->trans); j++) {
			t = (rex_transition_t *)r_array_slot(s->trans, j);
			r_array_add(trans, t);
		}
	}
}


static void rex_dfaconv_move(rexdfaconv_t *co, rexdb_t *nfa, const rarray_t *substates, unsigned long c1, unsigned long c2, rarray_t *moveset)
{
	long i, j;
	long uid;
	rexstate_t *s;
	rex_transition_t *t;

	rex_subset_clear(moveset);
	for (i = 0; i < r_array_length(substates); i++) {
		uid = rex_subset_slot(substates, i)->ss_uid;
		s = rex_db_getstate(nfa, uid);
		for (j = 0; j < r_array_length(s->trans); j++) {
			t = (rex_transition_t *)r_array_slot(s->trans, j);
			if ((c1 >= t->lowin && c1 <= t->highin)) {
				rex_subset_addnewsubstate(moveset, t->dstuid, REX_STATETYPE_NONE, NULL);
			}


//			if (t->type == REX_TRANSITION_INPUT && t->lowin == c1) {
//				rex_subset_addnewsubstate(moveset, t->dstuid, REX_STATETYPE_NONE, NULL);
//			} else if (t->type == REX_TRANSITION_RANGE && t->lowin <= c1 && t->highin >= c1) {
//				rex_subset_addnewsubstate(moveset, t->dstuid, REX_STATETYPE_NONE, NULL);
//			}
		}
	}
}


static void rex_dfaconv_insertdeadtransitions(rexdfaconv_t *co, rexstate_t *state)
{
	long i;
	rex_transition_t *t;

	r_array_setlength(co->temptrans, 0);
	for (i = 0; i < r_array_length(state->trans); i++) {
		t = (rex_transition_t *)r_array_slot(state->trans, i);
		rex_transitions_add(co->temptrans, t->lowin, t->highin, state->uid, 0);
	}
	rex_transitions_normalize(co->temptrans);
	rex_transitions_negative(state->trans, co->temptrans, state->uid, 0);
	rex_transitions_normalize(state->trans);
}


rexdb_t *rex_dfaconv_run(rexdfaconv_t *co, rexdb_t *nfa, unsigned long start)
{
	long i, j, k;
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
		rex_transitions_dump(co->trans);
		for (j = 0; j < r_array_length(co->trans); j++) {
			t = (rex_transition_t *)r_array_slot(co->trans, j);
			if (t->type != REX_TRANSITION_INPUT && t->type != REX_TRANSITION_RANGE) {
				/*
				 * Error. This should never happen!
				 */
				continue;
			}
			for (k = t->lowin; k <= t->highin; k++) {
				rex_dfaconv_move(co, nfa, s->subset, k, k, co->setU);
				if (!rex_subset_length(co->setU))
					continue;
				rex_dfaconv_eclosure(co, nfa, co->setU, co->setV);
				if ((uid = rex_db_findstate(dfa, co->setV)) < 0) {
					uid = rex_db_createstate(dfa, REX_STATETYPE_NONE);
					nextstate = rex_db_getstate(dfa, uid);
					rex_dfaconv_setsubstates(nextstate, nfa, co->setV);
				}
				rex_state_addtransition(s, k, uid);
			}
		}
		rex_transitions_normalize(s->trans);
//		rex_dfaconv_insertdeadtransitions(co, s);
	}
	return dfa;
}
