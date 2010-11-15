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

#ifndef _RPAPARSER_H_
#define _RPAPARSER_H_

#include "rpalist.h"
#include "rpamatch.h"
#include "rpawordstack.h"
#include "rpavm.h"
#include "rpastat.h"
#include "rpavarlink.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct rpa_parser_s {
	rpa_match_t* bnfroot;
	rpa_varlink_t *pVarLinkBnfRoot;
	rpa_head_t bnftree;
	rpa_stat_t stat;
	rpa_wordstack_t *stack;
	rpa_asmins_t *vmcode;
	rpa_word_t vmcode_size;
	rpa_word_t vmcode_off;
} rpa_parser_t;


rpa_parser_t *rpa_parser_create();
void rpa_parser_destroy(rpa_parser_t *parser);
void rpa_parser_setup_bnftree(rpa_parser_t *parser);
int rpa_parser_exec(rpa_parser_t *parser, const char *input, unsigned long size);


#ifdef __cplusplus
}
#endif

#endif
