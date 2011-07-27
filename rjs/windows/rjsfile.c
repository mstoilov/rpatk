#include <windows.h>

#include "rlib/rmem.h"
#include "rjs/rjsfile.h"

void rjs_file_unmap(rstr_t *buf)
{
	if (buf) {
		r_free(buf->str);
		r_free(buf);
	}
}


rstr_t *rjs_file_map(const char *filename)
{
	rstr_t *buf = NULL;
	char *pMappedView = NULL;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	HANDLE hMapping = 0;
	unsigned __int64 fileSize;
	DWORD sizeLo, sizeHi;

	buf = (rstr_t*)r_malloc(sizeof(*buf));
	if (!buf)
		return NULL;
	hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, 0);
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
	buf->size = (unsigned long)fileSize;
	buf->str = (char*)r_zmalloc((unsigned long)fileSize + 1);
	if (!buf->str)
		goto error;
	r_memcpy(buf->str, pMappedView, (unsigned long)fileSize);
	if (hMapping)
		CloseHandle(hMapping);
	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);
	if (pMappedView)
		UnmapViewOfFile(pMappedView);
	return buf;

error:
	if (hMapping)
		CloseHandle(hMapping);
	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);
	if (pMappedView)
		UnmapViewOfFile(pMappedView);
	r_free(buf->str);
	r_free(buf);
	return NULL;
}
