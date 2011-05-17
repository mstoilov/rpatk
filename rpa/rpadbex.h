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

/**
 * \file rpadbex.h
 * \brief The public interface to BNF expression database API
 *
 *
 * <h2>Synopsis</h2>
 * The following APIs are used to Create, Compile, Enumerate, etc. BNF expressions schema.
 */

#ifndef _RPADBEX_H_
#define _RPADBEX_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "rtypes.h"
#include "rvmcpu.h"
/**
 * \typedef rpadbex_t
 * \brief Database of BNF expression (The BNF schema)
 */
typedef struct rpadbex_s rpadbex_t;
typedef rlong rparule_t;

#define RPA_DBEXCFG_OPTIMIZATIONS 1

/**
 * \brief Creates and returns a pointer to an object of type rpadbex_t.
 *
 * The created object must be destroyed with rpa_dbex_destroy.
 */
rpadbex_t *rpa_dbex_create(void);

/**
 * \brief Destoies an object of type rpadbex_t, created by \ref rpa_dbex_create.
 *
 * Use this function to destroy the object and release all memories. After this call
 * the pointer to dbex should never be used.
 *
 * \param dbex pointer to an object of type \ref rpadbex_t
 */
void rpa_dbex_destroy(rpadbex_t *dbex);

/**
 * \brief Return the error code of the last occurred error.
 *
 * Return the error code of the last occurred error.
 * \param dbex Pointer to \ref rpadbex_t object.
 */
rlong rpa_dbex_getlasterror(rpadbex_t *dbex);

/**
 * \brief Open the BNF expressions database.
 *
 * This function must be called before inserting any BNF expression with
 * \ref  rpa_dbex_load or \ref rpa_dbex_load_s. To close the database you
 * must use \ref rpa_dbex_close. The database can opened and closed multiple
 * times as long it is not destroied with \ref rpa_dbex_destroy.
 *
 * \param dbex Pointer to \ref rpadbex_t object.
 */
rint rpa_dbex_open(rpadbex_t *dbex);

/**
 * \brief Close the BNF expressions database.
 *
 * This function must be called when the BNF schema is complete and ready
 * to be compiled.
 *
 * \param dbex Pointer to \ref rpadbex_t object.
 */
void rpa_dbex_close(rpadbex_t *dbex);

/**
 * \brief Compile the BNF expressions database to byte code.
 *
 * Compile the BNF expressions database to byte code. This function generates
 * the byte code (executable code) used to parse or search input stream.
 * Parsing, Matching, Searching (Running the code) is controlled by \ref rpastat_t.
 *
 * \param dbex Pointer to \ref rpadbex_t object.
 */
rint rpa_dbex_compile(rpadbex_t *dbex);
rlong rpa_dbex_load(rpadbex_t *dbex, const rchar *rules, rsize_t size);
rlong rpa_dbex_load_s(rpadbex_t *dbex, const rchar *rules);
const rchar *rpa_dbex_version();
rvm_asmins_t *rvm_dbex_getexecutable(rpadbex_t *dbex);
rlong rvm_dbex_executableoffset(rpadbex_t *dbex, rparule_t rid);

rparule_t rpa_dbex_lookup(rpadbex_t *dbex, const rchar *name, rsize_t namesize);
rparule_t rpa_dbex_lookup_s(rpadbex_t *dbex, const rchar *name);
rparule_t rpa_dbex_default(rpadbex_t *dbex);
rparule_t rpa_dbex_next(rpadbex_t *dbex, rparule_t rid);
rparule_t rpa_dbex_prev(rpadbex_t *dbex, rparule_t rid);
rparule_t rpa_dbex_first(rpadbex_t *dbex);
rparule_t rpa_dbex_last(rpadbex_t *dbex);
rsize_t rpa_dbex_copy(rpadbex_t *dbex, rparule_t rid, rchar *buf, rsize_t bufsize);

rint rpa_dbex_dumprecords(rpadbex_t *dbex);
rint rpa_dbex_dumptree_s(rpadbex_t *dbex, const rchar *name, rint level);
rint rpa_dbex_dumptree(rpadbex_t *dbex, const rchar *rulename, rsize_t namesize, rint level);
rint rpa_dbex_dumprules(rpadbex_t *dbex);
rint rpa_dbex_dumpinfo(rpadbex_t *dbex);
rint rpa_dbex_dumpcode(rpadbex_t* dbex, const rchar *rule);
rint rpa_dbex_dumpalias(rpadbex_t *dbex);
rlong rpa_dbex_cfgset(rpadbex_t *dbex, rulong cfg, rulong val);
rlong rpa_dbex_cfgget(rpadbex_t *dbex, rulong cfg);



#ifdef __cplusplus
}
#endif

#endif
