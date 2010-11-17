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

#include "rstring.h"

#include "rpacharconv.h"
#include "rmem.h"
#include "rpavar.h"
#include "rpavarlink.h"
#include "rpaparser.h"
#include "rpamnode.h"
#include "rpamatchstr.h"
#include "rpamatchspecial.h"
#include "rpamatchlist.h"
#include "rpamatchval.h"
#include "rpamatchrange.h"
#include "rpadbexpriv.h"
#include "rpadebug.h"

/*
 * Rules:
 * 
 * R[0] - name, matchptr
 * R[1] - size
 * R[2] - count
 * R[5] - val, match function
 * R[6] - val
 * R[7] - current match (list)
 * R[8] - current mnode
 * R[11] - mnode flags
 * 
 * during reason 'start'the current match is saved on the stack and the newly created match is saved in R[8]
 * during reason 'end' the match is assigned to the current mnode and the previous match is restored from the stack
 */ 

#define RPA_MATCH_NEWLINE_CLASSID	(('\0' << 24) | ('n' << 16) | ('l' << 8) | ('n' << 0))
#define RPA_MATCH_LASTCHR_CLASSID	(('\0' << 24) | ('c' << 16) | ('s' << 8) | ('l' << 0))
#define MATCH2LIST(m) rpa_varlink_list_add_match((m), &parser->bnftree)
#define MNODE2LIST(m) rpa_varlink_list_add_mnode((m), &parser->bnftree)
#define RPA_VMCODE_SIZE 256
#define RPA_VMCODE_GROW 2048

static rpa_mnode_t *rpa_varlink_list_add_mnode(rpa_mnode_t *mnode, rpa_head_t *head);
static rpa_match_t *rpa_varlink_list_add_match(rpa_match_t *match, rpa_head_t *head);

int rpa_parser_check_space(rpa_parser_t *parser)
{
	rpa_asmins_t *vmcode;
	rpa_word_t vmcode_size;

	if (parser->vmcode_size - parser->vmcode_off < RPA_VMCODE_GROW) {
		vmcode_size = parser->vmcode_size + RPA_VMCODE_GROW;
		vmcode = (rpa_asmins_t *)r_realloc(parser->vmcode, (unsigned long)(sizeof(rpa_asmins_t) * vmcode_size));
		if (!vmcode)
			return -1;
		parser->vmcode_size = vmcode_size;
		parser->vmcode = vmcode;
	}
	return 0;
}


rpa_parser_t *rpa_parser_create()
{
	rpa_parser_t *parser;

	parser = (rpa_parser_t *)r_malloc(sizeof(*parser));
	if (!parser)
		return ((void*)0);
	r_memset(parser, 0, sizeof(*parser));
	rpa_list_init(&parser->bnftree);
	rpa_stat_init(&parser->stat);
	rpa_stat_set_encoding(&parser->stat, RPA_ENCODING_UTF8);
	rpa_parser_setup_bnftree(parser);
	parser->stack = rpa_wordstack_create(256, 16);
	if (!parser->stack)
		goto error;
	parser->vmcode_size = RPA_VMCODE_SIZE;
	parser->vmcode_off = 0;
	parser->vmcode = (rpa_asmins_t *)r_malloc((unsigned long)(sizeof(rpa_asmins_t) * parser->vmcode_size));
	if (!parser->vmcode)
		goto error;
	return parser;
	
error:
	rpa_parser_destroy(parser);
	return (void*)0;
}


void rpa_parser_destroy(rpa_parser_t *parser)
{
	rpa_varlink_destroy_all(&parser->bnftree);
	r_free(parser->vmcode);
	if (parser->stack)
		rpa_wordstack_destroy(parser->stack);
	rpa_stat_cleanup(&parser->stat);
	r_free(parser);
}


int rpa_parser_exec(rpa_parser_t *parser, const char *input, unsigned long size)
{
	int ret = 0;

	if (parser->pVarLinkBnfRoot)
		ret = rpa_stat_match_lite(&parser->stat, parser->pVarLinkBnfRoot, input, input, input + size);
	return ret;
}

int rpa_parser_mnode_print(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
#ifdef CODEGENDEBUG
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;	
	fprintf(stdout, "%5lu: ", parser->vmcode_off);
#endif

	rpa_mnode_debug_print(mnode, stat, input, size, reason);
	return size;
}


int rpa_parser_mnode_print_dump_code(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason, rpa_word_t codepos)
{

#ifdef CODEGENDEBUG
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;	
	fprintf(stdout, "%5lu: ", codepos);
	rpa_mnode_print(mnode, stat, input, size, reason);
	if (parser->vmcode_off > codepos)
		rpa_asm_dump(&parser->vmcode[codepos], parser->vmcode_off - codepos);
#else
	rpa_mnode_debug_print(mnode, stat, input, size, reason);
#endif
	return size;
}


void rpa_parser_gencode_str_start(rpa_parser_t *parser, const char *input, unsigned int size)
{
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, DA, XX, (rpa_word_t)0);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, DA, XX, (rpa_word_t)0);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R2, DA, XX, (rpa_word_t)0);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R4, DA, XX, (rpa_word_t)0);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R5, DA, XX, (rpa_word_t)0);
}


void rpa_parser_gencode_str_end(rpa_parser_t *parser, const char *input, unsigned int size)
{
	/*
	 * Save the current match ptr on the stack
	 */
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, DA, XX, (rpa_word_t)input);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, DA, XX, (rpa_word_t)size);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R2, R4, XX, 0);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_CREATE_STR_MATCHPTR);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R6, R0, XX, 0);

	/*
	 * Pop all values off the stack
	 */
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_CMP, R4, DA, XX, 0);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_BLEQ, DA, XX, XX, 8);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_POP, R1, XX, XX, 0);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R2, R4, XX, 0);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_SUB, R2, R2, DA, 1);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, R6, XX, 0);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_SET_STRVAL);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_SUB, R4, R4, DA, 1);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_B, DA, XX, XX, -8);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, DA, XX, 0);

	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_PUSH, R7, XX, XX, 0);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R7, R6, XX, 0);
	
	
}


void rpa_parser_gencode_list_start(rpa_parser_t *parser, const char *input, unsigned int size, rpa_word_t lmf)
{
	/*
	 * Save the current list on the stack
	 */
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_PUSH, R7, XX, XX, 0);
	
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, DA, XX, (rpa_word_t)0);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, DA, XX, (rpa_word_t)0);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R2, DA, XX, (rpa_word_t)lmf);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_CREATE_LIST_MATCHPTR);
	
	/*
	 * Set the newly created list to be the current
	 */
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R7, R0, XX, 0);
}


void rpa_parser_gencode_rangelist_start(rpa_parser_t *parser, const char *input, unsigned int size)
{
	/*
	 * Save the current list on the stack
	 */
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_PUSH, R7, XX, XX, 0);
	
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, DA, XX, (rpa_word_t)0);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, DA, XX, (rpa_word_t)0);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_CREATE_RANGELIST_MATCHPTR);
	
	/*
	 * Set the newly created list to be the current
	 */
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R7, R0, XX, 0);
}


void rpa_parser_gencode_list_end(rpa_parser_t *parser, const char *input, unsigned int size)
{
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, R7, XX, 0);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, DA, XX, (rpa_word_t)input);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R2, DA, XX, (rpa_word_t)size);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_SET_MATCH_NAME);

}

#if 0
void rpa_parser_gencode_nlist_start(rpa_parser_t *parser, const char *input, unsigned int size, rpa_word_t lmf)
{
	/*
	 * Save the current list on the stack
	 */
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_PUSH, R7, XX, XX, 0);
	
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, DA, XX, (rpa_word_t)0);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, DA, XX, (rpa_word_t)0);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R2, DA, XX, (rpa_word_t)lmf);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_CREATE_NLIST_MATCHPTR);
	
	/*
	 * Set the newly created list to be the current
	 */
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R7, R0, XX, 0);
	
}
#endif

void rpa_parser_gencode_nlist_end(rpa_parser_t *parser, const char *input, unsigned int size)
{
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, R7, XX, 0);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, DA, XX, (rpa_word_t)input);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R2, DA, XX, (rpa_word_t)size);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_SET_MATCH_NAME);

}


void rpa_parser_gencode_hlist_start(rpa_parser_t *parser, const char *input, unsigned int size, rpa_word_t lmf)
{
	/*
	 * Save the current list on the stack
	 */
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_PUSH, R7, XX, XX, 0);
	
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, DA, XX, (rpa_word_t)0);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, DA, XX, (rpa_word_t)0);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R2, DA, XX, (rpa_word_t)lmf);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_CREATE_HLIST_MATCHPTR);
	
	/*
	 * Set the newly created list to be the current
	 */
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R7, R0, XX, 0);
}


void rpa_parser_gencode_hlist_end(rpa_parser_t *parser, const char *input, unsigned int size)
{
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, R7, XX, 0);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, DA, XX, (rpa_word_t)input);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R2, DA, XX, (rpa_word_t)size);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_SET_MATCH_NAME);

}


void rpa_parser_gencode_set_matchfunc(rpa_parser_t *parser, const char *input, unsigned int size, rpa_word_t lmf)
{
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, R7, XX, 0);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, DA, XX, (rpa_word_t)lmf);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_SET_MATCH_FUNCTION);

}


void rpa_parser_gencode_nlist_set_matchfunc(rpa_parser_t *parser, const char *input, unsigned int size, rpa_word_t lmf)
{
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, R7, XX, 0);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_GET_MATCH_FUNCTION);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_CMP, R0, DA, XX, lmf);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_BEQ, DA, XX, XX, 3);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, R7, XX, 0);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_RESET_LIST);

	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_CMP, R0, DA, XX, RPA_MATCHFUNC_LIST);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_BNEQ, DA, XX, XX, 3);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, R7, XX, 0);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_RESET_LIST);

	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, R7, XX, 0);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, DA, XX, (rpa_word_t)lmf);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_SET_MATCH_FUNCTION);

}


void rpa_parser_gencode_setup_list(rpa_parser_t *parser)
{
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, R7, XX, 0);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_SETUP_LIST);

}


void rpa_parser_gencode_nlist_get(rpa_parser_t *parser, const char *input, unsigned int size)
{
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, DA, XX, (rpa_word_t)input);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, DA, XX, (rpa_word_t)size);
	parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_GET_NLIST_MATCHPTR);
}


int rpa_parser_cb_assign(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	rpa_word_t codepos = parser->vmcode_off;

	switch (reason) {
		case RPA_REASON_START:
			break;
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:
			if (r_strncmp(input, "::=", size) == 0)
				rpa_parser_gencode_nlist_set_matchfunc(parser, input, size, RPA_MATCHFUNC_LIST);
			else if (r_strncmp(input, "++=", size) == 0)
				rpa_parser_gencode_nlist_set_matchfunc(parser, input, size, RPA_MATCHFUNC_NLIST_BESTALT);
			else if (r_strncmp(input, "||=", size) == 0)
				rpa_parser_gencode_nlist_set_matchfunc(parser, input, size, RPA_MATCHFUNC_NLIST_ALT);
			else
				rpa_parser_gencode_nlist_set_matchfunc(parser, input, size, RPA_MATCHFUNC_LIST);
			break;
		case RPA_REASON_END:
			break;
		default:
			break;
	}	
	
	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
	return size;
}


int rpa_parser_cb_opsign(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	rpa_word_t codepos = parser->vmcode_off;
	rpa_word_t lmf;
	
	switch (reason) {
		case RPA_REASON_START:
			break;
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:
			switch (*input) {
				case '^': 
					lmf = RPA_MATCHFUNC_LIST_NOT;
					break;
				case '|': 
					lmf = RPA_MATCHFUNC_LIST_ALT;				
					break;
				case '&': 
					lmf = RPA_MATCHFUNC_LIST_AND;
					break;
				case '-': 
					lmf = RPA_MATCHFUNC_LIST_MINUS;
					break;
				case '%': 
					lmf = RPA_MATCHFUNC_LIST_CONTAIN;
					break;
				case '@': 
					lmf = RPA_MATCHFUNC_LIST_AT;
					break;
				default:
					lmf = RPA_MATCHFUNC_LIST;
					break;
			}
			rpa_parser_gencode_set_matchfunc(parser, input, size, lmf);
			break;
		case RPA_REASON_END:
			break;
		default:
			break;
	}	
	
	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
	return size;
}


int rpa_parser_cb_newline_matchptr(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	rpa_word_t codepos = parser->vmcode_off;

	switch (reason) {
		case RPA_REASON_START:
			break;
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:
			/*
			 * Save the current match ptr on the stack
			 */
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_PUSH, R7, XX, XX, 0);

			/*
			 * Create a new val matchptr and set it in R7 to be the current one
			 */
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, DA, XX, (rpa_word_t)input);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, DA, XX, (rpa_word_t)size);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_CREATE_NEWLINE_MATCHPTR);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R7, R0, XX, 0);
			break;
		case RPA_REASON_END:
			break;
		default:
			break;
	}	
	
	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
				
	return size;
}


int rpa_parser_cb_anychar_matchptr(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	rpa_word_t codepos = parser->vmcode_off;

	switch (reason) {
		case RPA_REASON_START:
			break;
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:
			/*
			 * Save the current match ptr on the stack
			 */
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_PUSH, R7, XX, XX, 0);

			/*
			 * Create a new val matchptr and set it in R7 to be the current one
			 */
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, DA, XX, (rpa_word_t)input);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, DA, XX, (rpa_word_t)size);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_CREATE_ANYCHAR_MATCHPTR);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R7, R0, XX, 0);
			break;
		case RPA_REASON_END:
			break;
		default:
			break;
	}	
	
	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
				
	return size;
}


int rpa_parser_cb_fun_matchptr(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	rpa_word_t excall = ((rpa_mnode_callback_arg1_t*)mnode)->arg1;
	rpa_word_t codepos = parser->vmcode_off;

	switch (reason) {
		case RPA_REASON_START:
			break;
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:
			/*
			 * Save the current match ptr on the stack
			 */
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_PUSH, R7, XX, XX, 0);

			/*
			 * Create a new val matchptr and set it in R7 to be the current one
			 */
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, DA, XX, (rpa_word_t)input);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, DA, XX, (rpa_word_t)size);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, excall);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R7, R0, XX, 0);
			break;
		case RPA_REASON_END:
			break;
		default:
			break;
	}	
	
	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
				
	return size;
}


int rpa_parser_cb_val_matchptr(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	rpa_word_t codepos = parser->vmcode_off;

	switch (reason) {
		case RPA_REASON_START:
			break;
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:
			/*
			 * Save the current match ptr on the stack
			 */
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_PUSH, R7, XX, XX, 0);

			/*
			 * Create a new val matchptr and set it in R7 to be the current one
			 */
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, DA, XX, (rpa_word_t)input);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, DA, XX, (rpa_word_t)size);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R2, R5, XX, 0); 
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_CREATE_VAL_MATCHPTR);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R7, R0, XX, 0);
			break;
		case RPA_REASON_END:
			break;
		default:
			break;
	}	
	
	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
				
	return size;
}


int rpa_parser_cb_hexnum_r5(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	unsigned int wc = 0;
	char *endptr = (void*)0;
	rpa_word_t codepos = parser->vmcode_off;

	wc = r_strtoul(input, &endptr, 16);
	
	switch (reason) {
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:
			rpa_parser_check_space(parser);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R5, DA, XX, (rpa_word_t)wc);
			break;
		default:
			break;
	}

	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
	return size;
}


int rpa_parser_cb_hexnum_r6(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	unsigned int wc = 0;
	char *endptr = (void*)0;
	rpa_word_t codepos = parser->vmcode_off;

	wc = r_strtoul(input, &endptr, 16);
	
	switch (reason) {
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:
			rpa_parser_check_space(parser);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R6, DA, XX, (rpa_word_t)wc);
			break;
		default:
			break;
	}

	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
	return size;
}


int rpa_parser_cb_decnum_r5(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	unsigned int wc = 0;
	char *endptr = (void*)0;
	rpa_word_t codepos = parser->vmcode_off;

	wc = r_strtoul(input, &endptr, 10);
	
	switch (reason) {
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:
			rpa_parser_check_space(parser);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R5, DA, XX, (rpa_word_t)wc);
			break;
		default:
			break;
	}

	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
	return size;
}


int rpa_parser_cb_decnum_r6(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	unsigned int wc = 0;
	char *endptr = (void*)0;
	rpa_word_t codepos = parser->vmcode_off;

	wc = r_strtoul(input, &endptr, 10);
	
	switch (reason) {
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:
			rpa_parser_check_space(parser);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R6, DA, XX, (rpa_word_t)wc);
			break;
		default:
			break;
	}

	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
	return size;
}


int rpa_parser_cb_val_r5(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	unsigned int wc = 0;
	rpa_word_t codepos = parser->vmcode_off;

	if (rpa_utf8_mbtowc(&wc, (const unsigned char*)input, (const unsigned char*)(input + size)) < 0) {
		wc = *input;
	}
	switch (reason) {
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:
			rpa_parser_check_space(parser);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R5, DA, XX, (rpa_word_t)wc);
			break;
		default:
			break;
	}

	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
	return size;
}


int rpa_parser_cb_push_r5_inc_r4(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	unsigned int wc = 0;
	rpa_word_t codepos = parser->vmcode_off;

	if (rpa_utf8_mbtowc(&wc, (const unsigned char*)input, (const unsigned char*)(input + size)) < 0) {
		wc = *input;
	}
	switch (reason) {
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:
			rpa_parser_check_space(parser);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_PUSH, R5, XX, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_ADD, R4, R4, DA, 1);
			break;
		default:
			break;
	}

	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
	return size;
}


int rpa_parser_cb_esc_val_r5(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	unsigned int wc = 0;
	const char *chInput = input;
	rpa_word_t codepos = parser->vmcode_off;

	switch (reason) {
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:
			if (*chInput == '\\') {
				++chInput;
				if (rpa_utf8_mbtowc(&wc, (const unsigned char*)chInput, (const unsigned char*)(input + size)) < 0)
					wc = *input;
				switch (wc) {
				case 'n':
					wc = '\n';
					break;
				case 'r':
					wc = '\r';
					break;
				case 't':
					wc = '\t';
					break;
				case '\'':
					wc = '\'';
						break;
				case '\"':
					wc = '\"';
					break;
				case '\\':
					wc = '\\';
					break;
				}	
			}
			rpa_parser_check_space(parser);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R5, DA, XX, (rpa_word_t)wc);
			break;
		default:
			break;
	}

	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
	return size;
}


int rpa_parser_cb_val_r6(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	unsigned int wc = 0;
	rpa_word_t codepos = parser->vmcode_off;
	
	if (rpa_utf8_mbtowc(&wc, (const unsigned char*)input, (const unsigned char*)(input + size)) < 0) {
		wc = *input;
	}
	switch (reason) {
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:
			rpa_parser_check_space(parser);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R6, DA, XX, (rpa_word_t)wc);
			break;
			break;
		default:
			break;
	}			

	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
	return size;
}


int rpa_parser_cb_range_matchptr(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	rpa_word_t codepos = parser->vmcode_off;

	switch (reason) {
		case RPA_REASON_START:
			break;
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:
			/*
			 * Save the current match ptr on the stack
			 */
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_PUSH, R7, XX, XX, 0);

			/*
			 * Create a new val matchptr and set it in R7 to be the current one
			 */		
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, DA, XX, (rpa_word_t)input);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, DA, XX, (rpa_word_t)size);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R2, R5, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R3, R6, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_CREATE_RANGE_MATCHPTR);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R7, R0, XX, 0);
			break;
		case RPA_REASON_END:
			break;
		default:
			break;
	}
	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
	return size;
}


int rpa_parser_cb_mnodeptr_in_nlist(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	rpa_word_t codepos = parser->vmcode_off;

	switch (reason) {
		case RPA_REASON_START:
			break;
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, R7, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, DA, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_CREATE_MNODE);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R10, R0, XX, 0);	// save the mnode in R10


			/*
			 * Add the mnode to the current list
			 */
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, R0, XX, 0);	
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_POP, R7, XX, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, R7, XX, 0);	
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_ADD_MNODE_TO_LIST);
			
			/*
			 * Add the mnode to the hash
			 */
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, R7, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, R10, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_ADD_MNODE_TO_LIST_DATAPTR);
			
		case RPA_REASON_END:
			break;
		default:
			break;
		};

	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
	return size;
}


int rpa_parser_cb_mnodeptr_in_altlist(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	rpa_word_t codepos = parser->vmcode_off;

	switch (reason) {
		case RPA_REASON_START:
			break;
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, R7, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R10, R7, XX, 0);	// save current match in R10
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, DA, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_CREATE_MNODE);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R9, R0, XX, 0);		// save the newly created mnode in R9

			/*
			 * Add the mnode to the current list
			 */
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, R0, XX, 0);	
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_POP, R7, XX, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, R7, XX, 0);	
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_ADD_MNODE_TO_LIST);
			
			/*
			 * Add the mnode to the hash
			 */
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, R7, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, R9, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_ADD_MNODE_TO_LIST_DATAPTR);

			
		case RPA_REASON_END:
			break;
		default:
			break;
		};

	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
	return size;
}


int rpa_parser_cb_mnodeptr(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	rpa_word_t codepos = parser->vmcode_off;

	switch (reason) {
		case RPA_REASON_START:
			break;
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, R7, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, DA, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_CREATE_MNODE);

			/*
			 * Add the mnode to the current list
			 */
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, R0, XX, 0);	
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_POP, R7, XX, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, R7, XX, 0);	
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_ADD_MNODE_TO_LIST);			
		case RPA_REASON_END:
			break;
		default:
			break;
		};

	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
	return size;
}


int rpa_parser_cb_q_mnodeptr(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	rpa_word_t codepos = parser->vmcode_off;

	switch (reason) {
		case RPA_REASON_START:
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R11, DA, XX, 0);
			break;
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, R7, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, R11, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_CREATE_MNODE);

			/*
			 * Add the mnode to the current list
			 */
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, R0, XX, 0);	
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_POP, R7, XX, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, R7, XX, 0);	
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_ADD_MNODE_TO_LIST);			
		case RPA_REASON_END:
			break;
		default:
			break;
		};

	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
	return size;
}


static int rpa_parser_acb_q_mnodeptr_callback(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	rpa_word_t codepos = parser->vmcode_off;

	switch (reason) {
		case RPA_REASON_START:
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R11, DA, XX, 0);
			break;
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, R7, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, R11, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_ORR, R1, R1, DA, RPA_MNODE_CALLBACK);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_CREATE_MNODE_CALLBACK);

			/*
			 * Add the mnode to the current list
			 */
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, R0, XX, 0);	
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_POP, R7, XX, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, R7, XX, 0);	
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_ADD_MNODE_TO_LIST);			
		case RPA_REASON_END:
			break;
		default:
			break;
		};

	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
	return size;
}


static int rpa_parser_scb_q_mnodeptr_callback(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	rpa_word_t codepos = parser->vmcode_off;

	switch (reason) {
		case RPA_REASON_START:
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R11, DA, XX, 0);
			break;
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, R7, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, R11, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_ORR, R1, R1, DA, RPA_MNODE_CALLBACK | RPA_MNODE_SYNCRONOUS);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_CREATE_MNODE_CALLBACK);

			/*
			 * Add the mnode to the current list
			 */
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, R0, XX, 0);	
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_POP, R7, XX, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, R7, XX, 0);	
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_ADD_MNODE_TO_LIST);			
		case RPA_REASON_END:
			break;
		default:
			break;
		};

	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
	return size;
}


static int rpa_parser_ncb_q_mnodeptr_callback(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	rpa_word_t codepos = parser->vmcode_off;

	switch (reason) {
		case RPA_REASON_START:
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R11, DA, XX, 0);
			break;
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, R7, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, R11, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_ORR, R1, R1, DA, RPA_MNODE_CALLBACK | RPA_MNODE_NOCONNECT);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_CREATE_MNODE_CALLBACK);

			/*
			 * Add the mnode to the current list
			 */
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R1, R0, XX, 0);	
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_POP, R7, XX, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, R7, XX, 0);	
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_ADD_MNODE_TO_LIST);			
		case RPA_REASON_END:
			break;
		default:
			break;
		};

	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
	return size;
}


int rpa_parser_cb_quantity(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_word_t flags = 0;
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	rpa_word_t codepos = parser->vmcode_off;

	if (size) {
		switch (*input) {
		case '*':
			flags = RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE;
			break;
		case '+':
			flags = RPA_MATCH_MULTIPLE;
			break;
		case '?':
			flags = RPA_MATCH_OPTIONAL;
			break;
		default:
			flags = 0;
			break;
		}
	}

	switch (reason) {
		case RPA_REASON_START:
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R11, DA, XX, 0);
			break;
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R11, DA, XX, flags);
			break;
		case RPA_REASON_END:
			break;
		default:
			break;
	}

	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
	return size;
}

int rpa_parser_cb_class(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	rpa_word_t codepos = parser->vmcode_off;
	
	switch (reason) {
		case RPA_REASON_START:
			rpa_parser_check_space(parser);
			rpa_wordstack_check_space(parser->stack);
			rpa_wordstack_push(parser->stack, parser->vmcode_off);
			rpa_parser_gencode_rangelist_start(parser, input, size);
			break;
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:			
			rpa_parser_gencode_list_end(parser, input, size);
			rpa_parser_gencode_setup_list(parser);
			rpa_wordstack_pop(parser->stack);		
			break;
		case RPA_REASON_END:
			parser->vmcode_off = rpa_wordstack_pop(parser->stack);		
			break;
		default:
			break;
	}

	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
	return size;
}


int rpa_parser_cb_seqexpr(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	rpa_word_t codepos = parser->vmcode_off;

	switch (reason) {
		case RPA_REASON_START:
			rpa_parser_check_space(parser);
			rpa_wordstack_check_space(parser->stack);
			rpa_wordstack_push(parser->stack, parser->vmcode_off);
			rpa_parser_gencode_list_start(parser, input, size, RPA_MATCHFUNC_LIST);
			break;
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:			
			rpa_parser_gencode_list_end(parser, input, size);
			rpa_parser_gencode_setup_list(parser);
			rpa_wordstack_pop(parser->stack);		
			break;
		case RPA_REASON_END:
			parser->vmcode_off = rpa_wordstack_pop(parser->stack);		
			break;
		default:
			break;
	}


	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
	return size;
}


int rpa_parser_cb_strexpr(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	rpa_word_t codepos = parser->vmcode_off;

	switch (reason) {
		case RPA_REASON_START:
			rpa_parser_check_space(parser);
			rpa_wordstack_check_space(parser->stack);
			rpa_wordstack_push(parser->stack, parser->vmcode_off);
			rpa_parser_gencode_str_start(parser, input, size);
			break;
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:			
			rpa_parser_gencode_str_end(parser, input, size);
			rpa_wordstack_pop(parser->stack);
			break;
		case RPA_REASON_END:
			parser->vmcode_off = rpa_wordstack_pop(parser->stack);		
			break;
		default:
			break;
	}


	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
	return size;
}


int rpa_parser_cb_altexpr(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	rpa_word_t codepos = parser->vmcode_off;

	switch (reason) {
		case RPA_REASON_START:
			rpa_parser_check_space(parser);
			rpa_wordstack_check_space(parser->stack);
			rpa_wordstack_push(parser->stack, parser->vmcode_off);
			rpa_parser_gencode_list_start(parser, input, size, RPA_MATCHFUNC_LIST_ALT);
			break;
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:			
			rpa_parser_gencode_list_end(parser, input, size);
			rpa_parser_gencode_setup_list(parser);
			rpa_wordstack_pop(parser->stack);		
			break;
		case RPA_REASON_END:
			parser->vmcode_off = rpa_wordstack_pop(parser->stack);		
			break;
		default:
			break;
	}

	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
	return size;
}


int rpa_parser_cb_namedexpr(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	rpa_word_t codepos = parser->vmcode_off;

	switch (reason) {
		case RPA_REASON_START:
			rpa_parser_check_space(parser);
			rpa_wordstack_check_space(parser->stack);
			rpa_wordstack_push(parser->stack, parser->vmcode_off);
			break;
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:
			rpa_parser_gencode_setup_list(parser);			
			rpa_wordstack_pop(parser->stack);		
			break;
		case RPA_REASON_END:
			parser->vmcode_off = rpa_wordstack_pop(parser->stack);		
			break;
		default:
			break;
	}


	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
	return size;
}



int rpa_parser_cb_codeboundary(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	rpa_word_t codepos = parser->vmcode_off;
	
	switch (reason) {
		case RPA_REASON_START:
			rpa_wordstack_check_space(parser->stack);
			rpa_wordstack_push(parser->stack, parser->vmcode_off);
			break;
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:
			rpa_wordstack_pop(parser->stack);		
			break;
		case RPA_REASON_END:
			parser->vmcode_off = rpa_wordstack_pop(parser->stack);		
			break;
		default:
			break;
	}

	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
	return size;
}


int rpa_parser_cb_nlist(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	rpa_word_t codepos = parser->vmcode_off;
	
	switch (reason) {
		case RPA_REASON_START:
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_PUSH, R7, XX, XX, 0);
			break;	
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:
			rpa_parser_gencode_nlist_get(parser, input, size);
			/*
			 * The hlist match ptr will be in r[0]
			 * Make this the current match now
			 */
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R7, R0, XX, 0);
			
			break;
		case RPA_REASON_END:
			break;
		default:
			break;			
	}
	
	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
	return size;
}


int rpa_parser_cb_rpaexpr(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_parser_t *parser = (rpa_parser_t*) ((rpa_mnode_callback_t*)mnode)->userdata;
	rpa_word_t codepos = parser->vmcode_off;
	
	switch (reason) {
		case RPA_REASON_START:
			parser->vmcode_off = 0;
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, SP, DA, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R7, DA, XX, 0);
			break;
		case RPA_REASON_END|RPA_REASON_MATCHED:
		case RPA_REASON_MATCHED:
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_CMP, R7, DA, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_BEQ, DA, XX, XX, 3);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, R7, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_SET_DEFAULT_PATTERN);
			
#if 0			
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_MOV, R0, R7, XX, 0);
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_XCAL, DA, XX, XX, VM_DUMP_TREE);
#endif
			parser->vmcode[parser->vmcode_off++] = rpa_asm(RPA_EXT, R0, XX, XX, 0);

			break;
		case RPA_REASON_END:
			parser->vmcode_off = 0;	
			break;
		default:
			break;
	}

	rpa_parser_mnode_print_dump_code(mnode, stat, input, size, reason, codepos);
	return size;
}


rpa_match_t *rpa_parser_list_from_str(rpa_parser_t *parser, const char *name, rpa_matchfunc_t match_function_id, const char *str)
{
	rpa_match_val_t *val;
	rpa_mnode_t *mnode;
	rpa_match_list_t *list = (rpa_match_list_t*) rpa_match_list_create(name, match_function_id);
	
	while (str && *str) {
		val = (rpa_match_val_t *)MATCH2LIST(rpa_match_val_create_namesize(str, 1, RPA_MATCHFUNC_VAL_CHREQ, *str));
		mnode = MNODE2LIST(rpa_mnode_callback_create(&val->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_mnode_print, parser));
		rpa_list_addt(&list->head, &mnode->mlink);
		str += 1;
	}
	return (rpa_match_t*)list;
}


void rpa_parser_setup_bnftree(rpa_parser_t *parser)
{
	rpa_match_list_t *notExpressionOperand = NULL, *notExpression = NULL, *notExpressionMatch = NULL;
	rpa_match_list_t *embeddedExprMnodeScb = NULL, *embeddedExprMnodeAcb = NULL, *embeddedExprMnodeNcb = NULL;
	rpa_match_list_t *qMnode = NULL, *paddedQMnodeNcb = NULL, *paddedQMnodeAcb = NULL, *paddedQMnodeScb = NULL, *paddedQMnode = NULL, *qMnodeExpr = NULL, *simpMnode = NULL, *rpaExpr = NULL;
	rpa_match_list_t *exprMnode = NULL, *exprMnodeInNList = NULL, *exprMatch = NULL, *expr = NULL, *opOrSeqExprMatch = NULL, *opOrSeqExprMnode = NULL, *opOrSeqExpr = NULL;
	rpa_match_list_t *seqExpr = NULL, *seqExprMatch = NULL, *seqExprMnode = NULL, *eoe = NULL;
	rpa_match_list_t *altExprStartMember = NULL, *altExprEndMember = NULL, *opExprAlt = NULL, *opExpr = NULL, *opsign = NULL, *spOpsign = NULL, *spOrSign = NULL;
	rpa_match_list_t *embeddedExprNcb = NULL, *embeddedExprAcb = NULL, *embeddedExprScb = NULL;
	rpa_match_list_t *bracketExpr = NULL, *bracketExprMatch = NULL;
	rpa_match_list_t *namedExpr = NULL, *namedExprReg = NULL, *namedExprOrr = NULL, *namedExprBest = NULL, *namedExprMatch = NULL;
	rpa_match_list_t *exprName = NULL, *exprNameMatch = NULL, *prettyAssign = NULL, *prettyBest = NULL, *prettyOrr = NULL;
	rpa_match_list_t *comment = NULL, *spaceOrLf = NULL, *ignoreLine = NULL;
	rpa_match_list_t *alpha = NULL, *alphanum = NULL, *nsalphanum = NULL, *range = NULL, *ignore = NULL, *rangeMatch = NULL, *classMember = NULL, *classExpr = NULL, *classExprMatch = NULL;
	rpa_match_list_t *numClassMatch = NULL, *numClassDecimal_r5 = NULL, *numClassHex_r5 = NULL, *numClassNumberMatch_r5 = NULL, *numClassDecimal_r6 = NULL, *numClassHex_r6 = NULL, *numClassNumberMatch_r6 = NULL;
	rpa_match_list_t *numClassRange = NULL, *numClassSingle = NULL;
	rpa_match_list_t *notDblQuote = NULL, *notSnglQuote = NULL, *notDblQuoteMatch = NULL, *notSnglQuoteMatch = NULL, *notDblQuoteVal = NULL, *notSnglQuoteVal = NULL;
	rpa_match_list_t *spRegExprCh = NULL, *escCh = NULL, *classCh = NULL, *classChMatch = NULL, *exprCh = NULL, *exprChMatch = NULL;
	rpa_match_list_t *exprStrDblQuote = NULL, *exprStrSnglQuote = NULL, *exprStrVal = NULL, *exprQStrMatch = NULL;
	rpa_match_list_t *newlineMatch = NULL, *anycharMatch = NULL, *hex = NULL, *hexNum = NULL, *decNum = NULL;
	rpa_match_list_t *prettyQuantity = NULL;
	
	rpa_match_range_t *lower = NULL, *upper = NULL, *digit = NULL, *loHex = NULL, *upHex = NULL;
	rpa_match_val_t *pound = NULL, *lf = NULL, *cr = NULL, *notlf = NULL, *eos = NULL;
	rpa_match_val_t *underscore = NULL, *col = NULL, *semicol = NULL, *snglQuote = NULL, *dblQuote = NULL, *notquote = NULL, *or = NULL;
	rpa_match_val_t *and = NULL, *contain = NULL, *minus = NULL, *notSign = NULL, *dash = NULL, *zero = NULL, *dot = NULL, *at = NULL;
	rpa_match_val_t *bracketOpen = NULL, *bracketClose = NULL, *squareOpen = NULL, *squareClose = NULL, *backslash = NULL, *newline = NULL;
	rpa_match_val_t *curlyOpen = NULL, *curlyClose = NULL;
	rpa_match_list_t  *oname_ncb = NULL, *cname_ncb = NULL, *oname_acb = NULL, *cname_acb = NULL, *oname_scb = NULL, *cname_scb = NULL;
	rpa_match_list_t *exprDblQuoteStr = NULL, *exprDblQuoteStrMatch = NULL, *exprSnglQuoteStr = NULL, *exprSnglQuoteStrMatch = NULL, *regExprCh = NULL;
	
	rpa_match_list_t *x = NULL, *quantity = NULL, *crlf = NULL;
	rpa_match_list_t *assign = NULL, *best = NULL, *orr = NULL, *space = NULL;
	rpa_match_list_t *regClassCh = NULL, *nseparator = NULL;
	rpa_match_list_t *funFail = NULL, *funAbort = NULL, *funLast = NULL, *funMemberMatch;
	rpa_match_list_t *fnFail = NULL, *fnAbort = NULL, *fnLast = NULL;
	rpa_match_t *lastCh = NULL, *anychar = NULL;
	rpa_match_t *root;
	rpa_varlink_t *pVarLinkRoot;
	
	quantity = (rpa_match_list_t*) MATCH2LIST(rpa_parser_list_from_str(parser, "quantity", RPA_MATCHFUNC_LIST_ALT, "*+?"));
	assign = (rpa_match_list_t*) MATCH2LIST(rpa_parser_list_from_str(parser, "assign", RPA_MATCHFUNC_LIST, "::="));
	best = (rpa_match_list_t*) MATCH2LIST(rpa_parser_list_from_str(parser, "best", RPA_MATCHFUNC_LIST, "++="));
	orr = (rpa_match_list_t*) MATCH2LIST(rpa_parser_list_from_str(parser, "orr", RPA_MATCHFUNC_LIST, "||="));
	oname_ncb = (rpa_match_list_t*) MATCH2LIST(rpa_parser_list_from_str(parser, "oname_ncb", RPA_MATCHFUNC_LIST, "<"));
	cname_ncb = (rpa_match_list_t*) MATCH2LIST(rpa_parser_list_from_str(parser, "cname_ncb", RPA_MATCHFUNC_LIST, ">"));
	oname_acb = (rpa_match_list_t*) MATCH2LIST(rpa_parser_list_from_str(parser, "oname_acb", RPA_MATCHFUNC_LIST, "<:"));
	cname_acb = (rpa_match_list_t*) MATCH2LIST(rpa_parser_list_from_str(parser, "cname_acb", RPA_MATCHFUNC_LIST, ":>"));
	oname_scb = (rpa_match_list_t*) MATCH2LIST(rpa_parser_list_from_str(parser, "oname_scb", RPA_MATCHFUNC_LIST, "<;"));
	cname_scb = (rpa_match_list_t*) MATCH2LIST(rpa_parser_list_from_str(parser, "cname_scb", RPA_MATCHFUNC_LIST, ";>"));
	
	fnFail = (rpa_match_list_t*) MATCH2LIST(rpa_parser_list_from_str(parser, "fnFail", RPA_MATCHFUNC_LIST, "fail"));
	fnAbort = (rpa_match_list_t*) MATCH2LIST(rpa_parser_list_from_str(parser, "fnAbort", RPA_MATCHFUNC_LIST, "abort"));
	fnLast = (rpa_match_list_t*) MATCH2LIST(rpa_parser_list_from_str(parser, "fnLast", RPA_MATCHFUNC_LIST, "last"));
	regExprCh = (rpa_match_list_t*) MATCH2LIST(rpa_parser_list_from_str(parser, "regExprCh", RPA_MATCHFUNC_LIST_NOALT, " :#@^-|&%+*?\"\'[](){}<>;\n\r\0"));
	regClassCh = (rpa_match_list_t*) MATCH2LIST(rpa_parser_list_from_str(parser, "regClassCh", RPA_MATCHFUNC_LIST_NOALT, "\\-[]\n\r\0"));
	space = (rpa_match_list_t*) MATCH2LIST(rpa_parser_list_from_str(parser, "space", RPA_MATCHFUNC_LIST_ALT, " \t"));
	x = (rpa_match_list_t*) MATCH2LIST(rpa_parser_list_from_str(parser, "x", RPA_MATCHFUNC_LIST_ALT, "xX"));

	lastCh = MATCH2LIST(rpa_match_create("lastCh", RPA_MATCHFUNC_LSTCHR));
	lower = (rpa_match_range_t*) MATCH2LIST(rpa_match_range_create("lower", RPA_MATCHFUNC_RANGE_CHRINRANGE, 'a', 'z'));
	upper = (rpa_match_range_t*) MATCH2LIST(rpa_match_range_create("upper", RPA_MATCHFUNC_RANGE_CHRINRANGE, 'A', 'Z'));
	loHex = (rpa_match_range_t*) MATCH2LIST(rpa_match_range_create("loHex", RPA_MATCHFUNC_RANGE_CHRINRANGE, 'a', 'f'));
	upHex = (rpa_match_range_t*) MATCH2LIST(rpa_match_range_create("upHex", RPA_MATCHFUNC_RANGE_CHRINRANGE, 'A', 'F'));
	digit = (rpa_match_range_t*) MATCH2LIST(rpa_match_range_create("digit", RPA_MATCHFUNC_RANGE_CHRINRANGE, '0', '9'));
	
	semicol = (rpa_match_val_t*) MATCH2LIST(rpa_match_val_create("semicol", RPA_MATCHFUNC_VAL_CHREQ, ';'));
	underscore = (rpa_match_val_t*) MATCH2LIST(rpa_match_val_create("undersocre", RPA_MATCHFUNC_VAL_CHREQ, '_'));
	backslash = (rpa_match_val_t*) MATCH2LIST(rpa_match_val_create("backslash", RPA_MATCHFUNC_VAL_CHREQ, '\\'));
	anychar = (rpa_match_t*) MATCH2LIST(rpa_match_special_create("anychar", RPA_MATCHFUNC_CHREQANY));
	dblQuote = (rpa_match_val_t*) MATCH2LIST(rpa_match_val_create("dblQuote", RPA_MATCHFUNC_VAL_CHREQ, '"'));
	snglQuote = (rpa_match_val_t*) MATCH2LIST(rpa_match_val_create("snglQuote", RPA_MATCHFUNC_VAL_CHREQ, '\''));	
	notquote = (rpa_match_val_t*) MATCH2LIST(rpa_match_val_create("notquote", RPA_MATCHFUNC_VAL_CHRNOTEQ, '"'));		
	at = (rpa_match_val_t*) MATCH2LIST(rpa_match_val_create("at", RPA_MATCHFUNC_VAL_CHREQ, '@'));
	or = (rpa_match_val_t*) MATCH2LIST(rpa_match_val_create("or", RPA_MATCHFUNC_VAL_CHREQ, '|'));
	and = (rpa_match_val_t*) MATCH2LIST(rpa_match_val_create("and", RPA_MATCHFUNC_VAL_CHREQ, '&'));
	contain = (rpa_match_val_t*) MATCH2LIST(rpa_match_val_create("contain", RPA_MATCHFUNC_VAL_CHREQ, '%'));
	minus = (rpa_match_val_t*) MATCH2LIST(rpa_match_val_create("minus", RPA_MATCHFUNC_VAL_CHREQ, '-'));
	notSign= (rpa_match_val_t*) MATCH2LIST(rpa_match_val_create("notSign", RPA_MATCHFUNC_VAL_CHREQ, '^'));
	col = (rpa_match_val_t*) MATCH2LIST(rpa_match_val_create("col", RPA_MATCHFUNC_VAL_CHREQ, ':'));
	dash = (rpa_match_val_t*) MATCH2LIST(rpa_match_val_create("dash", RPA_MATCHFUNC_VAL_CHREQ, '-'));
	dot = (rpa_match_val_t*) MATCH2LIST(rpa_match_val_create("dot", RPA_MATCHFUNC_VAL_CHREQ, '.'));	
	newline = (rpa_match_val_t*) MATCH2LIST(rpa_match_val_create("newline", RPA_MATCHFUNC_VAL_CHREQ, '~'));
	bracketOpen = (rpa_match_val_t*) MATCH2LIST(rpa_match_val_create("bracketopen", RPA_MATCHFUNC_VAL_CHREQ, '('));
	bracketClose = (rpa_match_val_t*) MATCH2LIST(rpa_match_val_create("bracketclose", RPA_MATCHFUNC_VAL_CHREQ, ')'));
	curlyOpen = (rpa_match_val_t*) MATCH2LIST(rpa_match_val_create("curlyopen", RPA_MATCHFUNC_VAL_CHREQ, '{'));
	curlyClose = (rpa_match_val_t*) MATCH2LIST(rpa_match_val_create("curlyclose", RPA_MATCHFUNC_VAL_CHREQ, '}'));
	squareOpen = (rpa_match_val_t*) MATCH2LIST(rpa_match_val_create("squareopen", RPA_MATCHFUNC_VAL_CHREQ, '['));
	squareClose = (rpa_match_val_t*) MATCH2LIST(rpa_match_val_create("squareclose", RPA_MATCHFUNC_VAL_CHREQ, ']'));
	lf = (rpa_match_val_t*) MATCH2LIST(rpa_match_val_create("lf", RPA_MATCHFUNC_VAL_CHREQ, '\n'));
	cr = (rpa_match_val_t*) MATCH2LIST(rpa_match_val_create("cr", RPA_MATCHFUNC_VAL_CHREQ, '\r'));
	eos = (rpa_match_val_t*) MATCH2LIST(rpa_match_val_create("eos", RPA_MATCHFUNC_VAL_CHREQ, '\0'));
	
	notlf = (rpa_match_val_t*) MATCH2LIST(rpa_match_val_create("notlf", RPA_MATCHFUNC_VAL_CHRNOTEQ, '\n'));
	pound = (rpa_match_val_t*) MATCH2LIST(rpa_match_val_create("pound", RPA_MATCHFUNC_VAL_CHREQ, '#'));
	zero = (rpa_match_val_t*) MATCH2LIST(rpa_match_val_create("zero", RPA_MATCHFUNC_VAL_CHREQ, '0'));

	funFail = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("funFail", RPA_MATCHFUNC_LIST));
	funAbort = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("funAbort", RPA_MATCHFUNC_LIST));
	funLast = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("funLast", RPA_MATCHFUNC_LIST));
	funMemberMatch = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("funMemberMatch", RPA_MATCHFUNC_LIST_ALT));

	prettyQuantity = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("prettyQuantity", RPA_MATCHFUNC_LIST));
	nseparator = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("nseparator", RPA_MATCHFUNC_LIST_ALT));
	ignoreLine = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("ignoreLine", RPA_MATCHFUNC_LIST_ALT));
	bracketExprMatch = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("bracketExprMatch", RPA_MATCHFUNC_LIST));
	bracketExpr = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("bracketExpr", RPA_MATCHFUNC_LIST));
	notExpressionOperand = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("notExpressionOperand", RPA_MATCHFUNC_LIST_ALT));	
	notExpression = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("notExpression", RPA_MATCHFUNC_LIST));	
	notExpressionMatch = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("notExpressionMatch", RPA_MATCHFUNC_LIST));	
	exprQStrMatch = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("exprQStrMatch", RPA_MATCHFUNC_LIST_ALT));
	expr = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("expr", RPA_MATCHFUNC_LIST_ALT));
	opOrSeqExpr = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("opOrSeqExpr", RPA_MATCHFUNC_LIST_ALT));
	opOrSeqExprMatch = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("opOrSeqExprMatch", RPA_MATCHFUNC_LIST));
	opOrSeqExprMnode = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("opOrSeqExprMnode", RPA_MATCHFUNC_LIST));
	seqExpr = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("seqExpr", RPA_MATCHFUNC_LIST));
	seqExprMatch = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("seqExprMatch", RPA_MATCHFUNC_LIST_ALT));
	seqExprMnode = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("seqExprMnode", RPA_MATCHFUNC_LIST));
	opExpr = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("opExpr", RPA_MATCHFUNC_LIST));
	opExprAlt = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("opExprAlt", RPA_MATCHFUNC_LIST));
	altExprStartMember = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("altExprStartMember", RPA_MATCHFUNC_LIST));
	altExprEndMember = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("altExprEndMember", RPA_MATCHFUNC_LIST));
	exprStrVal = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("exprStrVal", RPA_MATCHFUNC_LIST_ALT));
	exprStrDblQuote = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("exprStrDblQuote", RPA_MATCHFUNC_LIST));
	exprStrSnglQuote = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("exprStrSnglQuote", RPA_MATCHFUNC_LIST));
	notSnglQuote = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("notSnglQuote", RPA_MATCHFUNC_LIST_NOT));
	notDblQuote = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("notDblQuote", RPA_MATCHFUNC_LIST_NOT));
	notSnglQuoteMatch = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("notSnglQuoteMatch", RPA_MATCHFUNC_LIST));
	notDblQuoteMatch = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("notDblQuoteMatch", RPA_MATCHFUNC_LIST));
	notSnglQuoteVal = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("notSnglQuoteVal", RPA_MATCHFUNC_LIST));
	notDblQuoteVal = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("notDblQuoteVal", RPA_MATCHFUNC_LIST));
	qMnodeExpr = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("qMnodeExpr", RPA_MATCHFUNC_LIST_ALT));
	paddedQMnodeNcb = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("paddedQMnodeNcb", RPA_MATCHFUNC_LIST));
	paddedQMnodeAcb = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("paddedQMnodeNcb", RPA_MATCHFUNC_LIST));
	paddedQMnodeScb = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("paddedQMnodeScb", RPA_MATCHFUNC_LIST));
	paddedQMnode = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("paddedQMnode", RPA_MATCHFUNC_LIST));
	qMnode = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("qMnode", RPA_MATCHFUNC_LIST));
	ignore = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("ignore", RPA_MATCHFUNC_LIST));
	simpMnode = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("simpMnode", RPA_MATCHFUNC_LIST_ALT));
	embeddedExprMnodeAcb = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("embeddedExprMnodeAcb", RPA_MATCHFUNC_LIST));
	embeddedExprMnodeScb = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("embeddedExprMnodeScb", RPA_MATCHFUNC_LIST));
	embeddedExprMnodeNcb = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("embeddedExprMnodeNcb", RPA_MATCHFUNC_LIST));
	exprChMatch = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("exprChMatch", RPA_MATCHFUNC_LIST));
	spOrSign = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("spOrSign", RPA_MATCHFUNC_LIST));
	spOpsign = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("spOpsign", RPA_MATCHFUNC_LIST));
	opsign = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("opsign", RPA_MATCHFUNC_LIST_ALT));
	namedExprMatch = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("namedExprMatch", RPA_MATCHFUNC_LIST));
	namedExprReg = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("namedExprReg", RPA_MATCHFUNC_LIST));
	namedExprOrr = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("namedExprOrr", RPA_MATCHFUNC_LIST));
	namedExprBest = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("namedExprBest", RPA_MATCHFUNC_LIST));
	namedExpr = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("namedExpr", RPA_MATCHFUNC_LIST_ALT));

	exprMatch = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("epxrMatch", RPA_MATCHFUNC_LIST_ALT));
	exprMnode = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("epxrMnode", RPA_MATCHFUNC_LIST));
	exprMnodeInNList = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("exprMnodeInNList", RPA_MATCHFUNC_LIST));
	eoe = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("eoe", RPA_MATCHFUNC_LIST_ALT));
	embeddedExprAcb = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("embeddedExprAcb", RPA_MATCHFUNC_LIST));
	embeddedExprScb = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("embeddedExprScb", RPA_MATCHFUNC_LIST));
	embeddedExprNcb = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("embeddedExprNcb", RPA_MATCHFUNC_LIST));
	anycharMatch = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("anycharMatch", RPA_MATCHFUNC_LIST));
	newlineMatch = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("newlineMatch", RPA_MATCHFUNC_LIST));
	prettyAssign = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("prettyAssign", RPA_MATCHFUNC_LIST));
	prettyOrr = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("prettyOrr", RPA_MATCHFUNC_LIST));
	prettyBest = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("prettyBest", RPA_MATCHFUNC_LIST));
	exprNameMatch = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("exprNameMatch", RPA_MATCHFUNC_LIST));
	exprName = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("exprName", RPA_MATCHFUNC_LIST));
	nsalphanum = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("nsalphanum", RPA_MATCHFUNC_LIST));
	alphanum = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("alphanum", RPA_MATCHFUNC_LIST_ALT));
	alpha = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("alpha", RPA_MATCHFUNC_LIST_ALT));
	numClassMatch = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("numClassMatch", RPA_MATCHFUNC_LIST_ALT));
	numClassSingle = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("numClassSingle", RPA_MATCHFUNC_LIST));
	numClassRange = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("numClassRange", RPA_MATCHFUNC_LIST));
	numClassNumberMatch_r5 = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("numClassNumberMatch_r5", RPA_MATCHFUNC_LIST_ALT));
	numClassNumberMatch_r6 = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("numClassNumberMatch_r6", RPA_MATCHFUNC_LIST_ALT));
	numClassHex_r5 = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("numClassHex_r5", RPA_MATCHFUNC_LIST));
	numClassHex_r6 = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("numClassHex_r6", RPA_MATCHFUNC_LIST));
	hex = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("hex", RPA_MATCHFUNC_LIST_ALT));
	hexNum = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("hexNum", RPA_MATCHFUNC_LIST));
	decNum = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("decNum", RPA_MATCHFUNC_LIST));
	numClassDecimal_r5 = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("numClassDecimal_r5", RPA_MATCHFUNC_LIST));
	numClassDecimal_r6 = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("numClassDecimal_r6", RPA_MATCHFUNC_LIST));
	classExpr = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("classExpr", RPA_MATCHFUNC_LIST));
	classExprMatch = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("classExprMatch", RPA_MATCHFUNC_LIST));
	classMember = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("classMember", RPA_MATCHFUNC_LIST_ALT));
	classChMatch = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("classChMatch", RPA_MATCHFUNC_LIST));
	rangeMatch = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("rangeMatch", RPA_MATCHFUNC_LIST));
	range = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("range", RPA_MATCHFUNC_LIST));
	classCh = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("classCh", RPA_MATCHFUNC_LIST_ALT));
	exprCh = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("exprCh", RPA_MATCHFUNC_LIST_ALT));
	spRegExprCh = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("spRegExprCh", RPA_MATCHFUNC_LIST));
	escCh = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("escCh", RPA_MATCHFUNC_LIST));
	spaceOrLf = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("spaceOrLf", RPA_MATCHFUNC_LIST_ALT));
	comment = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("comment", RPA_MATCHFUNC_LIST));
	rpaExpr = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("rpaExpr", RPA_MATCHFUNC_LIST_ALT));
	exprDblQuoteStr = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("exprDblQuoteStr", RPA_MATCHFUNC_LIST));
	exprDblQuoteStrMatch = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("exprDblQuoteStrMatch", RPA_MATCHFUNC_LIST));
	exprSnglQuoteStr = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("exprSnglQuoteStr", RPA_MATCHFUNC_LIST));
	exprSnglQuoteStrMatch = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("exprSnglQuoteStrMatch", RPA_MATCHFUNC_LIST));
	crlf = (rpa_match_list_t*) MATCH2LIST(rpa_match_list_create("crlf", RPA_MATCHFUNC_LIST));

	rpa_mnode_cat_list(&regExprCh->head,
					   MNODE2LIST(rpa_mnode_callback_create(&eos->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&regClassCh->head,
					   MNODE2LIST(rpa_mnode_callback_create(&eos->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&funAbort->head,
					   MNODE2LIST(rpa_mnode_callback_create(&curlyOpen->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&fnAbort->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&curlyClose->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&funFail->head,
					   MNODE2LIST(rpa_mnode_callback_create(&curlyOpen->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&fnFail->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&curlyClose->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&funLast->head,
					   MNODE2LIST(rpa_mnode_callback_create(&curlyOpen->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&fnLast->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&curlyClose->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);


	rpa_mnode_cat_list(&funMemberMatch->head,
					   MNODE2LIST(rpa_mnode_callback_arg1_create(&funFail->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_cb_fun_matchptr, parser, VM_CREATE_FUNFAIL_MATCHPTR)),
					   MNODE2LIST(rpa_mnode_callback_arg1_create(&funAbort->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_cb_fun_matchptr, parser, VM_CREATE_FUNABORT_MATCHPTR)),
					   MNODE2LIST(rpa_mnode_callback_arg1_create(&funLast->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_cb_fun_matchptr, parser, VM_CREATE_FUNLAST_MATCHPTR)),
					   NULL);
	
	rpa_mnode_cat_list(&comment->head,
					   MNODE2LIST(rpa_mnode_callback_create(&pound->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&notlf->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE|RPA_REASON_MATCHED, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&lf->base, RPA_MATCH_OPTIONAL|RPA_REASON_MATCHED, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&spaceOrLf->head,
					   MNODE2LIST(rpa_mnode_callback_create(&space->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&crlf->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&lf->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&escCh->head,
					   MNODE2LIST(rpa_mnode_callback_create(&backslash->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(anychar, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&classCh->head,
					   MNODE2LIST(rpa_mnode_callback_create(&escCh->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_cb_esc_val_r5, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&regClassCh->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_cb_val_r5, parser)),
					   NULL);

	rpa_mnode_cat_list(&classChMatch->head,
					   MNODE2LIST(rpa_mnode_callback_create(&classCh->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_cb_val_matchptr, parser)),
					   NULL);

	rpa_mnode_cat_list(&range->head,
					   MNODE2LIST(rpa_mnode_callback_create(&regClassCh->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_cb_val_r5, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&dash->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&regClassCh->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_cb_val_r6, parser)),
					   NULL);

	rpa_mnode_cat_list(&rangeMatch->head,
					   MNODE2LIST(rpa_mnode_callback_create(&range->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_cb_range_matchptr, parser)),
					   NULL);


	rpa_mnode_cat_list(&classMember->head,
					   MNODE2LIST(rpa_mnode_callback_create(&rangeMatch->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_mnodeptr, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&classChMatch->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_mnodeptr, parser)),
					   NULL);

	rpa_mnode_cat_list(&classExpr->head,
					   MNODE2LIST(rpa_mnode_callback_create(&squareOpen->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&classMember->base, RPA_MATCH_MULTIPLE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&squareClose->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&classExprMatch->head,
					   MNODE2LIST(rpa_mnode_callback_create(&classExpr->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_class, parser)),
					   NULL);

	rpa_mnode_cat_list(&exprCh->head,
					   MNODE2LIST(rpa_mnode_callback_create(&escCh->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_esc_val_r5, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&regExprCh->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_cb_val_r5, parser)),
					   NULL);

	rpa_mnode_cat_list(&exprChMatch->head,
					   MNODE2LIST(rpa_mnode_callback_create(&exprCh->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_cb_val_matchptr, parser)),
					   NULL);
					   
	rpa_mnode_cat_list(&exprDblQuoteStr->head,
					   MNODE2LIST(rpa_mnode_callback_create(&notDblQuoteVal->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE|RPA_REASON_ALL, rpa_parser_cb_push_r5_inc_r4, parser)),
					   NULL);
					   
	rpa_mnode_cat_list(&exprDblQuoteStrMatch->head,
					   MNODE2LIST(rpa_mnode_callback_create(&exprDblQuoteStr->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_strexpr, parser)),
					   NULL);

	rpa_mnode_cat_list(&exprSnglQuoteStr->head,
					   MNODE2LIST(rpa_mnode_callback_create(&notSnglQuoteVal->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE|RPA_REASON_ALL, rpa_parser_cb_push_r5_inc_r4, parser)),
					   NULL);
					   
	rpa_mnode_cat_list(&exprSnglQuoteStrMatch->head,
					   MNODE2LIST(rpa_mnode_callback_create(&exprSnglQuoteStr->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_strexpr, parser)),
					   NULL);

	rpa_mnode_cat_list(&embeddedExprMnodeScb->head,
					   MNODE2LIST(rpa_mnode_callback_create(&embeddedExprScb->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_codeboundary, parser)),
					   NULL);

	rpa_mnode_cat_list(&embeddedExprMnodeAcb->head,
					   MNODE2LIST(rpa_mnode_callback_create(&embeddedExprAcb->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_codeboundary, parser)),
					   NULL);

	rpa_mnode_cat_list(&embeddedExprMnodeNcb->head,
					   MNODE2LIST(rpa_mnode_callback_create(&embeddedExprNcb->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_codeboundary, parser)),
					   NULL);

	rpa_mnode_cat_list(&notExpressionOperand->head,
					   MNODE2LIST(rpa_mnode_callback_create(&embeddedExprMnodeAcb->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_acb_q_mnodeptr_callback, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&embeddedExprMnodeScb->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_scb_q_mnodeptr_callback, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&embeddedExprMnodeNcb->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_ncb_q_mnodeptr_callback, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&simpMnode->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_mnodeptr, parser)),
					   NULL);

	rpa_mnode_cat_list(&notExpression->head,
					   MNODE2LIST(rpa_mnode_callback_create(&notSign->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_cb_opsign, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&notExpressionOperand->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&notExpressionMatch->head,
					   MNODE2LIST(rpa_mnode_callback_create(&notExpression->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_seqexpr, parser)),
					   NULL);

	rpa_mnode_cat_list(&bracketExpr->head, 
					   MNODE2LIST(rpa_mnode_callback_create(&bracketOpen->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&exprMnode->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&bracketClose->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&bracketExprMatch->head,
					   MNODE2LIST(rpa_mnode_callback_create(&bracketExpr->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_seqexpr, parser)),
					   NULL);

	rpa_mnode_cat_list(&embeddedExprNcb->head,
					   MNODE2LIST(rpa_mnode_callback_create(&oname_ncb->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&exprName->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_nlist, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&cname_ncb->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&embeddedExprAcb->head,
					   MNODE2LIST(rpa_mnode_callback_create(&oname_acb->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&exprName->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_nlist, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&cname_acb->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&embeddedExprScb->head,
					   MNODE2LIST(rpa_mnode_callback_create(&oname_scb->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&exprName->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_nlist, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&cname_scb->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&newlineMatch->head,
					   MNODE2LIST(rpa_mnode_callback_create(&newline->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_cb_newline_matchptr, parser)),
					   NULL);

	rpa_mnode_cat_list(&anycharMatch->head,
					   MNODE2LIST(rpa_mnode_callback_create(&dot->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_cb_anychar_matchptr, parser)),
					   NULL);

	rpa_mnode_cat_list(&decNum->head,
					   MNODE2LIST(rpa_mnode_callback_create(&digit->base, RPA_MATCH_MULTIPLE|RPA_REASON_MATCHED, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&hex->head,
					   MNODE2LIST(rpa_mnode_callback_create(&digit->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&loHex->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&upHex->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&hexNum->head,
					   MNODE2LIST(rpa_mnode_callback_create(&hex->base, RPA_MATCH_MULTIPLE|RPA_REASON_MATCHED, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&numClassDecimal_r5->head,
					   MNODE2LIST(rpa_mnode_callback_create(&pound->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&decNum->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_cb_decnum_r5, parser)),
					   NULL);

	rpa_mnode_cat_list(&numClassHex_r5->head,
					   MNODE2LIST(rpa_mnode_callback_create(&pound->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&zero->base, RPA_MATCH_OPTIONAL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&x->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&hexNum->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_cb_hexnum_r5, parser)),					   
					   NULL);

	rpa_mnode_cat_list(&numClassNumberMatch_r5->head,
					   MNODE2LIST(rpa_mnode_callback_create(&numClassHex_r5->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&numClassDecimal_r5->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   NULL);


	rpa_mnode_cat_list(&numClassDecimal_r6->head,
					   MNODE2LIST(rpa_mnode_callback_create(&pound->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&decNum->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_cb_decnum_r6, parser)),
					   NULL);

	rpa_mnode_cat_list(&numClassHex_r6->head,
					   MNODE2LIST(rpa_mnode_callback_create(&pound->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&zero->base, RPA_MATCH_OPTIONAL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&x->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&hexNum->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_cb_hexnum_r6, parser)),					   
					   NULL);

	rpa_mnode_cat_list(&numClassNumberMatch_r6->head,
					   MNODE2LIST(rpa_mnode_callback_create(&numClassHex_r6->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&numClassDecimal_r6->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   NULL);


	rpa_mnode_cat_list(&numClassRange->head,
					   MNODE2LIST(rpa_mnode_callback_create(&squareOpen->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&numClassNumberMatch_r5->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&dash->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&numClassNumberMatch_r6->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&squareClose->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&numClassSingle->head,
					   MNODE2LIST(rpa_mnode_callback_create(&squareOpen->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&numClassNumberMatch_r5->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&squareClose->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&numClassMatch->head,
					   MNODE2LIST(rpa_mnode_callback_create(&numClassSingle->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_cb_val_matchptr, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&numClassRange->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_cb_range_matchptr, parser)),
					   NULL);


	rpa_mnode_cat_list(&notDblQuote->head,
					   MNODE2LIST(rpa_mnode_callback_create(&dblQuote->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&notSnglQuote->head,
					   MNODE2LIST(rpa_mnode_callback_create(&snglQuote->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&notSnglQuoteVal->head,
					   MNODE2LIST(rpa_mnode_callback_create(&notSnglQuote->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_cb_val_r5, parser)),
					   NULL);

	rpa_mnode_cat_list(&notDblQuoteVal->head,
					   MNODE2LIST(rpa_mnode_callback_create(&notDblQuote->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_cb_val_r5, parser)),
					   NULL);


	rpa_mnode_cat_list(&notSnglQuoteMatch->head,
					   MNODE2LIST(rpa_mnode_callback_create(&notSnglQuoteVal->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_cb_val_matchptr, parser)),
					   NULL);

	rpa_mnode_cat_list(&notDblQuoteMatch->head,
					   MNODE2LIST(rpa_mnode_callback_create(&notDblQuoteVal->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_cb_val_matchptr, parser)),
					   NULL);

	rpa_mnode_cat_list(&exprStrSnglQuote->head,
					   MNODE2LIST(rpa_mnode_callback_create(&snglQuote->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&notSnglQuoteMatch->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE|RPA_REASON_ALL, rpa_parser_cb_mnodeptr, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&snglQuote->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),					   
					   NULL);

	rpa_mnode_cat_list(&exprStrDblQuote->head,
					   MNODE2LIST(rpa_mnode_callback_create(&dblQuote->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&notDblQuoteMatch->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE|RPA_REASON_ALL, rpa_parser_cb_mnodeptr, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&dblQuote->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),					   
					   NULL);

	rpa_mnode_cat_list(&exprQStrMatch->head,
					   MNODE2LIST(rpa_mnode_callback_create(&exprStrDblQuote->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_seqexpr, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&exprStrSnglQuote->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_seqexpr, parser)),					   
					   NULL);


	rpa_mnode_cat_list(&simpMnode->head, 
					   MNODE2LIST(rpa_mnode_callback_create(&notExpressionMatch->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&funMemberMatch->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&exprQStrMatch->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
//					   MNODE2LIST(rpa_mnode_callback_create(&embeddedExprNcb->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_codeboundary, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&newlineMatch->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&anycharMatch->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&numClassMatch->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&classExprMatch->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&bracketExprMatch->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&exprChMatch->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&ignore->head,
					   MNODE2LIST(rpa_mnode_callback_create(&space->base, RPA_MATCH_MULTIPLE|RPA_REASON_MATCHED, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&prettyQuantity->head,
					   MNODE2LIST(rpa_mnode_callback_create(&space->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE|RPA_REASON_MATCHED, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&quantity->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_quantity, parser)),
					   NULL);	

	rpa_mnode_cat_list(&paddedQMnode->head,
					   MNODE2LIST(rpa_mnode_callback_create(&ignore->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&simpMnode->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&prettyQuantity->base, RPA_MATCH_OPTIONAL|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&ignore->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);


	rpa_mnode_cat_list(&paddedQMnodeNcb->head,
					   MNODE2LIST(rpa_mnode_callback_create(&ignore->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&embeddedExprNcb->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_codeboundary, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&prettyQuantity->base, RPA_MATCH_OPTIONAL|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&ignore->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&paddedQMnodeAcb->head,
					   MNODE2LIST(rpa_mnode_callback_create(&ignore->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&embeddedExprAcb->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_codeboundary, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&prettyQuantity->base, RPA_MATCH_OPTIONAL|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&ignore->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);


	rpa_mnode_cat_list(&paddedQMnodeScb->head,
					   MNODE2LIST(rpa_mnode_callback_create(&ignore->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&embeddedExprScb->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_codeboundary, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&prettyQuantity->base, RPA_MATCH_OPTIONAL|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&ignore->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&qMnodeExpr->head,
					   MNODE2LIST(rpa_mnode_callback_create(&paddedQMnodeAcb->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_acb_q_mnodeptr_callback, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&paddedQMnodeScb->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_scb_q_mnodeptr_callback, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&paddedQMnodeNcb->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_ncb_q_mnodeptr_callback, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&paddedQMnode->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_q_mnodeptr, parser)),
					   NULL);

	rpa_mnode_cat_list(&seqExpr->head,
						MNODE2LIST(rpa_mnode_callback_create(&qMnodeExpr->base, RPA_MATCH_MULTIPLE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
						NULL);

	rpa_mnode_cat_list(&seqExprMatch->head,
						MNODE2LIST(rpa_mnode_callback_create(&seqExpr->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_seqexpr, parser)),
						NULL);

	rpa_mnode_cat_list(&seqExprMnode->head,
						MNODE2LIST(rpa_mnode_callback_create(&seqExprMatch->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_mnodeptr, parser)),
						NULL);

	rpa_mnode_cat_list(&opOrSeqExpr->head,
						MNODE2LIST(rpa_mnode_callback_create(&opExpr->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_codeboundary, parser)),
					    MNODE2LIST(rpa_mnode_callback_create(&seqExpr->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_codeboundary, parser)),
						NULL);

	rpa_mnode_cat_list(&opOrSeqExprMatch->head,
						MNODE2LIST(rpa_mnode_callback_create(&opOrSeqExpr->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_seqexpr, parser)),
						NULL);

	rpa_mnode_cat_list(&opOrSeqExprMnode->head,
						MNODE2LIST(rpa_mnode_callback_create(&opOrSeqExprMatch->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_mnodeptr_in_altlist, parser)),
						NULL);


	rpa_mnode_cat_list(&expr->head,
					    MNODE2LIST(rpa_mnode_callback_create(&opOrSeqExpr->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
						NULL);

	rpa_mnode_cat_list(&opsign->head,
					   MNODE2LIST(rpa_mnode_callback_create(&at->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_cb_opsign, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&and->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_cb_opsign, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&contain->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_cb_opsign, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&minus->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_cb_opsign, parser)),
					   NULL);

	rpa_mnode_cat_list(&exprMatch->head,
						MNODE2LIST(rpa_mnode_callback_create(&opExprAlt->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_altexpr, parser)),
						MNODE2LIST(rpa_mnode_callback_create(&expr->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_seqexpr, parser)),
					    NULL);

	rpa_mnode_cat_list(&exprMnode->head,
					   MNODE2LIST(rpa_mnode_callback_create(&exprMatch->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_mnodeptr, parser)),
					   NULL);


	rpa_mnode_cat_list(&exprMnodeInNList->head,
					   MNODE2LIST(rpa_mnode_callback_create(&exprMatch->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_mnodeptr_in_nlist, parser)),
					   NULL);


	rpa_mnode_cat_list(&spOpsign->head,
					   MNODE2LIST(rpa_mnode_callback_create(&spaceOrLf->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&opsign->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&spaceOrLf->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&spOrSign->head,
					   MNODE2LIST(rpa_mnode_callback_create(&spaceOrLf->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&or->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&spaceOrLf->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&altExprStartMember->head,
					   MNODE2LIST(rpa_mnode_callback_create(&opOrSeqExprMnode->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&altExprEndMember->head,
					   MNODE2LIST(rpa_mnode_callback_create(&spOrSign->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&opOrSeqExprMnode->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&opExprAlt->head,
					   MNODE2LIST(rpa_mnode_callback_create(&altExprStartMember->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_codeboundary, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&altExprEndMember->base, RPA_MATCH_MULTIPLE|RPA_REASON_ALL, rpa_parser_cb_codeboundary, parser)),
					   NULL);

	rpa_mnode_cat_list(&opExpr->head,
					   MNODE2LIST(rpa_mnode_callback_create(&seqExprMnode->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&spOpsign->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&exprMnode->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&ignoreLine->head,
					   MNODE2LIST(rpa_mnode_callback_create(&eos->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&spaceOrLf->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&comment->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&prettyAssign->head,
					   MNODE2LIST(rpa_mnode_callback_create(&space->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&assign->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_assign, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&space->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&prettyOrr->head,
					   MNODE2LIST(rpa_mnode_callback_create(&space->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&orr->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_assign, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&space->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);


	rpa_mnode_cat_list(&prettyBest->head,
					   MNODE2LIST(rpa_mnode_callback_create(&space->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&best->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_assign, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&space->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&crlf->head,
					   MNODE2LIST(rpa_mnode_callback_create(&cr->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&lf->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&eoe->head,
					   MNODE2LIST(rpa_mnode_callback_create(&semicol->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&crlf->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&lf->base, RPA_MATCH_NONE, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&comment->base, RPA_MATCH_OPTIONAL, rpa_parser_mnode_print, parser)),					   
					   MNODE2LIST(rpa_mnode_callback_create(&eos->base, RPA_MATCH_OPTIONAL, rpa_parser_mnode_print, parser)),					   
					   NULL);

	rpa_mnode_cat_list(&alpha->head,
					   MNODE2LIST(rpa_mnode_callback_create(&lower->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&upper->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&alphanum->head,
					   MNODE2LIST(rpa_mnode_callback_create(&alpha->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&digit->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&underscore->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&nseparator->head,
					   MNODE2LIST(rpa_mnode_callback_create(&col->base, RPA_MATCH_OPTIONAL|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&dash->base, RPA_MATCH_OPTIONAL|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&nsalphanum->head,
					   MNODE2LIST(rpa_mnode_callback_create(&nseparator->base, RPA_MATCH_OPTIONAL|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&alphanum->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&exprName->head,
					   MNODE2LIST(rpa_mnode_callback_create(&alpha->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&nsalphanum->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&exprNameMatch->head,
					   MNODE2LIST(rpa_mnode_callback_create(&exprName->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_nlist, parser)),
					   NULL);

	rpa_mnode_cat_list(&namedExprReg->head,
					   MNODE2LIST(rpa_mnode_callback_create(&exprNameMatch->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&prettyAssign->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&exprMnode->base, RPA_MATCH_OPTIONAL|RPA_REASON_MATCHED, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&space->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&eoe->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&namedExprOrr->head,
					   MNODE2LIST(rpa_mnode_callback_create(&exprNameMatch->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&prettyOrr->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&exprMnodeInNList->base, RPA_MATCH_OPTIONAL|RPA_REASON_MATCHED, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&space->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&eoe->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);


	rpa_mnode_cat_list(&namedExprBest->head,
					   MNODE2LIST(rpa_mnode_callback_create(&exprNameMatch->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&prettyBest->base, RPA_MATCH_NONE|RPA_REASON_MATCHED, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&exprMnodeInNList->base, RPA_MATCH_OPTIONAL|RPA_REASON_MATCHED, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&space->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&eoe->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);

	rpa_mnode_cat_list(&namedExpr->head,
					   MNODE2LIST(rpa_mnode_callback_create(&namedExprReg->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&namedExprOrr->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&namedExprBest->base, RPA_MATCH_OPTIONAL|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);


	rpa_mnode_cat_list(&namedExprMatch->head,
					   MNODE2LIST(rpa_mnode_callback_create(&space->base, RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&namedExpr->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_namedexpr, parser)),					   
					   NULL);

	rpa_mnode_cat_list(&rpaExpr->head,
					   MNODE2LIST(rpa_mnode_callback_create(&ignoreLine->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&namedExprMatch->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   MNODE2LIST(rpa_mnode_callback_create(&exprMatch->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_mnode_print, parser)),
					   NULL);
					   
	root = rpa_match_list_create("root", RPA_MATCHFUNC_LIST);
	rpa_mnode_cat_list(&((rpa_match_list_t *)root)->head,
					   MNODE2LIST(rpa_mnode_callback_create(&rpaExpr->base, RPA_MATCH_NONE|RPA_REASON_ALL, rpa_parser_cb_rpaexpr, parser)),
					   NULL);
					   
	parser->pVarLinkBnfRoot = pVarLinkRoot = rpa_varlink_create(RPA_VAR_PTR, "root");
	if (pVarLinkRoot) {
		pVarLinkRoot->var.userdata4 = MATCH_CLASS_MATCHPTR;
		pVarLinkRoot->var.v.ptr = (void*) root;
		pVarLinkRoot->var.finalize = rpa_var_class_destroy;
		rpa_list_addt(&parser->bnftree, &pVarLinkRoot->lnk);
	}

	parser->bnfroot = root;
}


static rpa_mnode_t *rpa_varlink_list_add_mnode(rpa_mnode_t *mnode, rpa_head_t *head)
{
	rpa_varlink_t *pVarLinkMnodePtr;

	pVarLinkMnodePtr = rpa_varlink_create(RPA_VAR_PTR, "");
	if (pVarLinkMnodePtr) {
		pVarLinkMnodePtr->var.userdata4 = MATCH_CLASS_MNODEPTR;
		pVarLinkMnodePtr->var.v.ptr = (void*) mnode;
		pVarLinkMnodePtr->var.finalize = rpa_var_class_destroy;
		rpa_list_addt(head, &pVarLinkMnodePtr->lnk);
	}
	return mnode;
}


static rpa_match_t *rpa_varlink_list_add_match(rpa_match_t *match, rpa_head_t *head)
{
	rpa_varlink_t *pVarLinkMatch;

	pVarLinkMatch = rpa_varlink_create(RPA_VAR_PTR, "");
	if (pVarLinkMatch) {
		pVarLinkMatch->var.userdata4 = MATCH_CLASS_MATCHPTR;
		pVarLinkMatch->var.v.ptr = (void*) match;
		pVarLinkMatch->var.finalize = rpa_var_class_destroy;
		rpa_list_addt(head, &pVarLinkMatch->lnk);
	}
	return match;
}

