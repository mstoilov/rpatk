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
#include <ctype.h>
#include "rlib/rmem.h"
#include "rex/rextransition.h"


void rex_transitions_add_e(rarray_t *etrans, unsigned long srcuid, unsigned long dstuid)
{
	rex_transition_t ntrans;

	r_memset(&ntrans, 0, sizeof(ntrans));
	ntrans.type = REX_TRANSITION_EMPTY;
	ntrans.srcuid = srcuid;
	ntrans.dstuid = dstuid;
	r_array_add(etrans, &ntrans);
}


rex_transition_t *rex_transitions_find(rarray_t *trans, rexchar_t c)
{
	rex_transition_t *t;
	long min, max, mid;

	if (r_array_empty(trans))
		return NULL;
	min = 0;
	max = min + r_array_length(trans);
	while (max > min) {
		mid = (min + max)/2;
		t = (rex_transition_t *)r_array_slot(trans, mid);
		if (c >= t->lowin) {
			min = mid + 1;
		} else {
			max = mid;
		}
	}

	if (c >= t->lowin && c <= t->highin)
		return t;
	return NULL;
}


void rex_transitions_add(rarray_t *trans, rexchar_t c1, rexchar_t c2, unsigned long srcuid, unsigned long dstuid)
{
	rex_transition_t *t;
	rex_transition_t ntrans;
	long min, max, mid;

	if (c1 < c2) {
		ntrans.lowin = c1;
		ntrans.highin = c2;
	} else {
		ntrans.lowin = c2;
		ntrans.highin = c1;
	}
	ntrans.type = (c1 == c2) ? REX_TRANSITION_INPUT : REX_TRANSITION_RANGE;
	ntrans.dstuid = dstuid;
	ntrans.srcuid = srcuid;
	min = 0;
	max = min + r_array_length(trans);
	while (max > min) {
		mid = (min + max)/2;
		t = (rex_transition_t *)r_array_slot(trans, mid);
		if (c1 >= t->lowin) {
			min = mid + 1;
		} else {
			max = mid;
		}
	}
	r_array_insert(trans, min, &ntrans);
}


#if 1

/*
 * Source transitions must be normalized, for this functions to work properly
 */
void rex_transitions_negative(rarray_t *dtrans, rarray_t *strans, unsigned long srcuid, unsigned long dstuid)
{
	int i;
	rex_transition_t *t, *p;

	if (r_array_empty(strans)) {
		rex_transitions_add(dtrans, REX_CHAR_MIN, REX_CHAR_MAX, srcuid, dstuid);
		return;
	}

	for (i = 0; i < r_array_length(strans); i++) {
		t = (rex_transition_t *)r_array_slot(strans, i);
		if (i == 0) {
			if (t->lowin != 0) {
				rex_transitions_add(dtrans, REX_CHAR_MIN, t->lowin - 1, srcuid, dstuid);
			}
		}
		if (i > 0){
			p = (rex_transition_t *)r_array_slot(strans, i - 1);
			rex_transitions_add(dtrans, p->highin + 1, t->lowin - 1, srcuid, dstuid);
		}
		if (i == r_array_length(strans) - 1) {
			if (t->highin != REX_CHAR_MAX)
				rex_transitions_add(dtrans, t->highin + 1, REX_CHAR_MAX, srcuid, dstuid);
		}

	}
}
#endif


void rex_transitions_normalize(rarray_t *trans)
{
	long i, j;
	rex_transition_t *itrans, *jtrans;

startover:
	for (i = 0; i < r_array_length(trans); i++) {
		itrans = (rex_transition_t *)r_array_slot(trans, i);
		if (itrans->lowin == itrans->highin)
			itrans->type = REX_TRANSITION_INPUT;
		for (j = 0; j < r_array_length(trans); j++) {
			if (i == j) {
				/*
				 * Same transition.
				 */
				continue;
			}
			jtrans = (rex_transition_t *)r_array_slot(trans, j);
			if (itrans->dstuid != jtrans->dstuid) {
				/*
				 * These two can never be merged.
				 */
				continue;
			}
			if (jtrans->lowin >= itrans->lowin && jtrans->lowin <= itrans->highin) {
				/*
				 * Overlapping regions
				 * Merge jtrans into itrans and delete jtrans.
				 */
				if (jtrans->highin > itrans->highin)
					itrans->highin = jtrans->highin;
				if (itrans->lowin != itrans->highin)
					itrans->type = REX_TRANSITION_RANGE;
				r_array_delete(trans, j);
				goto startover;
			}
			if (itrans->highin != REX_CHAR_MAX && jtrans->lowin == itrans->highin + 1) {
				/*
				 * Adjacent regions
				 * Merge jtrans into itrans and delete jtrans.
				 */
				itrans->highin = jtrans->highin;
				if (itrans->lowin != itrans->highin)
					itrans->type = REX_TRANSITION_RANGE;
				r_array_delete(trans, j);
				goto startover;
			}
		}
	}
}
