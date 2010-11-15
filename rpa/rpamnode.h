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

#ifndef _RPAMNODE_H_
#define _RPAMNODE_H_

#include "rpamatch.h"
#include "rpalist.h"
#include "rpaclass.h"
#include "rpatypedef.h"


#ifdef __cplusplus
extern "C" {
#endif


struct rpa_mnode_s {
	rpa_class_t cls;
	rpa_head_t mlink;
	rpa_match_t *match;
	unsigned int flags;
};


struct rpa_mnode_callback_s {
	rpa_mnode_t base;
	rpa_head_t cblink;		/* future use */
	RPA_MATCH_CALLBACK matched_callback;
	void *userdata;
};


typedef struct rpa_mnode_callback_arg1_s {
	rpa_mnode_callback_t base;
	rpa_word_t arg1;
} rpa_mnode_callback_arg1_t;


typedef struct rpa_mnode_callback_arg2_s {
	rpa_mnode_callback_arg1_t base;
	rpa_word_t arg2;
} rpa_mnode_callback_arg2_t;


rpa_mnode_t *rpa_mnode_create(
	rpa_match_t *match, 
	unsigned int flags);
rpa_mnode_t *rpa_mnode_callback_create(
	rpa_match_t *match, 
	unsigned int flags, 
	RPA_MATCH_CALLBACK matched_callback, 
	void *userdata);
rpa_mnode_t *rpa_mnode_callback_arg1_create(
	rpa_match_t *match, 
	unsigned int flags, 
	RPA_MATCH_CALLBACK matched_callback, 
	void *userdata,
	rpa_word_t arg1);
rpa_mnode_t *rpa_mnode_callback_arg2_create(
	rpa_match_t *match, 
	unsigned int flags, 
	RPA_MATCH_CALLBACK matched_callback, 
	void *userdata,
	rpa_word_t arg1,
	rpa_word_t arg2);
	
rpa_mnode_t *rpa_mnode_init(
	rpa_mnode_t *node,
	rpa_match_t *match, 
	unsigned int flags,
	rpa_class_methods_t *vptr);
rpa_mnode_t *rpa_mnode_callback_init(
	rpa_mnode_callback_t *mnode,
	rpa_match_t *match, 
	unsigned int flags, 
	RPA_MATCH_CALLBACK matched_callback, 
	void *userdata,
	rpa_class_methods_t *vptr);	
int rpa_mnode_exec_callback(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason);
int rpa_mnode_print(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason);
int rpa_mnode_debug_print(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason);
int rpa_mnode_check_for_loop(rpa_mnode_t *mnode, rpa_match_t *loop, rpa_head_t *mhead, int reclevel);


int rpa_mnode_plain(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);
int rpa_mnode_multiple(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);
int rpa_mnode_optional(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);
int rpa_mnode_multiopt(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);

int rpa_mnode_nlist_plain(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);
int rpa_mnode_nlist_multiple(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);
int rpa_mnode_nlist_optional(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);
int rpa_mnode_nlist_multiopt(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);


int rpa_mnode_callback_plain(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);
int rpa_mnode_callback_multiple(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);
int rpa_mnode_callback_optional(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);
int rpa_mnode_callback_multiopt(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);

int rpa_mnode_callback_loop_plain(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);
int rpa_mnode_callback_loop_multiple(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);
int rpa_mnode_callback_loop_optional(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);
int rpa_mnode_callback_loop_multiopt(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);


/* Parser functionality */
int rpa_mnode_p_plain(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);
int rpa_mnode_p_multiple(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);
int rpa_mnode_p_optional(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);
int rpa_mnode_p_multiopt(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);

int rpa_mnode_p_callback_plain(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);
int rpa_mnode_p_callback_multiple(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);
int rpa_mnode_p_callback_optional(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);
int rpa_mnode_p_callback_multiopt(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);

int rpa_mnode_p_callback_loop_plain(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);
int rpa_mnode_p_callback_loop_multiple(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);
int rpa_mnode_p_callback_loop_optional(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);
int rpa_mnode_p_callback_loop_multiopt(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input);


void rpa_mnode_cat_list(rpa_head_t *head, rpa_mnode_t *first, ...);
rpa_mnode_t *rpa_mnode_list_append(rpa_head_t *head, rpa_mnode_t *node);


#ifdef __cplusplus
}
#endif


#endif
