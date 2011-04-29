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


int main(int argc, char *argv[])
{
	rint i;
	rstr_t *script = NULL, *unmapscript = NULL;
	rstr_t line;
	rjs_engine_t *jse = rjs_engine_create();

	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-L") == 0) {

		} else if (r_strcmp(argv[i], "-d") == 0) {
			debuginfo = 1;
		} else if (r_strcmp(argv[i], "-p") == 0) {
			parseinfo = 1;
		} else if (r_strcmp(argv[i], "-C") == 0) {
			debugcompileonly = 1;
		} else if (r_strcmp(argv[i], "-c") == 0) {
			compileonly = 1;
		} else if (r_strcmp(argv[i], "-m") == 0) {

		}
	}


	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-e") == 0) {
			if (++i < argc) {
				line.str = argv[i];
				line.size = r_strlen(argv[i]);
				script = &line;
			}
			goto exec;
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
			goto exec;
		}
	}

exec:
	if (!script)
		goto end;
	if (parseinfo) {
		rjs_engine_dumpast(jse, script->str, script->size);
	} else if (debugcompileonly) {
		jse->debugcompile = 1;
		rjs_engine_compile(jse, script->str, script->size);
		jse->debugcompile = 0;
	} else if (compileonly) {
		rjs_engine_compile(jse, script->str, script->size);
	} else {
		rjs_engine_compile(jse, script->str, script->size);
		if (debuginfo)
			jse->debugexec = 1;
		rjs_engine_run(jse);
	}

end:
	rjs_engine_destroy(jse);
	if (unmapscript)
		rjs_unmap_file(unmapscript);
	if (debuginfo)
		fprintf(stdout, "\nRJS Version: %s, memory: %ld KB (leaked %ld Bytes)\n", rjs_version(), (rlong)r_debug_get_maxmem()/1000, (rlong)r_debug_get_allocmem());
	return 0;
}
