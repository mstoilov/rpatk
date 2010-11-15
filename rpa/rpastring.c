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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "rpastring.h"
#include "rpamem.h"

#define LOWANSICHAR(c) (((c) >= 'A' && (c) <= 'Z') ? (c) + ('a' - 'A') : (c))
#define HIGHANSICHAR(c) (((c) >= 'a' && (c) <= 'z') ? (c) + ('A' - 'a') : (c))


char *rpa_strcpy(char *dest, const char *src)
{
	return strcpy(dest, src);
}


char *rpa_strncpy(char *dest, const char *src, unsigned long n)
{
	return strncpy(dest, src, (size_t)n);
}


int rpa_strcmp(const char *s1, const char *s2)
{
	return strcmp(s1, s2);
}


int rpa_strncmp(const char *s1, const char *s2, unsigned long n)
{
	return strncmp(s1, s2, (size_t)n);
}


char *rpa_strstr(const char *haystack, const char *needle)
{
	return strstr(haystack, needle);
}


char *rpa_strchr(const char *s, int c)
{
	return strchr(s, c);
}


unsigned long rpa_strlen(const char *s)
{
	if (!s)
		return 0;
	return (unsigned long)strlen(s);
}


unsigned long int rpa_strtoul(const char *nptr, char **endptr, int base)
{
	return strtoul(nptr, endptr,base);	
}


char *rpa_strcat(char *dest, const char *src)
{
	return strcat(dest, src);
}


char *rpa_strdup(const char *s)
{
#ifdef RPA_DEBUG_MEM
	long size = rpa_strlen(s) + 1;
	long *mem = NULL;
	size += sizeof(long);
	mem = (long*)malloc((size_t)(size));
	rpa_memset((char*)mem, 0, size);	
	*((long*)mem) = size;
	g_rpa_allocmem += size;
	mem += 1;
	return rpa_strcpy((char*)mem, s);
#else
	return strdup(s);
#endif

}


char *rpa_strncat(char *dest, const char *src, unsigned int n)
{
	return strncat(dest, src, n);
}


unsigned long rpa_lowchar(unsigned long c)
{
	return LOWANSICHAR(c);
}


unsigned long rpa_hichar(unsigned long c)
{
	return HIGHANSICHAR(c);
}


unsigned long rpa_icasechar(unsigned long c)
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


int rpa_vsnprintf(char *str, unsigned int size, const char *format, va_list ap)
{
	return vsnprintf(str, size, format, ap);	
}


int rpa_snprintf(char *str, unsigned int size, const char *format, ...)
{
	va_list ap;
	int ret;
	
	va_start(ap, format);
	ret = rpa_vsnprintf(str, size, format, ap);
	va_end(ap);
	return ret;
}


int rpa_printf(const char *format, ...)
{
	va_list ap;
	int ret;
	
	va_start(ap, format);
	ret = vfprintf(stdout, format, ap);
	va_end(ap);
	return ret;
}
