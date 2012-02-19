/*
 *  Regular Pattern Analyzer Toolkit (RPA/Tk)
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
#include "rlib/rhash.h"
#include "rlib/rmem.h"


int main(int argc, char *argv[])
{
	rhash_t *h;
	rhash_node_t *node = NULL;
	unsigned int idig[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	char *sdig[] = {"zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"};

	h = r_hash_create(5, r_hash_strequal, r_hash_strhash);
	r_hash_insert(h, sdig[0], &idig[0]);
	r_hash_insert(h, sdig[7], &idig[8]);
	r_hash_insert(h, sdig[7], &idig[7]);
	r_hash_insert(h, sdig[5], &idig[5]);
	fprintf(stdout, "key: %s, value: %d\n", "seven", *((unsigned int*)r_hash_lookup(h, "seven")));
	r_hash_remove(h, sdig[7]);
	if (!r_hash_lookup(h, "seven")) {
		r_hash_insert(h, sdig[7], &idig[7]);
		r_hash_insert(h, sdig[7], &idig[8]);
		r_hash_insert(h, sdig[7], &idig[9]);
	}
	for (node = r_hash_nodelookup(h, node, "seven"); node; node = r_hash_nodelookup(h, node, "seven"))
		fprintf(stdout, "key: %s, value: %d\n", "seven", *((unsigned int*)r_hash_value(node)));
	fprintf(stdout, "key: %s, value: %d\n", sdig[5], *((unsigned int*)r_hash_lookup(h, sdig[5])));
	fprintf(stdout, "key: %s, value: %d\n", sdig[0], *((unsigned int*)r_hash_lookup(h, sdig[0])));

	r_object_destroy((robject_t*)h);

	fprintf(stdout, "Max alloc mem: %ld\n", r_debug_get_maxmem());
	fprintf(stdout, "Leaked mem: %ld\n", r_debug_get_allocmem());
	return 0;
}
