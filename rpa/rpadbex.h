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
 * \brief The public interface to BNF productions database API
 *
 *
 * <h2>Synopsis</h2>
 * The following APIs are used to Create, Compile, Enumerate, etc. BNF productions.
 */

#ifndef _RPADBEX_H_
#define _RPADBEX_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "rtypes.h"
#include "rvmcpu.h"
#include "rpaerror.h"


/**
 * \typedef rpadbex_t
 * \brief Database of BNF productions (The BNF schema)
 */
typedef struct rpadbex_s rpadbex_t;

/**
 * \typedef rparule_t
 * \brief Unique BNF rule identifier.
 */
typedef rlong rparule_t;

#define RPA_DBEXCFG_OPTIMIZATIONS 1
#define RPA_DBEXCFG_DEBUG 2

/**
 * \brief Return the version string of the RPA library.
 *
 * \return NULL terminated version string.
 */
const rchar *rpa_dbex_version();


/**
 * \brief Create an object of type \ref rpadbex_t.
 *
 * The created object must be destroyed with rpa_dbex_destroy.
 * \return pointer to newly create BNF productions database object.
 */
rpadbex_t *rpa_dbex_create(void);


/**
 * \brief Destoies an object of type rpadbex_t, created by \ref rpa_dbex_create.
 *
 * Use this function to destroy the object. After this call the pointer to dbex should never be used again.
 *
 * \param dbex pointer to an object of type \ref rpadbex_t
 */
void rpa_dbex_destroy(rpadbex_t *dbex);


/**
 * \brief Return the error code of the last occurred error.
 *
 * \param dbex Pointer to \ref rpadbex_t object.
 * \return The error code of the last occurred error. If this function fails the
 * return value is negative.
 */
rlong rpa_dbex_lasterror(rpadbex_t *dbex);

/**
 * \brief Get error information for the last occurred error.
 *
 * \param dbex Pointer to \ref rpadbex_t object.
 * \param errinfo Pointer to \ref rpa_errinfo_t buffer that will be
 * filled with the error information. This parameter cannot be NULL.
 * \return Return 0 if the function is successful or negative otherwise. If this function fails the
 * return value is negative.
 */
rlong rpa_dbex_lasterrorinfo(rpadbex_t *dbex, rpa_errinfo_t *errinfo);


/**
 * \brief Open the BNF productions database.
 *
 * This function must be called before inserting any BNF production with
 * \ref  rpa_dbex_load or \ref rpa_dbex_load_s. To close the database you
 * must use \ref rpa_dbex_close. The database can opened and closed multiple
 * times as long it is not destroyed with \ref rpa_dbex_destroy.
 *
 * \param dbex Pointer to \ref rpadbex_t object.
 * \return If successful return 0, otherwise return negative.
 *         Use \ref rpa_dbex_lasterror or \ref rpa_dbex_lasterrorinfo to retrieve the error
 *         information.
 */
rinteger rpa_dbex_open(rpadbex_t *dbex);


/**
 * \brief Close the BNF productions database.
 *
 * This function must be called when the BNF schema is complete and ready
 * to be compiled.
 *
 * \param dbex Pointer to \ref rpadbex_t object.
 */
void rpa_dbex_close(rpadbex_t *dbex);


/**
 * \brief Compile the BNF productions database to byte code.
 *
 * Compile the BNF productions database to byte code. This function generates
 * the byte code (executable code) used to parse or search input stream.
 * Parsing, Matching, Searching (Running the code) is controlled by \ref rpastat_t.
 *
 * \param dbex Pointer to \ref rpadbex_t object.
 * \return If successful return 0, otherwise return negative.
 *         Use \ref rpa_dbex_lasterror or \ref rpa_dbex_lasterrorinfo to retrieve the error
 *         information.
 */
rinteger rpa_dbex_compile(rpadbex_t *dbex);


/**
 * \brief Load BNF production(s) into the database.
 *
 * Loads the BNF production(s) from the buffer into the dbex database.
 * \param dbex Pointer to \ref rpadbex_t object.
 * \param buffer UTF8 string containing BNF production(s).
 * \param size Buffer size.
 * \return Upon successful completion, the function returns the size of the loaded BNF production(s), otherwise returns negative.
 *         Use \ref rpa_dbex_lasterror or \ref rpa_dbex_lasterrorinfo to retrieve the error
 *         information.
 */
rlong rpa_dbex_load(rpadbex_t *dbex, const rchar *buffer, rsize_t size);


/**
 * \brief Load BNF production(s) into the database. Same as \ref rpa_dbex_load,
 * 			but the buffer is NULL terminated string.
 *
 * \param dbex Pointer to \ref rpadbex_t object.
 * \param buffer UTF8 string containing BNF production(s).
 * \return Upon successful completion, the function returns the size of the copied productions,
 *         otherwise returns negative.
 *         Use \ref rpa_dbex_lasterror or \ref rpa_dbex_lasterrorinfo to retrieve the error
 *         information.
 */
rlong rpa_dbex_load_s(rpadbex_t *dbex, const rchar *buffer);


/**
 * \brief Return a pointer to the executable code segment.
 *
 * This function will succeed only if it is called after a call to \ref rpa_dbex_compile.
 * \param dbex Pointer to \ref rpadbex_t object.
 * \return Pointer to the beginning of executable byte code. In case of an error it returns NULL.
 *         Use \ref rpa_dbex_lasterror or \ref rpa_dbex_lasterrorinfo to retrieve the error
 *         information.
 */
rvm_asmins_t *rpa_dbex_executable(rpadbex_t *dbex);


/**
 * \brief Return the offset of the executable byte code for the specified rule.
 *
 * Return the offset of the executable byte code segment returned by \ref rpa_dbex_executable for the specified rule.
 * The two functions \ref rpa_dbex_executable and \ref rpa_dbex_executableoffset work together to locate the
 * beginning of the executable byte code for the specified ID.
 * This function will succeed only if it is called after a call to \ref rpa_dbex_compile.
 *
 * \param dbex Pointer to \ref rpadbex_t object.
 * \param rid Unique ID of the requested BNF production.
 * \return Pointer the offset of the executable byte code. In case of an error it returns negative.
 *         Use \ref rpa_dbex_lasterror or \ref rpa_dbex_lasterrorinfo to retrieve the error
 *         information.
 */
rlong rpa_dbex_executableoffset(rpadbex_t *dbex, rparule_t rid);


/**
 * \brief Lookup BNF production ID in the database.
 *
 * Lookup the unique ID for BNF production in the dbex database. For example if you
 * have loaded and compiled a BNF production like: "alpha ::= [a-zA-Z]" with a previous call to
 * \ref rpa_dbex_load. You can lookup the unique ID for this production by using this
 * function.
 *
 * \param dbex Pointer to \ref rpadbex_t object.
 * \param name The production name (the identifier located before the operator '::= ')
 * \param namesize The size of the buffer to be used.
 * \return Unique ID for the specified production name.
 */
rparule_t rpa_dbex_lookup(rpadbex_t *dbex, const rchar *name, rsize_t namesize);


/**
 * \brief Lookup BNF production ID in the database.
 *
 * Same as \ref rpa_dbex_lookup, but the name parameter is NULL terminated string.
 */
rparule_t rpa_dbex_lookup_s(rpadbex_t *dbex, const rchar *name);


/**
 * \brief Return the first production ID in the database.
 *
 * Depending on how the BNF schema is structured, if the first production is also the
 * root production you should use this function to get the root ID.
 *
 * \param dbex Pointer to \ref rpadbex_t object.
 * \return Returns the ID of the first production.
 */
rparule_t rpa_dbex_first(rpadbex_t *dbex);


/**
 * \brief Return the last production ID
 *
 * Depending on how the BNF schema is structured, if the last production is also the
 * root production you should use this function to get the root ID.
 *
 * \param dbex Pointer to \ref rpadbex_t object.
 * \return Returns the ID of the last production.
 */
rparule_t rpa_dbex_last(rpadbex_t *dbex);


/**
 * \brief Return the next production ID
 *
 * \param dbex Pointer to \ref rpadbex_t object.
 * \param rid current production ID.
 * \return Returns the ID of the next production.
 */
rparule_t rpa_dbex_next(rpadbex_t *dbex, rparule_t rid);


/**
 * \brief Return the previous production ID
 *
 * \param dbex Pointer to \ref rpadbex_t object.
 * \param rid current production ID.
 * \return Returns the ID of the previous production.
 */
rparule_t rpa_dbex_prev(rpadbex_t *dbex, rparule_t rid);


/**
 * \brief Returns the string length of the specified BNF production
 *
 * \param dbex Pointer to \ref rpadbex_t object.
 * \param rid production ID.
 * \return the string length of the specified production
 */
rsize_t rpa_dbex_strlen(rpadbex_t *dbex, rparule_t rid);


/**
 * \brief Copy the string of the specified BNF production to the destination buffer
 *
 * \param dbex Pointer to \ref rpadbex_t object.
 * \param rid production ID.
 * \param dest destination buffer
 * \param size to be copied
 * \return Return the number of bytes written in the buffer.
 */
rsize_t rpa_dbex_strncpy(rpadbex_t *dbex, rchar *dest, rparule_t rid, rsize_t size);


/**
 * \brief Set a configuration value for the dbex object.
 *
 * \param dbex Pointer to \ref rpadbex_t object.
 * \param cfg Configuration ID
 * \param val Configuration value
 *
 * Supported configuration IDs / Values:
 * - RPA_DBEXCFG_OPTIMIZATIONS
 *   - 0 Dissable optimizations
 *   - 1 Enable optimizations
 * - RPA_DBEXCFG_DEBUG
 *   - 0 Dissable debugging
 *   - 1 Enable debugging
 */
rlong rpa_dbex_cfgset(rpadbex_t *dbex, rulong cfg, rulong val);


/**
 * \brief Get a configuration value for the dbex object.
 *
 * \param dbex Pointer to \ref rpadbex_t object.
 * \param cfg Configuration ID.
 * \return Return the value of the specified configuration ID. If an error occurs the return negative.
 *
 * Supported configuration IDs
 * - RPA_DBEXCFG_OPTIMIZATIONS
 * - RPA_DBEXCFG_DEBUG
 */
rlong rpa_dbex_cfgget(rpadbex_t *dbex, rulong cfg);


/**
 * \brief Printeger a BNF production in a tree format
 *
 * Use \ref rpa_dbex_lookup to find the ID of a BNF production
 * \param dbex Pointer to \ref rpadbex_t object.
 * \param rid production ID.
 * \return Return 0 on success or negative if error occurred.
 */
rinteger rpa_dbex_dumptree(rpadbex_t *dbex, rparule_t rid);

/**
 * \brief Dump the compiled byte code for the specified production ID
 *
 * This function is a debug helper, you shouldn't need it.
 * \param dbex Pointer to \ref rpadbex_t object.
 * \param rid production ID.
 * \return Return 0 on success or negative if error occurred.
 */
rinteger rpa_dbex_dumpcode(rpadbex_t* dbex, rparule_t rid);


/*
 * \brief Printeger the AST of the parsed BNF productions.
 *
 * This function is a debug helper, you shouldn't need it.
 * unless you are debugging this library.
 *
 * \param dbex Pointer to \ref rpadbex_t object.
 * \return Return 0 on success or negative if error occurred.
 */
rinteger rpa_dbex_dumprecords(rpadbex_t *dbex);

/*
 * \brief Printeger the content of BNF productions database.
 *
 * Enumerate all BNF productions and printeger them in a text format.
 * \param dbex Pointer to \ref rpadbex_t object.
 * \return Return 0 on success or negative if error occurred.
 */
rinteger rpa_dbex_dumpproductions(rpadbex_t *dbex);

/*
 * \brief Printeger debug information about the state of the BNF productions database.
 *
 * This is a debug helper, you shouldn't need it.
 * \param dbex Pointer to \ref rpadbex_t object.
 * \return Return 0 on success or negative if error occurred.
 */
rinteger rpa_dbex_dumpinfo(rpadbex_t *dbex);

/*
 * \brief Printeger the production user IDs in a format suitable to be
 * included in source code.
 *
 * If you define user IDs for your productions you can dump all user IDs
 * in a format suitable to be included in a C/C++ source code.
 */
rinteger rpa_dbex_dumpuids(rpadbex_t *dbex);



#ifdef __cplusplus
}
#endif

#endif
