#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "rmem.h"
#include "rpaparser.h"
#include "rpadbex.h"


typedef struct rpa_buffer_s {
	char *s;
	unsigned long size;
	void *userdata;
	void *userdata1;
} rpa_buffer_t;


int main(int argc, char *argv[])
{
	rint ret = 0, i;
	rpadbex_t *dbex = rpa_dbex_create();

//	rpa_parser_t *pa = rpa_parser_create();

	if (!dbex)
		goto error;
	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-e") == 0) {
			if (++i < argc) {
				rpa_buffer_t pattern;
				pattern.s = (char*)argv[i];
				pattern.size = r_strlen(argv[i]) + 1;
				rpa_dbex_open(dbex);
				ret = rpa_dbex_load_s(dbex, pattern.s);
				rpa_dbex_close(dbex);
				if (ret < 0)
					goto error;
			}
		}
	}

	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-r") == 0) {
			rpa_dbex_dumprecords(dbex);
		}
	}

	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-i") == 0) {
			rpa_dbex_dumprules(dbex);
		}
	}

	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-I") == 0) {
			rpa_dbex_dumpinfo(dbex);
		}
	}

	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-d") == 0) {
			if (++i < argc) {
				rpa_dbex_dumptree_s(dbex, argv[i], 0);
			}
		}
	}

	rpa_dbex_destroy(dbex);
	r_printf("Parsed Size = %d\n", ret);
	r_printf("Max alloc mem: %ld\n", r_debug_get_maxmem());
	r_printf("Leaked mem: %ld\n", r_debug_get_allocmem());
	return 0;

error:
	r_printf("ERROR: ret = %d\n", ret);

	rpa_dbex_destroy(dbex);
	return 1;
}
