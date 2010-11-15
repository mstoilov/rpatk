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

#ifndef _RPAWORDSTACK_H_
#define _RPAWORDSTACK_H_

#include "rpatypes.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct rpa_wordstack_s {
	rpa_word_t *buffer;
	rpa_word_t size;
	rpa_word_t grow;
	rpa_word_t *p;
} rpa_wordstack_t;


rpa_wordstack_t *rpa_wordstack_create(rpa_word_t initialsize, rpa_word_t grow);
int rpa_wordstack_check_space(rpa_wordstack_t *stack);
void rpa_wordstack_destroy(rpa_wordstack_t *stack);
void rpa_wordstack_reset(rpa_wordstack_t *stack);
void rpa_wordstack_push(rpa_wordstack_t *stack, rpa_word_t val);
rpa_word_t rpa_wordstack_pop(rpa_wordstack_t *stack);



#ifdef __cplusplus
}
#endif

#endif
