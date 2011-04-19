#include "rpavm.h"
#include "rpastat.h"
#include "rutf.h"
#include "rmem.h"


//static void rpavm_swi_shift(rvmcpu_t *cpu, rvm_asmins_t *ins)
//{
//	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
//	rlong tp = RVM_CPUREG_GETL(cpu, R_TOP);
//	rpainput_t * ptp = &stat->instack[tp];
//
//	if (ptp->eof)
//		return;
//	ptp++;
//	tp++;
//	if (tp >= (rlong)stat->ip.serial) {
//		rint inc = 0;
//		ptp->input = stat->ip.input;
//		if (ptp->input < stat->end) {
//			inc = r_utf8_mbtowc(&ptp->wc, (const ruchar*)stat->ip.input, (const ruchar*)stat->end);
//			stat->ip.input += inc;
//			stat->ip.serial += 1;
//			ptp->eof = 0;
//		} else {
//			ptp->wc = (ruint32)-1;
//			ptp->eof = 1;
//		}
//	}
//	RVM_CPUREG_SETL(cpu, R_TOP, tp);
//}

static void rpavm_swi_shift(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rlong top = RVM_CPUREG_GETL(cpu, R_TOP);

	if ((top = rpa_stat_shift(stat, top)) >= 0)
		RVM_CPUREG_SETL(cpu, R_TOP, top);
}

static void rpavm_matchchr_do(rvmcpu_t *cpu, rvm_asmins_t *ins, rword flags)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rword wc = RVM_CPUREG_GETU(cpu, ins->op1);
	rword matched = 0;

	if (flags == RPA_MATCH_OPTIONAL) {
		if (rpa_stat_matchchr(stat, RVM_CPUREG_GETL(cpu, R_TOP), wc) > 0) {
			rpavm_swi_shift(cpu, ins);
			matched = 1;
		}
		cpu->status = matched ? 0 : RVM_STATUS_Z;
		RVM_CPUREG_SETU(cpu, R0, matched);
	} else if (flags == RPA_MATCH_MULTIPLE) {
		while (rpa_stat_matchchr(stat, RVM_CPUREG_GETL(cpu, R_TOP), wc) > 0) {
			rpavm_swi_shift(cpu, ins);
			matched += 1;
		}
		cpu->status = matched ? 0 : RVM_STATUS_N;
		RVM_CPUREG_SETU(cpu, R0, matched ? matched : (rword)-1);
	} else if (flags == RPA_MATCH_MULTIOPT) {
		while (rpa_stat_matchchr(stat, RVM_CPUREG_GETL(cpu, R_TOP), wc) > 0) {
			rpavm_swi_shift(cpu, ins);
			matched += 1;
		}
		cpu->status = matched ? 0 : RVM_STATUS_Z;
		RVM_CPUREG_SETU(cpu, R0, matched );
	} else {
		if (rpa_stat_matchchr(stat, RVM_CPUREG_GETL(cpu, R_TOP), wc) > 0) {
			rpavm_swi_shift(cpu, ins);
			matched = 1;
		}
		cpu->status = matched ? 0 : RVM_STATUS_N;
		RVM_CPUREG_SETU(cpu, R0, matched ? matched : (rword)-1);
	}
}


static void rpavm_matchspchr_do(rvmcpu_t *cpu, rvm_asmins_t *ins, rword flags)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rword wc = RVM_CPUREG_GETU(cpu, ins->op1);
	rword matched = 0;

	if (flags == RPA_MATCH_OPTIONAL) {
		if (rpa_stat_matchspchr(stat, RVM_CPUREG_GETL(cpu, R_TOP), wc) > 0) {
			rpavm_swi_shift(cpu, ins);
			matched = 1;
		}
		cpu->status = matched ? 0 : RVM_STATUS_Z;
		RVM_CPUREG_SETU(cpu, R0, matched);
	} else if (flags == RPA_MATCH_MULTIPLE) {
		while (rpa_stat_matchspchr(stat, RVM_CPUREG_GETL(cpu, R_TOP), wc) > 0) {
			rpavm_swi_shift(cpu, ins);
			matched += 1;
		}
		cpu->status = matched ? 0 : RVM_STATUS_N;
		RVM_CPUREG_SETU(cpu, R0, matched ? matched : (rword)-1);
	} else if (flags == RPA_MATCH_MULTIOPT) {
		while (rpa_stat_matchspchr(stat, RVM_CPUREG_GETL(cpu, R_TOP), wc) > 0) {
			rpavm_swi_shift(cpu, ins);
			matched += 1;
		}
		cpu->status = matched ? 0 : RVM_STATUS_Z;
		RVM_CPUREG_SETU(cpu, R0, matched );
	} else {
		if (rpa_stat_matchspchr(stat, RVM_CPUREG_GETL(cpu, R_TOP), wc) > 0) {
			rpavm_swi_shift(cpu, ins);
			matched = 1;
		}
		cpu->status = matched ? 0 : RVM_STATUS_N;
		RVM_CPUREG_SETU(cpu, R0, matched ? matched : (rword)-1);
	}
}


static void rpavm_swi_matchchr_nan(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpavm_matchchr_do(cpu, ins, RPA_MATCH_NONE);
}


static void rpavm_swi_matchchr_opt(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpavm_matchchr_do(cpu, ins, RPA_MATCH_OPTIONAL);
}


static void rpavm_swi_matchchr_mul(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpavm_matchchr_do(cpu, ins, RPA_MATCH_MULTIPLE);
}


static void rpavm_swi_matchchr_mop(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpavm_matchchr_do(cpu, ins, RPA_MATCH_MULTIOPT);
}




static void rpavm_swi_matchspchr_nan(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpavm_matchspchr_do(cpu, ins, RPA_MATCH_NONE);
}


static void rpavm_swi_matchspchr_opt(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpavm_matchspchr_do(cpu, ins, RPA_MATCH_OPTIONAL);
}


static void rpavm_swi_matchspchr_mul(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpavm_matchspchr_do(cpu, ins, RPA_MATCH_MULTIPLE);
}


static void rpavm_swi_matchspchr_mop(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpavm_matchspchr_do(cpu, ins, RPA_MATCH_MULTIOPT);
}


static void rpavm_matchrng_do(rvmcpu_t *cpu, rvm_asmins_t *ins, rword flags)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rpair_t pr = RVM_CPUREG_GETPAIR(cpu, ins->op1);
	rword matched = 0;

	if (flags == RPA_MATCH_OPTIONAL) {
		if (rpa_stat_matchrng(stat, RVM_CPUREG_GETL(cpu, R_TOP), pr.p1, pr.p2) > 0) {
			rpavm_swi_shift(cpu, ins);
			matched = 1;
		}
		cpu->status = matched ? 0 : RVM_STATUS_Z;
		RVM_CPUREG_SETU(cpu, R0, matched);
	} else if (flags == RPA_MATCH_MULTIPLE) {
		while (rpa_stat_matchrng(stat, RVM_CPUREG_GETL(cpu, R_TOP), pr.p1, pr.p2) > 0) {
			rpavm_swi_shift(cpu, ins);
			matched += 1;
		}
		cpu->status = matched ? 0 : RVM_STATUS_N;
		RVM_CPUREG_SETU(cpu, R0, matched ? matched : (rword)-1);
	} else if (flags == RPA_MATCH_MULTIOPT) {
		while (rpa_stat_matchrng(stat, RVM_CPUREG_GETL(cpu, R_TOP), pr.p1, pr.p2) > 0) {
			rpavm_swi_shift(cpu, ins);
			matched += 1;
		}
		cpu->status = matched ? 0 : RVM_STATUS_Z;
		RVM_CPUREG_SETU(cpu, R0, matched );
	} else {
		if (rpa_stat_matchrng(stat, RVM_CPUREG_GETL(cpu, R_TOP), pr.p1, pr.p2) > 0) {
			rpavm_swi_shift(cpu, ins);
			matched = 1;
		}
		cpu->status = matched ? 0 : RVM_STATUS_N;
		RVM_CPUREG_SETU(cpu, R0, matched ? matched : (rword)-1);
	}
}


static void rpavm_swi_matchrng_nan(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpavm_matchrng_do(cpu, ins, RPA_MATCH_NONE);
}


static void rpavm_swi_matchrng_opt(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpavm_matchrng_do(cpu, ins, RPA_MATCH_OPTIONAL);
}


static void rpavm_swi_matchrng_mul(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpavm_matchrng_do(cpu, ins, RPA_MATCH_MULTIPLE);
}


static void rpavm_swi_matchrng_mop(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpavm_matchrng_do(cpu, ins, RPA_MATCH_MULTIOPT);
}



static void rpavm_swi_emithead(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rparecord_t *rec;
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rlong index = RVM_CPUREG_GETL(cpu, R_REC);

	index = r_array_add(stat->records, NULL);
	RVM_CPUREG_SETL(cpu, R_REC, index);
	rec = (rparecord_t *)r_array_slot(stat->records, index);
	rec->type = RPA_RECORD_HEAD;
}


static void rpavm_swi_emittail(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rparecord_t *rec, *crec;
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rlong index = RVM_CPUREG_GETL(cpu, R_REC);

	index = r_array_add(stat->records, NULL);
	crec = (rparecord_t *)r_array_slot(stat->records, RVM_CPUREG_GETL(cpu, R_REC));
	RVM_CPUREG_SETL(cpu, R_REC, index);
	rec = (rparecord_t *)r_array_slot(stat->records, index);
	rec->type = RPA_RECORD_TAIL;
	crec->next = index;
}


static void rpavm_swi_emitstart(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpa_ruledata_t *ruledata = RVM_CPUREG_GETP(cpu, ins->op1);
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rparecord_t *rec, *crec;
	rlong index = RVM_CPUREG_GETL(cpu, R_REC);
	rword tp = RVM_CPUREG_GETU(cpu, ins->op2);
	rstr_t name = {(rchar*)ruledata + ruledata->name, ruledata->namesize};

	if (!(ruledata->flags & RPA_RFLAG_EMITRECORD))
		return;
//	r_printf("%ld: %s, %s, tp = %ld\n", RVM_CPUREG_GETU(cpu, FP), "START", name.str, tp);
	R_ASSERT(RVM_CPUREG_GETL(cpu, R_REC) >= 0);
//	index = r_array_replace(stat->records, index + 1, NULL);
	index = r_array_add(stat->records, NULL);

	/*
	 * Important: get the pointer to crec after modifying the array, because if
	 * it gets reallocated the pointer will be invalid.
	 */
	crec = (rparecord_t *)r_array_slot(stat->records, RVM_CPUREG_GETL(cpu, R_REC));
	RVM_CPUREG_SETL(cpu, R_REC, index);
	rec = (rparecord_t *)r_array_slot(stat->records, index);
	rec->rule = name.str;
	rec->top = tp;
	rec->ruleid = ruledata->ruleid;
	rec->ruleuid = ruledata->ruleuid;
	rec->type = RPA_RECORD_START;
	rec->input = stat->instack[tp].input;
	rec->inputsiz = 0;
	crec->next = index;
}



static void rpavm_swi_emitend(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpa_ruledata_t *ruledata = RVM_CPUREG_GETP(cpu, ins->op1);
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rparecord_t *rec, *crec;
	rlong index = RVM_CPUREG_GETL(cpu, R_REC);
	rword tp = RVM_CPUREG_GETU(cpu, ins->op2);
	rword tplen = RVM_CPUREG_GETU(cpu, ins->op3);
	rstr_t name = {(rchar*)ruledata + ruledata->name, ruledata->namesize};

	if (!(ruledata->flags & RPA_RFLAG_EMITRECORD))
		return;
//	r_printf("%ld: %s, %s, tp = %ld, tplen = %ld\n", RVM_CPUREG_GETU(cpu, FP), "END ", name.str, tp, tplen);
	R_ASSERT(RVM_CPUREG_GETL(cpu, R_REC) >= 0);
//	index = r_array_replace(stat->records, index + 1, NULL);
	index = r_array_add(stat->records, NULL);

	/*
	 * Important: get the pointer to crec after modifying the array, because if
	 * it gets reallocated the pointer will be invalid.
	 */
	crec = (rparecord_t *)r_array_slot(stat->records, RVM_CPUREG_GETL(cpu, R_REC));
	RVM_CPUREG_SETL(cpu, R_REC, index);
	rec = (rparecord_t *)r_array_slot(stat->records, index);
	rec->rule = name.str;
	rec->top = tp;
	rec->size = tplen;
	rec->type = RPA_RECORD_END;
	rec->ruleid = ruledata->ruleid;
	rec->ruleuid = ruledata->ruleuid;
	rec->input = stat->instack[tp].input;
	rec->inputsiz = stat->instack[tp + tplen].input - stat->instack[tp].input;
	crec->next = index;

	if (tplen) {
		rec->type |= RPA_RECORD_MATCH;
	}
}


static void rpavm_swi_prninfo(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpa_ruledata_t *ruledata = RVM_CPUREG_GETP(cpu, ins->op1);
	rstr_t name = {(rchar*)ruledata + ruledata->name, ruledata->namesize};

	if (!(ruledata->flags & RPA_RFLAG_EMITRECORD))
		return;
	r_printf("%s: ", name.str);
	rvm_cpu_dumpregs(cpu, ins);
}


static void rpavm_swi_getnextrec(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rlong rec = RVM_CPUREG_GETL(cpu, ins->op2);

	rparecord_t *prec = (rparecord_t *)r_array_slot(stat->records, rec);

//	r_printf("%s, rec = %ld, next = %ld\n", __FUNCTION__, rec, prec->next);
	RVM_CPUREG_SETL(cpu, ins->op1, prec->next);
}


static void rpavm_swi_getreclen(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	RVM_CPUREG_SETU(cpu, ins->op1, r_array_length(stat->records));
}


static void rpavm_swi_setreclen(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;

	r_array_setlength(stat->records, (ruint)RVM_CPUREG_GETU(cpu, ins->op1));
	RVM_CPUREG_SETL(cpu, R_REC, RVM_CPUREG_GETU(cpu, ins->op1) - 1);
}


static void rpavm_swi_getcurrec(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_CPUREG_SETL(cpu, ins->op1, RVM_CPUREG_GETL(cpu, R_REC));
}


static void rpavm_swi_setcurrec(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_CPUREG_SETL(cpu, R_REC, RVM_CPUREG_GETU(cpu, ins->op1));
}


static void rpavm_swi_setcache(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rlong r0 = RVM_CPUREG_GETL(cpu, R0);
	rlong ruleid = RVM_CPUREG_GETL(cpu, ins->op1);
	rlong prevrec = RVM_CPUREG_GETL(cpu, ins->op2);
	rlong endrec = RVM_CPUREG_GETL(cpu, ins->op3);
	rlong top = RVM_CPUREG_GETL(cpu, R_OTP);
	rlong startrec = 0;
	rparecord_t *prec = NULL;

	if (stat->cache->disalbled)
		return;

	if (!RVM_STATUS_GETBIT(cpu, RVM_STATUS_N) && !RVM_STATUS_GETBIT(cpu, RVM_STATUS_Z) && r0 > 0) {
		prec = (rparecord_t *)r_array_slot(stat->records, prevrec);
		startrec = prec->next;
		prec = (rparecord_t *)r_array_slot(stat->records, startrec);
//		r_printf("Set the cache for: %s (%ld, %ld), top = %ld, ret = %ld, ruleid=%ld\n", prec->rule, startrec, endrec, prec->top, r0, ruleid);
		if (prevrec != endrec)
			rpa_cache_set(stat->cache, top, ruleid, r0, startrec, endrec);
		else
			rpa_cache_set(stat->cache, top, ruleid, r0, 0, 0);
	}
}


static void rpavm_swi_checkcache(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rpacachedentry_t *entry;
	rlong ruleid = RVM_CPUREG_GETL(cpu, ins->op1);
	rlong top = RVM_CPUREG_GETL(cpu, ins->op2);
	rlong r0 = 0;
	entry = rpa_cache_lookup(stat->cache, top, ruleid);
	if (entry) {
		rparecord_t *prec = (rparecord_t *)r_array_slot(stat->records, entry->startrec);
//		r_printf("Hit the cache for: %s (%ld, %ld), r0 = %ld\n", prec->rule, entry->startrec, entry->endrec, entry->ret);
		rparecord_t *crec = (rparecord_t *)r_array_slot(stat->records, RVM_CPUREG_GETL(cpu, R_REC));

		if (entry->startrec) {
			crec->next = entry->startrec;
			RVM_CPUREG_SETL(cpu, R_REC, entry->endrec);
		}
		r0 = entry->ret;
		top += r0;
		RVM_CPUREG_SETU(cpu, R_TOP, top);
	}

	RVM_STATUS_CLRALL(cpu);
	RVM_CPUREG_SETU(cpu, R0, r0);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r0);
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
		{"RPA_MATCHSPCHR_NAN", rpavm_swi_matchspchr_nan},
		{"RPA_MATCHSPCHR_OPT", rpavm_swi_matchspchr_opt},
		{"RPA_MATCHSPCHR_MUL", rpavm_swi_matchspchr_mul},
		{"RPA_MATCHSPCHR_MOP", rpavm_swi_matchspchr_mop},
		{"RPA_SHIFT", rpavm_swi_shift},
		{"RPA_SETCACHE", rpavm_swi_setcache},
		{"RPA_CHECKCACHE", rpavm_swi_checkcache},
		{"RPA_EMITSTART", rpavm_swi_emitstart},
		{"RPA_EMITEND", rpavm_swi_emitend},
		{"RPA_EMITHEAD", rpavm_swi_emithead},
		{"RPA_EMITTAIL", rpavm_swi_emittail},
		{"RPA_GETNEXTREC", rpavm_swi_getnextrec},
		{"RPA_PRNINFO", rpavm_swi_prninfo},

		{"RPA_GETRECLEN", rpavm_swi_getreclen},
		{"RPA_SETRECLEN", rpavm_swi_setreclen},
		{"RPA_GETCURREC", rpavm_swi_getcurrec},
		{"RPA_SETCURREC", rpavm_swi_setcurrec},
		{NULL, NULL},
};


rvmcpu_t *rpavm_cpu_create(rulong stacksize)
{
	rvmcpu_t *cpu = rvm_cpu_create(stacksize);
	rint tableid = rvm_cpu_addswitable(cpu, "rpavm_swi_table", rpavm_swi_table);

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


