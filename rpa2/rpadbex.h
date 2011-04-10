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

#ifndef _RPADBEX_H_
#define _RPADBEX_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "rtypes.h"
#include "rvmcpu.h"

typedef struct rpadbex_s rpadbex_t;
typedef rlong rparule_t;

rpadbex_t *rpa_dbex_create(void);
void rpa_dbex_destroy(rpadbex_t *dbex);
ruint rpa_dbex_get_error(rpadbex_t *dbex);
rint rpa_dbex_open(rpadbex_t *dbex);
void rpa_dbex_close(rpadbex_t *dbex);
rint rpa_dbex_compile(rpadbex_t *dbex);
rlong rpa_dbex_load(rpadbex_t *dbex, const rchar *rules, rsize_t size);
rlong rpa_dbex_load_s(rpadbex_t *dbex, const rchar *rules);
const rchar *rpa_dbex_version();
rvm_asmins_t *rvm_dbex_getcode(rpadbex_t *dbex);
rlong rvm_dbex_codeoffset(rpadbex_t *dbex, rparule_t rid);

rparule_t rpa_dbex_lookup(rpadbex_t *dbex, const rchar *name);
rparule_t rpa_dbex_default(rpadbex_t *dbex);
rparule_t rpa_dbex_next(rpadbex_t *dbex, rparule_t rid);
rparule_t rpa_dbex_prev(rpadbex_t *dbex, rparule_t rid);
rparule_t rpa_dbex_first(rpadbex_t *dbex);
rparule_t rpa_dbex_last(rpadbex_t *dbex);
rsize_t rpa_dbex_copy(rpadbex_t *dbex, rparule_t rid, rchar *buf, rsize_t bufsize);

rint rpa_dbex_dumprecords(rpadbex_t *dbex);
rint rpa_dbex_dumptree(rpadbex_t *dbex, const rchar *name);
rint rpa_dbex_dumprules(rpadbex_t *dbex);
rint rpa_dbex_dumpinfo(rpadbex_t *dbex);
rint rpa_dbex_dumpcode(rpadbex_t* dbex, const rchar *rule);

#ifdef __cplusplus
}
#endif

#endif
