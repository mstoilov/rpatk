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


static void rjs_exec_ltrim(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	const rchar *ptr, *list;
	rsize_t size;
	rvmreg_t *r = NULL, *l = NULL;
	rstring_t *src, *dest;

	if (RJS_SWI_PARAMS(cpu) == 0)
		RJS_SWI_ABORT(rjs_engine_get(cpu), NULL);
	r = (rvmreg_t *) RJS_SWI_PARAM(cpu, 1);
	if (RJS_SWI_PARAMS(cpu) > 1) {
		l = (rvmreg_t *) RJS_SWI_PARAM(cpu, 2);
		if (rvm_reg_gettype(l) != RVM_DTYPE_STRING)
			RJS_SWI_ABORT(rjs_engine_get(cpu), NULL);
	}
	if (rvm_reg_gettype(r) != RVM_DTYPE_STRING)
		RJS_SWI_ABORT(rjs_engine_get(cpu), NULL);
	if (l)
		list = ((rstring_t *)RVM_REG_GETP(l))->s.str;
	else
		list = " \t\n\r\0";
	src = (rstring_t *)RVM_REG_GETP(r);
	ptr = src->s.str;
	size = src->s.size;
	while (size > 0) {
		if (!r_strchr(list, *ptr))
			break;
		size--;
		ptr++;
	}
	dest = r_string_create_strsize(ptr, size);
	r_gc_add(cpu->gc, (robject_t*)dest);
	rvm_reg_setstring(RVM_CPUREG_PTR(cpu, R0), dest);
}


static void rjs_exec_rtrim(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	const rchar *ptr, *list;
	rsize_t size;
	rvmreg_t *r = NULL, *l = NULL;
	rstring_t *src, *dest;

	if (RJS_SWI_PARAMS(cpu) == 0)
		RJS_SWI_ABORT(rjs_engine_get(cpu), NULL);
	r = (rvmreg_t *) RJS_SWI_PARAM(cpu, 1);
	if (RJS_SWI_PARAMS(cpu) > 1) {
		l = (rvmreg_t *) RJS_SWI_PARAM(cpu, 2);
		if (rvm_reg_gettype(l) != RVM_DTYPE_STRING)
			RJS_SWI_ABORT(rjs_engine_get(cpu), NULL);
	}
	if (rvm_reg_gettype(r) != RVM_DTYPE_STRING)
		RJS_SWI_ABORT(rjs_engine_get(cpu), NULL);
	if (l)
		list = ((rstring_t *)RVM_REG_GETP(l))->s.str;
	else
		list = " \t\n\r\0";
	src = (rstring_t *)RVM_REG_GETP(r);
	size = src->s.size;
	ptr = src->s.str + size - 1;
	while (size > 0) {
		if (!r_strchr(list, *ptr))
			break;
		size--;
		ptr--;
	}
	dest = r_string_create_strsize(src->s.str, size);
	r_gc_add(cpu->gc, (robject_t*)dest);
	rvm_reg_setstring(RVM_CPUREG_PTR(cpu, R0), dest);
}


static void rjs_exec_trim(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	const rchar *start, *ptr, *list;
	rsize_t size;
	rvmreg_t *r = NULL, *l = NULL;
	rstring_t *src, *dest;

	if (RJS_SWI_PARAMS(cpu) == 0)
		RJS_SWI_ABORT(rjs_engine_get(cpu), NULL);

	r = (rvmreg_t *) RJS_SWI_PARAM(cpu, 1);
	if (RJS_SWI_PARAMS(cpu) > 1) {
		l = (rvmreg_t *) RJS_SWI_PARAM(cpu, 2);
		if (rvm_reg_gettype(l) != RVM_DTYPE_STRING)
			RJS_SWI_ABORT(rjs_engine_get(cpu), NULL);
	}
	if (rvm_reg_gettype(r) != RVM_DTYPE_STRING)
		RJS_SWI_ABORT(rjs_engine_get(cpu), NULL);
	if (l)
		list = ((rstring_t *)RVM_REG_GETP(l))->s.str;
	else
		list = " \t\n\r\0";
	src = (rstring_t *)RVM_REG_GETP(r);
	ptr = src->s.str;
	size = src->s.size;
	while (size > 0) {
		if (!r_strchr(list, *ptr))
			break;
		size--;
		ptr++;
	}
	start = ptr;
	ptr = start + size - 1;
	while (size > 0) {
		if (!r_strchr(list, *ptr))
			break;
		size--;
		ptr--;
	}
	dest = r_string_create_strsize(start, size);
	r_gc_add(cpu->gc, (robject_t*)dest);
	rvm_reg_setstring(RVM_CPUREG_PTR(cpu, R0), dest);
}


static rvm_switable_t swistring[] = {
		{"ltrim", rjs_exec_ltrim},
		{"rtrim", rjs_exec_rtrim},
		{"trim", rjs_exec_trim},
		{NULL, NULL},
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
	rlong i;
	rjs_error_t *err;

	for (i = 0; i < r_array_length(jse->errors); i++) {
		err = (rjs_error_t *)r_array_slot(jse->errors, i);
		fprintf(stdout, "Line: %ld (%ld, %ld), Error Code: %ld, ", (rlong)err->line, err->offset, err->lineoffset, err->error);
		fprintf(stdout, "%s: ", errormsg[err->error]);
		if (err->size) {
			fwrite(script->str + err->offset, sizeof(rchar), err->size, stdout);
		} else {
			fwrite(script->str + err->lineoffset, sizeof(rchar), err->offset - err->lineoffset, stdout);
		}
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
	if (!jse)
		return 1;
	if (rjs_engine_addswitable(jse, "string", swistring) < 0) {
		return 2;
	}
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
	if (jse && r_array_length(jse->errors))
		rjs_display_errors(jse, script);
	rjs_engine_destroy(jse);
	if (unmapscript)
		rjs_unmap_file(unmapscript);
	if (statinfo)
		fprintf(stdout, "\nRJS Version: %s, memory: %ld Bytes (leaked %ld Bytes)\n", rjs_version(), (rlong)r_debug_get_maxmem(), (rlong)r_debug_get_allocmem());
	return 0;
}
