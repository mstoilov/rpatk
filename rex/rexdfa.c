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

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "rlib/rmem.h"
#include "rlib/rstring.h"
#include "rex/rexdfa.h"


rexdfa_t *rex_dfa_create(rexuint_t nstates, rexuint_t ntrans, rexuint_t naccsubstates, rexuint_t nsubstates)
{
	rexdfa_t *dfa = (rexdfa_t *)r_zmalloc(sizeof(*dfa));
	dfa->states = r_zmalloc(sizeof(rexdfs_t) * nstates);
	dfa->trans = r_zmalloc(sizeof(rexdft_t) * ntrans);
	if (naccsubstates)
		dfa->accsubstates = r_zmalloc(sizeof(rexdfss_t) * naccsubstates);
	if (nsubstates)
		dfa->substates = r_zmalloc(sizeof(rexdfss_t) * nsubstates);
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
		r_free(dfa->bits);
		r_free(dfa);
	}
}


rexdfs_t *rex_dfa_state(rexdfa_t *dfa, rexuint_t nstate)
{
	rexdfs_t *s;
	if (nstate >= dfa->nstates)
		return NULL;
	s = REX_DFA_STATE(dfa, nstate);
	return s;
}


rexdft_t *rex_dfa_transition(rexdfa_t *dfa, rexuint_t nstate, rexuint_t ntrans)
{
	rexdft_t *t;
	rexdfs_t *s = rex_dfa_state(dfa, nstate);
	if (!s)
		return NULL;
	if (ntrans >= s->ntrans)
		return NULL;
	t = REX_DFA_TRANSITION(dfa, nstate, ntrans);
	return t;
}


rexdfss_t *rex_dfa_substate(rexdfa_t *dfa, rexuint_t nstate, rexuint_t nsubstate)
{
	rexdfss_t *ss;
	rexdfs_t *s = rex_dfa_state(dfa, nstate);
	if (!s)
		return NULL;
	if (nsubstate >= s->nsubstates)
		return NULL;
	ss = REX_DFA_SUBSTATE(dfa, nstate, nsubstate);
	return ss;
}


rexdfss_t *rex_dfa_accsubstate(rexdfa_t *dfa, rexuint_t nstate, rexuint_t naccsubstate)
{
	rexdfss_t *ss;
	rexdfs_t *s = rex_dfa_state(dfa, nstate);
	if (!s)
		return NULL;
	if (naccsubstate >= s->naccsubstates)
		return NULL;
	ss = REX_DFA_ACCSUBSTATE(dfa, nstate, naccsubstate);
	return ss;
}


rexuint_t rex_dfa_next(rexdfa_t *dfa, rexuint_t nstate, rexchar_t input)
{
	rexuint_t newstate = 0;
	REX_DFA_NEXT(dfa, nstate, input, &newstate);
	return newstate;
}


static void rex_dfa_statehash(rexdfa_t *dfa, rexuint_t state, unsigned int level, unsigned int hbits, unsigned int entry)
{
	rexdft_t *t;
	rexuint_t trans;
	rexchar_t in;
	unsigned char *bitarray = dfa->bits;
	unsigned int curentry = 0;
	unsigned int mask = (1 << hbits) - 1;
	unsigned int hbits_size = 1 << hbits;
	rexdfs_t *s = REX_DFA_STATE(dfa, state);

	if (s->type == REX_STATETYPE_ACCEPT) {
		for (curentry = 0; curentry < hbits_size; curentry++) {
			if (level) {
				rex_dfa_statehash(dfa, state, level - 1, hbits, (entry | curentry) << hbits);
			} else {
				REX_BITARRAY_SET(bitarray, entry | curentry);
			}
		}
	} else {
		for (trans = 0; trans < s->ntrans; trans++) {
			t = REX_DFA_TRANSITION(dfa, state, trans);
			if (!t->state)
				continue;
			for (in = t->lowin; in <= t->highin && (in - t->lowin) < hbits_size; in++) {
				curentry = in & mask;
				if (level) {
					rex_dfa_statehash(dfa, t->state, level - 1, hbits, (entry | curentry) << hbits);
				} else {
					REX_BITARRAY_SET(bitarray, entry | curentry);
				}
			}
		}
	}
}


int rex_dfa_hash(rexdfa_t *dfa, unsigned int hbytes, unsigned int hbits)
{
	if ((REX_DFA_HASHBITS(hbytes, hbits) - 3) > 24)
		return -1;
	if (!dfa)
		return -1;
	if (dfa->bits)
		r_free(dfa->bits);
	dfa->hbytes = hbytes;
	dfa->hbits = hbits;
	dfa->hsize = REX_DFA_HASHSIZE(hbytes, hbits) >> 3;
	dfa->bits = (unsigned char*)r_malloc(dfa->hsize);
	rex_dfa_statehash(dfa, REX_DFA_STARTSTATE, hbytes - 1, hbits, 0);
	return 0;
}


void rex_dfa_dumpstate(rexdfa_t *dfa, rexuint_t nstate)
{
	long index;
	char buf[240];
	int bufsize = sizeof(buf) - 1;
	int n = 0;
	rexdfs_t *s = rex_dfa_state(dfa, nstate);
	rexdfss_t *ss = NULL;
	rexdft_t *t = NULL;

	if (!s)
		return;
	fprintf(stdout, "State %ld", (size_t)nstate);
	if (s->nsubstates)
		fprintf(stdout, " : NFA-States(");
	for (index = 0; index < s->nsubstates; index++) {
		ss = REX_DFA_SUBSTATE(dfa, nstate, index);
		if (ss) {
			fprintf(stdout, "%ld", (size_t)ss->uid);
			if (ss->type == REX_STATETYPE_ACCEPT)
				fprintf(stdout, "*");
			if (index + 1 < s->nsubstates)
				fprintf(stdout, ",");
		}
	}
	if (s->nsubstates)
		fprintf(stdout, ")");

	fprintf(stdout, " : ");
	if (s->type == REX_STATETYPE_ACCEPT) {
		fprintf(stdout, " REX_STATETYPE_ACCEPT ");
		fprintf(stdout, " (");
		for (index = 0; index < s->naccsubstates; index++) {
			ss = REX_DFA_ACCSUBSTATE(dfa, nstate, index);
			if (ss) {
				fprintf(stdout, "%ld*", (size_t)ss->uid);
				if (index + 1 < s->naccsubstates)
					fprintf(stdout, ",");
			}
		}
		fprintf(stdout, ")");
	} else if (s->type == REX_STATETYPE_DEAD) {
		fprintf(stdout, " REX_STATETYPE_DEAD ");
	} else if (s->type == REX_STATETYPE_START) {
		fprintf(stdout, " REX_STATETYPE_START ");
	}
	fprintf(stdout, "\n");
	for (index = 0; index < s->ntrans; index++) {
		t = &dfa->trans[s->trans + index];
		n = 0;
		if (t->lowin != t->highin) {
			if (isprint(t->lowin) && !isspace(t->lowin) && isprint(t->highin) && !isspace(t->highin) && !r_strchr("[]-^", t->lowin) && !r_strchr("[]-^", t->highin))
				n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "    [%c - %c] ", t->lowin, t->highin);
			else
				n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "    [0x%X - 0x%X] ", t->lowin, t->highin);
		} else {
			if (isprint(t->lowin) && !isspace(t->lowin))
				n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "    '%c' ", t->lowin);
			else
				n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "    0x%X ", t->lowin);
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


