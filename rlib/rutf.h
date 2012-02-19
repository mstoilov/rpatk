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

#ifndef _RUTF_H_
#define _RUTF_H_

#include "rtypes.h"

#ifdef __cplusplus
extern "C" {
#endif


int r_utf8_mbtowc(ruint32 *pwc, const unsigned char *input, const unsigned char *end);
int r_utf8_wctomb(ruint32 wc, unsigned char *output, ruint32 size);
int r_utf16_mbtowc(ruint32 *pwc, const unsigned char *s, const unsigned char *end);
int r_utf16_wctomb(ruint32 wc, unsigned char *output, ruint32 size);


#ifdef __cplusplus
}
#endif

#endif
