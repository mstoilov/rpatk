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

#include "rpaerror.h"
#include "rpavarlink.h"
#include "rpadbexpriv.h"
#include "rpamatch.h"
#include "rpamnode.h"
#include "rstring.h"
#include "rmem.h"
#include "rpamatch.h"
#include "rpamatchspecial.h"
#include "rpamatchstr.h"
#include "rpamatchrange.h"
#include "rpamatchval.h"
#include "rpamatchlist.h"
#include "rpamatchrangelist.h"
#include "rpadebug.h"
#include "rpasearch.h"

#define RPA_VMSTACK_SIZE 256
#define RPA_VMSTACK_CHUNK 64


static rpa_word_t rpa_vmcb_create_val_matchptr(rpa_vm_t *vm);				/* VM_CREATE_VAL_MATCHPTR 0 */
static rpa_word_t rpa_vmcb_create_list_matchptr(rpa_vm_t *vm);				/* VM_CREATE_LIST_MATCHPTR 1 */
static rpa_word_t rpa_vmcb_create_nlist_matchptr(rpa_vm_t *vm);				/* VM_CREATE_NLIST_MATCHPTR 2 */
static rpa_word_t rpa_vmcb_get_nlist_matchptr(rpa_vm_t *vm);				/* VM_GET_NLIST_MATCHPTR 3 */
static rpa_word_t rpa_vmcb_create_newline_matchptr(rpa_vm_t *vm);			/* VM_CREATE_NEWLINE_MATCHPTR 4 */
static rpa_word_t rpa_vmcb_create_anychar_matchptr(rpa_vm_t *vm);			/* VM_CREATE_ANYCHAR_MATCHPTR 5 */
static rpa_word_t rpa_vmcb_create_str_matchptr(rpa_vm_t *vm);				/* VM_CREATE_STR_MATCHPTR 6 */
static rpa_word_t rpa_vmcb_create_range_matchptr(rpa_vm_t *vm);				/* VM_CREATE_RANGE_MATCHPTR 7 */
static rpa_word_t rpa_vmcb_create_mnode(rpa_vm_t *vm);						/* VM_CREATE_MNODE 8 */
static rpa_word_t rpa_vmcb_create_mnode_callback(rpa_vm_t *vm);				/* VM_CREATE_MNODE_CALLBACK 9 */
static rpa_word_t rpa_vmcb_varlink_ptr(rpa_vm_t *vm);						/* VM_VARLINK_PTR 10 */
static rpa_word_t rpa_vmcb_add_mnode_to_list(rpa_vm_t *vm);					/* VM_ADD_MNODE_TO_LIST 11 */
static rpa_word_t rpa_vmcb_set_match_function(rpa_vm_t *vm);				/* VM_SET_MATCH_FUNCTION 12 */
static rpa_word_t rpa_vmcb_dump_tree(rpa_vm_t *vm);							/* VM_DUMP_TREE 13 */
static rpa_word_t rpa_vmcb_set_match_name(rpa_vm_t *vm);					/* VM_SET_MATCH_NAME 14 */
static rpa_word_t rpa_vmcb_set_default_pattern(rpa_vm_t *vm);				/* VM_SET_DEFAULT_PATTERN 15 */
static rpa_word_t rpa_vmcb_setup_list(rpa_vm_t *vm);						/* VM_SETUP_LIST 16 */
static rpa_word_t rpa_vcmb_add_match_to_dataptr(rpa_vm_t *vm);				/* VM_ADD_MATCH_TO_DATAPTR 17 */
static rpa_word_t rpa_vmcb_create_hlist_matchptr(rpa_vm_t *vm);				/* VM_CREATE_HLIST_MATCHPTR 18 */
static rpa_word_t rpa_vmcb_create_funlast_matchptr(rpa_vm_t *vm);			/* VM_CREATE_FUNLAST_MATCHPTR 19 */
static rpa_word_t rpa_vmcb_create_funabort_matchptr(rpa_vm_t *vm);			/* VM_CREATE_FUNABORT_MATCHPTR 20 */
static rpa_word_t rpa_vmcb_create_funfail_matchptr(rpa_vm_t *vm);			/* VM_CREATE_FUNFAIL_MATCHPTR 21*/
static rpa_word_t rpa_vmcb_set_strval(rpa_vm_t *vm);						/* VM_SET_STRVAL 22*/
static rpa_word_t rpa_vmcb_noop(rpa_vm_t *vm);								/* VM_NOOP 23*/
static rpa_word_t rpa_vmcb_add_mnode_to_list_dataptr(rpa_vm_t *vm);		/* VM_ADD_MNODE_TO_NLIST_DATAPTR 24 */
static rpa_word_t rpa_vmcb_get_match_function(rpa_vm_t *vm);				/* VM_GET_MATCH_FUNCTION 25 */
static rpa_word_t rpa_vmcb_reset_list(rpa_vm_t *vm);						/* VM_RESET_LIST 26 */
static rpa_word_t rpa_vmcb_create_ragnelist_matchptr(rpa_vm_t *vm);			/* VM_CREATE_RANGELIST_MATCHPTR 27 */



static rpa_vm_callback calltable[] = {
	rpa_vmcb_create_val_matchptr,				/* VM_CREATE_VAL_MATCHPTR 0 */
	rpa_vmcb_create_list_matchptr,				/* VM_CREATE_LIST_MATCHPTR 1 */
	rpa_vmcb_create_nlist_matchptr,				/* VM_CREATE_NLIST_MATCHPTR 2 */
	rpa_vmcb_get_nlist_matchptr,				/* VM_GET_NLIST_MATCHPTR 3 */
	rpa_vmcb_create_newline_matchptr,			/* VM_CREATE_NEWLINE_MATCHPTR 4 */
	rpa_vmcb_create_anychar_matchptr,			/* VM_CREATE_ANYCHAR_MATCHPTR 5 */
	rpa_vmcb_create_str_matchptr,				/* VM_CREATE_STR_MATCHPTR 6 */
	rpa_vmcb_create_range_matchptr,				/* VM_CREATE_RANGE_MATCHPTR 7 */
	rpa_vmcb_create_mnode,						/* VM_CREATE_MNODE 8 */
	rpa_vmcb_create_mnode_callback,				/* VM_CREATE_MNODE_CALLBACK 9 */
	rpa_vmcb_varlink_ptr,						/* VM_VARLINK_PTR 10 */
	rpa_vmcb_add_mnode_to_list,					/* VM_ADD_MNODE_TO_LIST 11 */
	rpa_vmcb_set_match_function,				/* VM_SET_MATCH_FUNCTION 12 */
	rpa_vmcb_dump_tree,							/* VM_DUMP_TREE 13 */
	rpa_vmcb_set_match_name,					/* VM_SET_MATCH_NAME 14 */
	rpa_vmcb_set_default_pattern,				/* VM_SET_DEFAULT_PATTERN 15 */
	rpa_vmcb_setup_list,						/* VM_SETUP_LIST 16 */
	rpa_vcmb_add_match_to_dataptr,				/* VM_ADD_MATCH_TO_DATAPTR 17 */	
	rpa_vmcb_create_hlist_matchptr,				/* VM_CREATE_HLIST_MATCHPTR 18 */
	rpa_vmcb_create_funlast_matchptr,			/* VM_CREATE_FUNLAST_MATCHPTR 19 */
	rpa_vmcb_create_funabort_matchptr,			/* VM_CREATE_FUNABORT_MATCHPTR 20 */
	rpa_vmcb_create_funfail_matchptr,			/* VM_CREATE_FUNFAIL_MATCHPTR 21 */
	rpa_vmcb_set_strval,						/* VM_SET_STRVAL 22 */
	rpa_vmcb_noop,								/* VM_NOOP_23 23 */
	rpa_vmcb_add_mnode_to_list_dataptr,			/* VM_ADD_MNODE_TO_LIST_DATAPTR 24 */
	rpa_vmcb_get_match_function,				/* VM_GET_MATCH_FUNCTION 25 */
	rpa_vmcb_reset_list,						/* VM_RESET_LIST 26 */
	rpa_vmcb_create_ragnelist_matchptr,			/* VM_CREATE_RANGELIST_MATCHPTR 27 */
	NULL,
};


int rpa_common_callback(rpa_mnode_t *mnode, rpa_stat_t *stat, const char *input, unsigned int size, unsigned int reason)
{
	rpa_varlink_t *pVarLinkCallback;
	rpa_callbackdata_t *pCbData;
	
	if (!(mnode->flags & RPA_MNODE_CALLBACK))
		goto error;
	pVarLinkCallback = (rpa_varlink_t *)((rpa_mnode_callback_t*)mnode)->userdata;
	if (!pVarLinkCallback || pVarLinkCallback->var.userdata4 != MATCH_CLASS_CALLBACKPTR)
		goto error;
	pCbData = pVarLinkCallback->var.v.ptr;
	if  (pCbData->func)
		return pCbData->func(mnode->match->name, pCbData->userdata, input, size, reason, stat->start, stat->end);
error:
	return size;
}


void rpa_mnode_connect_callback_dontuse(rpa_dbex_handle hDbex, rpa_mnode_t *mnode)
{
	int ret = 0;
	rpa_varlink_t *pCallback;
	const char *name = (void*)0;
	int size = name ? r_strlen(name) : 0;

	if (!mnode || (mnode->flags & RPA_MNODE_NOCONNECT) || !mnode->match || !mnode->match->name)
		return;
	name = mnode->match->name;
	size = mnode->match->namesiz;
	for (pCallback = rpa_dbex_prev_callback(hDbex, 0); pCallback; pCallback = rpa_dbex_prev_callback(hDbex, pCallback)) {
		rpa_callbackdata_t *pCbData = (rpa_callbackdata_t *)pCallback->var.v.ptr;
		ret = rpa_dbex_strmatch(name, pCbData->namematch);
		if (size && ret == size) {
			if (mnode->flags & RPA_MNODE_CALLBACK) {
				mnode->flags |= pCbData->reason;
				((rpa_mnode_callback_t*)mnode)->matched_callback = rpa_common_callback;
				((rpa_mnode_callback_t*)mnode)->userdata = pCallback;
			}
			return;
		}
	}	


}


void rpa_dbex_connect_callbacks(rpa_dbex_handle hDbex)
{
	rpa_dbex_t strdbex;
	rpa_pattern_handle hDefault;
	rpa_mnode_callback_t *cbmnode;
	rpa_list_t *pos;
	int ret;
	rpa_varlink_t *pCallback;
	const char *name;
	int namelen;

	for (pCallback = rpa_dbex_next_callback(hDbex, 0); pCallback; pCallback = rpa_dbex_next_callback(hDbex, pCallback)) {
		rpa_callbackdata_t *pCbData = (rpa_callbackdata_t *)pCallback->var.v.ptr;
		rpa_dbex_init(&strdbex, 16);
		rpa_dbex_open(&strdbex);
		if (rpa_dbex_load_string(&strdbex, pCbData->namematch) < 0) {
			rpa_dbex_cleanup(&strdbex);
			continue;
		}
		rpa_dbex_close_do(&strdbex);
		hDefault = rpa_dbex_default_pattern(&strdbex);
		rpa_list_for_each(pos, &hDbex->callbackmnodes) {
				cbmnode = rpa_list_entry(pos, rpa_mnode_callback_t, cblink);
			if (!cbmnode->matched_callback) {
				name = ((rpa_mnode_t*)cbmnode)->match->name;
				namelen = ((rpa_mnode_t*)cbmnode)->match->namesiz;
				ret = rpa_stat_match_lite(&strdbex.stat, hDefault, name, name, name + namelen);
				if (namelen && ret == namelen) {
					((rpa_mnode_t*)cbmnode)->flags |= pCbData->reason;
					cbmnode->matched_callback = rpa_common_callback;
					cbmnode->userdata = pCallback;
				}
			}
		}
		rpa_dbex_cleanup(&strdbex);
	}	
}



rpa_word_t rpa_sdbm_hash(rpa_dbex_handle hDbex, const char *str)
{
	rpa_word_t hash = 0;
	int c;

	while ((c = *str++))
		hash = c + (hash << 6) + (hash << 16) - hash;
	return hash & (hDbex->namehashEntries - 1);
}


rpa_word_t rpa_sdbm_hash_strlen(rpa_dbex_handle hDbex, const char *str, unsigned int strlen)
{
	rpa_word_t hash = 0;
	int c;

	while (strlen--) {
		c = *str++;
		hash = c + (hash << 6) + (hash << 16) - hash;
	}
	return hash & (hDbex->namehashEntries - 1);
}


rpa_varlink_t *rpa_varlink_find_named_matchptr(rpa_head_t *head, const char *name, unsigned int size)
{
	rpa_list_t *pos;
	rpa_varlink_t *pVarLinkMatch;

	for (pos = rpa_list_last(head); pos; pos = rpa_list_prev(head, pos)) {
		pVarLinkMatch = rpa_list_entry(pos, rpa_varlink_t, hlnk);
		if (pVarLinkMatch->var.userdata4 == MATCH_CLASS_NAMEDMATCHPTR) {
			rpa_match_t *match = (rpa_match_t *)pVarLinkMatch->var.v.ptr;
			if (r_strncmp(match->name, name, size) == 0)
				return pVarLinkMatch;
		}
	}
	return ((void*)0);
}


int rpa_dbex_check_parser(rpa_dbex_handle hDbex)
{
	if (!hDbex->parser)
		hDbex->parser = rpa_parser_create();
		
	if (!hDbex->parser) {
		/*
		 * Error
		 */
		return -1;
	}
	return 0;
}


int rpa_dbex_check_vm(rpa_dbex_handle hDbex)
{
	int table;
	if (!hDbex->vm)
		hDbex->vm = rpa_vm_create();
	if (!hDbex->vm) {
		/*
		 * Error
		 */
		return -1;
	}
	table = rpa_vm_cbtable_add(hDbex->vm, calltable);
	/* This has to be the first(and only) call table */
	if (table != 0)
		return -1;
	hDbex->vm->userdata = (void*)hDbex;
	return 0;
	
}


/*
 * noop
 */
static rpa_word_t rpa_vmcb_noop(rpa_vm_t *vm)
{
	return vm->r[0];
}


/*
 * r[0] - varlink match ptr
 */
static rpa_word_t rpa_vmcb_dump_tree(rpa_vm_t *vm)
{
	rpa_dump_pattern_tree((rpa_varlink_t*)vm->r[0]);
	return 0;
}


/*
 * r[0] - varlink match ptr
 */
static rpa_word_t rpa_vmcb_set_default_pattern(rpa_vm_t *vm)
{
	rpa_dbex_handle hDbex = (rpa_dbex_handle)vm->userdata;
	
	hDbex->defaultPattern = (rpa_varlink_t*)vm->r[0];
	return 0;
}


rpa_varlink_t *rpa_dbex_create_dataptr(rpa_dbex_handle hDbex, void *dataptr)
{
	rpa_varlink_t *pVarLinkDataPtr;

	pVarLinkDataPtr = rpa_varlink_create(RPA_VAR_PTR, "DATA");
	if (!pVarLinkDataPtr)
		return (void*)0;
	pVarLinkDataPtr->var.v.ptr = dataptr;
	pVarLinkDataPtr->var.userdata4 = (rpa_word_t)MATCH_CLASS_DATAPTR;
	pVarLinkDataPtr->var.finalize = rpa_var_class_destroy;
	rpa_list_addt(&hDbex->treehead, &pVarLinkDataPtr->lnk);
	return pVarLinkDataPtr;
}


rpa_varlink_t *rpa_dbex_create_mnode(rpa_dbex_handle hDbex, rpa_match_t *match, unsigned int flags)
{
	rpa_varlink_t *pVarLinkMnodePtr;

	pVarLinkMnodePtr = rpa_varlink_create(RPA_VAR_PTR, "MNODE");
	if (!pVarLinkMnodePtr)
		return (void*)0;
	if ((pVarLinkMnodePtr->var.v.ptr = (void*) rpa_mnode_create(match, flags)) == (void*)0) {
		rpa_varlink_destroy(pVarLinkMnodePtr);
		return (void*)0;
	}
	pVarLinkMnodePtr->var.userdata4 = (rpa_word_t)MATCH_CLASS_MNODEPTR;
	pVarLinkMnodePtr->var.finalize = rpa_var_class_destroy;
	rpa_list_addt(&hDbex->treehead, &pVarLinkMnodePtr->lnk);
	return pVarLinkMnodePtr;
}

/*
 * r[0] - match
 * r[1] - flags
 */
static rpa_word_t rpa_vmcb_create_mnode(rpa_vm_t *vm)
{
	return (rpa_word_t)rpa_dbex_create_mnode(
		(rpa_dbex_handle)vm->userdata,
		(rpa_match_t*)((rpa_varlink_t*)vm->r[0])->var.v.ptr,
		(unsigned int) vm->r[1]);
}


/*
 * r[0] - hlist
 * r[1] - match
 */
static rpa_word_t rpa_vcmb_add_match_to_dataptr(rpa_vm_t *vm)
{
	return 0;
}


/*
 * r[0] - nlist
 * r[1] - mnode
 */
static rpa_word_t rpa_vmcb_add_mnode_to_list_dataptr(rpa_vm_t *vm)
{
	return (rpa_word_t)rpa_dbex_add_mnode_to_list_dataptr(
		(rpa_dbex_handle)vm->userdata,
		(rpa_match_t*)((rpa_varlink_t*)vm->r[0])->var.v.ptr,
		(rpa_mnode_t*)((rpa_varlink_t*)vm->r[1])->var.v.ptr);
	return vm->r[0];
}


/*
 * r[0] - match
 */
static rpa_word_t rpa_vmcb_get_match_function(rpa_vm_t *vm)
{
	rpa_match_t *match = (rpa_match_t*)((rpa_varlink_t*)vm->r[0])->var.v.ptr;
	
	return (rpa_word_t) match->match_function_id;
}


/*
 * r[0] - list
 */
static rpa_word_t rpa_vmcb_reset_list(rpa_vm_t *vm)
{
	rpa_match_list_t *matchlist = (rpa_match_list_t*)((rpa_varlink_t*)vm->r[0])->var.v.ptr;
	
	rpa_list_init(&matchlist->head);
	rpa_dbex_reset_list_dataptr((rpa_dbex_handle)vm->userdata, (rpa_match_t*)matchlist);
	return 0;
}


rpa_varlink_t *rpa_dbex_create_mnode_callback(rpa_dbex_handle hDbex, rpa_match_t *match, unsigned int flags)
{
	rpa_varlink_t *pVarLinkMnodePtr;
	rpa_mnode_callback_t *pCbMnode;


	pVarLinkMnodePtr = rpa_varlink_create(RPA_VAR_PTR, "MNODECB");
	if (!pVarLinkMnodePtr)
		return (void*)0;
	pCbMnode = (rpa_mnode_callback_t *)rpa_mnode_callback_create(match, flags, 0, 0);
	if ((pVarLinkMnodePtr->var.v.ptr = (void*) pCbMnode) == (void*)0) {
		rpa_varlink_destroy(pVarLinkMnodePtr);
		return (void*)0;
	}
	pVarLinkMnodePtr->var.userdata4 = (rpa_word_t)MATCH_CLASS_MNODEPTR;
	pVarLinkMnodePtr->var.finalize = rpa_var_class_destroy;
	rpa_list_addt(&hDbex->treehead, &pVarLinkMnodePtr->lnk);
	if ((((rpa_mnode_t *)pCbMnode)->flags & RPA_MNODE_NOCONNECT) == 0)
		rpa_list_addt(&hDbex->callbackmnodes, &pCbMnode->cblink);
	return pVarLinkMnodePtr;
}

/*
 * r[0] match 
 * r[1] flags
 */

static rpa_word_t rpa_vmcb_create_mnode_callback(rpa_vm_t *vm)
{
	
	return (rpa_word_t)rpa_dbex_create_mnode_callback(
		(rpa_dbex_handle)vm->userdata,
		(rpa_match_t*)((rpa_varlink_t*)vm->r[0])->var.v.ptr,
		(unsigned int) vm->r[1]);
}


static rpa_varlink_t *rpa_dbex_create_val_matchptr(rpa_dbex_handle hDbex, const char *name, unsigned int size, rpa_word_t val)
{
	rpa_varlink_t *pVarLinkMatchPtr;

	pVarLinkMatchPtr = rpa_varlink_create(RPA_VAR_PTR, "VAL");
	if (!pVarLinkMatchPtr) 
		return (void*)0;
	if ((pVarLinkMatchPtr->var.v.ptr = (void*) rpa_match_val_create_namesize(name, size, RPA_MATCHFUNC_VAL_CHREQ, (unsigned int)val)) == (void*)0) {
		rpa_varlink_destroy(pVarLinkMatchPtr);
		return (void*)0;
	}
	pVarLinkMatchPtr->var.userdata4 = (rpa_word_t)MATCH_CLASS_MATCHPTR;
	pVarLinkMatchPtr->var.finalize = rpa_var_class_destroy;
	rpa_list_addt(&hDbex->treehead, &pVarLinkMatchPtr->lnk);
	return pVarLinkMatchPtr;
}


int rpa_dbex_int_ascii_matchptr_array(rpa_dbex_handle hDbex)
{
	int i;
	
	for (i = 0; i < RPA_PREALLOC_VAL_COUNT; i++) {
		if ((hDbex->pVarLinkValMatchPtr[i] = rpa_dbex_create_val_matchptr(hDbex, 0, 0, i)) == (void*)0)
			return -1;
	}
	return 0;
}

/*
 * r[0] match (varlink)
 * r[1] name
 * r[2] namesize
 */
static rpa_word_t rpa_vmcb_set_match_name(rpa_vm_t *vm)
{
	rpa_varlink_t *pVarLink = (rpa_varlink_t*) vm->r[0];
	
	rpa_match_setup_name((rpa_match_t*)pVarLink->var.v.ptr, (const char*)vm->r[1], (unsigned int)vm->r[2]);
	return 0;
}


/*
 * r[0] name
 * r[1] namesize
 * r[2] vale
 */
static rpa_word_t rpa_vmcb_create_val_matchptr(rpa_vm_t *vm)
{
	if ((rpa_word_t) vm->r[2] < RPA_PREALLOC_VAL_COUNT)
		return (rpa_word_t)((rpa_dbex_handle)vm->userdata)->pVarLinkValMatchPtr[(rpa_word_t) vm->r[2]];

	return (rpa_word_t)rpa_dbex_create_val_matchptr(
		(rpa_dbex_handle)vm->userdata,
		(const char*) vm->r[0],
		(unsigned int) vm->r[1],
		(rpa_word_t) vm->r[2]);
}


rpa_varlink_t *rpa_dbex_create_list_matchptr(rpa_dbex_handle hDbex, const char *name, unsigned int size, rpa_matchfunc_t match_function_id)
{
	rpa_varlink_t *pVarLinkListMatchPtr;

	pVarLinkListMatchPtr = rpa_varlink_create(RPA_VAR_PTR, "LIST");
	if (!pVarLinkListMatchPtr) 
		return (void*)0;
	if ((pVarLinkListMatchPtr->var.v.ptr = (void*) rpa_match_list_create_namesize(name, size, match_function_id)) == (void*)0) {
		rpa_varlink_destroy(pVarLinkListMatchPtr);
		return (void*)0;
	}
	pVarLinkListMatchPtr->var.userdata4 = (rpa_word_t)MATCH_CLASS_MATCHPTR;
	pVarLinkListMatchPtr->var.finalize = rpa_var_class_destroy;
	rpa_list_addt(&hDbex->treehead, &pVarLinkListMatchPtr->lnk);
	return pVarLinkListMatchPtr;
}


/*
 * r[0] name
 * r[1] namesize
 * r[2] list match func
 */
static rpa_word_t rpa_vmcb_create_list_matchptr(rpa_vm_t *vm)
{
	return (rpa_word_t)rpa_dbex_create_list_matchptr(
		(rpa_dbex_handle)vm->userdata,
		(const char*) vm->r[0],
		(unsigned int) vm->r[1],
		vm->r[2]);
}


rpa_varlink_t *rpa_dbex_create_rangelist_matchptr(rpa_dbex_handle hDbex, const char *name, unsigned int size)
{
	rpa_varlink_t *pVarLinkListMatchPtr;

	pVarLinkListMatchPtr = rpa_varlink_create(RPA_VAR_PTR, "LIST");
	if (!pVarLinkListMatchPtr) 
		return (void*)0;
	if ((pVarLinkListMatchPtr->var.v.ptr = (void*) rpa_match_rangelist_create_namesize(name, size, RPA_MATCHFUNC_LIST_ALT)) == (void*)0) {
		rpa_varlink_destroy(pVarLinkListMatchPtr);
		return (void*)0;
	}
	pVarLinkListMatchPtr->var.userdata4 = (rpa_word_t)MATCH_CLASS_MATCHPTR;
	pVarLinkListMatchPtr->var.finalize = rpa_var_class_destroy;
	rpa_list_addt(&hDbex->treehead, &pVarLinkListMatchPtr->lnk);
	return pVarLinkListMatchPtr;
}

/*
 * r[0] name
 * r[1] namesize
 */
static rpa_word_t rpa_vmcb_create_ragnelist_matchptr(rpa_vm_t *vm)
{
	return (rpa_word_t)rpa_dbex_create_rangelist_matchptr(
		(rpa_dbex_handle)vm->userdata,
		(const char*) vm->r[0],
		(unsigned int) vm->r[1]);
}


rpa_varlink_t *rpa_dbex_create_nlist_matchptr(rpa_dbex_handle hDbex, const char *name, unsigned int size, rpa_matchfunc_t match_function_id)
{
	rpa_varlink_t *pVarLinkListMatchPtr;

	pVarLinkListMatchPtr = rpa_varlink_create(RPA_VAR_PTR, "NLIST");
	if (!pVarLinkListMatchPtr)
		return (void*)0;
	if ((pVarLinkListMatchPtr->var.v.ptr = (void*) rpa_match_nlist_create_namesize(name, size, match_function_id)) == (void*)0) {
		rpa_varlink_destroy(pVarLinkListMatchPtr);
		return (void*)0;
	}
	pVarLinkListMatchPtr->var.userdata4 = (rpa_word_t)MATCH_CLASS_NAMEDMATCHPTR;
	pVarLinkListMatchPtr->var.finalize = rpa_var_class_destroy;
	rpa_list_addt(&hDbex->treehead, &pVarLinkListMatchPtr->lnk);
	return pVarLinkListMatchPtr;
}


static rpa_word_t rpa_vmcb_create_nlist_matchptr(rpa_vm_t *vm)
{
	return (rpa_word_t)rpa_dbex_create_nlist_matchptr(
		(rpa_dbex_handle)vm->userdata,
		(const char*) vm->r[0],
		(unsigned int) vm->r[1],
		vm->r[2]);
}


rpa_varlink_t *rpa_dbex_create_hlist_matchptr(rpa_dbex_handle hDbex, const char *name, unsigned int size, rpa_matchfunc_t match_function_id)
{
	rpa_varlink_t *pVarLinkListMatchPtr;

	pVarLinkListMatchPtr = rpa_varlink_create(RPA_VAR_PTR, "HLIST");
	if (!pVarLinkListMatchPtr)
		return (void*)0;
	if ((pVarLinkListMatchPtr->var.v.ptr = (void*) rpa_match_list_create_namesize(name, size, match_function_id)) == (void*)0) {
		rpa_varlink_destroy(pVarLinkListMatchPtr);
		return (void*)0;
	}
	pVarLinkListMatchPtr->var.userdata4 = (rpa_word_t)MATCH_CLASS_MATCHPTR;
	pVarLinkListMatchPtr->var.finalize = rpa_var_class_destroy;
	rpa_list_addt(&hDbex->treehead, &pVarLinkListMatchPtr->lnk);
	return pVarLinkListMatchPtr;
}


static rpa_word_t rpa_vmcb_create_hlist_matchptr(rpa_vm_t *vm)
{
	return (rpa_word_t)rpa_dbex_create_hlist_matchptr(
		(rpa_dbex_handle)vm->userdata,
		(const char*) vm->r[0],
		(unsigned int) vm->r[1],
		vm->r[2]);
}


rpa_match_t *rpa_varlink_matchptr(rpa_varlink_t *pVarLink)
{
	if (pVarLink) {
		return (rpa_match_t*) pVarLink->var.v.ptr;
	}
	return (void*)0;
}


rpa_mnode_t *rpa_varlink_mnodeptr(rpa_varlink_t *pVarLink)
{
	if (pVarLink) {
		return (rpa_mnode_t*) pVarLink->var.v.ptr;
	}
	return (void*)0;
}


static rpa_word_t rpa_vmcb_varlink_ptr(rpa_vm_t *vm)
{
	rpa_varlink_t *pVarLink = (rpa_varlink_t*)vm->r[0];

	return (rpa_word_t) pVarLink->var.v.ptr;
}


/*
 * r[0] - varlink
 * r[1] - match function id
 */
static rpa_word_t rpa_vmcb_set_match_function(rpa_vm_t *vm)
{
	rpa_varlink_t *pVarLink = (rpa_varlink_t*)vm->r[0];

	rpa_match_set_mathfunc((rpa_match_t*) pVarLink->var.v.ptr, vm->r[1]);
	return (rpa_word_t) 0;
}


rpa_varlink_t *rpa_dbex_get_nlist_matchptr(rpa_dbex_handle hDbex, const char *name, unsigned int size)
{
	rpa_varlink_t *pVarLinkListMatchPtr = 0;
	rpa_head_t *bucket;
	
	bucket = &hDbex->namehash[rpa_sdbm_hash_strlen(hDbex, name, size)];
	pVarLinkListMatchPtr = rpa_varlink_find_named_matchptr(bucket, name, size);
	if (!pVarLinkListMatchPtr) {
		pVarLinkListMatchPtr = rpa_dbex_create_nlist_matchptr(hDbex, name, size, RPA_MATCHFUNC_LIST);
		if (pVarLinkListMatchPtr)
			rpa_list_addt(bucket, &pVarLinkListMatchPtr->hlnk);
	}

	return pVarLinkListMatchPtr;
}


/*
 * r[0] - name
 * r[1] - name size
 * 
 * return hlist matchptr (varlink)
 */
static rpa_word_t rpa_vmcb_get_nlist_matchptr(rpa_vm_t *vm)
{
	return (rpa_word_t)rpa_dbex_get_nlist_matchptr(
		(rpa_dbex_handle)vm->userdata,
		(const char*) vm->r[0],
		(unsigned int) vm->r[1]);
}


rpa_varlink_t *rpa_dbex_create_newline_matchptr(rpa_dbex_handle hDbex, const char *name, unsigned int size)
{
	rpa_varlink_t *pVarLinkMatch;

	pVarLinkMatch = rpa_varlink_create(RPA_VAR_PTR, "NEWLINE");
	if (!pVarLinkMatch)
		return (void*)0;
	if ((pVarLinkMatch->var.v.ptr = (void*) rpa_match_special_create_namesize(name, size, RPA_MATCHFUNC_NEWLINE)) == (void*)0) {
		rpa_varlink_destroy(pVarLinkMatch);
		return (void*)0;
	}
	pVarLinkMatch->var.userdata4 = (rpa_word_t)MATCH_CLASS_MATCHPTR;
	pVarLinkMatch->var.finalize = rpa_var_class_destroy;
	rpa_list_addt(&hDbex->treehead, &pVarLinkMatch->lnk);
	return pVarLinkMatch;
}


/*
 * r[0] - name
 * r[1] - size
 */
static rpa_word_t rpa_vmcb_create_newline_matchptr(rpa_vm_t *vm)
{
	return (rpa_word_t)rpa_dbex_create_newline_matchptr(
		(rpa_dbex_handle)vm->userdata,
		(const char*) vm->r[0],
		(unsigned int) vm->r[1]);	
}


rpa_varlink_t *rpa_dbex_create_anychar_matchptr(rpa_dbex_handle hDbex, const char *name, unsigned int size)
{
	rpa_varlink_t *pVarLinkMatch;

	pVarLinkMatch = rpa_varlink_create(RPA_VAR_PTR, "ANYCHAR");
	if (!pVarLinkMatch)
		return (void*)0;	
	if ((pVarLinkMatch->var.v.ptr = (void*) rpa_match_special_create_namesize(name, size, RPA_MATCHFUNC_CHREQANY)) == (void*)0) {
		rpa_varlink_destroy(pVarLinkMatch);
		return (void*)0;		
	}
	pVarLinkMatch->var.userdata4 = (rpa_word_t)MATCH_CLASS_MATCHPTR;
	pVarLinkMatch->var.finalize = rpa_var_class_destroy;
	rpa_list_addt(&hDbex->treehead, &pVarLinkMatch->lnk);
	return pVarLinkMatch;
}


/*
 * r[0] - name
 * r[1] - size
 */
static rpa_word_t rpa_vmcb_create_anychar_matchptr(rpa_vm_t *vm)
{
	return (rpa_word_t)rpa_dbex_create_anychar_matchptr(
		(rpa_dbex_handle)vm->userdata,
		(const char*) vm->r[0],
		(unsigned int) vm->r[1]);
}


rpa_varlink_t *rpa_dbex_create_str_matchptr(rpa_dbex_handle hDbex, const char *input, unsigned int size, rpa_word_t count)
{
	rpa_varlink_t *pVarLinkStr;

	pVarLinkStr = rpa_varlink_create(RPA_VAR_PTR, "STR");
	if (!pVarLinkStr) 
		return (void*)0;
	if ((pVarLinkStr->var.v.ptr = (void*) rpa_match_str_create_namesize(input, size, RPA_MATCHFUNC_STR)) == (void*)0) {
		rpa_varlink_destroy(pVarLinkStr);
		return (void*)0;
	}
	pVarLinkStr->var.userdata4 = (rpa_word_t)MATCH_CLASS_MATCHPTR;
	pVarLinkStr->var.finalize = rpa_var_class_destroy;
	rpa_list_addt(&hDbex->treehead, &pVarLinkStr->lnk);
	if (!rpa_match_str_alloc_strval((rpa_match_t*)pVarLinkStr->var.v.ptr, (unsigned long)count)) {
		/*
		 * TBD: Handle the error
		 */

		
	}
	return pVarLinkStr;
}

/*
 * r[0] - name
 * r[1] - size
 * r[2] - count
 */
static rpa_word_t rpa_vmcb_create_str_matchptr(rpa_vm_t *vm)
{
	return (rpa_word_t)rpa_dbex_create_str_matchptr(
		(rpa_dbex_handle)vm->userdata,
		(const char*) vm->r[0],
		(unsigned int) vm->r[1],
		vm->r[2]);
}

/*
 * r[0] - varlink
 * r[1] - val
 * r[2] - offset
 */
static rpa_word_t rpa_vmcb_set_strval(rpa_vm_t *vm)
{
	rpa_varlink_t *pVarLink = (rpa_varlink_t*)vm->r[0];

	rpa_match_str_setval((rpa_match_t*) pVarLink->var.v.ptr, (unsigned int)vm->r[1], vm->r[2]);
	return (rpa_word_t) 0;
}


/*
 * r[0] - varlink match ptr
 */
static rpa_word_t rpa_vmcb_setup_list(rpa_vm_t *vm)
{
	rpa_dbex_handle hDbex = (rpa_dbex_handle)vm->userdata;
	rpa_match_t *match = (rpa_match_t*)((rpa_varlink_t*)vm->r[0])->var.v.ptr;
	rpa_dbex_setup_list(hDbex, match);
	return 0;
}


rpa_varlink_t *rpa_dbex_create_range_matchptr(rpa_dbex_handle hDbex, const char *name, unsigned int size, rpa_word_t low, rpa_word_t high)
{
	rpa_varlink_t *pVarLinkRange;
	
	if (low > high) {
		/*
		 * Swap the values;
		 */
		high = high ^ low;
		low = high ^ low;
		high = high ^ low;
	}

	pVarLinkRange = rpa_varlink_create(RPA_VAR_PTR, "RANGE");
	if (!pVarLinkRange)
		return (void*)0;
	if ((pVarLinkRange->var.v.ptr = (void*) rpa_match_range_create_namesize(name, size, RPA_MATCHFUNC_RANGE_CHRINRANGE, 
		(unsigned int)low, (unsigned int)high)) == (void*)0) {
		rpa_varlink_destroy(pVarLinkRange);
		return (void*)0;
	}
	pVarLinkRange->var.userdata4 = (rpa_word_t)MATCH_CLASS_MATCHPTR;
	pVarLinkRange->var.finalize = rpa_var_class_destroy;
	rpa_list_addt(&hDbex->treehead, &pVarLinkRange->lnk);
	return pVarLinkRange;
}


/*
 * r[0] - name
 * r[1] - size
 * r[2] - low
 * r[3] - high
 */
static rpa_word_t rpa_vmcb_create_range_matchptr(rpa_vm_t *vm)
{
	return (rpa_word_t)rpa_dbex_create_range_matchptr(
		(rpa_dbex_handle)vm->userdata,
		(const char*) vm->r[0],
		(unsigned int) vm->r[1],
		(unsigned int) vm->r[2],
		(unsigned int) vm->r[3]);
}



rpa_varlink_t *rpa_dbex_create_funlast_matchptr(rpa_dbex_handle hDbex, const char *name, unsigned int size)
{
	rpa_varlink_t *pVarLinkMatch;

	pVarLinkMatch = rpa_varlink_create(RPA_VAR_PTR, "FUNLAST");
	if (!pVarLinkMatch)
		return (void*)0;
	if ((pVarLinkMatch->var.v.ptr = (void*) rpa_match_special_create_namesize(name, size, RPA_MATCHFUNC_LSTCHR)) == (void*)0) {
		rpa_varlink_destroy(pVarLinkMatch);
		return (void*)0;
	}
	pVarLinkMatch->var.userdata4 = (rpa_word_t)MATCH_CLASS_MATCHPTR;
	pVarLinkMatch->var.finalize = rpa_var_class_destroy;
	rpa_list_addt(&hDbex->treehead, &pVarLinkMatch->lnk);
	return pVarLinkMatch;
}


/*
 * r[0] - name
 * r[1] - size
 */
static rpa_word_t rpa_vmcb_create_funlast_matchptr(rpa_vm_t *vm)
{
	return (rpa_word_t)rpa_dbex_create_funlast_matchptr(
		(rpa_dbex_handle)vm->userdata,
		(const char*) vm->r[0],
		(unsigned int) vm->r[1]);	
}


rpa_varlink_t *rpa_dbex_create_funabort_matchptr(rpa_dbex_handle hDbex, const char *name, unsigned int size)
{
	rpa_varlink_t *pVarLinkMatch;

	pVarLinkMatch = rpa_varlink_create(RPA_VAR_PTR, "FUNLAST");
	if (!pVarLinkMatch)
		return (void*)0;
	if ((pVarLinkMatch->var.v.ptr = (void*) rpa_match_special_create_namesize(name, size, RPA_MATCHFUNC_ABORT)) == (void*)0) {
		rpa_varlink_destroy(pVarLinkMatch);
		return (void*)0;
	}
	pVarLinkMatch->var.userdata4 = (rpa_word_t)MATCH_CLASS_MATCHPTR;
	pVarLinkMatch->var.finalize = rpa_var_class_destroy;
	rpa_list_addt(&hDbex->treehead, &pVarLinkMatch->lnk);
	return pVarLinkMatch;
}


/*
 * r[0] - name
 * r[1] - size
 */
static rpa_word_t rpa_vmcb_create_funabort_matchptr(rpa_vm_t *vm)
{
	return (rpa_word_t)rpa_dbex_create_funabort_matchptr(
		(rpa_dbex_handle)vm->userdata,
		(const char*) vm->r[0],
		(unsigned int) vm->r[1]);	
}


rpa_varlink_t *rpa_dbex_create_funfail_matchptr(rpa_dbex_handle hDbex, const char *name, unsigned int size)
{
	rpa_varlink_t *pVarLinkMatch;

	pVarLinkMatch = rpa_varlink_create(RPA_VAR_PTR, "FUNFAIL");
	if (!pVarLinkMatch)
		return (void*)0;
	if ((pVarLinkMatch->var.v.ptr = (void*) rpa_match_special_create_namesize(name, size, RPA_MATCHFUNC_FAIL)) == (void*)0) {
		rpa_varlink_destroy(pVarLinkMatch);
		return (void*)0;
	}
	pVarLinkMatch->var.userdata4 = (rpa_word_t)MATCH_CLASS_MATCHPTR;
	pVarLinkMatch->var.finalize = rpa_var_class_destroy;
	rpa_list_addt(&hDbex->treehead, &pVarLinkMatch->lnk);
	return pVarLinkMatch;
}


/*
 * r[0] - name
 * r[1] - size
 */
static rpa_word_t rpa_vmcb_create_funfail_matchptr(rpa_vm_t *vm)
{
	return (rpa_word_t)rpa_dbex_create_funfail_matchptr(
		(rpa_dbex_handle)vm->userdata,
		(const char*) vm->r[0],
		(unsigned int) vm->r[1]);	
}


/*
 * r[0] - list (varlink)
 * r[1] - mnode (varlink)
 */
static rpa_word_t rpa_vmcb_add_mnode_to_list(rpa_vm_t *vm)
{
	rpa_varlink_t *pVarLinkList = (rpa_varlink_t *) vm->r[0];
	rpa_varlink_t *pVarLinkMnode = (rpa_varlink_t *) vm->r[1];
	rpa_match_list_t *list = (rpa_match_list_t *)pVarLinkList->var.v.ptr;
	rpa_mnode_t *mnode = (rpa_mnode_t *)pVarLinkMnode->var.v.ptr;
	rpa_list_addt(&list->head, &mnode->mlink);
	return 0;
}


void rpa_dbex_set_mnode_quantity(rpa_dbex_handle hDbex, rpa_mnode_t *mnode, unsigned int flags)
{
	mnode->flags |= flags;
}
