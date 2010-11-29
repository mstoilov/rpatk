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

#ifndef _RPACBRECORD_H_
#define _RPACBRECORD_H_

#include "rpamatch.h"
#include "rpamnode.h"
#include "rpatypedef.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct rpa_cbrecord_s {
	rpa_mnode_t *mnode;
	const char *input;
	unsigned int size;
} rpa_cbrecord_t;


typedef struct rpa_cbset_s {
	rpa_cbrecord_t *data;
	rpa_word_t size;
	rpa_word_t off;
} rpa_cbset_t;

rpa_cbset_t *rpa_cbset_init(rpa_cbset_t *cbset);
void rpa_cbset_cleanup(rpa_cbset_t *cbset);
int rpa_cbset_check_space(rpa_cbset_t *cbset);
int rpa_cbset_check_space_min(rpa_cbset_t *cbset, long min);
rpa_cbrecord_t *rpa_cbset_push(rpa_cbset_t *cbset);
void rpa_cbset_reset(rpa_cbset_t *cbset, rpa_word_t off);
rpa_word_t rpa_cbset_getpos(rpa_cbset_t *cbset);
rpa_cbrecord_t rpa_cbset_getrecord(rpa_cbset_t *cbset, rpa_word_t off);
rpa_cbrecord_t *rpa_cbset_getslot(rpa_cbset_t *cbset, rpa_word_t off);
long rpa_cbset_size_available(rpa_cbset_t *cbset);

#ifdef __cplusplus
}
#endif

#endif
