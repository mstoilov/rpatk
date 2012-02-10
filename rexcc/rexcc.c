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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

/*
 * Temporary here. Need to fix the encoding definitions.
 */
#include "rpa/rpastat.h"

#include "rlib/rutf.h"
#include "rlib/rmem.h"
#include "rexcc.h"




rexcc_t *rex_cc_create()
{
	rexcc_t *pCC;
	
	pCC = (rexcc_t *)r_malloc(sizeof(*pCC));
	if (!pCC)
		return (void *)0;
	r_memset(pCC, 0, sizeof(*pCC));
	pCC->nfa = rex_db_create(REXDB_TYPE_NFA);
	pCC->startuid = -1L;
	return pCC;
}


void rex_cc_destroy(rexcc_t *pCC)
{
	if (!pCC)
		return;
	rex_db_destroy(pCC->nfa);
	rex_dfa_destroy(pCC->dfa);
	r_free(pCC);
}


int rex_cc_load_string_pattern(rexcc_t *pCC, rbuffer_t *buf, rexuserdata_t userdata)
{
	return rex_cc_load_pattern(pCC, buf, userdata);
}


int rex_cc_load_pattern(rexcc_t *pCC, rbuffer_t *buf, rexuserdata_t userdata)
{
	pCC->startuid = rex_db_addexpression(pCC->nfa, pCC->startuid, buf->s, buf->size, userdata);
	if (pCC->startuid < 0) {
		return -1;
	}
	return 0;
}


int rex_cc_vfprintf(FILE *out, int indent, const char *format, va_list ap)
{
	while (indent > 0) {
		fprintf(out, "\t");
		--indent;
	}
	return vfprintf(out, format, ap);
}


int rex_cc_fprintf(FILE *out, int indent, const char *format, ...)
{
	va_list ap;
	int ret;

	va_start(ap, format);
	ret = rex_cc_vfprintf(out, indent, format, ap);
	va_end(ap);
	return ret;
}


static int rex_cc_output_statesubstates(rexcc_t *pCC, FILE *out, long nstate)
{
	long i;
	rexdfa_t *dfa = pCC->dfa;
	rexdfs_t *s = rex_dfa_state(dfa, nstate);
	rexdfss_t *ss;
	for (i = 0; i < s->nsubstates; i++) {
		ss = rex_dfa_substate(dfa, nstate, i);
		rex_cc_fprintf(out, 1, "{ %16lu, %16lu, %16lu },\n", ss->type, ss->uid, (unsigned long)ss->userdata);
	}
	return 0;
}


static int rex_cc_output_substates(rexcc_t *pCC, FILE *out)
{
	long i;
	rexdfa_t *dfa = pCC->dfa;

	rex_cc_fprintf(out, 0, "static rexdfss_t substates[] = {\n");
	for (i = 0; i < dfa->nstates; i++)
		rex_cc_output_statesubstates(pCC, out, i);
	rex_cc_fprintf(out, 1, "{ %16lu, %16lu, %16lu },\n", 0, 0, 0);
	rex_cc_fprintf(out, 0, "};\n");

	return 0;
}


static int rex_cc_output_stateaccsubstates(rexcc_t *pCC, FILE *out, long nstate)
{
	long i;
	rexdfa_t *dfa = pCC->dfa;
	rexdfs_t *s = rex_dfa_state(dfa, nstate);
	rexdfss_t *ss;
	for (i = 0; i < s->naccsubstates; i++) {
		ss = rex_dfa_accsubstate(dfa, nstate, i);
		rex_cc_fprintf(out, 1, "{ %16lu, %16lu, %16lu },\n", ss->type, ss->uid, (unsigned long)ss->userdata);
	}
	return 0;
}


static int rex_cc_output_accsubstates(rexcc_t *pCC, FILE *out)
{
	long i;
	rexdfa_t *dfa = pCC->dfa;

	rex_cc_fprintf(out, 0, "static rexdfss_t accsubstates[] = {\n");
	for (i = 0; i < dfa->nstates; i++)
		rex_cc_output_stateaccsubstates(pCC, out, i);
	rex_cc_fprintf(out, 1, "{ %16lu, %16lu, %16lu },\n", 0, 0, 0);
	rex_cc_fprintf(out, 0, "};\n");

	return 0;
}


static int rex_cc_output_statetransitions(rexcc_t *pCC, FILE *out, long nstate)
{
	long i;
	rexdfa_t *dfa = pCC->dfa;
	rexdfs_t *s = rex_dfa_state(dfa, nstate);
	rexdft_t *t;
	for (i = 0; i < s->ntrans; i++) {
		t = rex_dfa_transition(dfa, nstate, i);
		rex_cc_fprintf(out, 1, "{ %16lu, %16lu, %16lu },\n", t->lowin, t->highin, t->state);
	}
	return 0;
}


static int rex_cc_output_transitions(rexcc_t *pCC, FILE *out)
{
	long i;
	rexdfa_t *dfa = pCC->dfa;

	rex_cc_fprintf(out, 0, "static rexdft_t transitions[] = {\n");
	for (i = 0; i < dfa->nstates; i++)
		rex_cc_output_statetransitions(pCC, out, i);
	rex_cc_fprintf(out, 1, "{ %16lu, %16lu, %16lu },\n", 0, 0, 0);
	rex_cc_fprintf(out, 0, "};\n");

	return 0;
}


static int rex_cc_output_states(rexcc_t *pCC, FILE *out)
{
	long i;
	rexdfs_t *s;
	rexdfa_t *dfa = pCC->dfa;

	rex_cc_fprintf(out, 0, "static rexdfs_t states[] = {\n");
	for (i = 0; i < dfa->nstates; i++) {
		s = rex_dfa_state(dfa, i);
		rex_cc_fprintf(out, 1, "{ %16lu, %16lu, %16lu, %16lu, %16lu, %16lu , %16lu},\n",
				s->type, s->trans, s->ntrans, s->substates, s->nsubstates, s->accsubstates, s->naccsubstates);

	}
	rex_cc_fprintf(out, 1, "{ %16lu, %16lu, %16lu, %16lu, %16lu, %16lu , %16lu},\n", 0, 0, 0, 0, 0, 0, 0);
	rex_cc_fprintf(out, 0, "};\n");

	return 0;
}


static int rex_cc_output_dfa(rexcc_t *pCC, FILE *out)
{
	rexdfa_t *dfa = pCC->dfa;

	rex_cc_fprintf(out, 0, "static rexdfa_t ccdfa = {\n");
	rex_cc_fprintf(out, 1, "%lu,\n", dfa->nstates);
	rex_cc_fprintf(out, 1, "%s,\n", "states");
	rex_cc_fprintf(out, 1, "%lu,\n", dfa->ntrans);
	rex_cc_fprintf(out, 1, "%s,\n", "transitions");
	rex_cc_fprintf(out, 1, "%lu,\n", dfa->naccsubstates);
	rex_cc_fprintf(out, 1, "%s,\n", "accsubstates");
	rex_cc_fprintf(out, 1, "%lu,\n", dfa->nsubstates);
	rex_cc_fprintf(out, 1, "%s,\n", "substates");
	rex_cc_fprintf(out, 1, "{0, },\n");
	rex_cc_fprintf(out, 0, "};\n");

	return 0;
}



int rex_cc_output(rexcc_t *pCC, FILE *outc, FILE *outh)
{

	if (outc) {
		rex_cc_fprintf(outc, 0, "#include \"rexdfa.h\"\n\n");
		rex_cc_output_accsubstates(pCC, outc);
		rex_cc_fprintf(outc, 0, "\n\n");
		rex_cc_output_substates(pCC, outc);
		rex_cc_fprintf(outc, 0, "\n\n");
		rex_cc_output_transitions(pCC, outc);
		rex_cc_fprintf(outc, 0, "\n\n");
		rex_cc_output_states(pCC, outc);
		rex_cc_fprintf(outc, 0, "\n\n");
		rex_cc_output_dfa(pCC, outc);
	}

	return 0;
}
