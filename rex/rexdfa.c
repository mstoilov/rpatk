/*
 * rexdfa.c
 *
 *  Created on: Jan 31, 2012
 *      Author: mstoilov
 */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "rlib/rmem.h"
#include "rex/rexdfa.h"


struct rexdfa_ctx {
	unsigned long nstates;
	unsigned long ntrnas;
	unsigned long nsubstates;
	unsigned long naccsubstates;
};


rexdfa_t *rex_dfa_create(unsigned long nstates, unsigned long ntrans, unsigned long nsubstates, unsigned long naccsubstates)
{
	rexdfa_t *dfa = (rexdfa_t *)r_zmalloc(sizeof(*dfa));
	dfa->states = r_zmalloc(sizeof(rexdfs_t) * nstates);
	dfa->trans = r_zmalloc(sizeof(rexdft_t) * ntrans);
	dfa->substates = r_zmalloc(sizeof(rexsi_t) * nsubstates);
	dfa->accsubstates = r_zmalloc(sizeof(rexsi_t) * naccsubstates);
	dfa->nstates = nstates;
	dfa->ntrans = ntrans;
	dfa->nsubstates = nsubstates;
	dfa->naccsubstates = naccsubstates;
	return dfa;
}


void rex_dfa_destroy(rexdfa_t *dfa)
{
	if (dfa) {
		r_free(dfa->substates);
		r_free(dfa->accsubstates);
		r_free(dfa->states);
		r_free(dfa->trans);
	}
}


static void rex_dfa_fillstate(rexdfa_t *dfa, struct rexdfa_ctx *ctx, rexstate_t *state)
{
	long i;
	rex_transition_t *t = NULL;
	rexdfs_t *s = &dfa->states[ctx->nstates++];
	s->type = state->type;
	s->trans = &dfa->trans[ctx->ntrnas];
	s->ntrans = r_array_length(state->trans);
	for (i = 0; i < s->ntrans; i++) {
		t = (rex_transition_t *)r_array_slot(state->trans, i);
		s->trans[i].lowin = t->lowin;
		s->trans[i].highin = t->highin;
		s->trans[i].state = t->dstuid;
	}
	ctx->ntrnas += s->ntrans;
}


rexdfa_t *rex_dfa_create_from_db(rexdb_t *db)
{
	long i;
	rexdfa_t *dfa;
	struct rexdfa_ctx ctx;
	unsigned long nstates = rex_db_numstates(db);
	unsigned long ntrans = rex_db_numtransitions(db);
	unsigned long nsubstates = rex_db_numsubstates(db);
	dfa = rex_dfa_create(nstates, ntrans, nsubstates, nsubstates);
	r_memset(&ctx, 0, sizeof(ctx));

	for (i = 0; i < r_array_length(db->states); i++) {
		rexstate_t *state = rex_db_getstate(db, i);
		rex_dfa_fillstate(dfa, &ctx, state);
	}
	return dfa;
}


void rex_dfa_dumpstate(rexdfa_t *dfa, long off)
{
	long index;
	char buf[240];
	int bufsize = sizeof(buf) - 1;
	int n = 0;
	rexdfs_t *s = &dfa->states[off];
	rexdft_t *t = NULL;

	fprintf(stdout, "State %ld", off);

	fprintf(stdout, ": ");
	if (s->type == REX_STATETYPE_ACCEPT) {
		fprintf(stdout, " REX_STATETYPE_ACCEPT ");
	} else if (s->type == REX_STATETYPE_DEAD) {
		fprintf(stdout, " REX_STATETYPE_DEAD ");
	} else if (s->type == REX_STATETYPE_START) {
		fprintf(stdout, " REX_STATETYPE_START ");
	}
	fprintf(stdout, "\n");
	for (index = 0; index < s->ntrans; index++) {
		t = &s->trans[index];
		n = 0;
		if (t->lowin != t->highin) {
			if (isprint(t->lowin) && !isspace(t->lowin) && isprint(t->highin) && !isspace(t->highin))
				n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "        [%c - %c] ", t->lowin, t->highin);
			else
				n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "        [0x%X - 0x%X] ", t->lowin, t->highin);
		} else {
			if (isprint(t->lowin) && !isspace(t->lowin))
				n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "        '%c' ", t->lowin);
			else
				n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "        0x%X ", t->lowin);
		}
		r_memset(buf + n, ' ', bufsize - n);
		n = 40;
		n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "-> %ld", t->state);
		fprintf(stdout, "%s\n", buf);
	}
	if (!s->ntrans)
		fprintf(stdout, "        (none)\n");
	fprintf(stdout, "\n");

}


