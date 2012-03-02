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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "rlib/rmem.h"
#include "rexccdep.h"


void rex_buffer_unmap_file(rbuffer_t *buf)
{
	if (buf) {
		munmap(buf->s, buf->size);
		r_free(buf);
	}
}


rbuffer_t * rex_buffer_map_file(const char *filename)
{
	struct stat st;
	rbuffer_t *str;
	char *buffer;

	int fd = open(filename, O_RDONLY);
	if (fd < 0) {
		return (void*)0;
	}
	if (fstat(fd, &st) < 0) {
		close(fd);
		return (void*)0;
	}
	buffer = (char*)mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (buffer == (void*)-1) {
		close(fd);
		return (void*)0;
	}
	str = (rbuffer_t *)r_malloc(sizeof(rbuffer_t));
	if (!str)
		goto error;
	r_memset(str, 0, sizeof(*str));
	str->s = buffer;
	str->size = st.st_size;
	str->userdata = (void*)((unsigned long)fd);
	str->alt_destroy = rex_buffer_unmap_file;
	close(fd);
	return str;

error:
	munmap(buffer, st.st_size);
	close(fd);
	return str;
}
