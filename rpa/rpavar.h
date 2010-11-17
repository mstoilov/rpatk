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

#if !defined(RPAVAR_H)
#define RPAVAR_H

#include "rmem.h"
#include "rpatypes.h"

#ifdef __cplusplus
extern "C" {
#endif


#define RPA_VAR_NULL 0
#define RPA_VAR_CHAR 1
#define RPA_VAR_UCHAR 2
#define RPA_VAR_SHORT 3
#define RPA_VAR_USHORT 4
#define RPA_VAR_INT 5
#define RPA_VAR_UINT 6
#define RPA_VAR_LONG 7
#define RPA_VAR_ULONG 8
#define RPA_VAR_FLOAT 9
#define RPA_VAR_DOUBLE 10
#define RPA_VAR_STR 11
#define RPA_VAR_PTR 12
#define RPA_VAR_LAST 64


typedef struct rpa_var_s rpa_var_t;
typedef void (*RPA_VAR_FINALIZE)(rpa_var_t *pVar);

struct rpa_var_s {
	unsigned char type;
	char name[4];
	char nameend[1];
//	rpa_word_t userdata1;
//	rpa_word_t userdata2;
//	rpa_word_t userdata3;
	rpa_word_t userdata4;
	RPA_VAR_FINALIZE finalize;
	union {
		char c;
		unsigned char uc;
		short s;
		unsigned short us;
		int i;
		unsigned int ui;
		long l;
		unsigned long ul;
		float f;
		double d;
		void *ptr;
		char *str;
	} v;
};

void rpa_var_init(rpa_var_t *var, unsigned char type, const char *name);
void rpa_var_init_namesize(rpa_var_t *var, unsigned char type, const char *name, unsigned int namesize);
rpa_var_t *rpa_var_create(unsigned char type, const char *name);
rpa_var_t *rpa_var_create_namesize(unsigned char type, const char *name, unsigned int namesize);
void rpa_var_finalize(rpa_var_t *pVar);
void rpa_var_destroy(rpa_var_t *pVar);
void rpa_var_finalize_ptr(rpa_var_t *pVar);
void rpa_var_class_destroy(rpa_var_t *pVar);

#ifdef __cplusplus
}
#endif

#endif

