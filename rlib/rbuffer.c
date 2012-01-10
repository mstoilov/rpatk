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

#include "rlib/rbuffer.h"
#include "rlib/rmem.h"



void r_buffer_destroy(rpa_buffer_t *buffer)
{
	if (buffer) {
		free(buffer->s);
		free(buffer);
	}
}


rpa_buffer_t *rpa_buffer_alloc(unsigned long size)
{
	rpa_buffer_t *buffer;

	buffer = (rpa_buffer_t *)malloc(sizeof(rpa_buffer_t));
	if (!buffer)
		return (void*)0;
	memset(buffer, 0, sizeof(*buffer));
	if (!(buffer->s = (char *)malloc((size + 1) * sizeof(char)))) {
		free(buffer);
		return (void*)0;
	}
	memset(buffer->s, 0, size + 1);
	buffer->size = size;
	buffer->destroy = rpa_buffer_free;
	return buffer;
}


int rpa_buffer_realloc(rpa_buffer_t *buffer, unsigned int size)
{
	char *s;

	s = (char *)realloc(buffer->s, size);
	if (!s)
		return -1;
	buffer->s = s;
	buffer->size = size;
	return 0;
}


rbuffer_t *rpa_buffer_fromfile(FILE *pFile)
{
	unsigned long memchunk = 256;
	long ret = 0, inputsize = 0;
	rpa_buffer_t *buf;

	buf = rpa_buffer_alloc(2 * memchunk);
	if (!buf)
		return (void*)0;

	do {
		if ((buf->size - inputsize) < memchunk) {
			if (rpa_buffer_realloc(buf, buf->size + memchunk) < 0) {
				fprintf(stderr, "Out of memory!\n");
				exit(1);
			}
		}
		ret = (long)fread(&buf->s[inputsize], 1, memchunk - 1, pFile);
		if ((ret <= 0) && ferror(pFile)) {
			rpa_buffer_destroy(buf);
			return (void*)0;
		}
		inputsize += ret;
		buf->s[inputsize] = '\0';
		buf->size = inputsize;
	} while (!feof(pFile));

	return buf;
}
