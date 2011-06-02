#ifndef _RPASTAT_H_
#define _RPASTAT_H_

#include "rtypes.h"
#include "rarray.h"
#include "rvmreg.h"
#include "rpadbex.h"

#define RPA_ENCODING_UTF8 0
#define RPA_ENCODING_BYTE 1
#define RPA_ENCODING_UTF16LE 2
#define RPA_ENCODING_MASK ((1 << 8) - 1)
#define RPA_ENCODING_ICASE (1 << 8)
#define RPA_ENCODING_ICASE_BYTE (RPA_ENCODING_BYTE | RPA_ENCODING_ICASE)
#define RPA_ENCODING_ICASE_UTF8 (RPA_ENCODING_UTF8 | RPA_ENCODING_ICASE)
#define RPA_ENCODING_ICASE_UTF16LE (RPA_ENCODING_UTF16LE | RPA_ENCODING_ICASE)

#define RPA_DEFAULT_STACKSIZE (256 * 1024)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \file rpastat.h
 * \brief The public interface to the execution context.
 *
 *
 * <h2>Synopsis</h2>
 * The following APIs are used to parse, match, scan an input stream, using an existing
 * BNF productions database \ref rpadbex_t
 */


/**
 * \typedef rpastat_t
 * \brief Execution context. If you need mutli-threading, multiple objects of this
 * type must be created for every concurrent thread. All objects can be created/destroyed from the
 * main thread. Only the execution functions: \ref rpa_stat_parse, \ref rpa_stat_match and
 * \ref rpa_stat_scan have to be called from separate threads.
 */
typedef struct rpastat_s rpastat_t;

/**
 * \brief Create an object of type \ref rpastat_t.
 *
 * Multi-threading is supported by creating multiple \ref rpastat_t objects,
 * referencing the same \ref rpadbex_t BNF productions database. Every thread
 * must have its own \ref rpastat_t object, created with this function. This
 * function can be called from the main thread multiple times to allocate the objects and
 * then the returned pointer(s) passed to the child threads.
 *
 * If you don't need multi-threading, call this function and any one of the execution
 * functions \ref rpa_stat_parse, \ref rpa_stat_match and \ref rpa_stat_scan from the
 * main thread.
 *
 * The allocated \ref rpastat_t object must be destroyed with \ref rpa_stat_destroy.
 *
 * \param dbex BNF productions database created with \ref rpa_dbex_create.
 * \param stacksize Execution stack size. The size is specified in byts.
 * \return Returns a pointer to newly created \ref rpastat_t object or NULL if error occured.
 */
rpastat_t *rpa_stat_create(rpadbex_t *dbex, rulong stacksize);


/**
 * \brief Destroy an object of type \ref rpastat_t
 *
 * Destroy the object created with \ref rpa_stat_create. After calling this function
 * the pointer is invalid and must never be used again.
 *
 * \param stat Pointer to object of type \ref rpastat_t.
 */
void rpa_stat_destroy(rpastat_t *stat);


/**
 * \brief Scan an input stream
 *
 * Scan the stream using the specified rule id.
 * \param stat Pointer to object of type \ref rpastat_t
 * \param rid Rule ID of the BNF root.
 * \param input The starting point of the operation. <b>Important:</b> input >= start && input < end
 * \param start The start of the buffer.
 * \param end The end of the buffer, end will never be dereferenced.
 * \param where If this function returns a number greater than 0 (a match was found) this parameter will be
 * 				initialized with a pointer the place in the buffer where the match was found.
 * \return If successful return the size of the matched string in bytes, if no match was found
 * 			return 0, return negative in case of error.
 */
rlong rpa_stat_scan(rpastat_t *stat, rparule_t rid, const rchar *input, const rchar *start, const rchar *end, const rchar **where);
rlong rpa_stat_parse(rpastat_t *stat, rparule_t rid, const rchar *input, const rchar *start, const rchar *end, rarray_t *records);
rlong rpa_stat_match(rpastat_t *stat, rparule_t rid, const rchar *input, const rchar *start, const rchar *end);
rlong rpa_stat_exec(rpastat_t *stat, rvm_asmins_t *prog, rword off, const rchar *input, const rchar *start, const rchar *end, rarray_t *records);
rint rpa_stat_abort(rpastat_t *stat);
rint rpa_stat_setencoding(rpastat_t *stat, ruint encoding);


/**
 * \brief Return the error code of the last occurred error.
 *
 * \param dbex Pointer to \ref rpadbex_t object.
 * \return The error code of the last occurred error. If this function fails the
 * return value is negative.
 */
rlong rpa_stat_lasterror(rpastat_t *stat);

/**
 * \brief Get error information for the last occurred error.
 *
 * \param dbex Pointer to \ref rpadbex_t object.
 * \param errinfo Pointer to \ref rpa_errinfo_t buffer that will be
 * filled with the error information. This parameter cannot be NULL.
 * \return Return 0 if the function is successful or negative otherwise. If this function fails the
 * return value is negative.
 */
rlong rpa_stat_lasterrorinfo(rpastat_t *stat, rpa_errinfo_t *errinfo);


#ifdef __cplusplus
}
#endif

#endif
