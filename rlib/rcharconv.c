/*
 *  Regular Pattern Analyzer Toolkit (RPA/Tk)
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

#include "rlib/rcharconv.h"


#define LOWANSICHAR(c) (((c) >= 'A' && (c) <= 'Z') ? (c) + ('a' - 'A') : (c))
#define HIGHANSICHAR(c) (((c) >= 'a' && (c) <= 'z') ? (c) + ('A' - 'a') : (c))


unsigned long r_charlow(unsigned long c)
{
	return LOWANSICHAR(c);
}


unsigned long r_charhigh(unsigned long c)
{
	return HIGHANSICHAR(c);
}


unsigned long r_charicase(unsigned long c)
{
	if (c >= 'A' && c <= 'Z')
		return c + ('a' - 'A');
	else if (c >= 'a' && c <= 'z')
		return c + ('A' - 'a');
	else if (c >= 0x0410 && c <= 0x042F)   /* Cyrilic */
		return c + 0x0430 - 0x0410;
	else if (c >= 0x0430 && c <= 0x044F)
		return c + 0x0410 - 0x0430;
	else if (c >= 0x00C0 && c <= 0x00DF)   /* Latin "*/
		return c + 0x00E0 - 0x00C0;
	else if (c >= 0x00E0 && c <= 0x00FF)
		return c + 0x00C0 - 0x00E0;
	else if (c >= 0x0390 && c <= 0x03AB)   /* Greek */
		return c + 0x03B0 - 0x0390;
	else if (c >= 0x03B0 && c <= 0x03CB)
		return c + 0x0390 - 0x03B0;
		
	return c;
}

