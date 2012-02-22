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


#include <wchar.h>
#include <windows.h>
#include "rlib/rmem.h"
#include "rlib/rarray.h"
#include "rpa/rpadbex.h"
#include "rpagrep.h"
#include "rpagrepdep.h"

rpa_buffer_t * rpa_buffer_from_wchar(const wchar_t *wstr);


int usage(int argc, const wchar_t *argv[])
{
	    fwprintf(stderr, L"RPA Grep with RPA Engine: %s \n", rpa_dbex_version());
		fwprintf(stderr, L"Copyright (C) 2010 Martin Stoilov\n\n");

		fwprintf(stderr, L"Usage: \n %s [OPTIONS] <filename>\n", argv[0]);
		fwprintf(stderr, L" OPTIONS:\n");
		fwprintf(stderr, L"\t-e patterns              BNF Expression.\n");
		fwprintf(stderr, L"\t-f patternfile           Read the BNF rules from a file, the last pattern will be executed.\n");
		fwprintf(stderr, L"\t-i                       Ignore case.\n");
		fwprintf(stderr, L"\t-m                       Match.\n");
		fwprintf(stderr, L"\t-p                       Parse.\n");
		fwprintf(stderr, L"\t-l                       Line mode.\n");
		fwprintf(stderr, L"\t-16                      Force UTF16 encoding.\n");
		fwprintf(stderr, L"\t-b                       Force byte encoding.\n");
		fwprintf(stderr, L"\t-d                       Dump a production in a tree format.\n");
		fwprintf(stderr, L"\t-t                       Display time elapsed.\n");
		fwprintf(stderr, L"\t-L, --list-rules         List all patterns.\n");
		fwprintf(stderr, L"\t-v                       Display version information.\n");
		fwprintf(stderr, L"\t-h, --help               Display this help.\n");
		fwprintf(stderr, L"\t    --debug-compile      Display debug compilation information.\n");
		fwprintf(stderr, L"\t    --dump-info          Display rules info.\n");
		fwprintf(stderr, L"\t    --dump-code rule     Display compiled code for rule.\n");
		fwprintf(stderr, L"\t    --dump-alias         Display alias info.\n");
		fwprintf(stderr, L"\t    --dump-records       Display rules parsing records.\n");
		fwprintf(stderr, L"\t    --no-optimizations   Disable optimizations.\n");
		fwprintf(stderr, L"\t    --exec-debug         Execute in debug mode.\n");
		fwprintf(stderr, L"\t    --no-cache           Disable execution cache.\n");
		fwprintf(stderr, L"\t    --no-bitmap          Disable expression bitmap use.\n");

		return 0;
}


int wmain(int argc, const wchar_t* argv[])
{
	unsigned long sckb;
	int ret, scanned = 0;
	long i;
	rpa_grep_t *pGrep = NULL;
	rarray_t *buffers;

	buffers = r_array_create(sizeof(rpa_buffer_t *));
	pGrep = rpa_grep_create();

	if (argc <= 1) {
		usage(argc, argv);
		goto end;
	}

	for (i = 1; i < argc; i++) {
		if (wcscmp(argv[i], L"-t") == 0) {
			pGrep->showtime = 1;
		}
	}

	for (i = 1; i < argc; i++) {
		if (wcscmp(argv[i], L"--help") == 0 || wcscmp(argv[i], L"-help") == 0 || wcscmp(argv[i], L"/?") == 0 || wcscmp(argv[i], L"-h") == 0) {
			usage(argc, argv);
			goto end;
		}
	}

	for (i = 1; i < argc; i++) {
		if (wcscmp(argv[i], L"--no-bitmap") == 0) {
			rpa_dbex_cfgset(pGrep->hDbex, RPA_DBEXCFG_BITMAP, 0);
		}
	}

	for (i = 1; i < argc; i++) {
		if (wcscmp(argv[i], L"--no-optimizations") == 0) {
			rpa_grep_optimizations(pGrep, 0);
		}
	}

	for (i = 1; i < argc; i++) {
		if (wcscmp(argv[i], L"-c") == 0) {
			if (++i < argc) {
				rpa_buffer_t *pattern = rpa_buffer_from_wchar(argv[i]);
				if (!pattern) {
					goto error;
				}
				rpa_grep_setup_callback(pGrep, pattern);
				rpa_buffer_destroy(pattern);
			}
		}
	}


	for (i = 1; i < argc; i++) {
		if (wcscmp(argv[i], L"-f") == 0) {
			if (++i < argc) {
				rpa_buffer_t *pattern = rpa_buffer_map_file(argv[i]);
				if (pattern) {
					ret = rpa_grep_load_pattern(pGrep, pattern);
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
		if (wcscmp(argv[i], L"-e") == 0) {
			if (++i < argc) {
				rpa_buffer_t *pattern = rpa_buffer_from_wchar(argv[i]);
				if (!pattern) {
					goto error;
				}
				ret = rpa_grep_load_string_pattern(pGrep, pattern);
				rpa_buffer_destroy(pattern);
				if (ret < 0)
					goto error;

			}
		} 
	}


	for (i = 1; i < argc; i++) {
		if (wcscmp(argv[i], L"--dump-code") == 0) {
			if (rpa_dbex_compile(pGrep->hDbex) == 0) {
				if (++i < argc) {
					rpa_buffer_t *code = rpa_buffer_from_wchar(argv[i]);
					rpa_dbex_dumpcode(pGrep->hDbex, rpa_dbex_lookup_s(pGrep->hDbex, code->s));
					rpa_buffer_destroy(code);
				}
			}
			goto end;
		}
	}


	for (i = 1; i < argc; i++) {
		if (wcscmp(argv[i], L"--dump-info") == 0) {
			rpa_grep_dump_pattern_info(pGrep);
			goto end;
		}
	}

	for (i = 1; i < argc; i++) {
		if (wcscmp(argv[i], L"--debug-compile") == 0) {
			rpa_grep_debug_compile(pGrep);
			goto end;
		}
	}

	for (i = 1; i < argc; i++) {
		if (wcscmp(argv[i], L"--dump-alias") == 0) {
			rpa_grep_dump_alias_info(pGrep);
			goto end;
		}
	}

	for (i = 1; i < argc; i++) {
		if (wcscmp(argv[i], L"--dump-records") == 0) {
			rpa_grep_dump_pattern_records(pGrep);
			goto end;
		}
	}

	for (i = 1; i < argc; i++) {
		if (wcscmp(argv[i], L"--exec-debug") == 0) {
			pGrep->execdebug = 1;
		}
	}

	for (i = 1; i < argc; i++) {
		if (wcscmp(argv[i], L"--no-cache") == 0) {
			pGrep->disablecache = 1;
		}
	}


	if (rpa_dbex_compile(pGrep->hDbex) < 0) {
		rpa_errinfo_t errinfo;
		rpa_dbex_lasterrorinfo(pGrep->hDbex, &errinfo);
		if (errinfo.code == RPA_E_UNRESOLVEDSYMBOL) {
			fprintf(stdout, "ERROR: Unresolved Symbol: %s\n", errinfo.name);
		} else {
			fprintf(stdout, "ERROR %ld: Compilation failed.\n", errinfo.code);
		}
		goto end;
	}


	for (i = 1; i < argc; i++) {
		if (wcscmp(argv[i], L"-L") == 0) {
			rpa_grep_list_patterns(pGrep);
			goto end;
		} else if (wcscmp(argv[i], L"-d") == 0) {
			if (++i < argc) {
				rpa_buffer_t *pattern = rpa_buffer_from_wchar(argv[i]);
				if (!pattern) {
					goto error;
				}
				rpa_grep_dump_pattern_tree(pGrep, pattern);
				rpa_buffer_destroy(pattern);
				goto end;
			}
		} else if (wcscmp(argv[i], L"-i") == 0) {
			pGrep->icase = 1;
		} else if (wcscmp(argv[i], L"-l") == 0) {
			pGrep->greptype = RPA_GREPTYPE_SCANLINES;
		} else if (wcscmp(argv[i], L"-m") == 0) {
			pGrep->greptype = RPA_GREPTYPE_MATCH;
		} else if (wcscmp(argv[i], L"-p") == 0) {
			pGrep->greptype = RPA_GREPTYPE_PARSE;
		} else if (wcscmp(argv[i], L"-a") == 0) {
			pGrep->greptype = RPA_GREPTYPE_PARSEAST;
		} else if (wcscmp(argv[i], L"-16") == 0) {
			pGrep->forceEncoding = RPA_GREP_FORCE_UTF16;
		} else if (wcscmp(argv[i], L"-b") == 0) {
			pGrep->forceEncoding = RPA_GREP_FORCE_BYTE;
		}
		
	}


	for (i = 1; i < argc; i++) {
		if (wcscmp(argv[i], L"-s") == 0) {
			if (++i < argc) {
				rpa_buffer_t *buf = rpa_buffer_from_wchar(argv[i]);
				rpa_grep_scan_buffer(pGrep, buf);
				rpa_buffer_destroy(buf);
				++scanned;
			}
		}
	}

	/* scan files */
	for (i = 1; i < argc; i++) {
		if (argv[i][0] != L'-') {
			++scanned;
			rpa_grep_scan_path(pGrep, argv[i]);
		} else if (argv[i][1] == L'e' || argv[i][1] == L'f' || argv[i][1] == L'c' || argv[i][1] == L'C'){
			++i;
		}
		
	}

	if (!scanned) {
		rpa_buffer_t *buf = rpa_buffer_loadfile(stdin);
		if (buf) {
			rpa_grep_scan_buffer(pGrep, buf);
			rpa_buffer_destroy(buf);
		}
	}

end:
	for (i = 0; i < (long)r_array_length(buffers); i++) {
		rpa_buffer_destroy(r_array_index(buffers, i, rpa_buffer_t*));
	}
	r_object_destroy((robject_t*)buffers);
	rpa_grep_close(pGrep);


	rpa_grep_close(pGrep);
	sckb = (unsigned long)(pGrep->scsize/1024);

	if (pGrep->showtime) {
		unsigned long milsec;
		unsigned long minutes;
		double sec;
		milsec = pGrep->scanmilisec;
		if (milsec == 0)
			milsec = 1;
		minutes = milsec/60000;
		sec = (milsec%60000)/1000.0;
		fwprintf(stdout, L"\ntime: %0ldm%1.3fs, %ld KB (%ld KB/sec), stack: %ld KB, memory: %ld KB (leaked %ld Bytes), cachehit: %ld \n", 
				minutes, sec, sckb, 1000*sckb/milsec, pGrep->usedstack / 1000, (long)r_debug_get_maxmem()/1000, (long)r_debug_get_allocmem(),
				pGrep->cachehit);
	}

	rpa_grep_destroy(pGrep);
	return pGrep->ret;

error:
	rpa_grep_destroy(pGrep);
	return 2;
}
