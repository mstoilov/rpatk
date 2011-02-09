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

#include "rutf.h"


rint r_utf8_mbtowc(ruint32 *pwc, const ruchar *input, const ruchar *end)
{
	rint n;
	ruchar c;

	if (input >= end) {
		*pwc = (ruint32)0;
		return 0;
	}	

	if ((c = input[0]) < 0x80) {
		*pwc = c;
		return 1;
	}
	n = (rint)(end - input);
	if (c == 0xC0 || c == 0xC1 || (c >= 0xF5 && c <= 0xFF))
		goto error;
	if ((c >> 5) == 6) {
		if (n < 2 || (input[1] >> 6 != 0x02))
			goto error;
		*pwc = ((ruint32) (c & 0x1f) << 6)	| (ruint32) (input[1] ^ 0x80);
		return 2;
	} else if ((c >> 4) == 0x0E) {
		if (n < 3 || (input[1] >> 6 != 0x02) || (input[2] >> 6 != 0x02))
			goto error;
		*pwc = ((ruint32) (c & 0x0f) << 12) | ((ruint32) (input[1] ^ 0x80) << 6) | (ruint32) (input[2] ^ 0x80);
		return 3;
	} else if ((c >> 3) == 0x1E) {
		if (n < 4 || (input[1] >> 6 != 0x02) || (input[2] >> 6 != 0x02) || (input[3] >> 6 != 0x02))
			goto error;
		*pwc = ((ruint32) (c & 0x07) << 18)
			| ((ruint32) (input[1] ^ 0x80) << 12)
			| ((ruint32) (input[2] ^ 0x80) << 6)
			| (ruint32) (input[3] ^ 0x80);
		return 4;
	}
		
error:
	*pwc = c;
	return 1;
}


rint r_utf8_wctomb(ruint32 wc, ruchar *output, ruint32 size)
{
	ruint32 count;
	if (wc <= 0x007F)
		count = 1;
	else if (wc <= 0x07FF)
		count = 2;
	else if (wc <= 0xFFFF)
		count = 3;
	else if (wc <= 0x10FFFF)
		count = 4;
	else
		return 0;
	if (size < count)
		return 0;
	switch (count) {
		case 4: 
			output[3] = 0x80 | (wc & 0x3f); 
			wc = wc >> 6; 
			wc |= 0x10000;
		case 3: 
			output[2] = 0x80 | (wc & 0x3f); 
			wc = wc >> 6; 
			wc |= 0x800;
		case 2: 
			output[1] = 0x80 | (wc & 0x3f); 
			wc = wc >> 6; 
			wc |= 0xc0;
		case 1: 
			output[0] = wc;
	}
	return count;
}


rint r_utf16_mbtowc(ruint32 *pwc, const ruchar *s, const ruchar *end)
{
	rint n = (rint)(end - s);
	ruint32 wc1, wc2;

	if (s >= end) {
		*pwc = (ruint32)0;
		return 0;
	}
	
	if (n < 2)
		goto error;

	wc1 = s[0] + (s[1] << 8);
	if (wc1 >= 0xd800 && wc1 < 0xdc00) {
		if (n < 4)
			goto error;
		wc2 = s[2] + (s[3] << 8);
		if (!(wc2 >= 0xdc00 && wc2 < 0xe000))
			goto error;
		*pwc = 0x10000 + ((wc1 - 0xd800) << 10) + (wc2 - 0xdc00);
		return 4;
	} else if (wc1 >= 0xdc00 && wc1 < 0xe000) {
		goto error;
	} else {
		*pwc = wc1;
		return 2;
	}

error:
	*pwc = s[0];
	return 1;
}


rint r_utf16_wctomb(ruint32 wc, ruchar *output, ruint32 size)
{
	ruint32 wc1, wc2;

	if (wc <= 0x10FFFF && (wc < 0xD800 || wc >= 0xE000)) {
		if (wc < 0x10000) {
			if (size < 2)
				return 0;
			output[0] = (ruchar) wc;
			output[1] = (ruchar) (wc >> 8);
			return 2;
		} else if (wc <= 0x10FFFF) {
			if (size < 4) 
				return 0;
			wc1 = 0xd800 + ((wc - 0x10000) >> 10);
			wc2 = 0xdc00 + ((wc - 0x10000) & 0x3ff);
			output[0] = (ruchar) wc1;
			output[1] = (ruchar) (wc1 >> 8);
			output[2] = (ruchar) wc2;
			output[3] = (ruchar) (wc2 >> 8);
			return 4;
		}
	}
	return 0;
}
