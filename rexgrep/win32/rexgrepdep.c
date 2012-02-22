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

#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "rexgrep.h"
#include "rexgrepdep.h"
#include "fsenumwin.h"


void rex_buffer_unmap_file(rex_buffer_t *buf)
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


rex_buffer_t * rex_buffer_map_file(const wchar_t *pFileName)
{
	rex_buffer_t *buf;
	char *pMappedView = 0;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	HANDLE hMapping = 0;
	unsigned __int64 fileSize;
	DWORD sizeLo, sizeHi;

	buf = (rex_buffer_t *)malloc(sizeof(rex_buffer_t));
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
	buf->destroy = rex_buffer_unmap_file;
	return buf;

error:
	if (hMapping)
		CloseHandle(hMapping);
	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);
	if (pMappedView)
		UnmapViewOfFile(pMappedView);
	free(buf);
	return (rex_buffer_t*)0;
}


rex_buffer_t * rex_buffer_from_wchar(const wchar_t *wstr)
{
	rex_buffer_t *buf;
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
	if (!(buf = rex_buffer_alloc(sizeNeeded)))
		return (rex_buffer_t*)0;
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
		rex_buffer_destroy(buf);
		return NULL;
	}
	return buf;

}


void rex_grep_print_filename(rexgrep_t *pGrep)
{
	if (pGrep->filename) {
		rex_grep_output_utf16_string(pGrep, pGrep->filename);
		rex_grep_output_utf16_string(pGrep, L":\n");
	}
	
}


void rex_grep_scan_path(rexgrep_t *pGrep, const wchar_t *path)
{
	fs_enum_ptr pFSE;
	rex_buffer_t *buf;

	if ((pFSE = fse_create(path))) {
    	while (fse_next_file(pFSE)) {
    		pGrep->filename = (void*)fseFileName(pFSE);
    		buf = rex_buffer_map_file((const wchar_t*)pGrep->filename);
    		if (buf) {
				rex_grep_scan_buffer(pGrep, buf);
				pGrep->scsize += buf->size;
				rex_buffer_destroy(buf);
			}
		} 
		fse_destroy(pFSE);
	} else {
		pGrep->filename = (void*)path;
		buf = rex_buffer_map_file((const wchar_t*)pGrep->filename);
		if (buf) {
			rex_grep_scan_buffer(pGrep, buf);
			pGrep->scsize += buf->size;
			rex_buffer_destroy(buf);
		}
	}
}


void rex_grep_output_char(int c)
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
	ret = rex_grep_utf16_wctomb(c, (unsigned char*)output, sizeof(output));
	if (ret > 0) {
		fwprintf(stdout, L"%s", output);
	}
	return;
}
