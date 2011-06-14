// rgrep.cpp : Defines the entry point for the console application.
//

#include <wchar.h>
#include <windows.h>
#include "rpagrep.h"
#include "rpagrepdep.h"

rpa_buffer_t * rpa_buffer_from_wchar(const wchar_t *wstr);


int usage(int argc, const wchar_t *argv[])
{
		fwprintf(stderr, L"Usage: \n %s [OPTIONS] <filename>\n", argv[0]);
		fwprintf(stderr, L" OPTIONS:\n");
		fwprintf(stderr, L"\t-e patterns         execute pattern\n");
		fwprintf(stderr, L"\t-f patternfile      read the patterns from a file, the last pattern will be executed\n");
		fwprintf(stderr, L"\t-c printpatterns    printeger these patterns when there is a match.\n");
		fwprintf(stderr, L"\t-i                  ignore case.\n");		
		fwprintf(stderr, L"\t-m                  match only from the beginning.\n");		
		fwprintf(stderr, L"\t-p                  parse the stream.\n");		
		fwprintf(stderr, L"\t-l                  line mode.\n");
		fwprintf(stderr, L"\t-16                 force UTF16 encoding.\n");
		fwprintf(stderr, L"\t-d                  dump the pattern tree.\n");
		fwprintf(stderr, L"\t-t                  display time elapsed.\n");
		fwprintf(stderr, L"\t-L                  List all patterns.\n");
		fwprintf(stderr, L"\t-h, --help          Display this help.\n");

		return 0;
}


int wmain(int argc, const wchar_t* argv[])
{
	unsigned long sckb;
	int ret, scanned = 0, i;
	rpa_grep_t *pGrep = NULL;
	DWORD eticks, bticks = GetTickCount();

	pGrep = rpa_grep_create();

	if (argc <= 1) {
		usage(argc, argv);
		goto end;
	}
	for (i = 1; i < argc; i++) {
		if (wcscmp(argv[i], L"--help") == 0 || wcscmp(argv[i], L"-help") == 0 || wcscmp(argv[i], L"/?") == 0 || wcscmp(argv[i], L"-h") == 0) {
			usage(argc, argv);
			goto end;
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
				if (!pattern) {
					goto error;
				}
				ret = rpa_grep_load_pattern(pGrep, pattern);
				if (!pattern) {
					goto error;
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
			pGrep->linemode = 1;
		} else if (wcscmp(argv[i], L"-16") == 0) {
			pGrep->forceEncoding = RPA_GREP_FORCE_UTF16;
		} else if (wcscmp(argv[i], L"-b") == 0) {
			pGrep->forceEncoding = RPA_GREP_FORCE_BYTE;
		} else if (wcscmp(argv[i], L"-m") == 0) {
			pGrep->matchonly = 1;
		} else if (wcscmp(argv[i], L"-p") == 0) {
			pGrep->matchonly = 2;
		} else if (wcscmp(argv[i], L"-t") == 0) {
			pGrep->showtime = 1;
		}
		
	}

	/* scan files */
	for (i = 1; i < argc; i++) {
		if (argv[i][0] != '-') {
			++scanned;
			rpa_grep_scan_path(pGrep, argv[i]);
		} else if (argv[i][1] == L'e' || argv[i][1] == L'f' || argv[i][1] == L'c'){
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
	rpa_grep_close(pGrep);
	sckb = (unsigned long)(pGrep->scsize/1024);

	if (pGrep->showtime) {
		unsigned long milsec;
		unsigned long minutes;
		float sec;
		milsec = pGrep->scanmilisec;
		if (milsec == 0)
			milsec = 1;
		minutes = milsec/60000;
		sec = (milsec%60000)/1000.0;
		fwprintf(stdout, L"\ntime: %0ldm%1.3fs, %ld KB (%ld KB/sec), stack: %ld KB, fp = %ld\n", 
				minutes, sec, sckb, 1000*sckb/milsec, pGrep->usedstack / 1000, (unsigned long)pGrep->ud0);
	}

	rpa_grep_destroy(pGrep);
	return 0;


/*
	if (pGrep->showtime) {
		unsigned long sec;
		unsigned long sckb = (unsigned long)(pGrep->scsize/1024);
		eticks = GetTickCount();
		sec = (eticks - bticks)/1000;
		fwprintf(stdout, L"\n\ntime: %02ld:%02ld, %ld KB (%ld KB/sec), stack: %ld KB\n", 
			sec/60, sec%60, sckb, sckb/(sec ? sec : 1), pGrep->usedstack / 1000);
	}
end:
	rpa_grep_destroy(pGrep);
	return 0;
*/

error:
	rpa_grep_destroy(pGrep);
	return 1;
}
