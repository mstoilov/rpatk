#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "rmem.h"
#include "rpacompiler.h"
#include "rpastat.h"


static int debuginfo = 0;
static int parseinfo = 0;
static int compileonly = 0;



void codegen_rpa_match_aorb(rpa_compiler_t *co)
{
	rulong ruleidx;
	const rchar *rule = "rpa_match_aorb";
	const rchar *ruleend = "rpa_match_aorb_end";

	ruleidx = rvm_codegen_addstring_s(co->cg, NULL, rule);
	rvm_codegen_addlabel_s(co->cg, rule);

	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_STRING, ruleidx, rvm_asm(RPA_EMITSTART, DA, R_TOP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(R_TOP)|BIT(R_WHT)|BIT(LR)));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'a'));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BGRE, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_MOP, DA, XX, XX, 'b'));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BGRE, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(R_TOP)|BIT(R_WHT)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_addlabel_s(co->cg, ruleend);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(R1)|BIT(R_WHT)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R1, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_STRING, ruleidx, rvm_asm(RPA_EMITEND, DA, R1, R0, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
}


void codegen_rpa_match_xyz(rpa_compiler_t *co)
{
	rulong ruleidx;
	const rchar *rule = "rpa_match_xyz";
	const rchar *ruleend = "rpa_match_xyz_end";

	ruleidx = rvm_codegen_addstring_s(co->cg, NULL, rule);
	rvm_codegen_addlabel_s(co->cg, rule);

	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_STRING, ruleidx, rvm_asm(RPA_EMITSTART, DA, R_TOP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(R_TOP)|BIT(R_WHT)|BIT(LR)));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_match_squared", rvm_asm(RPA_BXLWHT, R_MNODE_MOP, DA, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'x'));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_MOP, DA, XX, XX, 'y'));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'z'));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(R1)|BIT(R_WHT)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R1, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_STRING, ruleidx, rvm_asm(RPA_EMITEND, DA, R1, R0, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_addlabel_s(co->cg, ruleend);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(R_TOP)|BIT(R_WHT)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
}


void codegen_rpa_match_abc(rpa_compiler_t *co)
{
	rulong ruleidx;
	const rchar *rule = "rpa_match_abc";
	const rchar *ruleend = "rpa_match_abc_end";

	ruleidx = rvm_codegen_addstring_s(co->cg, NULL, rule);
	rvm_codegen_addlabel_s(co->cg, rule);

	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_STRING, ruleidx, rvm_asm(RPA_EMITSTART, DA, R_TOP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(R_TOP)|BIT(R_WHT)|BIT(LR)));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'a'));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_MOP, DA, XX, XX, 'b'));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'c'));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BLES, DA, XX, XX, 0));


	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(R1)|BIT(R_WHT)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R1, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_STRING, ruleidx, rvm_asm(RPA_EMITEND, DA, R1, R0, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_addlabel_s(co->cg, ruleend);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(R_TOP)|BIT(R_WHT)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
}


void codegen_rpa_match_xyzorabc(rpa_compiler_t *co)
{
	rulong ruleidx;
	const rchar *rule = "rpa_match_xyzorabc";
	const rchar *ruleend = "rpa_match_xyzorabc_end";

	ruleidx = rvm_codegen_addstring_s(co->cg, NULL, rule);
	rvm_codegen_addlabel_s(co->cg, rule);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_STRING, ruleidx, rvm_asm(RPA_EMITSTART, DA, R_TOP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(R_TOP)|BIT(R_WHT)|BIT(LR)));


	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_match_xyz", rvm_asm(RPA_BXLWHT, R_MNODE_MOP, DA, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BGRE, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_match_abc", rvm_asm(RPA_BXLWHT, R_MNODE_MOP, DA, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BGRE, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(R_TOP)|BIT(R_WHT)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_addlabel_s(co->cg, ruleend);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(R1)|BIT(R_WHT)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R1, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_STRING, ruleidx, rvm_asm(RPA_EMITEND, DA, R1, R0, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
}



void codegen_rpa_match_squared(rpa_compiler_t *co)
{
	rulong ruleidx;
	const rchar *rule = "rpa_match_squared";
	const rchar *ruleend = "rpa_match_squared_end";

	ruleidx = rvm_codegen_addstring_s(co->cg, NULL, rule);
	rvm_codegen_addlabel_s(co->cg, rule);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_STRING, ruleidx, rvm_asm(RPA_EMITSTART, DA, R_TOP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(R_TOP)|BIT(R_WHT)|BIT(LR)));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '['));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_match_xyzorabc", rvm_asm(RPA_BXLWHT, R_MNODE_MOP, DA, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_match_aorb", rvm_asm(RPA_BXLWHT, R_MNODE_MOP, DA, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ']'));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHSPCHR_MOP, DA, XX, XX, 'n'));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(R1)|BIT(R_WHT)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R1, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_STRING, ruleidx, rvm_asm(RPA_EMITEND, DA, R1, R0, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_addlabel_s(co->cg, ruleend);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(R_TOP)|BIT(R_WHT)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
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


int main(int argc, char *argv[])
{
	rstr_t *script = NULL, *unmapscript = NULL;
	rvm_codelabel_t *err;
	rpa_compiler_t *co;
	rpastat_t *stat;
	ruint mainoff;
	rint i;

	co = rpa_compiler_create();
	stat = rpa_stat_create(4096);

	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-L") == 0) {
		} else if (r_strcmp(argv[i], "-d") == 0) {
			debuginfo = 1;
		} else if (r_strcmp(argv[i], "-c") == 0) {
			compileonly = 1;
		} else if (r_strcmp(argv[i], "-p") == 0) {
			parseinfo = 1;
		}
	}

	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-e") == 0) {
			if (++i < argc) {
				rstr_t bnfexpr = { argv[i], r_strlen(argv[i]) };
				rpa_stat_init(stat, bnfexpr.str, bnfexpr.str, bnfexpr.str + bnfexpr.size);
			}
		}
	}

	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-f") == 0) {
			if (++i < argc) {
				script = codegen_map_file(argv[i]);
				if (script) {
					rpa_stat_init(stat, script->str, script->str, script->str + script->size);
					unmapscript = script;
				}
			}
			goto exec;
		}
	}


exec:

	mainoff = rvm_codegen_addins(co->cg, rvm_asml(RVM_NOP, XX, XX, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, R_TOP, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, FP, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, SP, DA, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_nan", rvm_asm(RVM_MOV, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_mul", rvm_asm(RVM_MOV, R_MNODE_MUL, DA, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_opt", rvm_asm(RVM_MOV, R_MNODE_OPT, DA, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_mop", rvm_asm(RVM_MOV, R_MNODE_MOP, DA, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_SHIFT, XX, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_match_squared", rvm_asm(RPA_BXLWHT, R_MNODE_MUL, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0xabc));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));

	codegen_rpa_match_abc(co);
	codegen_rpa_match_xyz(co);
	codegen_rpa_match_xyzorabc(co);
	codegen_rpa_match_aorb(co);
	codegen_rpa_match_squared(co);

	if (rvm_codegen_relocate(co->cg, &err) < 0) {
		r_printf("Unresolved symbol: %s\n", err->name->str);
		goto end;
	}

	if (debuginfo) {
		fprintf(stdout, "\nGenerated Code:\n");
		rvm_asm_dump(rvm_codegen_getcode(co->cg, 0), rvm_codegen_getcodesize(co->cg));
		if (rvm_codegen_getcodesize(co->cg)) {
			if (!compileonly) {
				fprintf(stdout, "\nExecution:\n");
				rvm_cpu_exec_debug(stat->cpu, rvm_codegen_getcode(co->cg, 0), mainoff);
			}
		}
	} else {
		if (!compileonly)
			rvm_cpu_exec(stat->cpu, rvm_codegen_getcode(co->cg, 0), mainoff);
	}

	r_printf("Matched: %d\n", RVM_CPUREG_GETU(stat->cpu, R0));
end:

	for (i = 0; 0 && i < r_array_length(stat->records); i++) {
		rparecord_t *rec = (rparecord_t *)r_array_slot(stat->records, i);
		if (rec->type & RPA_RECORD_MATCH) {
			r_printf("%d: rule: %s(%d, %d)\n", i, rec->rule, (rint)rec->top, (rint)rec->size);
		}
	}

	rpa_stat_destroy(stat);
	rpa_compiler_destroy(co);
	if (unmapscript)
		codegen_unmap_file(unmapscript);


	if (debuginfo) {
		r_printf("Max alloc mem: %ld\n", r_debug_get_maxmem());
		r_printf("Leaked mem: %ld\n", r_debug_get_allocmem());
	}
	return 0;
}
