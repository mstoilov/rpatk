/*
 *  Regular Pattern Analyzer Toolkit (RPA/Tk)
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

#include "rpa/rpacompiler.h"
#include "rpa/rpastat.h"

#define RPA_PARSER_STACK 8196

#ifdef __cplusplus
extern "C" {
#endif

enum {
	RPA_PRODUCTION_NONE = 0,
	RPA_PRODUCTION_BNF,
	RPA_PRODUCTION_NAMEDRULE,
	RPA_PRODUCTION_ANONYMOUSRULE,
	RPA_PRODUCTION_ALIASNAME,
	RPA_PRODUCTION_RULENAME,
	RPA_PRODUCTION_CHAR,
	RPA_PRODUCTION_ESCAPEDCHAR,
	RPA_PRODUCTION_SPECIALCHAR,
	RPA_PRODUCTION_SPECIALCLSCHAR,
	RPA_PRODUCTION_OCCURENCE,
	RPA_PRODUCTION_CHARRNG,
	RPA_PRODUCTION_NUMRNG,
	RPA_PRODUCTION_CLS,
	RPA_PRODUCTION_CLSCHAR,
	RPA_PRODUCTION_CLSNUM,
	RPA_PRODUCTION_BEGINCHAR,
	RPA_PRODUCTION_ENDCHAR,
	RPA_PRODUCTION_DEC,
	RPA_PRODUCTION_HEX,
	RPA_PRODUCTION_AREF,
	RPA_PRODUCTION_CREF,
	RPA_PRODUCTION_EXP,
	RPA_PRODUCTION_ANCHOROP,
	RPA_PRODUCTION_REQOP,
	RPA_PRODUCTION_NOTOP,
	RPA_PRODUCTION_MINOP,
	RPA_PRODUCTION_OROP,
	RPA_PRODUCTION_ALTBRANCH,
	RPA_PRODUCTION_NOROP,
	RPA_PRODUCTION_NEGBRANCH,
	RPA_PRODUCTION_BRACKETEXP,
	RPA_PRODUCTION_DIRECTIVEEMIT,
	RPA_PRODUCTION_DIRECTIVENOEMIT,
	RPA_PRODUCTION_DIRECTIVEEMITNONE,
	RPA_PRODUCTION_DIRECTIVEEMITALL,
	RPA_PRODUCTION_DIRECTIVEABORT,
	RPA_PRODUCTION_DIRECTIVEEMITID,


/*
 * This must be the last one
 */
	RPA_PRODUCTION_COUNT,
};


typedef struct rpa_parser_s {
	rpa_compiler_t *co;
	rpastat_t *stat;
	unsigned long main;
} rpa_parser_t;


rpa_parser_t *rpa_parser_create();
void rpa_parser_destroy(rpa_parser_t *pa);
long rpa_parser_load(rpa_parser_t *pa, const char *prods, unsigned long size, rarray_t *records);
long rpa_parser_load_s(rpa_parser_t *pa, const char *prods, rarray_t *records);

#ifdef __cplusplus
}
#endif

#endif
