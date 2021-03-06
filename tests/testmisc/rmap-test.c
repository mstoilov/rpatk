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

#include <stdio.h>
#include <stdlib.h>
#include "rlib/rmem.h"
#include "rlib/rmap.h"
#include "rlib/rstring.h"


long test_rmap_add(rmap_t *map, const char *name, long val)
{
	return r_map_add_s(map, name, &val);
}


void test_rmap_print(rmap_t * map, unsigned long index)
{
	if (r_map_value(map, index))
		fprintf(stdout, "(Index: %3ld) Key: %s, Value: %ld\n", index, R_STRING2ANSI(r_map_key(map, index)), *((long*)r_map_value(map, index)));
	else
		fprintf(stdout, "Invalid Index: %ld\n", index);
}


void test_rmap_lookup(rmap_t * map, const char *name)
{
	long index = -1;

	do {
		index = r_map_lookup_s(map, index, name);
		if (index >= 0)
			fprintf(stdout, "(Lookup index: %3ld) Key: %s, Value: %ld\n", index, R_STRING2ANSI(r_map_key(map, index)), *((long*)r_map_value(map, index)));
	} while (index >= 0);
}


void test_rmap_taillookup(rmap_t * map, const char *name)
{
	long index = -1;

	do {
		index = r_map_taillookup_s(map, index, name);
		if (index >= 0)
			fprintf(stdout, "(Tail Lookup index: %3ld) Key: %s, Value: %ld\n", index, R_STRING2ANSI(r_map_key(map, index)), *((long*)r_map_value(map, index)));
	} while (index >= 0);

}


int main(int argc, char *argv[])
{
	long l, val = 17;
	rmap_t *map = r_map_create(sizeof(long), 7);

	test_rmap_add(map, "one", 1);
	test_rmap_add(map, "two", 2);
	test_rmap_add(map, "three", 3);
	r_map_delete(map, 1);
	r_map_delete(map, 1);
	test_rmap_add(map, "four", 4);
	test_rmap_add(map, "four", 5);
	test_rmap_add(map, "four", 9);

	test_rmap_print(map, 0);
	test_rmap_print(map, 1);
	test_rmap_print(map, 2);
	test_rmap_print(map, 3);

	test_rmap_lookup(map, "four");
	r_map_setvalue(map, r_map_lookup_s(map, -1, "four"), &val);
	test_rmap_taillookup(map, "four");

	for (l = r_map_first(map); l >= 0; l = r_map_next(map, l)) {
		test_rmap_print(map, l);
	}

	test_rmap_add(map, "three", 13);
	test_rmap_add(map, NULL, 23);
	test_rmap_add(map, "four", 44);
	test_rmap_add(map, "three", 33);

	for (l = r_map_first(map); l >= 0; l = r_map_next(map, l)) {
		test_rmap_print(map, l);
	}

	r_map_destroy(map);

	fprintf(stdout, "Max alloc mem: %ld\n", r_debug_get_maxmem());
	fprintf(stdout, "Leaked mem: %ld\n", r_debug_get_allocmem());
	return 0;
}
