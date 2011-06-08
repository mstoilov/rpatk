#include <stdio.h>
#include <stdlib.h>
#include "rmem.h"
#include "rmap.h"
#include "rstring.h"


rlong test_rmap_add(rmap_t *map, const rchar *name, rlong val)
{
	return r_map_add_s(map, name, &val);
}


void test_rmap_print(rmap_t * map, rsize_t index)
{
	if (r_map_value(map, index))
		fprintf(stdout, "(Index: %3ld) Key: %s, Value: %ld\n", index, r_map_key(map, index), *((rlong*)r_map_value(map, index)));
	else
		fprintf(stdout, "Invalid Index: %ld\n", index);
}


int main(int argc, char *argv[])
{
	rmap_t *map = r_map_create(sizeof(rlong), 7);

	test_rmap_add(map, "one", 1);
	test_rmap_add(map, "two", 2);
	test_rmap_add(map, "three", 3);
	r_map_delete(map, 1);
	r_map_delete(map, 1);
	test_rmap_add(map, "four", 4);

	test_rmap_print(map, 0);
	test_rmap_print(map, 1);
	test_rmap_print(map, 2);
	test_rmap_print(map, 3);

	r_map_destroy(map);

	fprintf(stdout, "Max alloc mem: %ld\n", r_debug_get_maxmem());
	fprintf(stdout, "Leaked mem: %ld\n", r_debug_get_allocmem());
	return 0;
}
