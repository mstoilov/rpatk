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
#include <string.h>
#include <stdarg.h>
#include "rvmcpu.h"
#include "rvmoperator.h"
#include "rvmoperatorcast.h"
#include "rmem.h"
#include "rstring.h"
#include "rvmreg.h"


static const char *stropcalls[] = {
	"RVM_EXT",
	"RVM_ASR",
	"RVM_SWI",
	"RVM_MOV",
	"RVM_ADD",
	"RVM_ADDS",
	"RVM_ADC",
	"RVM_AND",
	"RVM_BIC",
	"RVM_CLZ",
	"RVM_CMN",
	"RVM_EOR",
	"RVM_SUB",
	"RVM_SUBS",
	"RVM_SBC",
	"RVM_MUL",
	"RVM_MLS",
	"RVM_MULS",
	"RVM_NOT",
	"RVM_DIV",
	"RVM_DVS",
	"RVM_DIVS",
	"RVM_BL",
	"RVM_B",
	"RVM_STR",
	"RVM_STRP",
	"RVM_STRB",
	"RVM_STRH",
	"RVM_STRW",
	"RVM_STRR",
	"RVM_LDR",
	"RVM_LDRP",
	"RVM_LDRB",
	"RVM_LDRH",
	"RVM_LDRW",
	"RVM_LDRR",
	"RVM_LSL",
	"RVM_LSR",
	"RVM_STM",
	"RVM_LDM",
	"RVM_STS",
	"RVM_LDS",
	"RVM_ORR",
	"RVM_PUSH",
	"RVM_POP",
	"RVM_CMP",
	"RVM_NOP",
	"RVM_BEQ",
	"RVM_BNEQ",
	"RVM_BLEQ",
	"RVM_BGEQ",
	"RVM_BLES",
	"RVM_BGRE",
	"RVM_RET",
	"RVM_ROR",
	"RVM_PUSHM",
	"RVM_POPM",	
	"RVM_TST",	
	"RVM_TEQ",
	"ERVM_CAST",		/* Cast: op1 = (op3)op2 */
	"ERVM_TYPE",		/* Type: op1 = typeof(op2) */
	"ERVM_MOV",
};


static void rvm_op_b(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_CPUREG_INCIP(cpu, PC, RVM_CPUREG_GETU(cpu, ins->op1) - 1);
}


static void rvm_op_beq(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	if ((cpu->status & RVM_STATUS_Z))
		RVM_CPUREG_SETIP(cpu, PC, RVM_CPUREG_GETIP(cpu, PC) + RVM_CPUREG_GETU(cpu, ins->op1) - 1);
}


static void rvm_op_bneq(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	if ((cpu->status & RVM_STATUS_Z) == 0)
		RVM_CPUREG_SETIP(cpu, PC, RVM_CPUREG_GETIP(cpu, PC) + RVM_CPUREG_GETU(cpu, ins->op1) - 1);
}


static void rvm_op_bleq(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	if ((cpu->status & RVM_STATUS_N) || (cpu->status & RVM_STATUS_Z))
		RVM_CPUREG_SETIP(cpu, PC, RVM_CPUREG_GETIP(cpu, PC) + RVM_CPUREG_GETU(cpu, ins->op1) - 1);
}

static void rvm_op_bgeq(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	if ((cpu->status & RVM_STATUS_N) == 0 || (cpu->status & RVM_STATUS_Z) == 1)
		RVM_CPUREG_SETIP(cpu, PC, RVM_CPUREG_GETIP(cpu, PC) + RVM_CPUREG_GETU(cpu, ins->op1) - 1);
}


static void rvm_op_bles(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	if ((cpu->status & RVM_STATUS_N))
		RVM_CPUREG_SETIP(cpu, PC, RVM_CPUREG_GETIP(cpu, PC) + RVM_CPUREG_GETU(cpu, ins->op1) - 1);
}


static void rvm_op_bgre(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	if ((cpu->status & RVM_STATUS_N) == 0 && (cpu->status & RVM_STATUS_Z) == 0)
		RVM_CPUREG_SETIP(cpu, PC, RVM_CPUREG_GETU(cpu, PC) + RVM_CPUREG_GETU(cpu, ins->op1) - 1);
}


static void rvm_op_bl(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_CPUREG_SETIP(cpu, LR, RVM_CPUREG_GETIP(cpu, PC));
	RVM_CPUREG_INCIP(cpu, PC, RVM_CPUREG_GETU(cpu, ins->op1) - 1);
}


static void rvm_op_exit(rvmcpu_t *cpu, rvm_asmins_t *ins)
{

}


static void rvm_op_mov(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_CPUREG_SET(cpu, ins->op1, RVM_CPUREG_GET(cpu, ins->op2));
}


static void rvm_op_ldr(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_CPUREG_SETU(cpu, ins->op1, *((rword*)RVM_CPUREG_GETU(cpu, ins->op2)));
	
}


static void rvm_op_ldrp(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_CPUREG_SETU(cpu, ins->op1, *((rpointer*)RVM_CPUREG_GETU(cpu, ins->op2)));
}


static void rvm_op_ldrb(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_CPUREG_SETU(cpu, ins->op1, *((ruint8*)RVM_CPUREG_GETU(cpu, ins->op2)));
}


static void rvm_op_ldrh(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_CPUREG_SETU(cpu, ins->op1, *((ruint16*)RVM_CPUREG_GETU(cpu, ins->op2)));
}


static void rvm_op_ldrw(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_CPUREG_SETU(cpu, ins->op1, *((ruint32*)RVM_CPUREG_GETU(cpu, ins->op2)));
}


static void rvm_op_ldrr(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_CPUREG_SET(cpu, ins->op1, *((rvmreg_t*)RVM_CPUREG_GETU(cpu, ins->op2)));
}


static void rvm_op_str(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	*((rword*)RVM_CPUREG_GETP(cpu, ins->op2)) = RVM_CPUREG_GETU(cpu, ins->op1);
}


static void rvm_op_strp(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	*((rpointer*)RVM_CPUREG_GETP(cpu, ins->op2)) = (rpointer)RVM_CPUREG_GETP(cpu, ins->op1);
}


static void rvm_op_strb(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	*((ruint8*)RVM_CPUREG_GETP(cpu, ins->op2)) = (ruint8)RVM_CPUREG_GETU(cpu, ins->op1);
}


static void rvm_op_strh(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	*((ruint16*)RVM_CPUREG_GETP(cpu, ins->op2)) = (ruint16)RVM_CPUREG_GETU(cpu, ins->op1);
	
}


static void rvm_op_strw(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	*((ruint32*)RVM_CPUREG_GETP(cpu, ins->op2)) = (ruint32)RVM_CPUREG_GETU(cpu, ins->op1);

}


static void rvm_op_strr(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	*((rvmreg_t*)RVM_CPUREG_GETU(cpu, ins->op2)) = RVM_CPUREG_GET(cpu, ins->op1);
}


static void rvm_op_adds(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);
	
	res = op2 + op3;
	RVM_CPUREG_SETU(cpu, ins->op1, res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_C, res < op2 || res < op3);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_V, (op2 & RVM_SIGN_BIT) == (op3 & RVM_SIGN_BIT) &&
							(res & RVM_SIGN_BIT) != (op2 & RVM_SIGN_BIT));
}


static void rvm_op_adc(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);
	
	res = op2 + op3 + (RVM_STATUS_GETBIT(cpu, RVM_STATUS_C) ? 1 : 0);
	RVM_CPUREG_SETU(cpu, ins->op1, res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_C, res < op2 || res < op3);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_V, (op2 & RVM_SIGN_BIT) == (op3 & RVM_SIGN_BIT) &&
							(res & RVM_SIGN_BIT) != (op2 & RVM_SIGN_BIT));
}


static void rvm_op_and(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);
	
	res = op2 & op3;
	RVM_CPUREG_SETU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void rvm_op_orr(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);
	
	res = op2 | op3;
	RVM_CPUREG_SETU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void rvm_op_lsl(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);
	
	res = op2 << op3;
	RVM_CPUREG_SETU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void rvm_op_lsr(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);
	
	res = op2 >> op3;
	RVM_CPUREG_SETU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
}


static void rvm_op_asr(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);
	
	res = op2 >> op3;
	res |= op2 & RVM_SIGN_BIT;
	RVM_CPUREG_SETU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void rvm_op_ror(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	ruint i;
	rword res, op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);
	
	res = op2;
	for (i = 0; i < op3; i++) {
		if (res & 1) {
			res >>= 1;
			res |= RVM_SIGN_BIT;
		} else {
			res >>= 1;
		}
	}
	RVM_CPUREG_SETU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_C, res & RVM_SIGN_BIT);
}


static void rvm_op_tst(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);
	
	res = op2 & op3;
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void rvm_op_eor(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);
	
	res = op2 ^ op3;
	RVM_CPUREG_SETU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void rvm_op_not(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, ins->op2);
	
	res = ~op2;
	RVM_CPUREG_SETU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void rvm_op_teq(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);
	
	res = op2 ^ op3;
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void rvm_op_bic(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);
	
	res = op2 & ~op3;
	RVM_CPUREG_SETU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void rvm_op_clz(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, ins->op2);
	
	for (res = RVM_REGISTER_BITS; op2; ) {
		op2 >>= 1;
		res -= 1;
	}
	RVM_CPUREG_SETU(cpu, ins->op1, res);
}


static void rvm_op_add(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_CPUREG_SETU(cpu, ins->op1, RVM_CPUREG_GETU(cpu, ins->op2) + RVM_CPUREG_GETU(cpu, ins->op3));
}


static void rvm_op_swi(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
//	rvmcpu_swi *cbt = cpu->switable[RVM_CB_TABLE(RVM_CPUREG_GETU(cpu, ins->op1))];
//	cbt[RVM_CB_OFFSET(RVM_CPUREG_GETU(cpu, ins->op1))](cpu);

	rvmcpu_swi swi;
	rvm_switable_t *switable;
	ruint ntable = (ruint) RVM_SWI_TABLE(RVM_CPUREG_GETU(cpu, ins->op1));
	ruint nswi = (ruint) RVM_SWI_NUM(RVM_CPUREG_GETU(cpu, ins->op1));

	if (r_array_size(cpu->switables) <= ntable)
		RVM_ABORT(cpu, RVM_E_SWITABLE);
	switable = r_array_index(cpu->switables, ntable, rvm_switable_t*);
	swi = switable[nswi].op;
	swi(cpu, ins);
}


static void rvm_op_sub(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_CPUREG_SETU(cpu, ins->op1, RVM_CPUREG_GETU(cpu, ins->op2) - RVM_CPUREG_GETU(cpu, ins->op3));
}


static void rvm_op_subs(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);
	
	res = op2 - op3;
	RVM_CPUREG_SETU(cpu, ins->op1, res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_C, !(res > op2));  /* borrow = !carry */
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_V, (op2 & RVM_SIGN_BIT) != (op3 & RVM_SIGN_BIT) &&
							(res & RVM_SIGN_BIT) == (op2 & RVM_SIGN_BIT));
}


static void rvm_op_sbc(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);

	res = op2 - op3 + (RVM_STATUS_GETBIT(cpu, RVM_STATUS_C) ? 0 : -1);
	RVM_CPUREG_SETU(cpu, ins->op1, res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_C, !(res > op2));	/* borrow = !carry */
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_V, (op2 & RVM_SIGN_BIT) != (op3 & RVM_SIGN_BIT) &&
							(res & RVM_SIGN_BIT) == (op2 & RVM_SIGN_BIT));
}


static void rvm_op_mul(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_CPUREG_SETU(cpu, ins->op1, RVM_CPUREG_GETU(cpu, ins->op2) * RVM_CPUREG_GETU(cpu, ins->op3));
}


static void rvm_op_mls(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rsword res;
	rsword op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);

	res = (rsword)(op2 * op3);
	RVM_CPUREG_SETU(cpu, ins->op1, res);
	/* TBD: Not sure how to update the RVM_STATUS_C */
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void rvm_op_muls(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);
	
	res = op2 * op3;
	RVM_CPUREG_SETU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_C, op2 && (res / op2) != op3);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void rvm_op_div(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	if (!RVM_CPUREG_GETU(cpu, ins->op3))
		RVM_ABORT(cpu, RVM_E_DIVZERO);

	RVM_CPUREG_SETU(cpu, ins->op1, RVM_CPUREG_GETU(cpu, ins->op2) / RVM_CPUREG_GETU(cpu, ins->op3));
}


static void rvm_op_divs(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);

	if (!op3)
		RVM_ABORT(cpu, RVM_E_DIVZERO);
	res = op2 / op3;
	RVM_CPUREG_SETU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void rvm_op_dvs(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rsword res;
	rsword op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);

	if (!op3)
		RVM_ABORT(cpu, RVM_E_DIVZERO);
	res = (rsword)(op2 / op3);
	RVM_CPUREG_SETU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void rvm_op_push(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword sp = RVM_CPUREG_GETU(cpu, SP) + 1;

	r_array_replace(cpu->stack, sp, (rconstpointer)&RVM_CPUREG_GET(cpu, ins->op1));
	RVM_CPUREG_SETU(cpu, SP, sp);
}


static void rvm_op_sts(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword sp = RVM_CPUREG_GETU(cpu, ins->op2) + RVM_CPUREG_GETU(cpu, ins->op3);

	r_array_replace(cpu->stack, sp, (rconstpointer)&RVM_CPUREG_GET(cpu, ins->op1));
}


static void rvm_op_pushm(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	int n, i;
	rword bits = RVM_CPUREG_GETU(cpu, ins->op1);
	rword sp = RVM_CPUREG_GETU(cpu, SP);

	for (i = 0, n = 0; bits && n < RLST; n++) {
		if (((rword)(1 << n)) & bits) {
			i += 1;
			sp += 1;
			r_array_replace(cpu->stack, sp, (rconstpointer)&RVM_CPUREG_GET(cpu, n));
			bits &= ~(1<<n);
		}
	}
	RVM_CPUREG_SETU(cpu, SP, RVM_CPUREG_GETU(cpu, SP) + i);
}


static void rvm_op_pop(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword sp = RVM_CPUREG_GETU(cpu, SP);

	RVM_CPUREG_SET(cpu, ins->op1, r_array_index(cpu->stack, sp, rvmreg_t));
	if (ins->op1 != SP)
		RVM_CPUREG_SETU(cpu, SP, sp - 1);
}


static void rvm_op_popm(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	int n;
	rword bits = RVM_CPUREG_GETU(cpu, ins->op1);
	rword sp = RVM_CPUREG_GETU(cpu, SP);

	for (n = RLST - 1; bits && n >= 0; n--) {
		if (((rword)(1 << n)) & bits) {
			RVM_CPUREG_SET(cpu, n, r_array_index(cpu->stack, sp, rvmreg_t));
			sp -= 1;
			bits &= ~(1<<n);
		}
	}
	if (!(((rword)(1 << SP)) & bits))
		RVM_CPUREG_SETU(cpu, SP, sp);
}


static void rvm_op_lds(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword sp = RVM_CPUREG_GETU(cpu, ins->op2) + RVM_CPUREG_GETU(cpu, ins->op3);

	RVM_CPUREG_SET(cpu, ins->op1, r_array_index(cpu->stack, sp, rvmreg_t));
}


static void rvm_op_stm(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	int n;
	rword *dst = (rword*)	RVM_CPUREG_GETU(cpu, ins->op1);
	rword bits = RVM_CPUREG_GETU(cpu, ins->op2);
	
	for (n = 0; bits && n < RLST; n++) {
		if (((rword)(1 << n)) & bits) {
			*dst = RVM_CPUREG_GETU(cpu, n);
			dst += 1;
			bits &= ~(1<<n);
		}
	}
}


static void rvm_op_ldm(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	int n;
	rword *src = (rword*)RVM_CPUREG_GETU(cpu, ins->op1);
	rword bits = RVM_CPUREG_GETU(cpu, ins->op2);

	for (n = 0; bits && n < RLST; n++) {
		if (((rword)(1 << n)) & bits) {
			RVM_CPUREG_SETU(cpu, n, *src);
			src += 1;
			bits &= ~(1<<n);
		}
	}
}


static void rvm_op_cmp(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword op1 = RVM_CPUREG_GETU(cpu, ins->op1), op2 = RVM_CPUREG_GETU(cpu, ins->op2);
	rword res = (rword)(op1 - op2);
	
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_C, op1 < op2);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_V, (op1 & RVM_SIGN_BIT) != (op2 & RVM_SIGN_BIT) &&
							(res & RVM_SIGN_BIT) == (op1 & RVM_SIGN_BIT));
}


static void rvm_op_cmn(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword op1 = RVM_CPUREG_GETU(cpu, ins->op1), op2 = RVM_CPUREG_GETU(cpu, ins->op2);
	rword res = (rword)(op1 + op2);
	
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_C, res < op1 || res < op2);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_V, (op1 & RVM_SIGN_BIT) == (op2 & RVM_SIGN_BIT) &&
							(res & RVM_SIGN_BIT) != (op1 & RVM_SIGN_BIT));

}


static void rvm_op_nop(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
//	fprintf(stdout, "nop\n");
}


static void rvm_op_ret(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_CPUREG_SETU(cpu, PC, RVM_CPUREG_GETU(cpu, LR));	
}



static int rvm_vsnprintf(char *str, ruint size, const char *format, va_list ap)
{
	return vsnprintf(str, size, format, ap);
}


static int rvm_snprintf(char *str, ruint size, const char *format, ...)
{
	va_list ap;
	int ret;

	va_start(ap, format);
	ret = rvm_vsnprintf(str, size, format, ap);
	va_end(ap);
	return ret;
}


static int rvm_printf(const char *format, ...)
{
	va_list ap;
	int ret;

	va_start(ap, format);
	ret = vfprintf(stdout, format, ap);
	va_end(ap);
	return ret;
}


int rvm_asm_dump_reg_to_str(unsigned char reg, char *str, ruint size)
{
	int ret = 0;

	if (reg == XX)
		ret = rvm_snprintf(str, size, "XX ");
	else if (reg == FP)
		ret = rvm_snprintf(str, size, "FP ");
	else if (reg == SP)
		ret = rvm_snprintf(str, size, "SP ");
	else if (reg == LR)
		ret = rvm_snprintf(str, size, "LR ");
	else if (reg == PC)
		ret = rvm_snprintf(str, size, "PC ");
	else if (reg == DA)
		ret = rvm_snprintf(str, size, "DA ");
	else if (reg >= 0 && reg < 10)
		ret = rvm_snprintf(str, size, "R%d ",  reg);
	else if (reg >= 10)
		ret = rvm_snprintf(str, size, "R%d",  reg);

	return ret;
}


int rvm_asm_dump_pi_to_str(rvm_asmins_t *pi, char *str, ruint size)
{
	int ret = 0, sz = size;

	if ((ret = rvm_snprintf(str, sz, "%10s   ", stropcalls[pi->opcode])) < 0)
		return ret;
	str += ret;
	sz -= ret;
	
	if ((ret = rvm_asm_dump_reg_to_str(pi->op1, str, sz)) < 0)
		return ret;
	str += ret;
	sz -= ret;
		
	if ((ret = rvm_snprintf(str, sz, " ")) < 0)
		return ret;
	str += ret;
	sz -= ret;

	if ((ret = rvm_asm_dump_reg_to_str(pi->op2, str, sz)) < 0)
		return ret;
	str += ret;
	sz -= ret;
		
	if ((ret = rvm_snprintf(str, sz, " ")) < 0)
		return ret;
	str += ret;
	sz -= ret;

	if ((ret = rvm_asm_dump_reg_to_str(pi->op3, str, sz)) < 0)
		return ret;
	str += ret;
	sz -= ret;
		
	if ((ret = rvm_snprintf(str, sz, " ")) < 0)
		return ret;
	str += ret;
	sz -= ret;

//	if ((ret = rvm_snprintf(str, sz, "0x%lx  ", (unsigned long)RVM_REG_GETU(&pi->data))) < 0)
//		return ret;
	if ((ret = rvm_snprintf(str, sz, "0x%lx  ", (unsigned long)pi->data)) < 0)
		return ret;


	str += ret;
	sz -= ret;

	return size - sz;
}


void rvm_asm_dump(rvm_asmins_t *pi, ruint count)
{
	char buffer[256];
	while (count) {
		rvm_asm_dump_pi_to_str(pi, buffer, sizeof(buffer));
		rvm_printf("%s\n", buffer);
		++pi;
		--count;
	}
}


static void rvm_cpu_dumpregs(rvm_asmins_t *pi, rvmcpu_t *vm)
{
    int ret;
	char buffer[1024];
	
	ret = rvm_asm_dump_pi_to_str(pi, buffer, sizeof(buffer));
	if (ret < 0)
		return;
    ret = rvm_snprintf(buffer + ret, sizeof(buffer) - ret, "                                                                                        ");
	buffer[50] = '\0';
	rvm_printf("%s", buffer);

   	rvm_printf("0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, FP=%ld, SP=%ld, LR=%ld, PC=%ld, DA=0x%lx, S( %c%c%c%c )",
   		RVM_CPUREG_GETU(vm, 0), RVM_CPUREG_GETU(vm, 1), RVM_CPUREG_GETU(vm, 2), RVM_CPUREG_GETU(vm, 3),
   		RVM_CPUREG_GETU(vm, 4), RVM_CPUREG_GETU(vm, 5), RVM_CPUREG_GETU(vm, 6), RVM_CPUREG_GETU(vm, 7),
   		RVM_CPUREG_GETU(vm, 8), (long int)RVM_CPUREG_GETU(vm, FP), (long int)RVM_CPUREG_GETU(vm, SP),
   		(long int)RVM_CPUREG_GETU(vm, LR), (long int)RVM_CPUREG_GETU(vm, PC), RVM_CPUREG_GETU(vm, DA),
   		vm->status & RVM_STATUS_V ? 'V' : '_',
   		vm->status & RVM_STATUS_C ? 'C' : '_',
   		vm->status & RVM_STATUS_N ? 'N' : '_',
   		vm->status & RVM_STATUS_Z ? 'Z' : '_');
	rvm_printf("\n");
}


static void ervm_op_cast(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_type_t type = (rvmreg_type_t)RVM_CPUREG_GETU(cpu, ins->op3);
	rvmreg_t tmp;

	RVM_REG_CLEAR(&tmp);
	RVM_REG_SETTYPE(&tmp, type);
	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_CAST, cpu, RVM_CPUREG_PTR(cpu, ins->op1), RVM_CPUREG_PTR(cpu, ins->op2), &tmp);
}


static void ervm_op_type(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_type_t type = (rvmreg_type_t)RVM_CPUREG_GETTYPE(cpu, ins->op2);

	RVM_CPUREG_CLEAR(cpu, ins->op1);
	RVM_CPUREG_SETU(cpu, ins->op1, type);
}


static void ervm_op_mov(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvm_op_mov(cpu, ins);
}


static void ervm_op_adc(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);

	res = op2 + op3 + (RVM_STATUS_GETBIT(cpu, RVM_STATUS_C) ? 1 : 0);
	RVM_CPUREG_SETU(cpu, ins->op1, res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_C, res < op2 || res < op3);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_V, (op2 & RVM_SIGN_BIT) == (op3 & RVM_SIGN_BIT) &&
							(res & RVM_SIGN_BIT) != (op2 & RVM_SIGN_BIT));
}


static void ervm_op_adds(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);

	res = op2 + op3;
	RVM_CPUREG_SETU(cpu, ins->op1, res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_C, res < op2 || res < op3);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_V, (op2 & RVM_SIGN_BIT) == (op3 & RVM_SIGN_BIT) &&
							(res & RVM_SIGN_BIT) != (op2 & RVM_SIGN_BIT));
}


static void ervm_op_add(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_type_t type;
	rvmreg_t *r2 = rvm_reg_unshadow(RVM_CPUREG_PTR(cpu, ins->op2));
	rvmreg_t *r3 = rvm_reg_unshadow(RVM_CPUREG_PTR(cpu, ins->op3));

	type = RVM_REG_GETTYPE(r2);


	if (type != RVM_REG_GETTYPE(r3))
		RVM_ABORT(cpu, RVM_E_CAST);

	switch (type) {
	case RVM_DTYPE_UNSIGNED:
		RVM_CPUREG_SETU(cpu, ins->op1, RVM_REG_GETU(r2) + RVM_REG_GETU(r2));
		RVM_CPUREG_SETTYPE(cpu, ins->op1, RVM_DTYPE_NONE);
		RVM_CPUREG_CLRFLAG(cpu, ins->op1, RVM_INFOBIT_ROBJECT);
		break;
	case RVM_DTYPE_LONG:
		RVM_CPUREG_SETU(cpu, ins->op1, RVM_REG_GETL(r2) + RVM_REG_GETL(r2));
		RVM_CPUREG_SETTYPE(cpu, ins->op1, RVM_DTYPE_LONG);
		RVM_CPUREG_CLRFLAG(cpu, ins->op1, RVM_INFOBIT_ROBJECT);
		break;
	case RVM_DTYPE_DOUBLE:
		RVM_CPUREG_SETD(cpu, ins->op1, RVM_REG_GETD(r2) + RVM_REG_GETD(r2));
		RVM_CPUREG_SETTYPE(cpu, ins->op1, RVM_DTYPE_DOUBLE);
		RVM_CPUREG_CLRFLAG(cpu, ins->op1, RVM_INFOBIT_ROBJECT);
		break;
	default:
		RVM_ABORT(cpu, RVM_E_CAST);
	};
}


static void ervm_op_and(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);

	res = op2 & op3;
	RVM_CPUREG_SETU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void ervm_op_eor(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);

	res = op2 ^ op3;
	RVM_CPUREG_SETU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void ervm_op_mls(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rsword res;
	rsword op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);

	res = (rsword)(op2 * op3);
	RVM_CPUREG_SETU(cpu, ins->op1, res);
	/* TBD: Not sure how to update the RVM_STATUS_C */
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void ervm_op_mul(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_CPUREG_SETU(cpu, ins->op1, RVM_CPUREG_GETU(cpu, ins->op2) * RVM_CPUREG_GETU(cpu, ins->op3));
}


static void ervm_op_muls(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);

	res = op2 * op3;
	RVM_CPUREG_SETU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_C, op2 && (res / op2) != op3);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void ervm_op_sub(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_CPUREG_SETU(cpu, ins->op1, RVM_CPUREG_GETU(cpu, ins->op2) - RVM_CPUREG_GETU(cpu, ins->op3));
}


static void ervm_op_subs(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);

	res = op2 - op3;
	RVM_CPUREG_SETU(cpu, ins->op1, res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_C, !(res > op2));  /* borrow = !carry */
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_V, (op2 & RVM_SIGN_BIT) != (op3 & RVM_SIGN_BIT) &&
							(res & RVM_SIGN_BIT) == (op2 & RVM_SIGN_BIT));
}


static void ervm_op_sbc(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);

	res = op2 - op3 + (RVM_STATUS_GETBIT(cpu, RVM_STATUS_C) ? 0 : -1);
	RVM_CPUREG_SETU(cpu, ins->op1, res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_C, !(res > op2));	/* borrow = !carry */
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_V, (op2 & RVM_SIGN_BIT) != (op3 & RVM_SIGN_BIT) &&
							(res & RVM_SIGN_BIT) == (op2 & RVM_SIGN_BIT));
}


static void ervm_op_div(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	if (!RVM_CPUREG_GETU(cpu, ins->op3))
		RVM_ABORT(cpu, RVM_E_DIVZERO);

	RVM_CPUREG_SETU(cpu, ins->op1, RVM_CPUREG_GETU(cpu, ins->op2) / RVM_CPUREG_GETU(cpu, ins->op3));
}


static void ervm_op_divs(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);

	if (!op3)
		RVM_ABORT(cpu, RVM_E_DIVZERO);
	res = op2 / op3;
	RVM_CPUREG_SETU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void ervm_op_dvs(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rsword res;
	rsword op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);

	if (!op3)
		RVM_ABORT(cpu, RVM_E_DIVZERO);
	res = (rsword)(op2 / op3);
	RVM_CPUREG_SETU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static rvm_cpu_op ops[] = {
	rvm_op_exit,		// RVM_EXT
	rvm_op_asr,			// RVM_ASR
	rvm_op_swi,			// RVM_swi
	rvm_op_mov,			// RVM_MOV
	rvm_op_add,			// RVM_ADD
	rvm_op_adds,		// RVM_ADDS
	rvm_op_adc,			// RVM_ADC
	rvm_op_and,			// RVM_AND
	rvm_op_bic,			// RVM_BIC
	rvm_op_clz,			// RVM_CLZ
	rvm_op_cmn,			// RVM_CMN
	rvm_op_eor,			// RVM_EOR
	rvm_op_sub,			// RVM_SUB
	rvm_op_subs,		// RVM_SUBS
	rvm_op_sbc,			// RVM_SBC
	rvm_op_mul,			// RVM_MUL
	rvm_op_mls,			// RVM_MLS
	rvm_op_muls,		// RVM_MULS
	rvm_op_not,			// RVM_NOT
	rvm_op_div,			// RVM_DIV
	rvm_op_dvs,			// RVM_DVS
	rvm_op_divs,		// RVM_DIVS
	rvm_op_bl,			// RVM_BL
	rvm_op_b,			// RVM_B
	rvm_op_str,			// RVM_STR
	rvm_op_strp,		// RVM_STRP
	rvm_op_strb,		// RVM_STRB
	rvm_op_strh,		// RVM_STRH
	rvm_op_strw,		// RVM_STRW
	rvm_op_strr,		// RVM_STRR
	rvm_op_ldr,			// RVM_LDR
	rvm_op_ldrp,		// RVM_LDRP
	rvm_op_ldrb,		// RVM_LDRB
	rvm_op_ldrh,		// RVM_LDRH
	rvm_op_ldrw,		// RVM_LDRW
	rvm_op_ldrr,		// RVM_LDRR
	rvm_op_lsl,			// RVM_LSL
	rvm_op_lsr,			// RVM_LSR
	rvm_op_stm,			// RVM_STM
	rvm_op_ldm,			// RVM_LDS
	rvm_op_sts,			// RVM_STS
	rvm_op_lds,			// RVM_LDM
	rvm_op_orr,			// RVM_ORR
	rvm_op_push,		// RVM_PUSH
	rvm_op_pop,			// RVM_POP
	rvm_op_cmp,			// RVM_CMP
	rvm_op_nop,			// RVM_NOP
	rvm_op_beq,			// RVM_BEQ
	rvm_op_bneq,		// RVM_BNEQ
	rvm_op_bleq,		// RVM_BLEQ
	rvm_op_bgeq,		// RVM_BGEQ
	rvm_op_bles,		// RVM_BLES
	rvm_op_bgre,		// RVM_BGRE
	rvm_op_ret,			// RVM_RET
	rvm_op_ror,			// RVM_ROR
	rvm_op_pushm, 		// RVM_PUSHM
	rvm_op_popm, 		// RVM_POPM
	rvm_op_tst, 		// RVM_TST
	rvm_op_teq, 		// RVM_TEQ

/* Extended VM instructions */
	ervm_op_cast,		// ERVM_CAST
	ervm_op_type,		// ERVM_TYPE
	ervm_op_mov,		// ERVM_MOV
	ervm_op_add,		// ERVM_ADD
	ervm_op_adds,		// ERVM_ADDS
	ervm_op_adc,		// ERVM_ADC
	ervm_op_and,		// ERVM_AND
	ervm_op_eor,		// ERVM_EOR
	ervm_op_sub,		// ERVM_SUB
	ervm_op_subs,		// ERVM_SUBS
	ervm_op_sbc,		// ERVM_SBC
	ervm_op_mul,		// ERVM_MUL
	ervm_op_mls,		// ERVM_MLS
	ervm_op_muls,		// ERVM_MULS
	ervm_op_div,		// RVM_DIV
	ervm_op_dvs,		// RVM_DVS
	ervm_op_divs,		// RVM_DIVS
	(void*) 0,
	(void*) 0,
	(void*) 0,
	(void*) 0,
	(void*) 0,
	(void*) 0,
	(void*) 0,
	(void*) 0,
	
};
	

rvmcpu_t *rvm_cpu_create()
{
	rvmcpu_t *cpu;

	cpu = (rvmcpu_t *)r_malloc(sizeof(*cpu));
	if (!cpu)
		return ((void*)0);
	r_memset(cpu, 0, sizeof(*cpu));
	cpu->switables = r_array_create(sizeof(rvm_switable_t*));
	cpu->stack = r_array_create(sizeof(rvmreg_t));
	cpu->opmap = rvm_opmap_create();
	rvm_op_cast_init(cpu->opmap);
	return cpu;
}


void rvm_cpu_destroy(rvmcpu_t *cpu)
{
//	rvm_free(cpu->stack);
	r_array_destroy(cpu->switables);
	r_array_destroy(cpu->stack);
	rvm_opmap_destroy(cpu->opmap);
	r_free(cpu);
}


rint rvm_cpu_exec(rvmcpu_t *cpu, rvm_asmins_t *prog, rword off)
{
	rvm_asmins_t *pi;

	RVM_CPUREG_SETIP(cpu, PC, prog + off);
	cpu->abort = 0;
	cpu->error = 0;
	do {
		pi = RVM_CPUREG_GETIP(cpu, PC);
		RVM_CPUREG_SETU(cpu, DA, pi->data);
		ops[pi->opcode](cpu, pi);
#ifdef DEBUG
		if (RVM_CPUREG_GETTYPE(cpu, DA) || RVM_CPUREG_TSTFLAG(cpu, DA, RVM_INFOBIT_ALL))
			RVM_ABORT(cpu, RVM_E_ILLEGAL);
#endif
		if (cpu->abort)
			return -1;
		RVM_CPUREG_INCIP(cpu, PC, 1);
	} while (pi->opcode);
	return 0;
}


rint rvm_cpu_exec_debug(rvmcpu_t *cpu, rvm_asmins_t *prog, rword off)
{
	rvm_asmins_t *pi;

	RVM_CPUREG_SETIP(cpu, PC, prog + off);
	cpu->abort = 0;
	cpu->error = 0;
	do {
		pi = RVM_CPUREG_GETIP(cpu, PC);
		RVM_CPUREG_CLEAR(cpu, DA);
		RVM_CPUREG_SETU(cpu, DA, pi->data);
		ops[pi->opcode](cpu, pi);
#ifdef DEBUG
		if (RVM_CPUREG_GETTYPE(cpu, DA) || RVM_CPUREG_TSTFLAG(cpu, DA, RVM_INFOBIT_ALL))
			RVM_ABORT(cpu, RVM_E_ILLEGAL);
#endif
		if (cpu->abort)
			return -1;
		rvm_cpu_dumpregs(pi, cpu);		
		RVM_CPUREG_INCIP(cpu, PC, 1);
	} while (pi->opcode);
	return 0;
}


rint rvm_cpu_getswi(rvmcpu_t *cpu, const rchar *swiname)
{
	rint ntable, nswi;
	rvm_switable_t *swientry;

	for (ntable = 0; ntable < cpu->switables->len; ntable++) {
		swientry = r_array_index(cpu->switables, ntable, rvm_switable_t*);
		for (nswi = 0; swientry[nswi].name; nswi++) {
			if (r_strcmp(swientry[nswi].name, swiname) == 0)
				return (rint)RVM_SWI_ID(ntable, nswi);
		}
	}

	return -1;
}


rint rvmcpu_switable_add(rvmcpu_t *cpu, rvm_switable_t *switable)
{
	rint nswi;

	for (nswi = 0; switable[nswi].name; nswi++) {
		if (rvm_cpu_getswi(cpu, switable[nswi].name) >= 0)
			return -1;
	}

	return r_array_add(cpu->switables, &switable);
}


rvm_asmins_t rvm_asmp(rword opcode, rword op1, rword op2, rword op3, rpointer data)
{
	rvm_asmins_t a;

	r_memset(&a, 0, sizeof(a));
	a.opcode = (unsigned char) opcode;
	a.op1 = (unsigned char)op1;
	a.op2 = (unsigned char)op2;
	a.op3 = (unsigned char)op3;
	a.data = (rword)data;

//	RVM_REG_GETP(&a.data) = data;
//	RVM_REG_SETTYPE(&a.data, RVM_DTYPE_POINTER);
	return a;
}


rvm_asmins_t rvm_asm(rword opcode, rword op1, rword op2, rword op3, rword data)
{
	rvm_asmins_t a;

	r_memset(&a, 0, sizeof(a));
	a.opcode = (unsigned char) opcode;
	a.op1 = (unsigned char)op1;
	a.op2 = (unsigned char)op2;
	a.op3 = (unsigned char)op3;
	a.data = (rword)data;

//	RVM_REG_GETU(&a.data) = data;
//	RVM_REG_SETTYPE(&a.data, RVM_DTYPE_WORD);
	return a;
}


rvm_asmins_t rvm_asml(rword opcode, rword op1, rword op2, rword op3, rlong data)
{
	rvm_asmins_t a;

	r_memset(&a, 0, sizeof(a));
	a.opcode = (unsigned char) opcode;
	a.op1 = (unsigned char)op1;
	a.op2 = (unsigned char)op2;
	a.op3 = (unsigned char)op3;
	a.data = (rword)data;

//	RVM_REG_GETL(&a.data) = (rword)data;
//	RVM_REG_SETTYPE(&a.data, RVM_DTYPE_LONG);
	return a;
}



rvm_asmins_t rvm_asmr(rword opcode, rword op1, rword op2, rword op3, rpointer pReloc)
{
	rvm_asmins_t a;

	r_memset(&a, 0, sizeof(a));
	a.opcode = (unsigned char) opcode;
	a.op1 = (unsigned char)op1;
	a.op2 = (unsigned char)op2;
	a.op3 = (unsigned char)op3;
	a.data = (rword)pReloc;
	a.flags = RVM_ASMINS_RELOC | RVM_ASMINS_RELOCPTR;

//	RVM_REG_GETP(&a.data) = pReloc;
//	RVM_REG_SETTYPE(&a.data, RVM_DTYPE_RELOCPTR);
	return a;
}


rvm_asmins_t rvm_asmx(rword opcode, rword op1, rword op2, rword op3, rpointer pReloc)
{
	rvm_asmins_t a;

	r_memset(&a, 0, sizeof(a));
	a.opcode = (unsigned char) opcode;
	a.op1 = (unsigned char)op1;
	a.op2 = (unsigned char)op2;
	a.op3 = (unsigned char)op3;
	a.data = (rword)pReloc;
	a.flags = RVM_ASMINS_RELOC;

//	RVM_REG_GETP(&a.data) = pReloc;
//	RVM_REG_SETTYPE(&a.data, RVM_DTYPE_RELOCINDEX);
	return a;
}


void rvm_relocate(rvm_asmins_t *code, rsize_t size)
{
	rvm_asmins_t *reloc;
	rulong relocindex;
	rulong off;

	for (off = 0; off < size; off++, code++) {
		if (code->flags & RVM_ASMINS_RELOC) {
			if (code->flags & RVM_ASMINS_RELOCPTR) {
				reloc = *((rvm_asmins_t **)code->data);
				code->data = reloc - code;
			} else {
				relocindex = *((rulong *)code->data);
				code->data = relocindex - off;
			}
			code->flags &= ~(RVM_ASMINS_RELOCPTR | RVM_ASMINS_RELOC);
		}
	}
}

