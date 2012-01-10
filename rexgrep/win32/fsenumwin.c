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

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <tchar.h>
#include "fsenumwin.h"

#define SZ_SLASH _T("\\")
#define CH_SLASH _T('\\')
#define MAX_LEVELS (MAX_PATH / 2)

struct _fs_enum
{
    WIN32_FIND_DATA FD;
    TCHAR szCurDir[MAX_PATH];
    TCHAR szItemName[MAX_PATH];
    INT iFindOffset;
    INT iFilterOffset;
    HANDLE khFind[MAX_LEVELS];
};


static VOID fseCurDirCatFilter(fs_enum_ptr pFSE, LPCTSTR pszFilter);
static VOID fseCurDirRmFilter(fs_enum_ptr pFSE);
static BOOL fsePushDir(fs_enum_ptr pFSE, LPCTSTR pszDir);
static BOOL fsePopDir(fs_enum_ptr pFSE);
static BOOL fseDotDir(LPCTSTR pszDirName);


fs_enum_ptr fse_create(LPCTSTR pszStartDir)
{
    fs_enum_ptr pFSE;
	DWORD attr = GetFileAttributes(pszStartDir);

	if (attr == INVALID_FILE_ATTRIBUTES)
		return NULL;
	if ((attr & FILE_ATTRIBUTE_DIRECTORY) == 0)
		return NULL;
    if ((pFSE = (fs_enum_ptr)malloc(sizeof(*pFSE))) == NULL)
        return NULL;
    memset(pFSE, 0, sizeof(*pFSE));
    pFSE->iFindOffset = -1;
    pFSE->iFilterOffset = -1;
    fsePushDir(pFSE, pszStartDir ? pszStartDir : SZ_SLASH);
    
    return pFSE;
}


VOID fse_destroy(fs_enum_ptr pFSE)
{
    if (pFSE == NULL)
        return;

    while (pFSE->iFindOffset >= 0)
        FindClose(pFSE->khFind[pFSE->iFindOffset--]);

    free(pFSE);
}


BOOL fse_next_file(fs_enum_ptr pFSE)
{
    BOOL fRet = FALSE;
    HANDLE hFindHandle;

    while (fRet == FALSE)
    {
        if (pFSE->khFind[pFSE->iFindOffset] == NULL)
        {
            fseCurDirCatFilter(pFSE, _T("*"));
            hFindHandle = FindFirstFile(pFSE->szCurDir, &pFSE->FD);
            fseCurDirRmFilter(pFSE);

            if (hFindHandle != INVALID_HANDLE_VALUE)
            {
                pFSE->khFind[pFSE->iFindOffset] = hFindHandle;
                fRet = TRUE;
            }
        }
        else
        {
            fRet = FindNextFile(pFSE->khFind[pFSE->iFindOffset], &pFSE->FD);
        }

        if (fRet)
        {
            if (pFSE->FD.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (!fseDotDir(pFSE->FD.cFileName))
                    fsePushDir(pFSE, pFSE->FD.cFileName);
                fRet = FALSE;
                continue;
            }
        }
        else
        {
            if (!fsePopDir(pFSE))
                return FALSE;
            continue;
        }
    }

    _tcscpy(pFSE->szItemName, pFSE->szCurDir);
    _tcscat(pFSE->szItemName, pFSE->FD.cFileName);

    return fRet;
}


LPCTSTR fseFileName(fs_enum_ptr pFSE)
{
    return pFSE->szItemName;
}


static BOOL fseDotDir(LPCTSTR pszDirName)
{
    if (pszDirName[0] != '.' && (pszDirName[1] != '.' || pszDirName[1] != '\0'))
        return FALSE;

    return TRUE;
}


static VOID fseCurDirCatFilter(fs_enum_ptr pFSE, LPCTSTR pszFilter)
{
    pFSE->iFilterOffset = (INT) _tcslen(pFSE->szCurDir);
    _tcscat(pFSE->szCurDir, pszFilter);
}


static VOID fseCurDirRmFilter(fs_enum_ptr pFSE)
{
    if (pFSE->iFilterOffset >= 0)
        pFSE->szCurDir[pFSE->iFilterOffset] = _T('\0');

    pFSE->iFilterOffset = -1;
}   


static BOOL fsePushDir(fs_enum_ptr pFSE, LPCTSTR pszDir)
{
    if (pFSE->iFindOffset >= (int) (sizeof(pFSE->khFind)/sizeof(pFSE->khFind[0]) - 1) ||
        _tcslen(pszDir) >= sizeof(pFSE->szCurDir)/sizeof(pFSE->szCurDir[0]) - _tcslen(pFSE->szCurDir) )
    {
        /* No more space */
        return FALSE;
    }

    pFSE->iFindOffset++;
    pFSE->khFind[pFSE->iFindOffset] = NULL;
    _tcscat(pFSE->szCurDir, pszDir);
    if (pFSE->szCurDir[_tcslen(pFSE->szCurDir)-1] != CH_SLASH)
        _tcscat(pFSE->szCurDir, SZ_SLASH);

    return TRUE;
}


static BOOL fsePopDir(fs_enum_ptr pFSE)
{
    LPTSTR pszEnd;

    if (pFSE->iFindOffset < 0)
        return FALSE;

    if (pFSE->khFind[pFSE->iFindOffset])
        FindClose(pFSE->khFind[pFSE->iFindOffset]); 
    pFSE->khFind[pFSE->iFindOffset] = NULL;
    pFSE->iFindOffset--;

    pszEnd = pFSE->szCurDir + _tcslen(pFSE->szCurDir) - 1;
    while (pszEnd >= pFSE->szCurDir)
    {
        *pszEnd = _T('\0');
        if (pFSE->iFindOffset >= 0)
        {
            if (*(pszEnd - 1) == CH_SLASH)
                break;
        }

        pszEnd--;
    }

    return TRUE;
}
