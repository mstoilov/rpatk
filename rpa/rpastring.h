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

#ifndef _RPASTRING_H_
#define _RPASTRING_H_


#ifdef __cplusplus
extern "C" {
#endif


char *rpa_strcpy(char *dest, const char *src);
char *rpa_strncpy(char *dest, const char *src, unsigned long n);
int rpa_strcmp(const char *s1, const char *s2);
int rpa_strncmp(const char *s1, const char *s2, unsigned long n);
char *rpa_strstr(const char *haystack, const char *needle);
char *rpa_strchr(const char *s, int c);
unsigned long rpa_strlen(const char *s);
unsigned long int rpa_strtoul(const char *nptr, char **endptr, int base);
char *rpa_strcat(char *dest, const char *src);
char *rpa_strdup(const char *s);
char *rpa_strncat(char *dest, const char *src, unsigned int n);
unsigned long rpa_lowchar(unsigned long c);
unsigned long rpa_hichar(unsigned long c);
unsigned long rpa_icasechar(unsigned long c);
int rpa_snprintf(char *str, unsigned int size, const char *format, ...);
int rpa_printf(const char *format, ...);


#ifdef __cplusplus
}
#endif

#endif
