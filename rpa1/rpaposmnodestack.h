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

#ifndef _RPAPOSMNODESTACK_H_
#define _RPAPOSMNODESTACK_H_

#include "rpatypes.h"
#include "rpamnode.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct rpa_posmnode_s {
	const char *pos;
	rpa_mnode_t *mnode;
} rpa_posmnode_t;
		

typedef struct rpa_posmnodestack_s {
	rpa_posmnode_t *buffer;
	rpa_posmnode_t *p;
	rpa_word_t size;
	rpa_word_t grow;
} rpa_posmnodestack_t;


rpa_posmnodestack_t *rpa_posmnodestack_create(rpa_word_t initialsize, rpa_word_t grow);
int rpa_posmnodestack_check_space(rpa_posmnodestack_t *stack);
void rpa_posmnodestack_destroy(rpa_posmnodestack_t *stack);
void rpa_posmnodestack_reset(rpa_posmnodestack_t *stack);
rpa_posmnode_t *rpa_posmnodestack_push(rpa_posmnodestack_t *stack, const char *pos, rpa_mnode_t *mnode);
rpa_posmnode_t *rpa_posmnodestack_pop(rpa_posmnodestack_t *stack);



#ifdef __cplusplus
}
#endif

#endif
