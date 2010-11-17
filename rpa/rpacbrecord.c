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


#include "rpacbrecord.h"
#include "rmem.h"
#include "rpatypes.h"


#define RPA_CBSET_GROW 128

rpa_cbset_t *rpa_cbset_init(rpa_cbset_t *cbset)
{
	r_memset(cbset, 0, sizeof(*cbset));
	if (rpa_cbset_check_space(cbset) < 0)
		return (void*)0;
	return cbset;
}


void rpa_cbset_cleanup(rpa_cbset_t *cbset)
{
	r_free(cbset->data);
}


int rpa_cbset_check_space(rpa_cbset_t *cbset)
{
	rpa_cbrecord_t *data;
	rpa_word_t size;

	if (cbset->size - cbset->off < RPA_CBSET_GROW) {
		size = cbset->size + RPA_CBSET_GROW;
		data = (rpa_cbrecord_t *)r_realloc(cbset->data, (unsigned long)(sizeof(rpa_cbrecord_t) * size));
		if (!data)
			return -1;
		cbset->size = size;
		cbset->data = data;
	}
	return 0;
}


rpa_cbrecord_t *rpa_cbset_push(rpa_cbset_t *cbset)
{
	if (rpa_cbset_check_space(cbset) < 0)
		return (void*)0;
	cbset->off += 1;
	return &cbset->data[cbset->off];
}


void rpa_cbset_reset(rpa_cbset_t *cbset, rpa_word_t off)
{
	cbset->off = off;
}


rpa_word_t rpa_cbset_getpos(rpa_cbset_t *cbset)
{
	return cbset->off;
}

