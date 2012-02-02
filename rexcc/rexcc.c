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
	pCC->si = rex_nfasimulator_create();
	pCC->dfasi = rex_dfasimulator_create();
	pCC->ret = 1;
	pCC->startuid = -1L;
	return pCC;
}


void rex_cc_destroy(rexcc_t *pCC)
{
	if (!pCC)
		return;
	rex_db_destroy(pCC->nfa);
	rex_dfa_destroy(pCC->dfa);
	rex_nfasimulator_destroy(pCC->si);
	rex_dfasimulator_destroy(pCC->dfasi);
	r_free(pCC);
}


int rex_cc_load_string_pattern(rexcc_t *pCC, rbuffer_t *buf)
{
	return rex_cc_load_pattern(pCC, buf);
}


int rex_cc_load_pattern(rexcc_t *pCC, rbuffer_t *buf)
{
	pCC->startuid = rex_db_addexpression(pCC->nfa, pCC->startuid, buf->s, buf->size, 0);
	if (pCC->startuid < 0) {
		return -1;
	}
	return 0;
}

