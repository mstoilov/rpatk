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

#include "rlib/rbuffer.h"
#include "rlib/rmem.h"



void r_buffer_destroy(rbuffer_t *buffer)
{
	if (buffer) {
		if (buffer->alt_destroy) {
			buffer->alt_destroy(buffer);
			return;
		} else {
			r_free(buffer->s);
			r_free(buffer);
		}
	}
}


rbuffer_t *r_buffer_create(unsigned long size)
{
	rbuffer_t *buffer;

	buffer = (rbuffer_t *)r_malloc(sizeof(rbuffer_t));
	if (!buffer)
		return (void*)0;
	r_memset(buffer, 0, sizeof(*buffer));
	if (!(buffer->s = (char *)r_malloc((size + 1) * sizeof(char)))) {
		r_free(buffer);
		return (void*)0;
	}
	r_memset(buffer->s, 0, size + 1);
	buffer->size = size;
	return buffer;
}


int r_buffer_append(rbuffer_t *buffer, void *src, unsigned long size)
{
	char *s;

	s = (char *)r_realloc(buffer->s, size + 1);
	if (!s)
		return -1;
	buffer->s = s;
	buffer->size = size;
	r_memcpy(buffer->s, src, size);
	buffer->s[buffer->size] = '\0';
	return 0;
}


#if 0
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

#endif
