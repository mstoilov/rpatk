/*
 *  Regular Pattern Analyzer Toolkit (RPA/Tk)
 *  Copyright (c) 2009-2012 Martin Stoilov
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

#include "rpa/rpavm.h"
#include "rpa/rpastatpriv.h"
#include "rlib/rutf.h"
#include "rlib/rmem.h"


static void rpavm_swi_shift(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rword top = RVM_CPUREG_GETL(cpu, R_TOP);

	if ((top = rpa_stat_shift(stat, (long)top)) >= 0)
		RVM_CPUREG_SETL(cpu, R_TOP, top);
}


static void rpavm_swi_matchbitmap(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rpabitmap_t bitmap = RVM_CPUREG_GETU(cpu, ins->op1);

	if (rpa_stat_matchbitmap(stat, (long)RVM_CPUREG_GETL(cpu, R_TOP), bitmap)) {
		cpu->status = 0;
	} else {
		cpu->status = RVM_STATUS_N;
		RVM_CPUREG_SETU(cpu, R0, (ruword)-1);
	}
}


static void rpavm_swi_exitonbitmap(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rpabitmap_t bitmap = RVM_CPUREG_GETU(cpu, ins->op1);

	if (rpa_stat_matchbitmap(stat, (long)RVM_CPUREG_GETL(cpu, R_TOP), bitmap)) {
		cpu->status = 0;
	} else {
		cpu->status = RVM_STATUS_N;
		RVM_CPUREG_SETU(cpu, R0, (ruword)-1);
		cpu->abort = 1;
	}
}


static void rpavm_swi_verifybitmap(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rpabitmap_t bitmap = RVM_CPUREG_GETU(cpu, ins->op1);
	ruword r0 = RVM_CPUREG_GETU(cpu, R0);

	if (bitmap && r0 > 0) {
		if (!rpa_stat_matchbitmap(stat, (long)(RVM_CPUREG_GETL(cpu, R_TOP) - r0), bitmap)) {
			if ((cpu->status & RVM_STATUS_N) == 0)
				rvm_cpu_abort(cpu);
		}
	}
}


static void rpavm_swi_matchchr_nan(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	ruword wc = RVM_CPUREG_GETU(cpu, ins->op1);
	ruword matched = 0;

	if (rpa_stat_matchchr(stat, (long)RVM_CPUREG_GETL(cpu, R_TOP), (unsigned long)wc) > 0) {
		rpavm_swi_shift(cpu, ins);
		matched = 1;
	}
	cpu->status = matched ? 0 : RVM_STATUS_N;
	RVM_CPUREG_SETU(cpu, R0, matched ? matched : (ruword)-1);
}


static void rpavm_swi_matchchr_opt(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	ruword wc = RVM_CPUREG_GETU(cpu, ins->op1);
	ruword matched = 0;

	if (rpa_stat_matchchr(stat, (long)RVM_CPUREG_GETL(cpu, R_TOP), (unsigned long)wc) > 0) {
		rpavm_swi_shift(cpu, ins);
		matched = 1;
	}
	cpu->status = matched ? 0 : RVM_STATUS_Z;
	RVM_CPUREG_SETU(cpu, R0, matched);
}


static void rpavm_swi_matchchr_mul(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	ruword wc = RVM_CPUREG_GETU(cpu, ins->op1);
	ruword matched = 0;

	while (rpa_stat_matchchr(stat, (long)RVM_CPUREG_GETL(cpu, R_TOP), (unsigned long)wc) > 0) {
		rpavm_swi_shift(cpu, ins);
		matched += 1;
	}
	cpu->status = matched ? 0 : RVM_STATUS_N;
	RVM_CPUREG_SETU(cpu, R0, matched ? matched : (ruword)-1);
}


static void rpavm_swi_matchchr_mop(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	ruword wc = RVM_CPUREG_GETU(cpu, ins->op1);
	ruword matched = 0;

	while (rpa_stat_matchchr(stat, (long)RVM_CPUREG_GETL(cpu, R_TOP), (unsigned long)wc) > 0) {
		rpavm_swi_shift(cpu, ins);
		matched += 1;
	}
	cpu->status = matched ? 0 : RVM_STATUS_Z;
	RVM_CPUREG_SETU(cpu, R0, matched );
}


static void rpavm_swi_matchspchr_nan(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	ruword wc = RVM_CPUREG_GETU(cpu, ins->op1);
	ruword matched = 0;

	if (rpa_stat_matchspchr(stat, (long)RVM_CPUREG_GETL(cpu, R_TOP), (unsigned long)wc) > 0) {
		rpavm_swi_shift(cpu, ins);
		matched = 1;
	}
	cpu->status = matched ? 0 : RVM_STATUS_N;
	RVM_CPUREG_SETU(cpu, R0, matched ? matched : (ruword)-1);
}


static void rpavm_swi_matchspchr_opt(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	ruword wc = RVM_CPUREG_GETU(cpu, ins->op1);
	ruword matched = 0;

	if (rpa_stat_matchspchr(stat, (long)RVM_CPUREG_GETL(cpu, R_TOP), (unsigned long)wc) > 0) {
		rpavm_swi_shift(cpu, ins);
		matched = 1;
	}
	cpu->status = matched ? 0 : RVM_STATUS_Z;
	RVM_CPUREG_SETU(cpu, R0, matched);

}


static void rpavm_swi_matchspchr_mul(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	ruword wc = RVM_CPUREG_GETU(cpu, ins->op1);
	ruword matched = 0;

	while (rpa_stat_matchspchr(stat, (long)RVM_CPUREG_GETL(cpu, R_TOP), (unsigned long)wc) > 0) {
		rpavm_swi_shift(cpu, ins);
		matched += 1;
	}
	cpu->status = matched ? 0 : RVM_STATUS_N;
	RVM_CPUREG_SETU(cpu, R0, matched ? matched : (ruword)-1);

}


static void rpavm_swi_matchspchr_mop(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	ruword wc = RVM_CPUREG_GETU(cpu, ins->op1);
	ruword matched = 0;

	while (rpa_stat_matchspchr(stat, (long)RVM_CPUREG_GETL(cpu, R_TOP), (unsigned long)wc) > 0) {
		rpavm_swi_shift(cpu, ins);
		matched += 1;
	}
	cpu->status = matched ? 0 : RVM_STATUS_Z;
	RVM_CPUREG_SETU(cpu, R0, matched );
}


static void rpavm_swi_matchrng_peek(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rpair_t pr = RVM_CPUREG_GETPAIR(cpu, ins->op1);
	ruword matched = 0;

	if (rpa_stat_matchrng(stat, (long)RVM_CPUREG_GETL(cpu, R_TOP), pr.p1, pr.p2) <= 0) {
		cpu->status = matched ? 0 : RVM_STATUS_N;
		RVM_CPUREG_SETU(cpu, R0, matched ? matched : (ruword)-1);
	}
}


static void rpavm_swi_matchrng_nan(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rpair_t pr = RVM_CPUREG_GETPAIR(cpu, ins->op1);
	ruword matched = 0;

	if (rpa_stat_matchrng(stat, (long)RVM_CPUREG_GETL(cpu, R_TOP), pr.p1, pr.p2) > 0) {
		rpavm_swi_shift(cpu, ins);
		matched = 1;
	}
	cpu->status = matched ? 0 : RVM_STATUS_N;
	RVM_CPUREG_SETU(cpu, R0, matched ? matched : (ruword)-1);
}


static void rpavm_swi_matchrng_opt(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rpair_t pr = RVM_CPUREG_GETPAIR(cpu, ins->op1);
	ruword matched = 0;

	if (rpa_stat_matchrng(stat, (long)RVM_CPUREG_GETL(cpu, R_TOP), pr.p1, pr.p2) > 0) {
		rpavm_swi_shift(cpu, ins);
		matched = 1;
	}
	cpu->status = matched ? 0 : RVM_STATUS_Z;
	RVM_CPUREG_SETU(cpu, R0, matched);
}


static void rpavm_swi_matchrng_mul(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rpair_t pr = RVM_CPUREG_GETPAIR(cpu, ins->op1);
	ruword matched = 0;

	while (rpa_stat_matchrng(stat, (long)RVM_CPUREG_GETL(cpu, R_TOP), pr.p1, pr.p2) > 0) {
		rpavm_swi_shift(cpu, ins);
		matched += 1;
	}
	cpu->status = matched ? 0 : RVM_STATUS_N;
	RVM_CPUREG_SETU(cpu, R0, matched ? matched : (ruword)-1);
}


static void rpavm_swi_matchrng_mop(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rpair_t pr = RVM_CPUREG_GETPAIR(cpu, ins->op1);
	ruword matched = 0;

	while (rpa_stat_matchrng(stat, (long)RVM_CPUREG_GETL(cpu, R_TOP), pr.p1, pr.p2) > 0) {
		rpavm_swi_shift(cpu, ins);
		matched += 1;
	}
	cpu->status = matched ? 0 : RVM_STATUS_Z;
	RVM_CPUREG_SETU(cpu, R0, matched );
}


static void rpavm_swi_emittail(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rword index = RVM_CPUREG_GETL(cpu, R_REC);

	if (stat->records)
		r_array_setlength(stat->records, (unsigned long)(index + 1));
}


static void rpavm_swi_abort(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpa_ruledata_t *ruledata = RVM_CPUREG_GETP(cpu, ins->op1);
	rstr_t name = {(char*)ruledata + ruledata->name, ruledata->namesize};
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rword index = RVM_CPUREG_GETL(cpu, R_REC);
	ruword tp = RVM_CPUREG_GETU(cpu, ins->op2);


	if (stat->records)
		r_array_setlength(stat->records, (long)(index + 1));
	RPA_STAT_SETERROR_CODE(stat, RPA_E_RULEABORT);
	RPA_STAT_SETERRINFO_RULEUID(stat, ruledata->ruleuid);
	if (name.str) {
		RPA_STAT_SETERRINFO_NAME(stat, name.str, name.size);
	}
	if (stat->instack[tp].input) {
		RPA_STAT_SETERRINFO_OFFSET(stat, stat->instack[tp].input - stat->start);
	}
	rvm_cpu_abort(cpu);
}


static void rpavm_swi_emitstart(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpa_ruledata_t *ruledata = RVM_CPUREG_GETP(cpu, ins->op1);
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rparecord_t *rec;
	rword index = RVM_CPUREG_GETL(cpu, R_REC);
	ruword tp = RVM_CPUREG_GETU(cpu, ins->op2);
	rstr_t name = {(char*)ruledata + ruledata->name, ruledata->namesize};

	if (stat->debug)
		r_printf("%ld: %s, %s, tp = %ld\n", RVM_CPUREG_GETU(cpu, FP), "START", name.str, tp);
	if (!stat->records || !(ruledata->flags & RPA_RFLAG_EMITRECORD))
		return;
	index = r_array_replace(stat->records, (unsigned long)(index + 1), NULL);

	/*
	 * Important: get the pointer to crec after modifying the array, because if
	 * it gets reallocated the pointer will be invalid.
	 */
	RVM_CPUREG_SETL(cpu, R_REC, index);
	rec = (rparecord_t *)r_array_slot(stat->records, index);
	rec->rule = name.str;
	rec->top = (ruint32)tp;
	rec->ruleid = (ruint32)ruledata->ruleid;
	rec->ruleuid = (ruint32)ruledata->ruleuid;
	rec->type = RPA_RECORD_START;
	rec->input = stat->instack[tp].input;
	rec->inputoff = (unsigned long)(stat->instack[tp].input - stat->start);
	rec->inputsiz = 0;
	if (ruledata->flags & RPA_RFLAG_LEFTRECURSION)
		rec->usertype = RPA_LOOP_PATH;
}



static void rpavm_swi_emitend(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpa_ruledata_t *ruledata = RVM_CPUREG_GETP(cpu, ins->op1);
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rparecord_t *rec, *startrec;
	rword startindex = RVM_CPUREG_GETL(cpu, R1) + 1;
	rword index = RVM_CPUREG_GETL(cpu, R_REC);
	ruword tp = RVM_CPUREG_GETU(cpu, ins->op2);
	ruword tplen = RVM_CPUREG_GETU(cpu, ins->op3);
	rstr_t name = {(char*)ruledata + ruledata->name, ruledata->namesize};

	if (stat->debug)
		r_printf("%ld: %s, %s, tp = %ld, tplen = %ld\n", RVM_CPUREG_GETU(cpu, FP), "END ", name.str, tp, tplen);
	if (!stat->records || !(ruledata->flags & RPA_RFLAG_EMITRECORD))
		return;

	index = r_array_replace(stat->records, (unsigned long)(index + 1), NULL);
	RVM_CPUREG_SETL(cpu, R_REC, index);
	rec = (rparecord_t *)r_array_slot(stat->records, index);
	startrec = (rparecord_t *)r_array_slot(stat->records, startindex);
	rec->rule = name.str;
	rec->top = (ruint32)tp;
	rec->size = (ruint32)tplen;
	rec->type = RPA_RECORD_END;
	rec->ruleid = (ruint32)ruledata->ruleid;
	rec->ruleuid = (ruint32)ruledata->ruleuid;
	rec->input = stat->instack[tp].input;
	rec->inputsiz = (unsigned long)(stat->instack[tp + tplen].input - stat->instack[tp].input);
	rec->inputoff = (unsigned long)(stat->instack[tp].input - stat->start);
	startrec->size = (ruint32)tplen;
	startrec->inputsiz = rec->inputsiz;
	if (ruledata->flags & RPA_RFLAG_LEFTRECURSION)
		rec->usertype = RPA_LOOP_PATH;
}




static void rpavm_swi_prninfo(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rpa_ruledata_t *ruledata = RVM_CPUREG_GETP(cpu, ins->op1);
	rstr_t name = {"unknown", 7};
	if (!stat->debug)
		return;
	if (ruledata) {
		name.str = (char*)ruledata + ruledata->name;
		name.size = ruledata->namesize;
	}

	r_printf("%s: ", name.str);
	rvm_cpu_dumpregs(cpu, ins);
}


static void rpavm_swi_setcache(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rword r0 = RVM_CPUREG_GETL(cpu, R0);
	rword ruleid = RVM_CPUREG_GETL(cpu, ins->op1);
	rword prevrec = RVM_CPUREG_GETL(cpu, ins->op2);
	rword endrec = RVM_CPUREG_GETL(cpu, ins->op3);
	rword top = RVM_CPUREG_GETL(cpu, R_OTP);
	rword startrec = 0;

	if (r0 > 0 && prevrec != endrec) {
		startrec = prevrec + 1;
		rpa_cache_set(stat->cache, (long)top, (long)ruleid, (long)r0, stat->records, (long)startrec, (long)(endrec - prevrec));
	} else {
		rpa_cache_set(stat->cache, (long)top, (long)ruleid, (long)r0, stat->records, 0, 0);
	}
}


static void rpavm_swi_checkcache(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rpacachedentry_t *entry;
	rword ruleid = RVM_CPUREG_GETL(cpu, ins->op1);
	rword top = RVM_CPUREG_GETL(cpu, ins->op2);
	rword r0 = 0;
	entry = rpa_cache_lookup(stat->cache, (long)top, (long)ruleid);
	if (entry) {
		r0 = entry->ret;

		if (r0 > 0) {
			if (entry->recsize) {
				unsigned long i;
				if (stat->records) {
					r_array_setlength(stat->records, (unsigned long)(RVM_CPUREG_GETL(cpu, R_REC) + 1));
					for (i = 0; i < r_array_length(entry->records); i++) {
						r_array_add(stat->records, r_array_slot(entry->records, i));
					}
				}
				RVM_CPUREG_SETL(cpu, R_REC, r_array_length(stat->records) - 1);
			}
			top += r0;
			RVM_CPUREG_SETU(cpu, R_TOP, top);
		}
	}

	RVM_STATUS_CLRALL(cpu);
	RVM_CPUREG_SETU(cpu, R0, r0);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r0);
	if (r0 < 0 && entry) {
		RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, (r0 < 0));
//		r_printf("Hit the cache for: %ld, r0 = %ld\n", entry->ruleid, entry->ret);
	}
}


static rvm_switable_t rpavm_swi_table[] = {
		{"RPA_MATCHCHR_NAN", rpavm_swi_matchchr_nan},
		{"RPA_MATCHCHR_OPT", rpavm_swi_matchchr_opt},
		{"RPA_MATCHCHR_MUL", rpavm_swi_matchchr_mul},
		{"RPA_MATCHCHR_MOP", rpavm_swi_matchchr_mop},
		{"RPA_MATCHRNG_NAN", rpavm_swi_matchrng_nan},
		{"RPA_MATCHRNG_OPT", rpavm_swi_matchrng_opt},
		{"RPA_MATCHRNG_MUL", rpavm_swi_matchrng_mul},
		{"RPA_MATCHRNG_MOP", rpavm_swi_matchrng_mop},
		{"RPA_MATCHRNG_PEEK", rpavm_swi_matchrng_peek},
		{"RPA_MATCHSPCHR_NAN", rpavm_swi_matchspchr_nan},
		{"RPA_MATCHSPCHR_OPT", rpavm_swi_matchspchr_opt},
		{"RPA_MATCHSPCHR_MUL", rpavm_swi_matchspchr_mul},
		{"RPA_MATCHSPCHR_MOP", rpavm_swi_matchspchr_mop},
		{"RPA_SHIFT", rpavm_swi_shift},
		{"RPA_SETCACHE", rpavm_swi_setcache},
		{"RPA_CHECKCACHE", rpavm_swi_checkcache},
		{"RPA_EMITSTART", rpavm_swi_emitstart},
		{"RPA_EMITEND", rpavm_swi_emitend},
		{"RPA_EMITTAIL", rpavm_swi_emittail},
		{"RPA_ABORT", rpavm_swi_abort},
		{"RPA_PRNINFO", rpavm_swi_prninfo},
		{"RPA_MATCHBITMAP", rpavm_swi_matchbitmap},
		{"RPA_EXITONBITMAP", rpavm_swi_exitonbitmap},
		{"RPA_VERIFYBITMAP", rpavm_swi_verifybitmap},
		{NULL, NULL},
};


rvmcpu_t *rpavm_cpu_create(unsigned long stacksize)
{
	rvmcpu_t *cpu = rvm_cpu_create(stacksize);
	int tableid = rvm_cpu_addswitable(cpu, "rpavm_swi_table", rpavm_swi_table);

	if (tableid != RPAVM_SWI_TABLEID) {
		rpavm_cpu_destroy(cpu);
		return NULL;
	}
	return cpu;
}


void rpavm_cpu_destroy(rvmcpu_t * cpu)
{
	rvm_cpu_destroy(cpu);
}


