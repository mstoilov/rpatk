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
