#include <stdio.h>
#include <stdlib.h>
#include "rvmcpu.h"
#include "rmem.h"


static ruint regextable;

#define RPA_MATCHCHR 		RVM_OPSWI(RVM_SWI_ID(regextable, 0))
#define RPA_MATCHCHR_OPT 	RVM_OPSWI(RVM_SWI_ID(regextable, 1))
#define RPA_MATCHCHR_MUL 	RVM_OPSWI(RVM_SWI_ID(regextable, 2))
#define RPA_MATCHCHR_MOP 	RVM_OPSWI(RVM_SWI_ID(regextable, 3))
#define RPA_MATCHRNG 		RVM_OPSWI(RVM_SWI_ID(regextable, 4))
#define RPA_MATCHRNG_OPT 	RVM_OPSWI(RVM_SWI_ID(regextable, 5))
#define RPA_MATCHRNG_MUL 	RVM_OPSWI(RVM_SWI_ID(regextable, 6))
#define RPA_MATCHRNG_MOP 	RVM_OPSWI(RVM_SWI_ID(regextable, 7))


typedef struct rpainput_s {
	ruint32 wc;
	const rchar *input;
} rpainput_t;


typedef struct rpastat_s {
	const rchar *input;
	const rchar *start;
	const rchar *end;
	ruint error;
	rpainput_t *instack;
	rulong instacksize;
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
	stat->end = input;
	stat->error = 0;
	if (size < stat->instacksize) {
		stat->instack = r_realloc(stat->instack, size * sizeof(rpainput_t));
		stat->instacksize = size;
	}
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
	rword res, op2 = RVM_CPUREG_GETU(cpu, R0), op3 = RVM_CPUREG_GETU(cpu, R1);

	res = op2;
	RVM_CPUREG_SETU(cpu, R0, res);
}


static void rpa_matchchr_mul(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, R0), op3 = RVM_CPUREG_GETU(cpu, R1);

	res = op2;
	RVM_CPUREG_SETU(cpu, R0, res);
}


static void rpa_matchchr_mop(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, R0), op3 = RVM_CPUREG_GETU(cpu, R1);

	res = op2;
	RVM_CPUREG_SETU(cpu, R0, res);
}


static void rpa_matchrng(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, R0), op3 = RVM_CPUREG_GETU(cpu, R1);

	res = op2;
	RVM_CPUREG_SETU(cpu, R0, res);
}


static void rpa_matchrng_opt(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, R0), op3 = RVM_CPUREG_GETU(cpu, R1);

	res = op2;
	RVM_CPUREG_SETU(cpu, R0, res);
}


static void rpa_matchrng_mul(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, R0), op3 = RVM_CPUREG_GETU(cpu, R1);

	res = op2;
	RVM_CPUREG_SETU(cpu, R0, res);
}


static void rpa_matchrng_mop(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, R0), op3 = RVM_CPUREG_GETU(cpu, R1);

	res = op2;
	RVM_CPUREG_SETU(cpu, R0, res);
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
		{NULL, NULL},
};


int main(int argc, char *argv[])
{
	rvmcpu_t *cpu;
	rvm_asmins_t code[1024];
	ruint off = 0;

	cpu = rvm_cpu_create_default();
	regextable = rvm_cpu_addswitable(cpu, switable);

	code[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 1);
	code[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 2);
	code[off++] = rvm_asm(RVM_ADD, R2, R0, R1, 0);
	code[off++] = rvm_asm(RPA_MATCHCHR, DA, XX, XX, 0);
	code[off++] = rvm_asm(RPA_MATCHCHR, DA, XX, XX, 0);
	code[off++] = rvm_asm(RVM_EXT, XX, XX, XX, 0);
	rvm_cpu_exec_debug(cpu, code, 0);
	rvm_cpu_destroy(cpu);


	fprintf(stdout, "It works!\n");
	return 0;
}
