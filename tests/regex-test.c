#include <stdio.h>
#include <stdlib.h>
#include "rvmcpu.h"
#include "rmem.h"
#include "rutf.h"


static ruint regextable;
static int debuginfo = 0;
static int parseinfo = 0;

#define RPA_MATCHCHR 		RVM_OPSWI(RVM_SWI_ID(regextable, 0))
#define RPA_MATCHCHR_OPT 	RVM_OPSWI(RVM_SWI_ID(regextable, 1))
#define RPA_MATCHCHR_MUL 	RVM_OPSWI(RVM_SWI_ID(regextable, 2))
#define RPA_MATCHCHR_MOP 	RVM_OPSWI(RVM_SWI_ID(regextable, 3))
#define RPA_MATCHRNG 		RVM_OPSWI(RVM_SWI_ID(regextable, 4))
#define RPA_MATCHRNG_OPT 	RVM_OPSWI(RVM_SWI_ID(regextable, 5))
#define RPA_MATCHRNG_MUL 	RVM_OPSWI(RVM_SWI_ID(regextable, 6))
#define RPA_MATCHRNG_MOP 	RVM_OPSWI(RVM_SWI_ID(regextable, 7))
#define RPA_SHIFT		 	RVM_OPSWI(RVM_SWI_ID(regextable, 8))


typedef struct rpainput_s {
	ruint32 wc;
	const rchar *input;
} rpainput_t;


typedef struct rpainmap_s {
	const rchar *input;
	rulong serial;
} rpainmap_t;


typedef struct rpastat_s {
	const rchar *input;
	const rchar *start;
	const rchar *end;
	ruint error;
	rpainput_t *instack;
	rulong instacksize;
	rulong cursize;
	rpainmap_t ip;
} rpastat_t;


rpastat_t *rpa_stat_create()
{
	rpastat_t *stat = (rpastat_t *) r_zmalloc(sizeof(*stat));
	return stat;
}


int rpa_stat_init(rpastat_t *stat, const rchar *input, const rchar *start, const rchar *end)
{
	rulong size;

	if (start > end) {

		return -1;
	}
	if (input < start || input > end) {

		return -1;
	}
	size = end - start;
	stat->start = start;
	stat->end = end;
	stat->input = input;
	stat->error = 0;
	stat->cursize = 0;
	if (stat->instacksize < size) {
		stat->instack = r_realloc(stat->instack, (size + 1) * sizeof(rpainput_t));
		stat->instacksize = size + 1;
	}
	stat->ip.input = input;
	stat->ip.serial = 0;
	return 0;
}


void rpa_stat_destroy(rpastat_t *stat)
{
	if (stat->instack)
		r_free(stat->instack);
	r_free(stat);
}


static void rpa_matchchr(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, R0), op3 = RVM_CPUREG_GETU(cpu, R1);

	res = op2;
	RVM_CPUREG_SETU(cpu, R0, res);
}


static void rpa_matchchr_opt(rvmcpu_t *cpu, rvm_asmins_t *ins)
{

}


static void rpa_matchchr_mul(rvmcpu_t *cpu, rvm_asmins_t *ins)
{

}


static void rpa_matchchr_mop(rvmcpu_t *cpu, rvm_asmins_t *ins)
{

}


static void rpa_matchrng(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, R0), op3 = RVM_CPUREG_GETU(cpu, R1);

	res = op2;
	RVM_CPUREG_SETU(cpu, R0, res);
}


static void rpa_matchrng_opt(rvmcpu_t *cpu, rvm_asmins_t *ins)
{

}


static void rpa_matchrng_mul(rvmcpu_t *cpu, rvm_asmins_t *ins)
{

}


static void rpa_matchrng_mop(rvmcpu_t *cpu, rvm_asmins_t *ins)
{


}


static void rpa_shift(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rlong tp = RVM_CPUREG_GETL(cpu, TP);

	if (stat->ip.input >= stat->end) {
		RVM_STATUS_UPDATE(cpu, RVM_STATUS_E, 1);
		return;
	}

	tp += 1;
	if (tp >= (rlong)stat->ip.serial) {
		rint inc;
		inc = r_utf8_mbtowc(&stat->instack[tp].wc, (const ruchar*)stat->ip.input, (const ruchar*)stat->end);
		stat->instack[tp].input = stat->ip.input;
		stat->ip.input += inc;
		stat->ip.serial += inc;
	}
	RVM_CPUREG_SETL(cpu, IP, stat->instack[tp].wc);
	RVM_CPUREG_SETL(cpu, TP, tp);
}


static rvm_switable_t switable[] = {
		{"RPA_MATCHCHR", rpa_matchchr},
		{"RPA_MATCHCHR_OPT", rpa_matchchr_opt},
		{"RPA_MATCHCHR_MUL", rpa_matchchr_mul},
		{"RPA_MATCHCHR_MOP", rpa_matchchr_mop},
		{"RPA_MATCHRNG", rpa_matchrng},
		{"RPA_MATCHRNG_OPT", rpa_matchrng_opt},
		{"RPA_MATCHRNG_MUL", rpa_matchrng_mul},
		{"RPA_MATCHRNG_MOP", rpa_matchrng_mop},
		{"RPA_SHIFT", rpa_shift},
		{NULL, NULL},
};


int main(int argc, char *argv[])
{
	rvmcpu_t *cpu;
	rvm_asmins_t code[1024];
	ruint off = 0;
	rint i;

	cpu = rvm_cpu_create_default();
	cpu->userdata1 = rpa_stat_create();
	regextable = rvm_cpu_addswitable(cpu, switable);

	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-L") == 0) {
		} else if (r_strcmp(argv[i], "-d") == 0) {
			debuginfo = 1;
		} else if (r_strcmp(argv[i], "-p") == 0) {
			parseinfo = 1;
		}
	}

	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-e") == 0) {
			if (++i < argc) {
				rstr_t bnfexpr = { argv[i], r_strlen(argv[i]) };
				rpa_stat_init((rpastat_t *)cpu->userdata1, bnfexpr.str, bnfexpr.str, bnfexpr.str + bnfexpr.size);
			}
		}
	}


	code[off++] = rvm_asml(RVM_MOV, TP, DA, XX, -1);
	code[off++] = rvm_asm(RPA_SHIFT, XX, XX, XX, 0);
	code[off++] = rvm_asm(RPA_SHIFT, XX, XX, XX, 0);
	code[off++] = rvm_asm(RPA_SHIFT, XX, XX, XX, 0);
	code[off++] = rvm_asm(RPA_SHIFT, XX, XX, XX, 0);
	code[off++] = rvm_asm(RPA_SHIFT, XX, XX, XX, 0);

	rvm_cpu_exec_debug(cpu, code, 0);
	rpa_stat_destroy((rpastat_t *)cpu->userdata1);
	rvm_cpu_destroy(cpu);

	if (debuginfo) {
		r_printf("Max alloc mem: %ld\n", r_debug_get_maxmem());
		r_printf("Leaked mem: %ld\n", r_debug_get_allocmem());
	}
	return 0;
}
