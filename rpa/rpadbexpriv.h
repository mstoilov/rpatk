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


#ifndef _RPADBEXPRIV_H_
#define _RPADBEXPRIV_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "rtypes.h"
#include "rvm/rvmcpu.h"
#include "rpa/rpaerror.h"
#include "rpa/rpacompiler.h"
#include "rpa/rpaparser.h"
#include "rpa/rpaoptimization.h"

typedef int (*rpa_dbex_recordhandler)(rpadbex_t *dbex, long rec);

#define RPA_RULEINFO_NONE 0
#define RPA_RULEINFO_NAMEDRULE 1
#define RPA_RULEINFO_ANONYMOUSRULE 2
#define RPA_RULEINFO_DIRECTIVE 3


typedef struct rpa_ruleinfo_s {
	long startrec;
	long sizerecs;
	long codeoff;
	long codesiz;
	unsigned long type;
} rpa_ruleinfo_t;


struct rpadbex_s {
	rpa_compiler_t *co;
	rpa_parser_t *pa;
	rarray_t *records;
	rarray_t *temprecords;
	rharray_t *rules;
	rarray_t *recstack;
	rarray_t *inlinestack;
	rarray_t *text;
	rpa_dbex_recordhandler *handlers;
	rpa_errinfo_t err;
	unsigned long headoff;
	unsigned long optimizations:1;
	unsigned long debug:1;
	unsigned long compiled:1;
};


#ifdef __cplusplus
}
#endif

#endif
