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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <time.h>
#include "rlib/rmem.h"
#include "rlib/rarray.h"
#include "rex/rexdfaconv.h"
#include "rexgrep.h"
#include "rexgrepdep.h"


int usage(int argc, const char *argv[])
{
		fprintf(stderr, "REX Grep - using library version: %s \n", rex_db_version());
		fprintf(stderr, "Copyright (C) 2012 Martin Stoilov\n\n");

		fprintf(stderr, "Usage: \n %s [OPTIONS] <filename>\n", argv[0]);
		fprintf(stderr, " OPTIONS:\n");
		fprintf(stderr, "\t-e patterns              Regular Expression.\n");
		fprintf(stderr, "\t-f patternfile           Read Regular Expressions from a file.\n");
		fprintf(stderr, "\t-o, --only-matching      Show only the part of a line matching PATTERN\n");
		fprintf(stderr, "\t-l                       Line mode.\n");
		fprintf(stderr, "\t-N                       Use NFA.\n");
		fprintf(stderr, "\t-q                       Quiet mode.\n");
		fprintf(stderr, "\t-t                       Display time elapsed.\n");
		fprintf(stderr, "\t-s string                Scan string.\n");
		fprintf(stderr, "\t-v                       Display version information.\n");
		fprintf(stderr, "\t-h, --help               Display this help.\n");
		
		return 0;
}


int grep_buffer_realloc(rbuffer_t *buffer, unsigned long size)
{
	char *s;

	s = (char *)r_realloc(buffer->s, size);
	if (!s)
		return -1;
	buffer->s = s;
	buffer->size = size;
	return 0;

}


rbuffer_t *grep_buffer_loadfile(FILE *pFile)
{
	unsigned long memchunk = 256;
	long ret = 0, inputsize = 0;
	rbuffer_t *buf;

	buf = r_buffer_create(2 * memchunk);
	if (!buf)
		return (void*)0;

	do {
		if ((buf->size - inputsize) < memchunk) {
			if (grep_buffer_realloc(buf, buf->size + memchunk) < 0) {
				fprintf(stderr, "Out of memory!\n");
				exit(1);
			}
		}
		ret = (long)fread(&buf->s[inputsize], 1, memchunk - 1, pFile);
		if ((ret <= 0) && ferror(pFile)) {
			r_buffer_destroy(buf);
			return (void*)0;
		}
		inputsize += ret;
		buf->s[inputsize] = '\0';
		buf->size = inputsize;
	} while (!feof(pFile));

	return buf;
}


int main(int argc, const char *argv[])
{
	int ret, scanned = 0, i;
	rexgrep_t *pGrep;
	rarray_t *buffers;
	FILE *devnull = NULL;

	buffers = r_array_create(sizeof(rbuffer_t *));
	pGrep = rex_grep_create();
	pGrep->greptype = REX_GREPTYPE_SCANLINES;
	pGrep->usedfa = 1;
	if (argc <= 1) {
		usage(argc, argv);
		goto end;
	}

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-t") == 0) {
			pGrep->showtime = 1;
		}
	}

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-H") == 0 || strcmp(argv[i], "--with-filename") == 0) {
			pGrep->showfilename = 1;
		}
	}

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "/?") == 0 || strcmp(argv[i], "-h") == 0) {
			usage(argc, argv);
			goto end;
		}
	}

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-v") == 0) {
			fprintf(stderr, "REX Grep with REX Engine: %s\n", rex_db_version());
			goto end;
		}
	}

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-f") == 0) {
			if (++i < argc) {
				rbuffer_t *pattern = rex_buffer_map_file(argv[i]);
				if (pattern) {
					ret = rex_grep_load_pattern(pGrep, pattern);
					r_array_add(buffers, &pattern);
				} else {
					ret = -1;
				}
				if (ret < 0)
					goto error;
			}
		}
	}

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-e") == 0) {
			if (++i < argc) {
				rbuffer_t pattern;
				pattern.s = (char*)argv[i];
				pattern.size = strlen(argv[i]);
				ret = rex_grep_load_string_pattern(pGrep, &pattern);
				if (ret < 0)
					goto error;
			}
			
		}
	}

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-l") == 0) {
			pGrep->greptype = REX_GREPTYPE_SCANLINES;
		} else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--only-matching") == 0) {
			pGrep->greptype = REX_GREPTYPE_MATCH;
		} else if (strcmp(argv[i], "-q") == 0) {
			devnull = fopen("/dev/null", "w");
			stdout = devnull;
		}
	}

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-s") == 0) {
			if (++i < argc) {
				rbuffer_t buf;
				buf.s = (char*)argv[i];
				buf.size = r_strlen(argv[i]);
				rex_grep_scan_buffer(pGrep, &buf);
				++scanned;
			}
		}
	}

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-N") == 0) {
			pGrep->usedfa = 0;
		}
	}

	if (pGrep->usedfa) {
		rexdfaconv_t *conv = rex_dfaconv_create();
		pGrep->dfa = rex_dfaconv_run(conv, pGrep->nfa, pGrep->startuid);
		rex_dfaconv_destroy(conv);
	}

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-D") == 0) {
			int j;
			rexdb_t *db = pGrep->usedfa ? pGrep->dfa : pGrep->nfa;
			for (j = 0; j < r_array_length(db->states); j++) {
				rex_db_dumpstate(db, j);
			}
			goto end;
		}
	}

	/* scan files */
	for (i = 1; i < argc; i++) {
		if (argv[i][0] != '-') {
			++scanned;
			rex_grep_scan_path(pGrep, argv[i]);
		} else if (argv[i][1] == 'e' || argv[i][1] == 'f' || argv[i][1] == 'c' || argv[i][1] == 'C'){
			++i;
		}
		
	}

	if (!scanned) {
		rbuffer_t *buf = grep_buffer_loadfile(stdin);
		if (buf) {
			rex_grep_scan_buffer(pGrep, buf);
			r_buffer_destroy(buf);
		}
	}

end:
	for (i = 0; i < r_array_length(buffers); i++) {
		r_buffer_destroy(r_array_index(buffers, i, rbuffer_t*));
	}
	r_object_destroy((robject_t*)buffers);
	ret = pGrep->ret;
	rex_grep_destroy(pGrep);
	if (pGrep->showtime) {
		fprintf(stdout, "memory: %ld KB (leaked %ld Bytes)\n", (long)r_debug_get_maxmem()/1024, (long)r_debug_get_allocmem());
	}

	if (devnull)
		fclose(devnull);
	return ret;

error:
	if (devnull)
		fclose(devnull);
	rex_grep_destroy(pGrep);
	return 2;
}
