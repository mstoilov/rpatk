/*
 *  Regular Pattern Analyzer (RPA)
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

#include <stdio.h>
#include <stdlib.h>

#include "rlib/rmem.h"
#include "rjs/rjs.h"
#include "rjs/rjsfile.h"
#include "rpa/rparecord.h"


static int debuginfo = 0;
static int parseinfo = 0;
static int compileonly = 0;
static int debugcompileonly = 0;
static int statinfo = 0;


static char *errormsg[] = {
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
	const char *ptr, *list;
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
	const char *ptr, *list;
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
	const char *start, *ptr, *list;
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


long jrs_offset2line(const char *script, long offset)
{
	long line = 0;
	const char *ptr;

	for (line = 1, ptr = script + offset; ptr >= script; --ptr) {
		if (*ptr == '\n')
			line += 1;
	}

	return line;
}


void rjs_display_errors(rjs_engine_t *jse, rstr_t *script)
{
	long i;
	rjs_error_t *err;

	for (i = 0; i < r_array_length(jse->errors); i++) {
		err = (rjs_error_t *)r_array_slot(jse->errors, i);
		fprintf(stdout, "Line: %ld (%ld, %ld), Error Code: %ld, ", (long)err->line, err->offset, err->lineoffset, err->error);
		fprintf(stdout, "%s: ", errormsg[err->error]);
		if (err->size) {
			fwrite(script->str + err->offset, sizeof(char), err->size, stdout);
		} else {
			fwrite(script->str + err->lineoffset, sizeof(char), err->offset - err->lineoffset, stdout);
		}
		fprintf(stdout, "\n");
	}
}


int main(int argc, char *argv[])
{
	int i;
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
				script = rjs_file_map(argv[i]);
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
		rjs_file_unmap(unmapscript);
	if (statinfo)
		fprintf(stdout, "\nRJS Version: %s, memory: %ld Bytes (leaked %ld Bytes)\n", rjs_version(), (long)r_debug_get_maxmem(), (long)r_debug_get_allocmem());
	return 0;
}
