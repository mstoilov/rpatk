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

#ifndef _REXCOMPILER_H_
#define _REXCOMPILER_H_

#include "rtypes.h"
#include "rlib/robject.h"
#include "rlib/rarray.h"
#include "rex/rexdb.h"


#ifdef __cplusplus
extern "C" {
#endif

#define REX_COMPILER_TOKENSIZE	128

typedef struct rexcompiler_s rexcompiler_t;


rexcompiler_t *rex_compiler_create();
void rex_compiler_destroy(rexcompiler_t *co);
long rex_compiler_expression(rexcompiler_t *co, rexdb_t *rexdb, const char *str, unsigned int size, rexuserdata_t userdata);
long rex_compiler_expression_s(rexcompiler_t *co, rexdb_t *rexdb, const char *str, rexuserdata_t userdata);
long rex_compiler_addexpression(rexcompiler_t *co, rexdb_t *rexdb, unsigned long prev, const char *str, unsigned int size, rexuserdata_t userdata);
long rex_compiler_addexpression_s(rexcompiler_t *co, rexdb_t *rexdb, unsigned long prev, const char *str, rexuserdata_t userdata);

#ifdef __cplusplus
}
#endif


#endif
