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
#include "rex/rexdfa.h"
#include "rexgrep.h"
#include "rexgrepdep.h"


int usage(int argc, const char *argv[])
{
		fprintf(stderr, "REX Code Compiler - using library version: %s \n", rex_db_version());
		fprintf(stderr, "Copyright (C) 2012 Martin Stoilov\n\n");

		fprintf(stderr, "Usage: \n %s [OPTIONS] <filename>\n", argv[0]);
		fprintf(stderr, " OPTIONS:\n");
		fprintf(stderr, "\t-e patterns              Regular Expression.\n");
		fprintf(stderr, "\t-f patternfile           Read Regular Expressions from a file.\n");
		fprintf(stderr, "\t-D                       Dump states.\n");
		fprintf(stderr, "\t-t                       Display time elapsed.\n");
		fprintf(stderr, "\t-v                       Display version information.\n");
		fprintf(stderr, "\t-h, --help               Display this help.\n");
		
		return 0;
}


int rexcc_buffer_realloc(rbuffer_t *buffer, unsigned long size)
{
	char *s;

	s = (char *)r_realloc(buffer->s, size);
	if (!s)
		return -1;
	buffer->s = s;
	buffer->size = size;
	return 0;

}


rbuffer_t *rexcc_buffer_loadfile(FILE *pFile)
{
	unsigned long memchunk = 256;
	long ret = 0, inputsize = 0;
	rbuffer_t *buf;

	buf = r_buffer_create(2 * memchunk);
	if (!buf)
		return (void*)0;

	do {
		if ((buf->size - inputsize) < memchunk) {
			if (rexcc_buffer_realloc(buf, buf->size + memchunk) < 0) {
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
	int i, ret = 0;
	rexcc_t *pCC;
	rarray_t *buffers;
	FILE *devnull = NULL;

	buffers = r_array_create(sizeof(rbuffer_t *));
	pCC = rex_cc_create();
	if (argc <= 1) {
		usage(argc, argv);
		goto end;
	}

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-t") == 0) {
			pCC->showtime = 1;
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
					ret = rex_cc_load_pattern(pCC, pattern);
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
				ret = rex_cc_load_string_pattern(pCC, &pattern);
				if (ret < 0)
					goto error;
			}
			
		}
	}

	if (pCC->usedfa) {
		rexdb_t *dfadb = rex_db_createdfa(pCC->nfa, pCC->startuid);
		pCC->dfa = rex_dfa_create_from_db(dfadb);
		rex_db_destroy(dfadb);
	}

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-D") == 0) {
			int j;
			if (pCC->dfa) {
				for (j = 0; j < pCC->dfa->nstates; j++) {
					rex_dfa_dumpstate(pCC->dfa, j);
				}
			} else if (pCC->nfa) {
				rexdb_t *db = pCC->nfa;
				for (j = 0; j < r_array_length(db->states); j++) {
					rex_db_dumpstate(db, j);
				}
			}
			goto end;
		}
	}


end:
	rex_cc_destroy(pCC);
	if (pCC->showtime) {
		fprintf(stdout, "memory: %ld KB (leaked %ld Bytes)\n", (long)r_debug_get_maxmem()/1024, (long)r_debug_get_allocmem());
	}

	if (devnull)
		fclose(devnull);
	return ret;

error:
	if (devnull)
		fclose(devnull);
	rex_cc_destroy(pCC);
	return 2;
}
