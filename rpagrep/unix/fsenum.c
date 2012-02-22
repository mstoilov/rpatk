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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "fsenum.h"

#define SZ_SLASH "/"
#define CH_SLASH '/'
#define MAX_LEVELS (MAX_PATH / 2)
#define MAX_PATH 4096

struct fs_enum_s
{
	char szCurDir[MAX_PATH];
	char szFileName[MAX_PATH];
	int iDirOffset;
	DIR *kDIRS[MAX_LEVELS];
	double fProgress;
	double kProgSlice[MAX_LEVELS];
};

static int fsePushDir(fs_enum_ptr pFSE, const char *pszDir);
static int fsePopDir(fs_enum_ptr pFSE);
static int fseDotDir(const char * pszDirName);
static void fseCurDirRmItem(fs_enum_ptr pFSE, int iItemOffset);
static int fseCurDirCatItem(fs_enum_ptr pFSE, const char *pszItem);


fs_enum_ptr fse_create(const char * pszStartDir)
{
	fs_enum_ptr pFSE;
	struct stat st;
	
	memset(&st, 0, sizeof(st));
	if (lstat(pszStartDir, &st) < 0)
		return NULL;
	st.st_mode &= S_IFMT;	
	if (st.st_mode != S_IFDIR)
		return NULL;
	if ((pFSE = (fs_enum_ptr)malloc(sizeof(*pFSE))) == NULL)
		return NULL;
	memset(pFSE, 0, sizeof(*pFSE));
	pFSE->iDirOffset = -1;
	fsePushDir(pFSE, pszStartDir ? pszStartDir : SZ_SLASH);
	
	return pFSE;
}


void fse_destroy(fs_enum_ptr pFSE)
{
	if (pFSE == NULL)
		return;

	while (fsePopDir(pFSE) == 0)
		;

	free((void*) pFSE);
}


int fse_next_file(fs_enum_ptr pFSE)
{
	int fRet = -1;
	int iItemOffset = 0;
	int iTmp;
	struct stat st;
	struct dirent *pDE;

	while (fRet)
	{
		if ((pDE = readdir(pFSE->kDIRS[pFSE->iDirOffset])) == NULL)
		{
			if (fsePopDir(pFSE) < 0)
				return -1;
			continue;
		}

		if ((iItemOffset = fseCurDirCatItem(pFSE, pDE->d_name)) < 0)
			continue;
		iTmp = lstat(pFSE->szCurDir, &st);
		fseCurDirRmItem(pFSE, iItemOffset);
		if (iTmp < 0)
			continue;
		st.st_mode &= S_IFMT;
		if (st.st_mode == S_IFREG)
		{
			strcpy(pFSE->szFileName, pFSE->szCurDir);
			strcat(pFSE->szFileName, pDE->d_name);
			pFSE->fProgress += pFSE->kProgSlice[pFSE->iDirOffset];
			fRet = 0;
		}
		else if (st.st_mode == S_IFDIR && !fseDotDir(pDE->d_name))
		{
			fsePushDir(pFSE, pDE->d_name);
		}
	}

	return fRet;
}


static int fseCurDirCatItem(fs_enum_ptr pFSE, const char *pszItem)
{
	int iItemOffset = strlen(pFSE->szCurDir);

	if (strlen(pszItem) >= sizeof(pFSE->szCurDir) - iItemOffset)
		return -1;
	strcat(pFSE->szCurDir, pszItem);
	return iItemOffset;
}


static void fseCurDirRmItem(fs_enum_ptr pFSE, int iItemOffset)
{
	if (iItemOffset >= 0)
		pFSE->szCurDir[iItemOffset] = '\0';
}


const char *fseFileName(fs_enum_ptr pFSE)
{
	return pFSE->szFileName;
}


float fse_progress(fs_enum_ptr pFSE)
{
	return pFSE->fProgress;
}


static int fseDotDir(const char * pszDirName)
{
	if (pszDirName[0] != '.' && (pszDirName[1] != '.' || pszDirName[1] != '\0'))
		return 0;
	return 1;
}


static int fsePushDir(fs_enum_ptr pFSE, const char *pszDir)
{
	int iItemOffset;
	int iCount;
	int iTmp;
	struct stat st;
	struct dirent *pDE;
	float fCurSlice = (pFSE->iDirOffset < 0) ? 100.0 : pFSE->kProgSlice[pFSE->iDirOffset];

	if (pFSE->iDirOffset >= (int) (sizeof(pFSE->kDIRS)/sizeof(pFSE->kDIRS[0]) - 1) ||
		strlen(pszDir) >= sizeof(pFSE->szCurDir)/sizeof(pFSE->szCurDir[0]) - strlen(pFSE->szCurDir) )
	{
		/* No more space */
		return -1;
	}

	if ((iItemOffset = fseCurDirCatItem(pFSE, pszDir)) < 0)
		return -1;

	if (pszDir[strlen(pszDir)-1] != CH_SLASH)
		strcat(pFSE->szCurDir, SZ_SLASH);

	pFSE->iDirOffset++;
	if ((pFSE->kDIRS[pFSE->iDirOffset] = opendir(pFSE->szCurDir)) == NULL)
	{
		fseCurDirRmItem(pFSE, iItemOffset);
		pFSE->iDirOffset--;
		return -1;
	}

	for (iCount = 0; (pDE = readdir(pFSE->kDIRS[pFSE->iDirOffset])) != NULL;)
	{
		if ((iItemOffset = fseCurDirCatItem(pFSE, pDE->d_name)) < 0)
			continue;
		iTmp = lstat(pFSE->szCurDir, &st);
		fseCurDirRmItem(pFSE, iItemOffset);
		if (iTmp < 0)
			continue;
		st.st_mode &= S_IFMT;
		if (st.st_mode == S_IFREG || (st.st_mode == S_IFDIR	 && !fseDotDir(pDE->d_name)))
			iCount++;
	}

	pFSE->kProgSlice[pFSE->iDirOffset] = 0;
	if (iCount)
		pFSE->kProgSlice[pFSE->iDirOffset] = fCurSlice / iCount;
	else
		pFSE->fProgress += fCurSlice;

	rewinddir(pFSE->kDIRS[pFSE->iDirOffset]);

	return 0;
}


static int fsePopDir(fs_enum_ptr pFSE)
{
	char *pszEnd;

	if (pFSE->iDirOffset < 0)
		return -1;

	pszEnd = pFSE->szCurDir + strlen(pFSE->szCurDir) - 1;
	while (pszEnd >= pFSE->szCurDir)
	{
		*pszEnd = '\0';
		if (pFSE->iDirOffset >= 0)
		{
			if (*(pszEnd - 1) == CH_SLASH)
				break;
		}

		pszEnd--;
	}

	if (pFSE->kDIRS[pFSE->iDirOffset])
		closedir(pFSE->kDIRS[pFSE->iDirOffset]); 
	pFSE->kDIRS[pFSE->iDirOffset] = NULL;
	pFSE->iDirOffset--;

	return (pFSE->iDirOffset >= 0) ? 0 : -1;
}
