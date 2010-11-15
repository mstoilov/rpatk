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

#include "rpaclass.h"
#include "rpadbexpriv.h"
#include "rpamatch.h"
#include "rpastring.h"
#include "rpamem.h"
#include "rpamatch.h"
#include "rpamnode.h"
#include "rpamatchstr.h"
#include "rpamatchrange.h"
#include "rpamatchval.h"
#include "rpamatchlist.h"
#include "rpautf.h"
#include "rpadebug.h"
#include "rpavarlink.h"
#include "rpasearch.h"

#define PRINT_STACK_SIZE 3000


const char *get_match_func(rpa_match_t *match)
{
	if (match->match_function_id == RPA_MATCHFUNC_LIST)
		return "all";
	else if (match->match_function_id == RPA_MATCHFUNC_LIST_ALT || match->match_function_id == RPA_MATCHFUNC_NLIST_ALT)
		return "or";
	else if (match->match_function_id == RPA_MATCHFUNC_NLIST_BESTALT)
		return "best";
	else if (match->match_function_id == RPA_MATCHFUNC_LIST_CONTAIN)
		return "cont";
	else if (match->match_function_id == RPA_MATCHFUNC_LIST_AND)
		return "and";
	else if (match->match_function_id == RPA_MATCHFUNC_LIST_MINUS)
		return "minus";
	else if (match->match_function_id == RPA_MATCHFUNC_LIST_NOT)
		return "not";
	else if (match->match_function_id == RPA_MATCHFUNC_VAL_CHREQ)
		return "eq";
	else if (match->match_function_id == RPA_MATCHFUNC_VAL_CHRNOTEQ)
		return "noteq";
	else if (match->match_function_id == RPA_MATCHFUNC_CHREQANY)
		return "any";
	else if (match->match_function_id == RPA_MATCHFUNC_RANGE_CHRINRANGE)
		return "inrng";
	else if (match->match_function_id == RPA_MATCHFUNC_RANGE_CHRNOTINRANGE)
		return "notinrng";
	else if (match->match_function_id == RPA_MATCHFUNC_STR)
		return "eq";
	else if (match->match_function_id == RPA_MATCHFUNC_STR_CHRINSTR) 
		return "chrstr";
	else if (match->match_function_id == RPA_MATCHFUNC_STR_CHRNOTINSTR) 
		return "chrnstr";

	return "unk";
}


void rpa_dump_tree(rpa_match_t *match, unsigned int flags, int level, rpa_match_t **stack, int cur, int max)
{
	int i;
	char q = 'x', l = 'x';
	
	if (level >= max)
		return;

	if ((flags & (RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE)) == (RPA_MATCH_OPTIONAL|RPA_MATCH_MULTIPLE))
		q = '*';
	else if ((flags & RPA_MATCH_MULTIPLE) == RPA_MATCH_MULTIPLE)
		q = '+';
	else if ((flags & RPA_MATCH_OPTIONAL) == RPA_MATCH_OPTIONAL)
		q = '?';
	else
		q = 'x';
		
	if ((flags & RPA_MNODE_LOOP)) {
		l = 'l';
	}	
	
	if ((rpa_class_getid((rpa_class_t*)match) & RPA_MATCH_NLIST_CLASSID) && level > 0 ) {
		char szdataptr[4096];
		szdataptr[0] = '\0';
		rpa_dump_list_dataptr(match, szdataptr, sizeof(szdataptr));
		for (i = 0; i < level; i++)
			rpa_printf("    ");
		rpa_printf("(%3s, %s, %c %c) <:%s:> %s\n", rpa_class_getstr((rpa_class_t*)match), get_match_func(match), q, l, match->name, szdataptr);
		for (i = cur; i > 0; i--) {
			if (stack[i] == match)
				return;
		}
		if (cur >= max)
			return;
		stack[++cur] = match;
	} else if (rpa_class_getid((rpa_class_t*)match) & RPA_MATCH_LIST_CLASSID || (rpa_class_getid((rpa_class_t*)match) & RPA_MATCH_NLIST_CLASSID && level < 1)) {
		char szdataptr[4096];
		rpa_mnode_t *node;
		rpa_list_t *pos;
		rpa_match_list_t *matchList = (rpa_match_list_t *)match;
		szdataptr[0] = '\0';
		rpa_dump_list_dataptr(match, szdataptr, sizeof(szdataptr));
		for (i = 0; i < level; i++)
			rpa_printf("    ");
		rpa_printf("(%3s, %s, %c %c) %s %s\n", rpa_class_getstr((rpa_class_t*)match), get_match_func(match), q, l, match->name, szdataptr);
		for (i = cur; i > 0; i--) {
			if (stack[i] == match)
				return;
		}
		if (cur >= max)
			return;
		stack[++cur] = match;
		rpa_list_for_each(pos, &matchList->head) {
			node = rpa_list_entry(pos, rpa_mnode_t, mlink);
			rpa_dump_tree(node->match, node->flags, level + 1, stack, cur, max);
		}
	} else if (rpa_class_getid((rpa_class_t*)match) & RPA_MATCH_STR_CLASSID) {
		for (i = 0; i < level; i++)
			rpa_printf("    ");
		rpa_printf("(%3s, %s, %c) %s\n", rpa_class_getstr((rpa_class_t*)match), get_match_func(match), 
						q, match->name);
	} else if (rpa_class_getid((rpa_class_t*)match) & RPA_MATCH_VAL_CLASSID) {
			unsigned char mb[7];
			rpa_memset(mb, 0, sizeof(mb));
			rpa_utf8_wctomb(((rpa_match_val_t*)match)->val, mb, sizeof(mb));
			for (i = 0; i < level; i++)
				rpa_printf("    ");
			rpa_printf("(%3s, %s, %c) %s (0x%x)\n", rpa_class_getstr((rpa_class_t*)match), get_match_func(match), q, mb, ((rpa_match_val_t*)match)->val);
	} else {
		for (i = 0; i < level; i++)
			rpa_printf("    ");
		rpa_printf("(%3s, %s, %c) %s\n", rpa_class_getstr((rpa_class_t*)match), get_match_func(match), q, match->name);
	}
}


void rpa_dump_match_tree(rpa_match_t *match)
{
	rpa_match_t *printStack[PRINT_STACK_SIZE];
	if (match)
		rpa_dump_tree(match, 0, 0, printStack, 0, PRINT_STACK_SIZE - 1);
}


void rpa_dump_pattern_tree(rpa_pattern_handle pattern)
{
	if (pattern)
		rpa_dump_match_tree((rpa_match_t*)pattern->var.v.ptr);
}



#ifdef DEBUG


long rpa_get_alloc_mem()
{
	return g_rpa_allocmem;
}


long rpa_get_alloc_maxmem()
{
	return g_rpa_maxmem;
}


void rpa_varlink_dump(rpa_varlink_ptr pVarLink)
{
	if (pVarLink->var.userdata4 == MATCH_CLASS_NAMEDMATCHPTR && pVarLink->var.type == RPA_VAR_PTR) {
		rpa_printf("%35s: ", "NAMEDMATCHPTR");
		if ( rpa_class_getid((rpa_class_t*)pVarLink->var.v.ptr) & RPA_MATCH_LIST_CLASSID) {
			rpa_match_list_t *match = (rpa_match_list_t*)pVarLink->var.v.ptr;
			rpa_printf("%s (rpa_match_list_t)", match->base.name);
		}
	} else if (pVarLink->var.userdata4 == MATCH_CLASS_MATCHPTR && pVarLink->var.type == RPA_VAR_PTR) {
		rpa_printf("%35s: ", "MATCHPTR");
		if ( rpa_class_getid((rpa_class_t*)pVarLink->var.v.ptr) == RPA_MATCH_NEWLINE_CLASSID) {
			rpa_printf("~ (rpa_match_t)");
		} else if ( rpa_class_getid((rpa_class_t*)pVarLink->var.v.ptr) & RPA_MATCH_VAL_CLASSID) {
			rpa_match_val_t *match = (rpa_match_val_t*)pVarLink->var.v.ptr;
			unsigned char mb[7];
			rpa_memset(mb, 0, sizeof(mb));
			rpa_utf8_wctomb(match->val, mb, sizeof(mb));
			rpa_printf("%s (rpa_match_val_t)", mb);
		} else if (rpa_class_getid((rpa_class_t*)pVarLink->var.v.ptr) & RPA_MATCH_RANGE_CLASSID) {
			unsigned char low[7], high[7];
			rpa_match_range_t *match = (rpa_match_range_t*)pVarLink->var.v.ptr;
			rpa_memset(low, 0, sizeof(low));
			rpa_memset(high, 0, sizeof(high));
			rpa_utf8_wctomb((int)match->low, low, sizeof(low));
			rpa_utf8_wctomb((int)match->high, high, sizeof(high));
			rpa_printf("%s-%s (rpa_match_range_t)", low, high);
		} else if (rpa_class_getid((rpa_class_t*)pVarLink->var.v.ptr) & RPA_MATCH_STR_CLASSID) {
			rpa_match_str_t *match = (rpa_match_str_t*)pVarLink->var.v.ptr;
			rpa_printf("%s (rpa_match_str_t)", match->base.name);
		} else if (rpa_class_getid((rpa_class_t*)pVarLink->var.v.ptr) & RPA_MATCH_LIST_CLASSID) {
			rpa_match_list_t *match = (rpa_match_list_t*)pVarLink->var.v.ptr;
			rpa_printf("%s (rpa_match_list_t)", match->base.name);
		}

	} else if (pVarLink->var.userdata4 == MATCH_CLASS_MNODEPTR && pVarLink->var.type == RPA_VAR_PTR) {
		rpa_mnode_t *mnode = (rpa_mnode_t*)pVarLink->var.v.ptr;
		rpa_printf("%35s: ", "MNODEPTR");
		rpa_printf("%s (rpa_mnode_t), flags = 0x%x", mnode->match->name, mnode->flags);
	}

	rpa_printf("\n");
}


void rpa_dump_all(rpa_head_t *head)
{
	rpa_list_t *pos;
	rpa_varlink_t *pVarLink;

	rpa_list_for_each(pos, head) {
		pVarLink = rpa_list_entry(pos, rpa_varlink_t, lnk);
		rpa_varlink_dump(pVarLink);
	}
}


void rpa_dump_stack(rpa_dbex_handle hDbex)
{
	rpa_dump_all(&hDbex->treehead);
}


int rpa_mnode_debug_print(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
#ifdef DEBUGPRINT
	rpa_mnode_print(mnode, stat, input, size, reason);
#endif
	return size;
}

int rpa_mnode_print(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	char *reasonstr;

	if (reason & RPA_REASON_START)
		reasonstr = " s";
	else if ((reason & RPA_REASON_END) && (reason & RPA_REASON_MATCHED))
		reasonstr = "me";
	else if (reason & RPA_REASON_END)
		reasonstr = " e";
	else if (reason & RPA_REASON_MATCHED)
		reasonstr = "m ";
	else 
		reasonstr = "no";

	rpa_printf("%s (%ld, %2d) %30s: ", reasonstr, (long)(input - stat->start), size, mnode->match->name);
	while (input && size) {
		rpa_printf("%c", *input++);
		size -= 1;
		// max one char for RPA_REASON_START callbacks
		if (reason & RPA_REASON_START) {
			rpa_printf(" ...");
			break;
		}
	}
	rpa_printf("\n");
	return size;
}


#else


void rpa_dump_stack(rpa_dbex_handle hDbex)
{


}


int rpa_mnode_print(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)

{
	return size;
}


int rpa_mnode_debug_print(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	return size;
}


#endif
