#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "rstring.h"
#include "rmem.h"
#include "rjs.h"
#include "rparecord.h"


static int debuginfo = 0;
static int parseinfo = 0;
static int compileonly = 0;
static int debugcompileonly = 0;
static int statinfo = 0;


static rchar *errormsg[] = {
	"OK",
	"Undefined identifier",
	"Syntax error",
	"Not a function",
	"Not a function call",
	"Not a loop",
	"Not a if statement",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
};

void rjs_unmap_file(rstr_t *buf)
{
	if (buf) {
		munmap(buf->str, buf->size);
		r_free(buf);
	}
}


rstr_t *rjs_map_file(const char *filename)
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


int rjs_exec_script(rjs_engine_t *jse, rstr_t  *script)
{
	if (!script)
		return -1;
	if (parseinfo) {
		rjs_engine_dumpast(jse, script->str, script->size);
	} else if (debugcompileonly) {
		int res = 0;
		jse->debugcompile = 1;
		res = rjs_engine_compile(jse, script->str, script->size);
		jse->debugcompile = 0;
		if (res < 0)
			return -1;
	} else if (compileonly) {
		if (rjs_engine_compile(jse, script->str, script->size) < 0)
			return -1;
	} else {
		if (rjs_engine_compile(jse, script->str, script->size) < 0)
			return -1;
		if (rjs_engine_run(jse) < 0)
			return -1;
	}

	return 0;
}


rlong jrs_offset2line(const rchar *script, rlong offset)
{
	rlong line = 0;
	const rchar *ptr;

	for (line = 1, ptr = script + offset; ptr >= script; --ptr) {
		if (*ptr == '\n')
			line += 1;
	}

	return line;
}


void rjs_display_errors(rjs_engine_t *jse, rstr_t *script)
{
	rlong line = 0;
	rlong i;
	rjs_coerror_t *err;

	for (i = 0; i < r_array_length(jse->co->errors); i++) {
		err = (rjs_coerror_t *)r_array_slot(jse->co->errors, i);
		line = jrs_offset2line(script->str, (rlong)(err->script - script->str));
		fprintf(stdout, "Line: %ld (%ld, %ld), Error Code: %ld, ", (rlong)line, (rlong)(err->script - script->str), (rlong)err->scriptsize, err->code);
		fprintf(stdout, "%s: ", errormsg[err->code]);
		fwrite(err->script, sizeof(rchar), err->scriptsize, stdout);
		fprintf(stdout, "\n");
	}
}


int main(int argc, char *argv[])
{
	rint i;
	rstr_t *script = NULL, *unmapscript = NULL;
	rstr_t line;
	rjs_engine_t *jse;

	jse = rjs_engine_create();
	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-L") == 0) {

		} else if (r_strcmp(argv[i], "-d") == 0) {
			debuginfo = 1;
			jse->debugexec = 1;
		} else if (r_strcmp(argv[i], "-p") == 0) {
			parseinfo = 1;
		} else if (r_strcmp(argv[i], "-C") == 0) {
			debugcompileonly = 1;
		} else if (r_strcmp(argv[i], "-c") == 0) {
			compileonly = 1;
		} else if (r_strcmp(argv[i], "-t") == 0) {
			statinfo = 1;
		}
	}

	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-e") == 0) {
			if (++i < argc) {
				line.str = argv[i];
				line.size = r_strlen(argv[i]);
				script = &line;
			}
			if (rjs_exec_script(jse, script) < 0)
				goto end;
		}
	}

	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-f") == 0) {
			if (++i < argc) {
				script = rjs_map_file(argv[i]);
				if (script) {
					unmapscript = script;
				}
			}
			if (rjs_exec_script(jse, script) < 0)
				goto end;
		}
	}

end:
	if (jse->co && r_array_length(jse->co->errors))
		rjs_display_errors(jse, script);
	rjs_engine_destroy(jse);
	if (unmapscript)
		rjs_unmap_file(unmapscript);
	if (statinfo)
		fprintf(stdout, "\nRJS Version: %s, memory: %ld Bytes (leaked %ld Bytes)\n", rjs_version(), (rlong)r_debug_get_maxmem(), (rlong)r_debug_get_allocmem());
	return 0;
}
