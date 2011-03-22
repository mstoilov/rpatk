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
#include "rpaparseinfo.h"


typedef struct rpa_buffer_s {
	char *s;
	unsigned long size;
	void *userdata;
	void *userdata1;
} rpa_buffer_t;


int main(int argc, char *argv[])
{
	rint ret = 0, i;
	rpa_parser_t *pa = rpa_parser_create();

	if (!pa)
		goto error;
	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-e") == 0) {
			if (++i < argc) {
				rpa_buffer_t pattern;
				pattern.s = (char*)argv[i];
				pattern.size = r_strlen(argv[i]) + 1;
				ret = rpa_parser_load_s(pa, pattern.s);
				if (ret < 0)
					goto error;
			}

		}
	}

	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-r") == 0) {
			rpa_parseinfo_t *pi = rpa_parseinfo_create(pa->stat);
			rpa_parseinfo_dump_records(pi);
			rpa_parseinfo_destroy(pi);
		}
	}


	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-i") == 0) {
			rpa_parseinfo_t *pi = rpa_parseinfo_create(pa->stat);
			rpa_parseinfo_dump(pi);
			rpa_parseinfo_destroy(pi);
		}
	}


	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-d") == 0) {
			if (++i < argc) {
				rpa_parseinfo_t *pi = rpa_parseinfo_create(pa->stat);
				rpa_parseinfo_dump_ruletree(pi, argv[i]);
				rpa_parseinfo_destroy(pi);
			}
		}
	}


	rpa_parser_destroy(pa);
	r_printf("Parsed Size = %d\n", ret);
	r_printf("Cache hit = %d\n", pa->stat->cache.hit);
	r_printf("Max alloc mem: %ld\n", r_debug_get_maxmem());
	r_printf("Leaked mem: %ld\n", r_debug_get_allocmem());
	return 0;

error:
	r_printf("ERROR: ret = %d\n", ret);

	rpa_parser_destroy(pa);
	return 1;
}
