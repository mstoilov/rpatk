/*
 *  Regular Pattern Analyzer Toolkit (RPA/Tk)
 *  Copyright (c) 2009-2012 Martin Stoilov
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
 * @file rpa/rparecord.h
 * @brief The public interface for working with Abstract Syntax Tree (AST), produced by @ref rpa_stat_parse.
 *
 *
 * <h2>Synopsis</h2>
 * Upon a successful call to @ref rpa_stat_parse, the parser produces a stack of @ref rparecord_t records.
 * There are two kinds of records: @ref RPA_RECORD_START and @ref RPA_RECORD_END. @ref RPA_RECORD_START marks
 * the beginning of a branch and @ref RPA_RECORD_END marks the end of that branch. Empty branches are specified by
 * a record @ref RPA_RECORD_START followed immediately by @ref RPA_RECORD_END (no child records in between).
 * Empty branches are considered leaves.
 *
 * Example:
 * Consider parsing a person name:
 * @verbatim John M. Smith @endverbatim
 *
 * with the following BNF:
 * @code
 * first  ::= [A-Za-z]+
 * middle ::= [A-Za-z]+ '.'?
 * last   ::= [A-Za-z]+
 * name   ::= <first> ' ' <middle> ' ' <last>
 * @endcode
 *
 * The records produced by rpa_stat_parse would look like this:
 * @code
 * [record offset]    [record type]       [rule name]  [input offset]  [input size]  [input]
 *  0                  RPA_RECORD_START    name         0               13            John M. Smith
 *  1                  RPA_RECORD_START    first        0                4            John
 *  2                  RPA_RECORD_END      first        0                4            John
 *  3                  RPA_RECORD_START    middle       5                2            M.
 *  4                  RPA_RECORD_END      middle       5                2            M.
 *  5                  RPA_RECORD_START    last         8                5            Smith
 *  6                  RPA_RECORD_END      last         8                5            Smith
 *  7                  RPA_RECORD_END      name         0               13            John M. Smith
 * @endcode
 *
 * Note: first, middle and last are enclosed within name's RPA_RECORD_START and RPA_RECORD_END
 */


#ifndef _RPARECORD_H_
#define _RPARECORD_H_

#include "rtypes.h"
#include "rlib/rarray.h"
#include "rlib/rlist.h"
#include "rpa/rpavm.h"


#ifdef __cplusplus
extern "C" {
#endif

#define RPA_RECORD_NONE (0)						/**< No record type - the record type is not initialized */
#define RPA_RECORD_START (1 << 0)				/**< Start record - the parser generates this record before evaluating the rule. */
#define RPA_RECORD_END (1 << 1)					/**< End record - the parser generates this record after evaluating the rule and the rule matched some input. */

/**
 * Abstract Syntax Tree (AST) construction element
 */
typedef struct rparecord_s rparecord_t;

/**
 * Tree walk callback
 */
typedef long (*rpa_recordtree_callback)(rarray_t *records, long rec, rpointer userdata);


/**
 * \struct rparecord_s <rpa/rparecord.h> <rpa/rparecord.h>
 * Abstract Syntax Tree (AST) construction element.
 */
struct rparecord_s {
	ruint32 top;			/**< This is a private member, used by the engine and is not significant to the user */
	ruint32 size;			/**< This is a private member, used by the engine and is not significant to the user */
	const char *rule;		/**< Name of the rule that generated this record. This pointer points to memory allocated
	 	 	 	 	 	 	 *   inside @ref rpadbex_t, so make sure rpadbex_t object is still valid while accessing this pointer. */
	const char *input;		/**< Pointer in the input stream. This pointer points to memory inside the input buffer
	 	 	 	 	 	 	 *   passed to @ref rpa_stat_parse, make sure this memory is still valid while accessing this pointer. */
	unsigned long inputoff;		/**< Input offset, calculated from the start parameter passed to @ref rpa_stat_parse */
	unsigned long inputsiz;		/**< Size of input */
	ruint32 type;			/**< Record Type: @ref RPA_RECORD_START or @ref RPA_RECORD_END */
	ruint32 ruleid;			/**< Unique ID, identifying the BNF rule that created the record */
	ruint32 ruleuid;		/**< User specified Rule ID. If you used directive @ref emitid for this rulename, this member will contain the specified ID */
	ruint32 usertype;		/**< User specified type. */
	ruword userdata;			/**< Scratch area. This member can be used to associate some user specific data with this record. */
};


long rpa_recordtree_walk(rarray_t *src, long rec, long level, rpa_recordtree_callback callaback, rpointer userdata);

/**
 * Return record offset for the corresponding type. Use this function to
 * locate the start or end record for a given record. For example if you
 * have a record r1 (of type RPA_RECORD_START) marking the beginning of a
 * branch and you need to locate its counterpart, you can do it with this
 * function by specifying RPA_RECORD_END for type parameter. If the rec type
 * is the same as the specified type parameter this function will return rec.
 * In the example above: rpa_recordtree_get(records, 0, RPA_RECORD_END) will
 * return 7. rpa_recordtree_get(records, 7, RPA_RECORD_START) will return 0.
 * rpa_recordtree_get(records, 1, RPA_RECORD_END) will return 2.
 * rpa_recordtree_get(records, 1, RPA_RECORD_START) will return 1.
 * @param records array of AST records (populated by @ref rpa_stat_parse in most cases).
 * @param rec record offset in the records array.
 * @param type The type of the requested record.
 * @return Return the counterpart record of rec if type is different than the rec type,
 * or the specified rec if the type is the same as the rec type.
 */
long rpa_recordtree_get(rarray_t *records, long rec, unsigned long type);


/**
 * Return the first child.
 * @param records array of AST records.
 * @param rec record offset in the records array.
 * @param type The type of the requested record.
 * @return first child record of the specified type. If there are no children return -1.
 */
long rpa_recordtree_firstchild(rarray_t *records, long rec, unsigned long type);


/**
 * Return the last child.
 * @param records array of AST records.
 * @param rec record offset in the records array.
 * @param type The type of the requested record.
 * @return last child record of the specified type. If there are no children return -1.
 */
long rpa_recordtree_lastchild(rarray_t *records, long rec, unsigned long type);


/**
 * Return the next sibling.
 * @param records array of AST records.
 * @param rec record offset in the records array.
 * @param type The type of the requested record.
 * @return next sibling record of the specified type. If rec is the last sibling, return -1.
 */
long rpa_recordtree_next(rarray_t *records, long rec, unsigned long type);


/**
 * Return the prev sibling.
 * @param records array of AST records.
 * @param rec record offset in the records array.
 * @param type The type of the requested record.
 * @return previous sibling record of the specified type. If rec is the first sibling, return -1.
 */
long rpa_recordtree_prev(rarray_t *records, long rec, unsigned long type);


/**
 * Return the parent node.
 * @param records array of AST records.
 * @param rec record offset in the records array.
 * @param type The type of the requested record.
 * @return Return the parent of rec. If rec has no parent return -1.
 */
long rpa_recordtree_parent(rarray_t *records, long rec, unsigned long type);


/**
 * Return the number of records in the branch specified by rec.
 * @param records array of AST records.
 * @param rec record offset in the records array.
 * @return Return the records count in the specified branch.
 */
long rpa_recordtree_size(rarray_t *records, long rec);					/* Size of the tree */


/**
 * Copy a branch to another array.
 * @param dst Destination array.
 * @param src Source array.
 * @param rec Source branch.
 * @return Return the number of records copied.
 */
long rpa_recordtree_copy(rarray_t *dst, rarray_t *src, long rec);


/**
 * Return a pointer to a record at offset rec from the records array.
 * @param records An array of records populated by a @ref rpa_stat_parse operation.
 * @param rec record offset.
 */
rparecord_t *rpa_record_get(rarray_t *records, long rec);


/**
 * Create array for rparecord_t elements.
 * @return Return empty array or NULL if creation failed.
 */
rarray_t *rpa_records_create();

/**
 * Destroy array created with @ref rpa_records_create
 * @param records Specifies the records array to be destroyed.
 */
void rpa_records_destroy(rarray_t *records);

int rpa_recordtree_move(rarray_t *records, long dst, long src, long size);

void rpa_record_dumpindented(rarray_t *records, long rec, int level);
void rpa_record_dump(rarray_t *records, long rec, int wantuserdata);
long rpa_record_getruleuid(rarray_t *records, long rec);
void rpa_record_setusertype(rarray_t *records, long rec, ruint32 usertype, rvalset_t op);
long rpa_record_getusertype(rarray_t *records, long rec);
int rpa_record_optchar(rparecord_t *prec, int defc);
int rpa_record_loopchar(rparecord_t *prec, int defc);
long rpa_records_length(rarray_t *records);
rparecord_t *rpa_records_slot(rarray_t *records, long index);

#ifdef __cplusplus
}
#endif

#endif
