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

#ifndef _RPAGREPUTF_H_
#define _RPAGREPUTF_H_


#ifdef __cplusplus
extern "C" {
#endif


unsigned int rpa_grep_utf8_mbtowc(unsigned int *pwc, const unsigned char *input, const unsigned char *end);
unsigned int rpa_grep_utf16_mbtowc(unsigned int *pwc, const unsigned char *s, const unsigned char *end);
int rpa_grep_utf16_wctomb(unsigned int wc, unsigned char *output, unsigned int size);
int rpa_grep_utf8_wctomb(unsigned int wc, unsigned char *output, unsigned int size);


#ifdef __cplusplus
}
#endif

#endif
