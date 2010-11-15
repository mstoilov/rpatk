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

#include "rpalist.h"


void rpa_list_init(rpa_list_t *ptr)
{
	ptr->pNext = ptr; 
	ptr->pPrev = ptr;
}


void rpa_list_add(rpa_list_t *pNew, rpa_list_t *pPrev, rpa_list_t *pNext)
{
	pNext->pPrev = pNew;
	pNew->pNext = pNext;
	pNew->pPrev = pPrev;
	pPrev->pNext = pNew;
}


void rpa_list_addh(rpa_list_t *pHead, rpa_list_t *pNew)
{
	rpa_list_add(pNew, pHead, (pHead)->pNext);
}


void rpa_list_addt(rpa_list_t *pHead, rpa_list_t *pNew)
{
	rpa_list_add(pNew, (pHead)->pPrev, pHead);
}


void rpa_list_unlink(rpa_list_t *pPrev, rpa_list_t *pNext)
{
	pNext->pPrev = pPrev;
	pPrev->pNext = pNext;
}


void rpa_list_del(rpa_list_t *pEntry) 
{
	rpa_list_unlink((pEntry)->pPrev, (pEntry)->pNext);
}


rpa_list_t *rpa_list_first(rpa_list_t *pHead) 
{
	return (((pHead)->pNext != (pHead)) ? (pHead)->pNext: ((void*)0));
}


rpa_list_t *rpa_list_last(rpa_list_t *pHead)
{
	return (((pHead)->pPrev != (pHead)) ? (pHead)->pPrev: ((void*)0));
}


rpa_list_t *rpa_list_prev(rpa_list_t *pHead, rpa_list_t *pElem)
{
	return (pElem && pElem->pPrev != pHead) ? (pElem)->pPrev: ((void*)0);
}


rpa_list_t *rpa_list_next(rpa_list_t *pHead, rpa_list_t *pElem)
{
	return (pElem && pElem->pNext != pHead) ? pElem->pNext: ((void*)0);
}


void rpa_list_splice(rpa_list_t *pList, rpa_list_t *pHead)
{
	rpa_list_t *pFirst;

	pFirst = pList->pNext;
	if (pFirst != pList) {
		rpa_list_t *pLast = pList->pPrev;
		rpa_list_t *pAt = pHead->pNext;
		pFirst->pPrev = pHead;
		pHead->pNext = pFirst;
		pLast->pNext = pAt;
		pAt->pPrev = pLast;
	}
}


int rpa_list_count(rpa_list_t *pHead)
{
	rpa_list_t *pCur;
	int n = 0;

	rpa_list_for_each(pCur, pHead)
		n++;
	return n;
}
