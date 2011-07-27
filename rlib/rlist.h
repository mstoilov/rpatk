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

#ifndef _RLIST_H_
#define _RLIST_H_

#include "rtypes.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct rlist_s rlist_t, rlink_t, rhead_t;

#define R_LIST_HEAD(__name__) {&(__name__), &(__name__) }
#define r_list(__name__) rlist_t __name__ = { &(__name__), &(__name__) }
#define r_list_empty(__head__) ((__head__)->next == ((void*)0) || (__head__)->next == __head__)
#define r_list_entry(__ptr__, __type__, __member__) ((__type__ *)((char *)(__ptr__)-(ruword)(&((__type__ *)0)->__member__)))
#define r_list_foreach(__pos__, __head__) for (__pos__ = (__head__)->next; __pos__ != (__head__); __pos__ = (__pos__)->next)
#define r_list_foreachreverse(__pos__, __head__) for (__pos__ = (__head__)->prev; __pos__ != (__head__); __pos__ = (__pos__)->prev)
#define r_list_fo_eachsafe(__pos__, __n__, __head__) \
		  for (__pos__ = (__head__)->next, __n__ = __pos__->next; __pos__ != (__head__); __pos__ = __n__, __n__ = __pos__->next)

struct rlist_s {
	rlist_t *next;
	rlist_t *prev;
};


void r_list_init(rlist_t *ptr);
void r_list_check(rlist_t *ptr);
void r_list_add(rlist_t *pNew, rlist_t *pPrev, rlist_t *pNext);
void r_list_addh(rlist_t *pHead, rlist_t *pNew);
void r_list_addt(rlist_t *pHead, rlist_t *pNew);
void r_list_unlink(rlist_t *pPrev, rlist_t *pNext);
void r_list_del(rlist_t *pEntry);
rlist_t *r_list_first(rlist_t *pHead);
rlist_t *r_list_last(rlist_t *pHead);
rlist_t *r_list_prev(rlist_t *pHead, rlist_t *pElem);
rlist_t *r_list_next(rlist_t *pHead, rlist_t *pElem);
void r_list_splice(rlist_t *pList, rlist_t *pHead);
int r_list_count(rlist_t *pHead);


#ifdef __cplusplus
}
#endif

#endif
