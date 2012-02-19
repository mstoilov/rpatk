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

/*
 * To build:
 * # gcc -I/usr/include/rpatk -o js-tokenizer js-tokenizer.c -lrex -lrlib
 *
 * To run:
 * # echo "function add(a,b) { var c = a + b; return c; } print('здравей means hello');" | ./js-tokenizer
 */

#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include "rex/rexdb.h"
#include "rex/rexdfa.h"

#define TOKEN_SELF 256
#define TOKEN_IDENTIFIER 257
#define TOKEN_SPACE 258
#define TOKEN_KEYWORD 259
#define TOKEN_OPERATOR 260
#define TOKEN_STRING 261
#define TOKEN_DECIMAL 262

int get_token(rexdfa_t *dfa, wint_t *buffer, int size)
{
	rexdfss_t *acc_ss = NULL;
	rexuint_t nstate = REX_DFA_STARTSTATE;
	int ret = -1, i = 0;
	wint_t wc;
	
	while ((wc = fgetwc(stdin)) != WEOF) {
		if ((nstate = REX_DFA_NEXT(dfa, nstate, wc)) == REX_DFA_DEADSTATE) {
			ungetc(wc, stdin);
			break;
		}
		if (i + 1 < size) {
			buffer[i++] = wc;
		}
		if (REX_DFA_STATE(dfa, nstate)->type == REX_STATETYPE_ACCEPT) {
			/*
			 * The DFA is in accepting state, lets find out what exactly is
			 * being accepted.
			 * The token ID is recorder in the substate's userdata
			 *
			 * Note: There are may be more than one accepting substate,
			 * but we only check the first one (at offset 0). A real implementation
			 * might need to check the rest of the accepting substates(and userdata)
			 * to decide which one to use.
			 *
			 * Note: Some of the conflicts might be resolved simply be reordering
			 * the regular expressions. For example TOKEN_KEYWORD such as 
			 * 'while', 'if', etc. also match TOKEN_IDENTIFIER, but because
			 * TOKEN_KEYWORD appears before TOKEN_IDENTIFIER it is placed first.
			 *
			 * Note: We will not break out of the loop here. We will keep going
			 * in order to find the longest match.
			 */
			acc_ss = REX_DFA_ACCSUBSTATE(dfa, nstate, 0);
			ret = (int) acc_ss->userdata;
			if (ret == TOKEN_SELF)
				ret = wc;
		}
	}
	buffer[i++] = '\0';
	return ret;
}

int main(int argc, char *argv[])
{
	rexdb_t *nfadb;
	rexdb_t *dfadb;
	rexdfa_t *dfa;
	long startstate = 0;
	wint_t buffer[4000];
	int token;
	
	if (!setlocale(LC_CTYPE, "")) {
		printf("Can not set the specified locale, please check LANG, LC_CTYPE, LC_ALL.\n");
		return 1;
    }

	/*
	 * Create empty automaton of type NFA.
	 */
	nfadb = rex_db_create(REXDB_TYPE_NFA);

	/*
	 * Load the automaton with regular expressions, defining JavaScript language tokens.
	 */
	startstate = rex_db_addexpression_s(nfadb, startstate,
							"instanceof | typeof | break | do | new | var | case | else | "
							"return | void | catch | finally | continue | for | "
							"switch | while | this | with |debugger | function | throw | default | "
							"if | try | delete | in | class | enum | extends | import | const | export | "
							"implements | let | private | public | static | interface | package | protected",
							TOKEN_KEYWORD);
	startstate = rex_db_addexpression_s(nfadb, startstate,
							"([#0x0041-#0x005A] | [#0x00C0-#0x00DE] | [#0x0100-#0x0232] | [#0x0061-#0x007A] | "
							"[#0x00C0-#0x00DE] | $ | _ )([#0x0041-#0x005A] | [#0x00C0-#0x00DE] | "
							"[#0x0100-#0x0232] | [#0x0061-#0x007A] | [#0x00C0-#0x00DE] | $ | _ | [0-9] | [#0x0660-#0x0669])*",
							TOKEN_IDENTIFIER);
	startstate = rex_db_addexpression_s(nfadb, startstate,
							"=== | !== | >= | <= | == | != | << | >>> | >> | & | ^= | ^ | ! | ~ | && | [|][|] | [?] | : | "
							">>= | >>>= | &= | [|]= | = | [+]= | -= | [*]= | %= | <<= | [.] | ; | , | < | > | [|] | "
							"[+] | - | [*] | % | [+][+] | -- | / | /=",
							TOKEN_OPERATOR);
	startstate = rex_db_addexpression_s(nfadb, startstate, "[1-9][0-9]*", TOKEN_DECIMAL);
	startstate = rex_db_addexpression_s(nfadb, startstate, "'[^']*'|\"[^\"]*\"", TOKEN_STRING);
	startstate = rex_db_addexpression_s(nfadb, startstate, "[\\t\\r\\n ]+", TOKEN_SPACE);
	startstate = rex_db_addexpression_s(nfadb, startstate, "[^\\t\\r\\n'\" ]", TOKEN_SELF);

	/*
	 * Construct the DFA from the NFA
	 */
	dfadb = rex_db_createdfa(nfadb, startstate);

	/*
	 * At this point you can start using the dfadb for matching, but it is better
	 * to generate the more compact representation rexdfa_t.
	 */
	dfa = rex_db_todfa(dfadb, 0);

	/*
	 * Destroy the rexdb_t objects. We don't need them any more.
	 */
	rex_db_destroy(nfadb);
	rex_db_destroy(dfadb);

	/*
	 * Read the tokens and print them.
	 */
	while ((token = get_token(dfa, buffer, sizeof(buffer)/sizeof(buffer[0]))) > 0) {
		if (token != TOKEN_SPACE)
			fwprintf(stdout, L"token(%3d): %ls\n", token, buffer);
	}
	rex_dfa_destroy(dfa);
	return 0;
}
