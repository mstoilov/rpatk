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
 * \file rparecord.h
 * \brief The public interface for working with the produced Abstract Syntax Tree (AST).
 *
 *
 * <h2>Synopsis</h2>
 * Upon a successful call to \ref rpa_stat_parse, the parser produces a stack of \ref rparecord_t records.
 * There are two kinds of records: \ref RPA_RECORD_START and \ref RPA_RECORD_END. \ref RPA_RECORD_START marks
 * the beginning of a branch and \ref RPA_RECORD_END marks the end of that branch. Empty branches are specified by
 * a record \ref RPA_RECORD_START followed immediately by \ref RPA_RECORD_END (no child records in between).
 * Empty branches are considered leaves.
 *
 */


#ifndef _RPARECORD_H_
#define _RPARECORD_H_

#include "rtypes.h"
#include "rarray.h"
#include "rlist.h"
#include "rpavm.h"


#ifdef __cplusplus
extern "C" {
#endif

#define RPA_RECORD_NONE (0)
#define RPA_RECORD_START (1 << 0)
#define RPA_RECORD_END (1 << 1)
#define RPA_RECORD_MATCH (1 << 2)
#define RPA_RECORD_HEAD (1 << 3)
#define RPA_RECORD_TAIL (1 << 4)

#define RPA_RECORD_INVALID_UID ((ruint32)-1)

/**
 * \typedef rparecord_t
 * \brief Abstract Syntax Tree (AST) construction element.
 */
typedef struct rparecord_s {
	rlong next;
	const rchar *rule;
	const rchar *input;
	rsize_t inputsiz;
	ruint32 type;
	ruint32 top;
	ruint32 size;
	ruint32 ruleuid;
	ruint32 usertype;
	rword userdata;
} rparecord_t;


typedef rlong (*rpa_recordtree_callback)(rarray_t *records, rlong rec, rpointer userdata);

rlong rpa_recordtree_walk(rarray_t *src, rlong rec, rlong level, rpa_recordtree_callback callaback, rpointer userdata);
rlong rpa_recordtree_get(rarray_t *records, rlong rec, rulong type);
rlong rpa_recordtree_firstchild(rarray_t *records, rlong rec, rulong type);
rlong rpa_recordtree_lastchild(rarray_t *records, rlong rec, rulong type);
rlong rpa_recordtree_next(rarray_t *records, rlong rec, rulong type);
rlong rpa_recordtree_prev(rarray_t *records, rlong rec, rulong type);
rlong rpa_recordtree_parent(rarray_t *records, rlong rec, rulong type);
rlong rpa_recordtree_rotatedown(rarray_t *records, rlong parent);			/* Rotate children down, the last child becomes the first */
rlong rpa_recordtree_size(rarray_t *records, rlong rec);					/* Size of the tree */
rlong rpa_recordtree_copy(rarray_t *dst, rarray_t *src, rlong rec);
rparecord_t *rpa_record_get(rarray_t *records, rlong rec);

void rpa_record_dumpindented(rarray_t *records, rlong rec, rinteger level);
void rpa_record_dump(rarray_t *records, rlong rec);
rlong rpa_record_getruleuid(rarray_t *records, rlong rec);
void rpa_record_setusertype(rarray_t *records, rlong rec, ruint32 usertype, rvalset_t op);
rlong rpa_record_getusertype(rarray_t *records, rlong rec);
rinteger rpa_record_optchar(rparecord_t *prec, rinteger defc);
rinteger rpa_record_loopchar(rparecord_t *prec, rinteger defc);
rarray_t *rpa_records_create();
void rpa_records_destroy(rarray_t *records);
rlong rpa_records_length(rarray_t *records);
rparecord_t *rpa_records_slot(rarray_t *records, rlong index);

#ifdef __cplusplus
}
#endif

#endif
