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

#ifndef _FSENUM_H_
#define _FSENUM_H_

#if defined(__cplusplus)
extern "C" {
#endif


typedef struct fs_enum_s fs_enum_t, *fs_enum_ptr;


fs_enum_ptr fse_create(const char * pszStartDir);
void fse_destroy(fs_enum_ptr pFSE);
int fse_next_file(fs_enum_ptr pFSE);
const char *fseFileName(fs_enum_ptr pFSE);
float fse_progress(fs_enum_ptr pFSE);


#if defined(__cplusplus)
}
#endif

#endif /* _fs_enum_H_ */
