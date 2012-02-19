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

#ifndef _fs_enumWIN_H_
#define _fs_enumWIN_H_

#include <windows.h>
#include <tchar.h>

#if defined(__cplusplus)
extern "C" {
#endif


typedef struct _fs_enum fs_enum, *fs_enum_ptr;


fs_enum_ptr fse_create(LPCTSTR pszStartDir);
VOID fse_destroy(fs_enum_ptr pFSE);
BOOL fse_next_file(fs_enum_ptr pFSE);
LPCTSTR fseFileName(fs_enum_ptr pFSE);


#if defined(__cplusplus)
}
#endif

#endif /* _fs_enum_H_ */
