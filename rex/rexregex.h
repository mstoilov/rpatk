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

/**
 * @file rex/rexregex.h
 * @brief Definition of RegEx interface
 *
 *
 * <h2>Synopsis</h2>
 * This file defines the data structures and API for the RegEx implementation.
 */

#ifndef _REXREGEX_H_
#define _REXREGEX_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Define RegEx object
 */
typedef struct rexregex_s rexregex_t;

#define REX_ENCODING_UTF8 0			/**< Input is encoded in UTF8. */
#define REX_ENCODING_BYTE 1			/**< The encoding is one byte per character */

/**
 * Create regular expression object.
 * @param str Regular Expression
 * @param size The size of str in bytes.
 * @return Regular Expression object.
 */
rexregex_t *rex_regex_create(const char *str, unsigned int size);

/**
 * Create regular expression object.
 * @param str Regular Expression. The string must be NULL terminated
 * @return Regular Expression object.
 */
rexregex_t *rex_regex_create_s(const char *str);

/**
 * Match regular expression.
 * @param regex regular expression object created with @ref rex_regex_create or @ref rex_regex_create_s
 * @param encoding Input encoding. Supported encodings:
 * 		- @ref REX_ENCODING_UTF8
 * 		- @ref REX_ENCODING_BYTE
 * @param start Input start pointer
 * @param end Input end pointer
 * @return Returns the size in bytes of the matched substring or 0 if no match, -1 in case of error.
 */
int rex_regex_match(rexregex_t *regex, unsigned int encoding, const char *start, const char *end);

/**
 * Search for regular expression match within the input specified with start and end.
 * @param regex regular expression object created with @ref rex_regex_create or @ref rex_regex_create_s
 * @param encoding Input encoding. Supported encodings:
 * 		- @ref REX_ENCODING_UTF8
 * 		- @ref REX_ENCODING_BYTE
 * @param start Input start pointer
 * @param end Input end pointer
 * @param where It would be set to the beginning of the matched substring.
 * @return Returns the size in bytes of the matched substring or 0 if no match, -1 in case of error.
 * If 'where' is not NULL, it would be set to the beginning of the matched substring.
 */
int rex_regex_scan(rexregex_t *regex, unsigned int encoding, const char *start, const char *end, const char **where);

/**
 * Destroy regex object created with @ref rex_regex_create or @ref rex_regex_create_s
 * @param regex regular expression object to be destroyed.
 */
void rex_regex_destroy(rexregex_t *regex);


#ifdef __cplusplus
}
#endif

#endif
