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

#include "rpastring.h"
#include "rpamem.h"
#include "rpavar.h"
#include "rpaclass.h"


void rpa_var_init_namesize(rpa_var_t *var, unsigned char type, const char *name, unsigned int namesize)
{
	int size = (sizeof(var->name) - 1) < namesize ? (sizeof(var->name) - 1) : namesize;
	rpa_memset(var, 0, sizeof(*var));
	var->type = type;
	if (name)
		rpa_strncpy(var->name, name, size);
}


void rpa_var_init(rpa_var_t *var, unsigned char type, const char *name)
{
	rpa_var_init_namesize(var, type, name, (unsigned int)-1);
}


rpa_var_t *rpa_var_create(unsigned char type, const char *name)
{
	return rpa_var_create_namesize(type, name, (unsigned int)-1);
}


rpa_var_t *rpa_var_create_namesize(unsigned char type, const char *name, unsigned int namesize)
{
	rpa_var_t *pVar;

	pVar = (rpa_var_t*)rpa_malloc(sizeof(*pVar));
	if (!pVar)
		return ((void*)0);
	rpa_var_init_namesize(pVar, type, name, namesize);
	return pVar;
}


void rpa_var_finalize_ptr(rpa_var_t *pVar)
{
	rpa_free(pVar->v.ptr);
}


void rpa_var_finalize(rpa_var_t *pVar)
{
	if (pVar && pVar->finalize)
		pVar->finalize(pVar);
}


void rpa_var_destroy(rpa_var_t *pVar)
{
	rpa_var_finalize(pVar);
	rpa_free((void*)pVar);
}


void rpa_var_class_destroy(rpa_var_t *pVar)
{
	rpa_class_destroy((rpa_class_t*)pVar->v.ptr);
}


