#ifndef _RPAGREPDEP_H_
#define _RPAGREPDEP_H_


#ifdef __cplusplus
extern "C" {
#endif

rpa_buffer_t * rpa_buffer_map_file(const wchar_t *filename);
void rpa_grep_scan_path(rpa_grep_t *pGrep, const wchar_t *path);


#ifdef __cplusplus
}
#endif

#endif
