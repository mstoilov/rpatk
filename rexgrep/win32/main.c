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
#include "rlib/rbuffer.h"
#include "rex/rexdfaconv.h"
#include "rex/rexdfa.h"
//#include "rexgrep.h"
//#include "rexgrepdep.h"

rbuffer_t * rex_buffer_from_wchar(const wchar_t *wstr);


int usage(int argc, const wchar_t *argv[])
{
		fwprintf(stderr, L"REX Grep - Work in progress...\n");
		fwprintf(stderr, L"Copyright (C) 2012 Martin Stoilov\n\n");

		fwprintf(stderr, L"Usage: \n %s [OPTIONS] <filename>\n", argv[0]);
		fwprintf(stderr, L" OPTIONS:\n");
		fwprintf(stderr, L"\t-e patterns              Regular Expression.\n");
		fwprintf(stderr, L"\t-f patternfile           Read Regular Expressions from a file.\n");
		fwprintf(stderr, L"\t-b binfile               Use DFA from binfile.\n");
		fwprintf(stderr, L"\t-c                       Compile DFA and save to binfile. Use -b option to specify the name of the file.\n");
		fwprintf(stderr, L"\t-o, --only-matching      Show only the part of a line matching PATTERN\n");
		fwprintf(stderr, L"\t-l                       Line mode.\n");
		fwprintf(stderr, L"\t-N                       Use NFA.\n");
		fwprintf(stderr, L"\t-D                       Dump states.\n");
		fwprintf(stderr, L"\t-S                       Include DFA substates.\n");
		fwprintf(stderr, L"\t-q                       Quiet mode.\n");
		fwprintf(stderr, L"\t-t                       Display statistics. Works only when built in DEBUG mode.\n");
		fwprintf(stderr, L"\t-s string                Search in string.\n");
		fwprintf(stderr, L"\t-v                       Display version information.\n");
		fwprintf(stderr, L"\t-h, --help               Display this help.\n");

		return 0;
}


int wmain(int argc, const wchar_t* argv[])
{

	usage(argc, argv);
	return 0;
}
