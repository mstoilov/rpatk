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

/*
 * Initialize ptr to point itself
 */
void r_list_init(rlist_t *ptr)
{
	ptr->next = ptr;
	ptr->prev = ptr;
}

/*
 * insert pNew between pPrev and pNext
 */
void r_list_add(rlist_t *pnew, rlist_t *pprev, rlist_t *pnext)
{
	pnext->prev = pnew;
	pnew->next = pnext;
	pnew->prev = pprev;
	pprev->next = pnew;
}

/*
 * Insert in the beginning (head) of the list
 */
void r_list_addh(rlist_t *phead, rlist_t *pnew)
{
	r_list_add(pnew, phead, (phead)->next);
}

/*
 * Insert at the end (tail) of the list
 */
void r_list_addt(rlist_t *phead, rlist_t *pnew)
{
	r_list_add(pnew, (phead)->prev, phead);
}

/*
 * Unlink the node between pprev and pnext
 */
void r_list_unlink(rlist_t *pprev, rlist_t *pnext)
{
	pnext->prev = pprev;
	pprev->next = pnext;
}

/*
 * Unlink from the list
 */
void r_list_del(rlist_t *pentry)
{
	r_list_unlink((pentry)->prev, (pentry)->next);
}

/*
 * Return the first node if the list is not empty,
 * otherwise return NULL;
 */
rlist_t *r_list_first(rlist_t *pHead)
{
	return (((pHead)->next != (pHead)) ? (pHead)->next : NULL);
}

/*
 * Return the last node if the list is not empty,
 * otherwise return NULL;
 */
rlist_t *r_list_last(rlist_t *pHead)
{
	return (((pHead)->prev != (pHead)) ? (pHead)->prev : NULL);
}

/*
 * Return the previous node, if there is one.
 * Otherwise return NULL
 */
rlist_t *r_list_prev(rlist_t *phead, rlist_t *pelem)
{
	return (pelem && pelem->prev != phead) ? (pelem)->prev : NULL;
}

/*
 * Return the next node, if there is one.
 * Otherwise return NULL
 */
rlist_t *r_list_next(rlist_t *phead, rlist_t *pelem)
{
	return (pelem && pelem->next != phead) ? pelem->next : NULL;
}

/*
 * Insert plist elements between phead and phead->next
 */
void r_list_splice(rlist_t *plist, rlist_t *phead)
{
	rlist_t *first;

	first = plist->next;
	if (first != plist) {
		rlist_t *last = plist->prev;
		rlist_t *at = phead->next;
		first->prev = phead;
		phead->next = first;
		last->next = at;
		at->prev = last;
		r_list_init(plist);
	}
}

/*
 * Return the number of nodes in the list
 */
ruint32 r_list_count(rlist_t *phead)
{
	rlist_t *pcur;
	ruint32 n = 0;

	r_list_foreach(pcur, phead)
		n++;
	return n;
}
