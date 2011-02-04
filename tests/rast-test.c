#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "rmem.h"
#include "rjsobject.h"
#include "rvmcpu.h"
#include "rpadbex.h"
#include "rpaerror.h"
#include "rvmcodegen.h"
#include "rvmcodemap.h"
#include "rvmscope.h"
#include "rvmoperator.h"
#include "rgc.h"
#include "rastnode.h"


typedef struct rastcompiler_s {
	rpa_dbex_handle dbex;
	rgc_t *gc;
	rastnode_t *root;
	rint optimized;
} rastcompiler_t;

void r_astcompiler_loadrules(rastcompiler_t *aco);


static int debuginfo = 0;
static int parseinfo = 0;
static int verboseinfo = 0;
static int compileonly = 0;


rastcompiler_t *r_astcompiler_create()
{
	rastcompiler_t *aco;

	aco = r_malloc(sizeof(*aco));
	r_memset(aco, 0, sizeof(*aco));
	aco->gc = r_gc_create();
	return aco;
}


void r_astcompiler_destroy(rastcompiler_t *aco)
{
	if (aco) {
		r_object_destroy((robject_t*)aco->gc);
	}
	r_free(aco);
}


void r_astcompiler_dumptree(rastcompiler_t *aco)
{

}


void codegen_unmap_file(rstr_t *buf)
{
	if (buf) {
		munmap(buf->str, buf->size);
		r_free(buf);
	}
}


rstr_t *codegen_map_file(const char *filename)
{
	struct stat st;
	rstr_t *str;
	char *buffer;


	int fd = open(filename, O_RDONLY);
	if (fd < 0) {
		return (void*)0;
	}
	if (fstat(fd, &st) < 0) {
		close(fd);
		return (void*)0;
	}
	buffer = (char*)mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (buffer == (void*)-1) {
		close(fd);
		return (void*)0;
	}
	str = (rstr_t *)r_malloc(sizeof(*str));
	if (!str)
		goto error;
	r_memset(str, 0, sizeof(*str));
	str->str = buffer;
	str->size = st.st_size;
	close(fd);
	return str;

error:
	munmap(buffer, st.st_size);
	close(fd);
	return str;
}


inline int r_astcompiler_dumpnotification(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rastcompiler_t *aco = (rastcompiler_t *)userdata;

	fprintf(stdout, "%s: ", name);
	fprintf(stdout, "\n");
	return size;
}


inline int r_astcompiler_notify(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rastcompiler_t *aco = (rastcompiler_t *)userdata;

	if (parseinfo)
		r_astcompiler_dumpnotification(stat, name, userdata, input, size, reason);

	if (reason & RPA_REASON_START) {

	} else if (reason & RPA_REASON_MATCHED) {

	} else {

	}

	return size;
}


int main(int argc, char *argv[])
{
	int res, i;
	rstr_t *script = NULL, *unmapscript = NULL;
	rastcompiler_t *aco = r_astcompiler_create();
	aco->dbex = rpa_dbex_create();


	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-L") == 0) {
		} else if (r_strcmp(argv[i], "-d") == 0) {
			debuginfo = 1;
		} else if (r_strcmp(argv[i], "-p") == 0) {
			parseinfo = 1;
		} else if (r_strcmp(argv[i], "-P") == 0) {
			parseinfo = 1;
			verboseinfo = 1;
		} else if (r_strcmp(argv[i], "-c") == 0) {
			compileonly = 1;
		} else if (r_strcmp(argv[i], "-o") == 0) {
			aco->optimized = 1;
		} else if (r_strcmp(argv[i], "-m") == 0) {

		}
	}

	r_astcompiler_loadrules(aco);

	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-e") == 0) {
			if (++i < argc) {
				rstr_t script = { argv[i], r_strlen(argv[i]) };
				res = rpa_dbex_parse(aco->dbex, rpa_dbex_default_pattern(aco->dbex), script.str, script.str, script.str + script.size);
				if (res <= 0)
					goto end;
			}
			goto exec;
		}
	}

	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-f") == 0) {

			if (++i < argc) {
				script = codegen_map_file(argv[i]);
				if (script) {
					res = rpa_dbex_parse(aco->dbex, rpa_dbex_default_pattern(aco->dbex), script->str, script->str, script->str + script->size);
					unmapscript = script;
					if (res <= 0)
						goto end;
				}

			}
			goto exec;
		}
	}


exec:
	r_astcompiler_dumptree(aco);


end:
	if (unmapscript)
		codegen_unmap_file(unmapscript);
	rpa_dbex_destroy(aco->dbex);
	r_astcompiler_destroy(aco);

	if (debuginfo) {
		r_printf("Max alloc mem: %ld\n", r_debug_get_maxmem());
		r_printf("Leaked mem: %ld\n", r_debug_get_allocmem());
	}
	return 0;
}



extern char _binary_____________tests_ecma262_rpa_start[];
extern char _binary_____________tests_ecma262_rpa_end[];
extern unsigned long *_binary_____________tests_ecma262_rpa_size;

void r_astcompiler_loadrules(rastcompiler_t *aco)
{
	int ret, line;
	int inputsize = _binary_____________tests_ecma262_rpa_end - _binary_____________tests_ecma262_rpa_start;
	const char *buffer = _binary_____________tests_ecma262_rpa_start;
	const char *pattern = buffer;

	rpa_dbex_open(aco->dbex);
	rpa_dbex_add_callback(aco->dbex, ".*", RPA_REASON_ALL, r_astcompiler_notify, aco);

	while ((ret = rpa_dbex_load(aco->dbex, pattern, inputsize)) > 0) {
		inputsize -= ret;
		pattern += ret;
	}
	if (ret < 0) {
		for (line = 1; pattern >= buffer; --pattern) {
			if (*pattern == '\n')
				line += 1;
		}
		fprintf(stdout, "Line: %d, RPA LOAD ERROR: %s\n", line, (rpa_dbex_get_error(aco->dbex) == RPA_E_SYNTAX_ERROR) ? "Syntax Error." : "Pattern Loading failed.");
		goto error;
	}

error:
	rpa_dbex_close(aco->dbex);
}
