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


const char *rpa_class_to_str(rpa_class_t *cls)
{
	return "unk";
}


void rpa_class_init(rpa_class_t *cls, rpa_class_methods_t *vptr)
{
	cls->vptr = vptr;	
}


unsigned int rpa_class_getid(rpa_class_t *cls)
{
	return cls->vptr->rpa_class_getid(cls);	
}


const char *rpa_class_getstr(rpa_class_t *cls)
{
	return cls->vptr->rpa_class_getstr(cls);	
}


int rpa_class_dump(rpa_class_t *cls, char *buffer, unsigned int size)
{
	return cls->vptr->rpa_class_dump(cls, buffer, size);	
}


void rpa_class_destroy(rpa_class_t *cls)
{
	cls->vptr->rpa_class_destroy(cls);
}
