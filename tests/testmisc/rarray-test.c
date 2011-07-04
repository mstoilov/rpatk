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

#include <stdio.h>
#include <stdlib.h>
#include "rlib/rarray.h"
#include "rlib/rmem.h"


static void r_array_addint(rarray_t *array, int elt)
{
	r_array_add(array, &elt);
}


int main(int argc, char *argv[])
{
	int i, j;
	rarray_t *a;

	a = r_array_create(sizeof(int));
	r_array_push(a, 20, int);
	r_array_push(a, 21, int);
	r_array_push(a, 22, int);
	r_array_push(a, 23, int);
	r_array_push(a, 24, int);

	for (i = 0; i < 5; i++) {
		fprintf(stdout, "r_array_pop %d: %d\n", i, r_array_pop(a, int));
	}


	for (i = 0; i < 10; i++) {
		r_array_addint(a, i);
	}
	i = 100;
	r_array_insert(a, 5, &i);
	r_array_insert(a, 4, &i);
	r_array_insert(a, 20, &i);
	r_array_replace(a, 19, &i);
	i = 99;
	r_array_replace(a, 19, &i);



	for (i = 0; i < r_array_length(a); i++) {
		fprintf(stdout, "index %d: %d\n", i, *((int*)r_array_slot(a, i)));
	}
	fprintf(stdout, "\n");

	r_array_delete(a, 0);
	fprintf(stdout, "r_array_delete %d\n", 0);
	for (i = 0; i < r_array_length(a); i++) {
		fprintf(stdout, "index %d: %d\n", i, *((int*)r_array_slot(a, i)));
	}
	fprintf(stdout, "\n");

	r_array_delete(a, 20);
	fprintf(stdout, "r_array_delete %d\n", 20);
	for (i = 0; i < r_array_length(a); i++) {
		fprintf(stdout, "index %d: %d\n", i, *((int*)r_array_slot(a, i)));
	}
	fprintf(stdout, "\n");

	r_array_delete(a, 19);
	fprintf(stdout, "r_array_delete %d\n", 19);
	for (i = 0; i < r_array_length(a); i++) {
		fprintf(stdout, "index %d: %d\n", i, *((int*)r_array_slot(a, i)));
	}
	fprintf(stdout, "\n");

	for (j = 0; j < 21; j++) {
		r_array_delete(a, 0);
		fprintf(stdout, "r_array_delete %d\n", 0);
		for (i = 0; i < r_array_length(a); i++) {
			fprintf(stdout, "index %d: %d\n", i, *((int*)r_array_slot(a, i)));
		}
		fprintf(stdout, "\n");
	}


	r_object_destroy((robject_t*)a);

	fprintf(stdout, "Max alloc mem: %ld\n", r_debug_get_maxmem());
	fprintf(stdout, "Leaked mem: %ld\n", r_debug_get_allocmem());
	return 0;
}
