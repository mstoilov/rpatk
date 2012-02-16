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
#include <errno.h>
#include "rlib/rmem.h"
#include "rlib/rarray.h"
#include "rex/rexdfaconv.h"
#include "rex/rexdfa.h"
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
		fprintf(stderr, "\t-b binfile               Use DFA from binfile.\n");
		fprintf(stderr, "\t-c                       Compile DFA and save to binfile. Use -b option to specify the name of the file.\n");
		fprintf(stderr, "\t-o, --only-matching      Show only the part of a line matching PATTERN\n");
		fprintf(stderr, "\t-l                       Line mode.\n");
		fprintf(stderr, "\t-N                       Use NFA.\n");
		fprintf(stderr, "\t-D                       Dump states.\n");
		fprintf(stderr, "\t-S                       Include DFA substates.\n");
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


#define REXGREP_BINOP_NONE 0
#define REXGREP_BINOP_READ 1
#define REXGREP_BINOP_WRITE 2


int main(int argc, const char *argv[])
{
	int ret, scanned = 0, i;
	rexgrep_t *pGrep;
	rarray_t *buffers;
	const char *binfile = NULL;
	int binop = REXGREP_BINOP_NONE;
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
		if (strcmp(argv[i], "-S") == 0) {
			pGrep->withsubstates = 1;
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
		if (strcmp(argv[i], "-b") == 0) {
			if (++i < argc) {
				binfile = argv[i];
				binop = REXGREP_BINOP_READ;
			}
		}
	}

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-c") == 0) {
			binop = REXGREP_BINOP_WRITE;
			if (!binfile)
				binfile = "rex.bin";
		}
	}

	if (binop != REXGREP_BINOP_READ) {
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
			if (strcmp(argv[i], "-N") == 0) {
				pGrep->usedfa = 0;
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

	if (!pGrep->dfa && binop == REXGREP_BINOP_READ) {
		FILE *pfile = NULL;
		rexdfa_t dfa;
		r_memset(&dfa, 0, sizeof(dfa));
		pfile = fopen(binfile, "rb");
		if (!pfile) {
			fprintf(stderr, "Failed to open file: %s, %s\n", binfile, strerror(errno));
			goto error;
		}
		if (fread(&dfa, sizeof(dfa), 1, pfile) != 1)
			goto error;
		pGrep->dfa = rex_dfa_create(dfa.nstates, dfa.ntrans, dfa.naccsubstates, dfa.nsubstates);
		if (fread(pGrep->dfa->states, sizeof(*dfa.states), dfa.nstates, pfile) != dfa.nstates)
			goto error;
		if (fread(pGrep->dfa->trans, sizeof(*dfa.trans), dfa.ntrans, pfile) != dfa.ntrans)
			goto error;
		if (fread(pGrep->dfa->accsubstates, sizeof(*dfa.accsubstates), dfa.naccsubstates, pfile) != dfa.naccsubstates)
			goto error;
		if (fread(pGrep->dfa->substates, sizeof(*dfa.substates), dfa.nsubstates, pfile) != dfa.nsubstates)
			goto error;
		fclose(pfile);
	}

	if (!pGrep->dfa && pGrep->usedfa) {
		rexdb_t *dfadb = rex_db_createdfa(pGrep->nfa, pGrep->startuid);
		pGrep->dfa = rex_db_todfa(dfadb, pGrep->withsubstates);
		rex_db_destroy(dfadb);
	}

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-D") == 0) {
			int j;
			if (pGrep->dfa) {
				for (j = 0; j < pGrep->dfa->nstates; j++) {
					rex_dfa_dumpstate(pGrep->dfa, j);
				}
			} else if (pGrep->nfa) {
				rexdb_t *db = pGrep->nfa;
				for (j = 0; j < r_array_length(db->states); j++) {
					rex_db_dumpstate(db, j);
				}
			}
			goto end;
		}
	}

	if (pGrep->dfa && binop == REXGREP_BINOP_WRITE) {
		rexdfa_t dfa = *pGrep->dfa;
		FILE *pfile = fopen(binfile, "wb");
		dfa.substates = NULL;
		dfa.states = NULL;
		dfa.trans = NULL;
		dfa.accsubstates = NULL;
		if (!pfile) {
			fprintf(stderr, "Failed to create file: %s, %s\n", binfile, strerror(errno));
			goto error;
		}
		fwrite(&dfa, sizeof(dfa), 1, pfile);
		dfa.states = pGrep->dfa->states;
		dfa.trans = pGrep->dfa->trans;
		dfa.accsubstates = pGrep->dfa->accsubstates;
		dfa.substates = pGrep->dfa->substates;
		fwrite(dfa.states, sizeof(*dfa.states), dfa.nstates, pfile);
		fwrite(dfa.trans, sizeof(*dfa.trans), dfa.ntrans, pfile);
		fwrite(dfa.accsubstates, sizeof(*dfa.accsubstates), dfa.naccsubstates, pfile);
		fwrite(dfa.substates, sizeof(*dfa.substates), dfa.nsubstates, pfile);
		fclose(pfile);
		goto end;
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

	/* scan files */
	for (i = 1; i < argc; i++) {
		if (argv[i][0] != '-') {
			++scanned;
			rex_grep_scan_path(pGrep, argv[i]);
		} else if (argv[i][1] == 'e' || argv[i][1] == 'f' || argv[i][1] == 'b'){
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
	if (pGrep->showtime && pGrep->dfa) {
		rexdfa_t *dfa = pGrep->dfa;
		unsigned long sizestates = dfa->nstates * sizeof(rexdfs_t);
		unsigned long sizetrans = dfa->ntrans * sizeof(rexdft_t);
		unsigned long sizeaccsubs = dfa->naccsubstates * sizeof(rexdfss_t);
		unsigned long sizesubs = dfa->nsubstates * sizeof(rexdfss_t);
		unsigned long sizetotal = sizestates + sizetrans + sizeaccsubs + sizesubs;
		fprintf(stdout, "\n\n");
		fprintf(stdout, "\tDFA Memory: %ld KB, States: %ld KB (%.2f), Transitions: %ld KB (%.2f), Accecpting Substates: %ld KB(%.2f), Substates: %ld KB (%.2f)\n",
				sizetotal/1024, sizestates/1024, (100.0*sizestates/sizetotal), sizetrans/1024, (100.0*sizetrans/sizetotal),
				sizeaccsubs/1024, (100.0*sizeaccsubs/sizetotal), sizesubs/1024, (100.0*sizesubs/sizetotal));
	}
	rex_grep_destroy(pGrep);
	if (pGrep->showtime) {
		fprintf(stdout, "\tmemory: %ld KB (leaked %ld Bytes)\n", (long)r_debug_get_maxmem()/1024, (long)r_debug_get_allocmem());
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
