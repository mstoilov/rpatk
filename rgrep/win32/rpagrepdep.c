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

#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "rpagrep.h"
#include "rpagrepdep.h"
#include "fsenumwin.h"
#include "rpagreputf.h"


void rpa_buffer_unmap_file(rpa_buffer_t *buf)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	HANDLE hMapping = 0;

	if (!buf)
		return;
	hFile = (HANDLE) buf->userdata;
	hMapping = (HANDLE) buf->userdata1;

	if (hMapping)
		CloseHandle(hMapping);
	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);
	if (buf->s)
		UnmapViewOfFile(buf->s);
	free(buf);
}


rpa_buffer_t * rpa_buffer_map_file(const wchar_t *pFileName)
{
	rpa_buffer_t *buf;
	char *pMappedView = 0;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	HANDLE hMapping = 0;
	unsigned __int64 fileSize;
	DWORD sizeLo, sizeHi;

	buf = (rpa_buffer_t *)malloc(sizeof(rpa_buffer_t));
	if (!buf)
		goto error;
	hFile = CreateFile(pFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, 0);
	if (INVALID_HANDLE_VALUE == hFile)
		goto error;
	sizeLo = GetFileSize(hFile, &sizeHi);
	fileSize = (UINT64)sizeLo | ((UINT64)sizeHi << 32);
	hMapping = CreateFileMapping(hFile, 0, PAGE_READONLY, sizeHi, sizeLo, 0);
	if (!hMapping)
		goto error;
	pMappedView = (char*)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
	if (NULL == pMappedView)
		goto error;
	buf->userdata = (void*)hFile;
	buf->userdata1 = (void*)hMapping;
	buf->s = pMappedView;
	buf->size = (unsigned long)fileSize;
	buf->destroy = rpa_buffer_unmap_file;
	return buf;

error:
	if (hMapping)
		CloseHandle(hMapping);
	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);
	if (pMappedView)
		UnmapViewOfFile(pMappedView);
	free(buf);
	return (rpa_buffer_t*)0;
}


rpa_buffer_t * rpa_buffer_from_wchar(const wchar_t *wstr)
{
	rpa_buffer_t *buf;
	int ret;
	int wideLen = (int)wcslen(wstr) + 1;
	int sizeNeeded = WideCharToMultiByte(
		CP_UTF8,
		0,
		wstr,
		wideLen,
		NULL,
		0,
		NULL,
		NULL);
	if (!(buf = rpa_buffer_alloc(sizeNeeded)))
		return (rpa_buffer_t*)0;
	ret = WideCharToMultiByte(
				CP_UTF8,
				0,
				wstr,
				wideLen,
				buf->s,
				sizeNeeded,
				NULL,
				NULL);
	if (ret <= 0) {
		rpa_buffer_destroy(buf);
		return NULL;
	}
	return buf;

}


void rpa_grep_print_filename(rpa_grep_t *pGrep)
{
	if (pGrep->filename) {
		rpa_grep_output_utf16_string(pGrep, pGrep->filename);
		rpa_grep_output_utf16_string(pGrep, L":\n");
	}
	
}


void rpa_grep_scan_path(rpa_grep_t *pGrep, const wchar_t *path)
{
	fs_enum_ptr pFSE;
	rpa_buffer_t *buf;

	if ((pFSE = fse_create(path))) {
    	while (fse_next_file(pFSE)) {
    		pGrep->filename = (void*)fseFileName(pFSE);
    		buf = rpa_buffer_map_file((const wchar_t*)pGrep->filename);
    		if (buf) {
				rpa_grep_scan_buffer(pGrep, buf);
				pGrep->scsize += buf->size;
				rpa_buffer_destroy(buf);
			}
		} 
		fse_destroy(pFSE);
	} else {
		pGrep->filename = (void*)path;
		buf = rpa_buffer_map_file((const wchar_t*)pGrep->filename);
		if (buf) {
			rpa_grep_scan_buffer(pGrep, buf);
			pGrep->scsize += buf->size;
			rpa_buffer_destroy(buf);
		}
	}
}


void rpa_grep_output_char(int c)
{
	int ret;
	int garbage = 0;
	wchar_t output[3];

	if ((unsigned char)c < 32 && !(c == '\t' || c == '\n' || c == '\r')) {
		c = garbage;
	} else if (c >= 127 && c <= 255) {
		c = garbage;
	}

	output[0] = output[1] = output[2] = 0;
	ret = rpa_grep_utf16_wctomb(c, (unsigned char*)output, sizeof(output));
	if (ret > 0) {
		fwprintf(stdout, L"%s", output);
	}
	return;
}
