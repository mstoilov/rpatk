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

#ifndef _RPACLASS_H_
#define _RPACLASS_H_


#ifdef __cplusplus
extern "C" {
#endif


#define RPA_MATCH_CLASSID				(1<<0)
#define RPA_MATCH_SPECIAL_CLASSID		(1<<1)
#define RPA_MATCH_VAL_CLASSID			(1<<2)
#define RPA_MATCH_RANGE_CLASSID			(1<<3)
#define RPA_MATCH_RANGELIST_CLASSID		(1<<4)
#define RPA_MATCH_STR_CLASSID			(1<<5)
#define RPA_MATCH_LIST_CLASSID			(1<<6)
#define RPA_MATCH_NLIST_CLASSID			(1<<7)
#define RPA_MNODE_CLASSID				(1<<8)
#define RPA_HASHENTRY_CLASSID			(1<<9)
#define RPA_RESERVED1_CLASSID			(1<<24)
#define RPA_RESERVED2_CLASSID			(1<<25)
#define RPA_RESERVED3_CLASSID			(1<<26)
#define RPA_RESERVED4_CLASSID			(1<<27)
#define RPA_RESERVED5_CLASSID			(1<<28)
#define RPA_RESERVED6_CLASSID			(1<<29)
#define RPA_RESERVED7_CLASSID			(1<<30)
#define RPA_RESERVED8_CLASSID			(1<<31)


typedef struct rpa_class_s rpa_class_t;

typedef struct rpa_class_methods_s {
	unsigned int (*rpa_class_getid)(rpa_class_t *cls);
	const char* (*rpa_class_getstr)(rpa_class_t *cls);
	int (*rpa_class_dump)(rpa_class_t *cls, char *buffer, unsigned int size);
	void (*rpa_class_destroy)(rpa_class_t *cls);	
} rpa_class_methods_t;


struct rpa_class_s {
	rpa_class_methods_t *vptr;
};


void rpa_class_init(rpa_class_t *cls, rpa_class_methods_t *vptr);
unsigned int rpa_class_getid(rpa_class_t *cls);
const char *rpa_class_getstr(rpa_class_t *cls);
int rpa_class_dump(rpa_class_t *cls, char *buffer, unsigned int size);
void rpa_class_destroy(rpa_class_t *cls);


const char *rpa_class_to_str(rpa_class_t *cls);


#ifdef __cplusplus
}
#endif

#endif
