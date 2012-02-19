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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>

#include "rlib/rbuffer.h"
#include "rlib/rmem.h"
#include "rlib/rutf.h"
#include "rexgrep.h"
#include "rexgrepdep.h"
#include "fsenum.h"


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
	memset(str, 0, sizeof(*str));
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


void rex_grep_print_filename(rexgrep_t *pGrep)
{
	if (pGrep->filename) {
		rex_grep_output_utf8_string(pGrep, pGrep->filename);
		rex_grep_output_utf8_string(pGrep, ":");
	}
	
}


void rex_grep_scan_path(rexgrep_t *pGrep, const char *path)
{
	fs_enum_ptr pFSE;
	rbuffer_t *buf;

	if ((pFSE = fse_create(path))) {
    	while (fse_next_file(pFSE) == 0) {
    		pGrep->filename = (void*)fseFileName(pFSE);
    		buf = rex_buffer_map_file((const char*)pGrep->filename);
    		if (buf) {
				rex_grep_scan_buffer(pGrep, buf);
				pGrep->scsize += buf->size;
				r_buffer_destroy(buf);
			}
		} 
		fse_destroy(pFSE);
	} else {
		pGrep->filename = (void*)path;
		buf = rex_buffer_map_file((const char*)pGrep->filename);
		if (buf) {
			rex_grep_scan_buffer(pGrep, buf);
			pGrep->scsize += buf->size;
			r_buffer_destroy(buf);
		}
	}
	
	
}


void rex_grep_output_char(int c)
{
	int ret;
	int garbage = 0;
	unsigned char output[16];

	if ((unsigned int)c < 32 && !(c == '\t' || c == '\n' || c == '\r')) {
		c = garbage;
	} else if (c >= 127 && c <= 255) {
		c = garbage;
	}

	ret = r_utf8_wctomb(c, output, sizeof(output));
	if (ret > 0) {
		fwrite(output, 1, ret, stdout);
	}
	return;
}
