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
	rlong res;
	rstr_t *script = NULL, *unmapscript = NULL;
	rjs_parser_t *parser = rjs_parser_create();
	rarray_t *records = 0;

	if (!parser)
		return 1;
	r_printf("RJS Version: %s\n", rjs_version());

	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-e") == 0) {
			if (++i < argc) {
				rstr_t script = { argv[i], r_strlen(argv[i]) };
				res = rjs_parser_exec(parser, script.str, script.size, &records);
			}
			goto exec;
		}
	}

	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-f") == 0) {
			if (++i < argc) {
				script = rjs_map_file(argv[i]);
				if (script) {
					res = rjs_parser_exec(parser, script->str, script->size, &records);;
					unmapscript = script;
				}
			}
			goto exec;
		}
	}

exec:
	if (records) {
		rlong i;
		for (i = 0; i < r_array_length(records); i++)
			rpa_record_dump(records, i);
	}
	rjs_parser_destroy(parser);
	if (unmapscript)
		rjs_unmap_file(unmapscript);
	return 0;
}
