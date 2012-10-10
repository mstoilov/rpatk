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
#include <tchar.h>
#include "rlib/rbuffer.h"
#include "rexccdep.h"


void rex_buffer_unmap_file(rbuffer_t *buf)
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


rbuffer_t * rex_buffer_map_file(LPCTSTR pFileName)
{
	rbuffer_t *buf;
	char *pMappedView = 0;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	HANDLE hMapping = 0;
	unsigned __int64 fileSize;
	DWORD sizeLo, sizeHi;

	buf = (rbuffer_t *)malloc(sizeof(rbuffer_t));
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
	buf->alt_destroy = rex_buffer_unmap_file;
	return buf;

error:
	if (hMapping)
		CloseHandle(hMapping);
	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);
	if (pMappedView)
		UnmapViewOfFile(pMappedView);
	free(buf);
	return (rbuffer_t*)0;
}

