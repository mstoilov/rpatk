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



static void rpavm_swi_emitstart(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rparecord_t *rec;
	rlong index;
	rword tp = RVM_CPUREG_GETU(cpu, ins->op2);
	rstr_t name = {RVM_CPUREG_GETSTR(cpu, ins->op1), RVM_CPUREG_GETSIZE(cpu, ins->op1)};

	index = r_array_add(stat->records, NULL);
	rec = (rparecord_t *)r_array_slot(stat->records, index);
	rec->rule = name.str;
	rec->top = tp;
	rec->type = RPA_RECORD_START;
	rec->input = stat->instack[tp].input;
	rec->inputsiz = 0;
	RVM_CPUREG_SETU(cpu, R_REC, r_array_length(stat->records));

//	r_printf("START: %s(%ld)\n", name.str, (rulong)tp);
}


static void rpavm_swi_emitend(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rparecord_t *rec;
	rlong index;
	rword tp = RVM_CPUREG_GETU(cpu, ins->op2);
	rword tplen = RVM_CPUREG_GETU(cpu, ins->op3);
	rstr_t name = {RVM_CPUREG_GETSTR(cpu, ins->op1), RVM_CPUREG_GETSIZE(cpu, ins->op1)};

	index = r_array_add(stat->records, NULL);
	rec = (rparecord_t *)r_array_slot(stat->records, index);
	rec->rule = name.str;
	rec->top = tp;
	rec->size = tplen;
	rec->type = RPA_RECORD_START;
	rec->input = stat->instack[tp].input;
	rec->inputsiz = stat->instack[tp + tplen].input - stat->instack[tp].input;

	if (tplen) {
		rec->type = RPA_RECORD_END | RPA_RECORD_MATCH;
//		r_printf("MATCHED: %s(%ld, %ld): %p(%d)\n", name.str, (rulong)tp, (rulong)tplen, name.str, name.size);
	} else {
		rec->type = RPA_RECORD_END;
//		r_printf("MATCHED: %s(%ld, %ld)\n", name.str, (rulong)tp, (rulong)tplen);
	}
	RVM_CPUREG_SETU(cpu, R_REC, r_array_length(stat->records));
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


//	if (RVM_CPUREG_GETU(cpu, ins->op1) != RVM_CPUREG_GETU(cpu, R_REC)) {
//		r_printf("setreclen = %ld, R_REC = %ld\n", RVM_CPUREG_GETU(cpu, ins->op1), RVM_CPUREG_GETU(cpu, R_REC));
//	}
}


static void rpavm_swi_setrecid(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rparecord_t *rec;
	rword ruleid = RVM_CPUREG_GETU(cpu, ins->op1);

	rec = (rparecord_t *)r_array_lastslot(stat->records);
	if (rec) {
		rec->ruleid = ruleid;
	}
}


static void rpavm_swi_loopdetect(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rparecord_t *rec;
	rlong len;
	rword ruleid = RVM_CPUREG_GETU(cpu, ins->op1);
	rword tp = RVM_CPUREG_GETU(cpu, ins->op2);
	rword fp = RVM_CPUREG_GETU(cpu, FP);
	rvmreg_t loo, rid, top, pfp;

	RVM_CPUREG_SETU(cpu, R0, -1);

	while (fp > 5) {
		top = RVM_STACK_READ(cpu->stack, fp - 3);
		loo = RVM_STACK_READ(cpu->stack, fp - 4);
		rid = RVM_STACK_READ(cpu->stack, fp - 5);

		if (rid.v.l == ruleid && top.v.l == tp) {
			RVM_CPUREG_SETU(cpu, R0, loo.v.l);
			break;
		}
		RVM_REG_CLEAR(&loo);
		pfp = RVM_STACK_READ(cpu->stack, fp - 1);
		fp = pfp.v.l;
	}

//	for (len = r_array_length(stat->records); len > 0; len--) {
//		rec = (rparecord_t *)r_array_slot(stat->records, len - 1);
//		if (rec->type == (RPA_RECORD_END | RPA_RECORD_MATCH)) {
//			RVM_CPUREG_SETU(cpu, R0, -1);
//			break;
//		} else if (rec->ruleid == ruleid && rec->top == tp) {
//			RVM_CPUREG_SETU(cpu, R0, RVM_CPUREG_GETU(cpu, R_LOO));
//			RVM_CPUREG_SETU(cpu, R_TOP, RVM_CPUREG_GETU(cpu, R_TOP) + RVM_CPUREG_GETU(cpu, R_LOO));
//			r_printf("LOO = %ld, loo = %ld\n", RVM_CPUREG_GETU(cpu, R0), loo.v.l);
//			RVM_CPUREG_SETU(cpu, R0, loo.v.l);
//			break;
//		}
//	}
}


static void rpavm_swi_setcache(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rparecord_t *prec;
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rlong ruleid = RVM_CPUREG_GETL(cpu, ins->op1);
	rlong r0 = RVM_CPUREG_GETL(cpu, ins->op2);
	rlong rec = RVM_CPUREG_GETL(cpu, ins->op3);
	rlong nrecords = r_array_length(stat->records) - rec;

	/*
	 * If the record set is too big, don't use the cache
	 */
	if (nrecords > 50)
		return;

	if (!RVM_STATUS_GETBIT(cpu, RVM_STATUS_N) && !RVM_STATUS_GETBIT(cpu, RVM_STATUS_Z)) {
		prec = (rparecord_t *)r_array_slot(stat->records, rec);
//		r_printf("Set the cache for: %s (%ld)\n", prec->rule, nrecords);
		R_ASSERT(nrecords);
		rpa_cache_set(stat->cache, prec->top, ruleid, r0, prec, nrecords);
	}
}


static void rpavm_swi_checkcache(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rlong i;
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rpacachedentry_t *entry;
	rlong ruleid = RVM_CPUREG_GETL(cpu, ins->op1);
	rlong top = RVM_CPUREG_GETL(cpu, ins->op2);
	rlong r0 = 0;
	entry = rpa_cache_lookup(stat->cache, top, ruleid);
	if (entry) {
//		rparecord_t *prec = (rparecord_t *)r_array_slot(entry->records, 0);
//		r_printf("Hit the cache for: %s (%ld), r0 = %ld\n", prec->rule, r_array_length(entry->records), entry->ret);
		for (i = 0; i < r_array_length(entry->records); i++) {
			r_array_add(stat->records, r_array_slot(entry->records, i));
		}
		r0 = entry->ret;
		top += r0;
		RVM_CPUREG_SETU(cpu, R_TOP, top);
	}

	RVM_STATUS_CLRALL(cpu);
	RVM_CPUREG_SETU(cpu, R0, r0);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r0);
}


static void rpavm_swi_setrecuid(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rparecord_t *rec;
	rword userid = RVM_CPUREG_GETU(cpu, ins->op1);

	rec = (rparecord_t *)r_array_lastslot(stat->records);
	if (rec) {
		rec->userid = (ruint32)userid;
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
		{"RPA_MATCHSPCHR_NAN", rpavm_swi_matchspchr_nan},
		{"RPA_MATCHSPCHR_OPT", rpavm_swi_matchspchr_opt},
		{"RPA_MATCHSPCHR_MUL", rpavm_swi_matchspchr_mul},
		{"RPA_MATCHSPCHR_MOP", rpavm_swi_matchspchr_mop},
		{"RPA_SHIFT", rpavm_swi_shift},
		{"RPA_EMITSTART", rpavm_swi_emitstart},
		{"RPA_EMITEND", rpavm_swi_emitend},
		{"RPA_GETRECLEN", rpavm_swi_getreclen},
		{"RPA_SETRECLEN", rpavm_swi_setreclen},
		{"RPA_SETRECID", rpavm_swi_setrecid},
		{"RPA_LOOPDETECT", rpavm_swi_loopdetect},
		{"RPA_SETCACHE", rpavm_swi_setcache},
		{"RPA_CHECKCACHE", rpavm_swi_checkcache},
		{"RPA_SETRECUID", rpavm_swi_setrecuid},
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


