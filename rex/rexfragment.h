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

#ifndef _REXFRAGMENT_H_
#define _REXFRAGMENT_H_

#include "rtypes.h"
#include "rex/rexstate.h"
#include "rex/rexdb.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct rexfragment_s {
	rexdb_t *rexdb;
	long start;
	long end;
} rexfragment_t;


void rex_fragment_init(rexfragment_t *frag, rexdb_t *rexdb);
rexfragment_t *rex_fragment_create(rexdb_t *rexdb);
void rex_fragment_destroy(rexfragment_t *frag);
rexfragment_t *rex_fragment_create_singletransition(rexdb_t *rexdb, unsigned int input);
void rex_fragment_rangetransition(rexfragment_t *frag, unsigned int c1, unsigned int c2);
void rex_fragment_singletransition(rexfragment_t *frag, unsigned int input);
void rex_fragment_set_startstatetype(rexfragment_t *frag, rex_statetype_t statetype);
void rex_fragment_set_endstatetype(rexfragment_t *frag, rex_statetype_t statetype);
rexstate_t *rex_fragment_startstate(rexfragment_t *frag);
rexstate_t *rex_fragment_endstate(rexfragment_t *frag);
rexfragment_t *rex_fragment_cat(rexfragment_t *frag1, rexfragment_t *frag2);
rexfragment_t *rex_fragment_alt(rexfragment_t *frag1, rexfragment_t *frag2);
rexfragment_t *rex_fragment_opt(rexfragment_t *frag);
rexfragment_t *rex_fragment_mop(rexfragment_t *frag);
rexfragment_t *rex_fragment_mul(rexfragment_t *frag);


#ifdef __cplusplus
}
#endif


#endif /* REXFRAGMENT_H_ */
