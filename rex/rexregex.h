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

typedef struct rexregex_s rexregex_t;

#define REX_ENCODING_UTF8 0
#define REX_ENCODING_BYTE 1

rexregex_t *rex_regex_create(const char *str, unsigned int size);
rexregex_t *rex_regex_create_s(const char *str);
int rex_regex_match(rexregex_t *regex, unsigned int encoding, const char *start, const char *end);
int rex_regex_scan(rexregex_t *regex, unsigned int encoding, const char *start, const char *end, const char **where);
void rex_regex_destroy(rexregex_t *regex);


#ifdef __cplusplus
}
#endif

#endif
