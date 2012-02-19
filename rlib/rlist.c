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

#include "rlib/rlist.h"


void r_list_init(rlist_t *ptr)
{
	ptr->next = ptr;
	ptr->prev = ptr;
}


void r_list_add(rlist_t *pNew, rlist_t *pPrev, rlist_t *pNext)
{
	pNext->prev = pNew;
	pNew->next = pNext;
	pNew->prev = pPrev;
	pPrev->next = pNew;
}


void r_list_addh(rlist_t *pHead, rlist_t *pNew)
{
	r_list_add(pNew, pHead, (pHead)->next);
}


void r_list_addt(rlist_t *pHead, rlist_t *pNew)
{
	r_list_add(pNew, (pHead)->prev, pHead);
}


void r_list_unlink(rlist_t *pPrev, rlist_t *pNext)
{
	pNext->prev = pPrev;
	pPrev->next = pNext;
}


void r_list_del(rlist_t *pEntry)
{
	r_list_unlink((pEntry)->prev, (pEntry)->next);
}


rlist_t *r_list_first(rlist_t *pHead)
{
	return (((pHead)->next != (pHead)) ? (pHead)->next: ((void*)0));
}


rlist_t *r_list_last(rlist_t *pHead)
{
	return (((pHead)->prev != (pHead)) ? (pHead)->prev: ((void*)0));
}


rlist_t *r_list_prev(rlist_t *pHead, rlist_t *pElem)
{
	return (pElem && pElem->prev != pHead) ? (pElem)->prev: ((void*)0);
}


rlist_t *r_list_next(rlist_t *pHead, rlist_t *pElem)
{
	return (pElem && pElem->next != pHead) ? pElem->next: ((void*)0);
}


void r_list_splice(rlist_t *pList, rlist_t *pHead)
{
	rlist_t *pFirst;

	pFirst = pList->next;
	if (pFirst != pList) {
		rlist_t *pLast = pList->prev;
		rlist_t *pAt = pHead->next;
		pFirst->prev = pHead;
		pHead->next = pFirst;
		pLast->next = pAt;
		pAt->prev = pLast;
	}
}


int r_list_count(rlist_t *pHead)
{
	rlist_t *pCur;
	int n = 0;

	r_list_foreach(pCur, pHead)
		n++;
	return n;
}
