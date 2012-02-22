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
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "rlib/rmem.h"
#include "rlib/rarray.h"
#include "rex/rexdfaconv.h"
#include "rex/rexdfa.h"
#include "rexccdep.h"
#include "rexcc.h"


int usage(int argc, const char *argv[])
{
		fprintf(stderr, "REX Code Compiler - using REX library version: %s \n", rex_db_version());
		fprintf(stderr, "Copyright (C) 2012 Martin Stoilov\n\n");

		fprintf(stderr, "Usage: \n %s [OPTIONS] <filename>\n", argv[0]);
		fprintf(stderr, " OPTIONS:\n");
		fprintf(stderr, "\t-o <cfile>               Output .c file.\n");
		fprintf(stderr, "\t-d                       Dump regular expressions.\n");
		fprintf(stderr, "\t-D                       Dump DFA states.\n");
		fprintf(stderr, "\t-N                       Dump NFA states.\n");
		fprintf(stderr, "\t-s                       Include substates.\n");
		fprintf(stderr, "\t-t                       Display statistics.\n");
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
	rexcc_t *pCC;
	int i, ret = 0, dumponly = 0;
	int withsubstates = 0;
	FILE *devnull = NULL;
	rexdb_t *tempdb = NULL;
	FILE *cfile = stdout;
	FILE *hfile = NULL;

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
			fprintf(stderr, "REXCC with REX library version: %s\n", rex_db_version());
			goto end;
		}
	}
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-o") == 0) {
			if (++i < argc) {
				cfile = fopen(argv[i], "wb");
				if (!cfile) {
					fprintf(stderr, "Failed to create file: %s\n", argv[i]);
					goto error;
				}

			}
		}
	}
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-s") == 0) {
			withsubstates = 1;
		}
	}
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-D") == 0) {
			dumponly = 1;
		}
	}
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-N") == 0) {
			dumponly = 2;
		}
	}
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-d") == 0) {
			dumponly = 3;
		}
	}
	for (i = 1; i < argc; i++) {
		if (argv[i][0] != '-') {
			rbuffer_t *text = rex_buffer_map_file(argv[i]);
			if (text) {
				if (rex_cc_load_buffer(pCC, text) < 0) {
					/*
					 * Error
					 */
				}
				if (pCC->startuid >= 0) {
					tempdb = rex_db_createdfa(pCC->nfa, pCC->startuid);
					pCC->dfa = rex_db_todfa(tempdb, withsubstates);
					rex_db_destroy(tempdb);
					if (pCC->dfa && !dumponly)
						rex_cc_output(pCC, cfile);
					if (dumponly == 3) {
						rex_cc_parseinfodump(pCC);
					}
				}
				r_buffer_destroy(text);
			} else {
				/*
				 * Error
				 */
			}
			break;
		} else if (argv[i][1] == 'o'){
			++i;
		}
	}
	if (dumponly == 1) {
		for (i = 0; i < pCC->dfa->nstates; i++) {
			rex_dfa_dumpstate(pCC->dfa, i);
		}
		goto end;
	} else if (dumponly == 2) {
		rexdb_t *db = pCC->nfa;
		for (i = 0; i < r_array_length(db->states); i++) {
			rex_db_dumpstate(db, i);
		}
		goto end;
	}

end:
	rex_cc_destroy(pCC);
	if (pCC->showtime) {
		fprintf(stdout, "memory: %ld KB (leaked %ld Bytes)\n", (long)r_debug_get_maxmem()/1024, (long)r_debug_get_allocmem());
	}
	if (devnull)
		fclose(devnull);
	if (cfile)
		fclose(cfile);
	if (hfile)
		fclose(hfile);
	return ret;

error:
	if (devnull)
		fclose(devnull);
	if (cfile)
		fclose(cfile);
	if (hfile)
		fclose(hfile);
	rex_cc_destroy(pCC);
	return 2;
}
