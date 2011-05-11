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
#include <stdarg.h>
#include "rvmcpu.h"
#include "rvmoperator.h"
#include "rvmoperatorbin.h"
#include "rvmoperatorcast.h"
#include "rvmoperatornot.h"
#include "rvmoperatorlogicnot.h"
#include "rvmcodemap.h"
#include "rmem.h"
#include "rstring.h"
#include "rvmreg.h"
#include "rjsobject.h"

#define RVM_DEFAULT_STACKSIZE (4 * 1024)

static const char *stropcalls[] = {
	"RVM_EXT",
	"RVM_ABORT",
	"RVM_PRN",
	"RVM_ASR",
	"RVM_SWI",
	"RVM_SWIID",
	"RVM_CALL",
	"RVM_MOV",
	"RVM_MOVS",
	"RVM_SWP",
	"RVM_INC",
	"RVM_DEC",
	"RVM_ADD",
	"RVM_ADDS",
	"RVM_ADC",
	"RVM_AND",
	"RVM_BIC",
	"RVM_CLZ",
	"RVM_CMN",
	"RVM_XOR",
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
	"RVM_MOD",
	"RVM_MODS",
	"RVM_BX",
	"RVM_BXEQ",
	"RVM_BXNEQ",
	"RVM_BXLEQ",
	"RVM_BXGEQ",
	"RVM_BXLES",
	"RVM_BXGRE",
	"RVM_BXL",
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
	"RVM_CFLAG",
	"RVM_CLR",
	"RVM_CLRR",
	"RVM_LSL",
	"RVM_LSR",
	"RVM_LSRS",
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
	"RVM_ADDRS",

	"RVM_CAST",		/* Cast: op1 = (op3)op2 */
	"RVM_TYPE",		/* Type: op1 = typeof(op2) */
	"RVM_SETTYPE",	/* Type: op1.type = op2 */
	"RVM_EMOV",
	"RVM_ENEG",
	"RVM_EADD",
	"RVM_ESUB",
	"RVM_EMUL",
	"RVM_EDIV",
	"RVM_EMOD",
	"RVM_ELSL",
	"RVM_ELSR",
	"RVM_ELSRU",
	"RVM_EAND",
	"RVM_EORR",
	"RVM_EXOR",
	"RVM_ENOT",
	"RVM_ELAND",
	"RVM_ELOR",
	"RVM_ELNOT",
	"RVM_EEQ",
	"RVM_ENOTEQ",
	"RVM_EGREAT",
	"RVM_EGREATEQ",
	"RVM_ELESS",
	"RVM_ELESSEQ",
	"RVM_ECMP",
	"RVM_ECMN",
	"RVM_ALLOCSTR",
	"RVM_ALLOCARR",
	"RVM_ADDRA",
	"RVM_LDA",
	"RVM_STA",
	"RVM_ALLOCOBJ",
	"RVM_ADDROBJN",
	"RVM_ADDROBJH",
	"RVM_LDOBJN",
	"RVM_STOBJN",
	"RVM_LDOBJH",
	"RVM_STOBJH",
	"RVM_OBJLKUP",
	"RVM_OBJADD",
	"RVM_OBJLKUPADD",
	"UNKNOWN",
	"UNKNOWN",
	"UNKNOWN",
	"UNKNOWN",
	"UNKNOWN",
	"UNKNOWN",
	"UNKNOWN",
	"UNKNOWN",
	"UNKNOWN",
	"UNKNOWN",
	"UNKNOWN",
};


static void rvm_op_b(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword pc = 0;

//	if (ins->op1 != XX)
//		pc += RVM_CPUREG_GETU(cpu, ins->op1);
//	if (ins->op2 != XX)
//		pc += RVM_CPUREG_GETU(cpu, ins->op2);
//	if (ins->op3 != XX)
//		pc += RVM_CPUREG_GETU(cpu, ins->op3);
	pc += RVM_CPUREG_GETU(cpu, ins->op1);
	RVM_CPUREG_INCIP(cpu, PC, pc - 1);
}


static void rvm_op_beq(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword pc = 0;

	if ((cpu->status & RVM_STATUS_Z)) {
//		if (ins->op1 != XX)
//			pc += RVM_CPUREG_GETU(cpu, ins->op1);
//		if (ins->op2 != XX)
//			pc += RVM_CPUREG_GETU(cpu, ins->op2);
//		if (ins->op3 != XX)
//			pc += RVM_CPUREG_GETU(cpu, ins->op3);
		pc += RVM_CPUREG_GETU(cpu, ins->op1);
		RVM_CPUREG_INCIP(cpu, PC, pc - 1);
	}
}


static void rvm_op_bneq(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword pc = 0;

	if ((cpu->status & RVM_STATUS_Z) == 0) {
//		if (ins->op1 != XX)
//			pc += RVM_CPUREG_GETU(cpu, ins->op1);
//		if (ins->op2 != XX)
//			pc += RVM_CPUREG_GETU(cpu, ins->op2);
//		if (ins->op3 != XX)
//			pc += RVM_CPUREG_GETU(cpu, ins->op3);
		pc += RVM_CPUREG_GETU(cpu, ins->op1);
		RVM_CPUREG_INCIP(cpu, PC, pc - 1);
	}
}


static void rvm_op_bleq(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword pc = 0;

	if ((cpu->status & RVM_STATUS_N) || (cpu->status & RVM_STATUS_Z)) {
//		if (ins->op1 != XX)
//			pc += RVM_CPUREG_GETU(cpu, ins->op1);
//		if (ins->op2 != XX)
//			pc += RVM_CPUREG_GETU(cpu, ins->op2);
//		if (ins->op3 != XX)
//			pc += RVM_CPUREG_GETU(cpu, ins->op3);
		pc += RVM_CPUREG_GETU(cpu, ins->op1);
		RVM_CPUREG_INCIP(cpu, PC, pc - 1);
	}
}

static void rvm_op_bgeq(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword pc = 0;

	if ((cpu->status & RVM_STATUS_N) == 0 || (cpu->status & RVM_STATUS_Z) == 1) {
//		if (ins->op1 != XX)
//			pc += RVM_CPUREG_GETU(cpu, ins->op1);
//		if (ins->op2 != XX)
//			pc += RVM_CPUREG_GETU(cpu, ins->op2);
//		if (ins->op3 != XX)
//			pc += RVM_CPUREG_GETU(cpu, ins->op3);
		pc += RVM_CPUREG_GETU(cpu, ins->op1);
		RVM_CPUREG_INCIP(cpu, PC, pc - 1);
	}
}


static void rvm_op_bles(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword pc = 0;


	if ((cpu->status & RVM_STATUS_N)) {
//		if (ins->op1 != XX)
//			pc += RVM_CPUREG_GETU(cpu, ins->op1);
//		if (ins->op2 != XX)
//			pc += RVM_CPUREG_GETU(cpu, ins->op2);
//		if (ins->op3 != XX)
//			pc += RVM_CPUREG_GETU(cpu, ins->op3);
		pc += RVM_CPUREG_GETU(cpu, ins->op1);
		RVM_CPUREG_INCIP(cpu, PC, pc - 1);
	}
}


static void rvm_op_bgre(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword pc = 0;

	if ((cpu->status & RVM_STATUS_N) == 0 && (cpu->status & RVM_STATUS_Z) == 0) {
//		if (ins->op1 != XX)
//			pc += RVM_CPUREG_GETU(cpu, ins->op1);
//		if (ins->op2 != XX)
//			pc += RVM_CPUREG_GETU(cpu, ins->op2);
//		if (ins->op3 != XX)
//			pc += RVM_CPUREG_GETU(cpu, ins->op3);
		pc += RVM_CPUREG_GETU(cpu, ins->op1);
		RVM_CPUREG_INCIP(cpu, PC, pc - 1);
	}
}


static void rvm_op_bx(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_CPUREG_SETIP(cpu, PC, RVM_CPUREG_GETIP(cpu, ins->op1));
}


static void rvm_op_bxleq(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	if ((cpu->status & RVM_STATUS_N) || (cpu->status & RVM_STATUS_Z)) {
		RVM_CPUREG_SETIP(cpu, PC, RVM_CPUREG_GETIP(cpu, ins->op1));
	}
}


static void rvm_op_bxgeq(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	if ((cpu->status & RVM_STATUS_N) == 0 || (cpu->status & RVM_STATUS_Z) == 1){
		RVM_CPUREG_SETIP(cpu, PC, RVM_CPUREG_GETIP(cpu, ins->op1));
	}
}


static void rvm_op_bxles(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	if ((cpu->status & RVM_STATUS_N)) {
		RVM_CPUREG_SETIP(cpu, PC, RVM_CPUREG_GETIP(cpu, ins->op1));
	}
}


static void rvm_op_bxgre(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	if ((cpu->status & RVM_STATUS_N) == 0 && (cpu->status & RVM_STATUS_Z) == 0) {
		RVM_CPUREG_SETIP(cpu, PC, RVM_CPUREG_GETIP(cpu, ins->op1));
	}
}


static void rvm_op_bxeq(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	if ((cpu->status & RVM_STATUS_Z)) {
		RVM_CPUREG_SETIP(cpu, PC, RVM_CPUREG_GETIP(cpu, ins->op1));
	}
}


static void rvm_op_bxneq(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	if (!(cpu->status & RVM_STATUS_Z)) {
		RVM_CPUREG_SETIP(cpu, PC, RVM_CPUREG_GETIP(cpu, ins->op1));
	}
}


static void rvm_op_bxl(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_CPUREG_SETIP(cpu, LR, RVM_CPUREG_GETIP(cpu, PC));
	RVM_CPUREG_SETIP(cpu, PC, RVM_CPUREG_GETIP(cpu, ins->op1));
}


static void rvm_op_bl(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword pc = 0;

//	if (ins->op1 != XX)
//		pc += RVM_CPUREG_GETU(cpu, ins->op1);
//	if (ins->op2 != XX)
//		pc += RVM_CPUREG_GETU(cpu, ins->op2);
//	if (ins->op3 != XX)
//		pc += RVM_CPUREG_GETU(cpu, ins->op3);
	pc += RVM_CPUREG_GETU(cpu, ins->op1);
	RVM_CPUREG_SETIP(cpu, LR, RVM_CPUREG_GETIP(cpu, PC));
	RVM_CPUREG_INCIP(cpu, PC, pc - 1);
}


static void rvm_op_exit(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	cpu->abort = 1;
}


static void rvm_op_mov(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_CPUREG_SET(cpu, ins->op1, RVM_CPUREG_GET(cpu, ins->op2));
}


static void rvm_op_movs(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword op2 = RVM_CPUREG_GETU(cpu, ins->op2);;

	RVM_CPUREG_SETU(cpu, ins->op1, op2);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !op2);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, op2 & RVM_SIGN_BIT);
}


static void rvm_op_swp(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t tmp = RVM_CPUREG_GET(cpu, ins->op1);
	RVM_CPUREG_SET(cpu, ins->op1, RVM_CPUREG_GET(cpu, ins->op2));
	RVM_CPUREG_SET(cpu, ins->op2, tmp);
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


static void rvm_op_clr(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_REG_CLEAR(((rvmreg_t*)RVM_CPUREG_PTR(cpu, ins->op1)));
	RVM_REG_SETTYPE(((rvmreg_t*)RVM_CPUREG_PTR(cpu, ins->op1)), RVM_DTYPE_UNDEF);
}


static void rvm_op_cflag(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword op1 = RVM_CPUREG_GETU(cpu, ins->op1);
	RVM_STATUS_CLRBIT(cpu, op1);
}


static void rvm_op_clrr(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_REG_CLEAR(((rvmreg_t*)RVM_CPUREG_GETP(cpu, ins->op1)));
	RVM_REG_SETTYPE(((rvmreg_t*)RVM_CPUREG_GETP(cpu, ins->op1)), RVM_DTYPE_UNDEF);
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
	rvmreg_t *dest = RVM_CPUREG_PTR(cpu, ins->op2);
	if (RVM_REG_GETTYPE(dest) != RVM_DTYPE_POINTER)
		RVM_ABORT(cpu, RVM_E_LVALUE);
	else
		*((rvmreg_t*)RVM_REG_GETP(dest)) = RVM_CPUREG_GET(cpu, ins->op1);
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


static void rvm_op_lsrs(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rsword res, op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);

	res = ((rsword)op2) >> op3;
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
	rword res, op1 = RVM_CPUREG_GETU(cpu, ins->op1), op2 = RVM_CPUREG_GETU(cpu, ins->op2);
	
	res = op1 & op2;
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void rvm_op_xor(rvmcpu_t *cpu, rvm_asmins_t *ins)
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


static void rvm_op_inc(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_CPUREG_SETU(cpu, ins->op1, RVM_CPUREG_GETU(cpu, ins->op1) + 1);
}


static void rvm_op_dec(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_CPUREG_SETU(cpu, ins->op1, RVM_CPUREG_GETU(cpu, ins->op1) - 1);
}


static void rvm_op_swi(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmcpu_swi swi;
	rvm_switable_t *switable;
	ruint ntable = (ruint) RVM_SWI_TABLE(ins->swi);
	ruint nswi = (ruint) RVM_SWI_NUM(ins->swi);

	if (r_harray_length(cpu->switables) <= ntable)
		RVM_ABORT(cpu, RVM_E_SWITABLE);
	switable = r_harray_index(cpu->switables, ntable, rvm_switable_t*);
	swi = switable[nswi].op;
	swi(cpu, ins);
}


static void rvm_op_swiid(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmcpu_swi swi;
	rvm_switable_t *switable;
	ruint ntable = (ruint) RVM_SWI_TABLE(RVM_CPUREG_GETU(cpu, ins->op1));
	ruint nswi = (ruint) RVM_SWI_NUM(RVM_CPUREG_GETU(cpu, ins->op1));

	if (r_harray_length(cpu->switables) <= ntable)
		RVM_ABORT(cpu, RVM_E_SWITABLE);
	switable = r_harray_index(cpu->switables, ntable, rvm_switable_t*);
	swi = switable[nswi].op;
	swi(cpu, ins);
}


static void rvm_op_call(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);

	if (RVM_REG_GETTYPE(arg1) == RVM_DTYPE_SWIID) {
		RVM_CPUREG_SETIP(cpu, LR, RVM_CPUREG_GETIP(cpu, PC));
		rvm_op_swiid(cpu, ins);
	} else if (RVM_REG_GETTYPE(arg1) == RVM_DTYPE_FUNCTION) {
		rvm_op_bxl(cpu, ins);
	} else {
		RVM_ABORT(cpu, RVM_E_NOTFUNCTION);
	}
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


static void rvm_op_mod(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	if (!RVM_CPUREG_GETU(cpu, ins->op3))
		RVM_ABORT(cpu, RVM_E_DIVZERO);

	RVM_CPUREG_SETU(cpu, ins->op1, RVM_CPUREG_GETU(cpu, ins->op2) % RVM_CPUREG_GETU(cpu, ins->op3));
}


static void rvm_op_mods(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rsword res, op2 = RVM_CPUREG_GETU(cpu, ins->op2), op3 = RVM_CPUREG_GETU(cpu, ins->op3);

	if (!op3)
		RVM_ABORT(cpu, RVM_E_DIVZERO);
	res = op2 % op3;
	r_printf("mod RES: %ld\n", res);
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

	if (!RVM_STACK_CHECKSIZE(cpu, cpu->stack, sp))
		RVM_ABORT(cpu, RVM_E_NOMEM);
	RVM_STACK_WRITE(cpu->stack, sp, &RVM_CPUREG_GET(cpu, ins->op1));
	RVM_CPUREG_SETU(cpu, SP, sp);
}


static void rvm_op_lds(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword sp = ((ins->op2 != XX) ? RVM_CPUREG_GETU(cpu, ins->op2) : 0) + ((ins->op3 != XX) ? RVM_CPUREG_GETU(cpu, ins->op3) : 0);

	RVM_CPUREG_SET(cpu, ins->op1, RVM_STACK_READ(cpu->stack, sp));
}


static void rvm_op_sts(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword sp = ((ins->op2 != XX) ? RVM_CPUREG_GETU(cpu, ins->op2) : 0) + ((ins->op3 != XX) ? RVM_CPUREG_GETU(cpu, ins->op3) : 0);

	if (!RVM_STACK_CHECKSIZE(cpu, cpu->stack, sp))
		RVM_ABORT(cpu, RVM_E_NOMEM);
	RVM_STACK_WRITE(cpu->stack, sp, &RVM_CPUREG_GET(cpu, ins->op1));
}


static void rvm_op_pushm(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword n, i = 0;
	rword bits = RVM_CPUREG_GETU(cpu, ins->op1);
	rword sp = RVM_CPUREG_GETU(cpu, SP);

	if (!RVM_STACK_CHECKSIZE(cpu, cpu->stack, sp + RVM_REGS_NUM))
		RVM_ABORT(cpu, RVM_E_NOMEM);

	if (!(bits & ((1<<(RVM_REGS_NUM / 2)) - 1)))
		i = RVM_REGS_NUM / 2;
	for (;bits && i < RLST; i++) {
		n = 1 << i;
		if (n & bits) {
			sp += 1;
			RVM_STACK_WRITE(cpu->stack, sp, &RVM_CPUREG_GET(cpu, i));
			bits &= ~n;
		}
	}
	RVM_CPUREG_SETU(cpu, SP, sp);
}


static void rvm_op_popm(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	int n, i = RLST - 1;
	rword bits = RVM_CPUREG_GETU(cpu, ins->op1);
	rword savedbits = bits;
	rword sp = RVM_CPUREG_GETU(cpu, SP);

	if (!(bits & ~((1 << (RVM_REGS_NUM / 2)) - 1)))
		i = RVM_REGS_NUM / 2 - 1;
	for (; bits && i >= 0; i--) {
		n = 1 << i;
		if (n & bits) {
			RVM_CPUREG_SET(cpu, i, RVM_STACK_READ(cpu->stack, sp));
			sp -= 1;
			bits &= ~n;
		}
	}
	if (!(((rword)(1 << SP)) & savedbits))
		RVM_CPUREG_SETU(cpu, SP, sp);
}


static void rvm_op_pop(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword sp = RVM_CPUREG_GETU(cpu, SP);

	RVM_CPUREG_SET(cpu, ins->op1, RVM_STACK_READ(cpu->stack, sp));
	if (ins->op1 != SP)
		RVM_CPUREG_SETU(cpu, SP, sp - 1);
}


static void rvm_op_addrs(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword sp = ((ins->op2 != XX) ? RVM_CPUREG_GETU(cpu, ins->op2) : 0) + ((ins->op3 != XX) ? RVM_CPUREG_GETU(cpu, ins->op3) : 0);

	if (!RVM_STACK_CHECKSIZE(cpu, cpu->stack, sp))
		RVM_ABORT(cpu, RVM_E_NOMEM);

	RVM_CPUREG_CLEAR(cpu, ins->op1);
	RVM_CPUREG_SETTYPE(cpu, ins->op1, RVM_DTYPE_POINTER);
	RVM_CPUREG_SETP(cpu, ins->op1, RVM_STACK_ADDR(cpu->stack, sp));
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
			RVM_CPUREG_CLEAR(cpu, n);
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
	else if (reg == IP)
		ret = rvm_snprintf(str, size, "IP ");
	else if (reg == TP)
		ret = rvm_snprintf(str, size, "TP ");
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
	else
		ret = rvm_snprintf(str, size, "R%d ",  reg);

	return ret;
}


static void rvm_op_prn(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *r = RVM_CPUREG_PTR(cpu, ins->op1);

	if (rvm_reg_gettype(r) == RVM_DTYPE_UNSIGNED)
		rvm_printf("(UNSIGNED) R%d = %lu(0x%lx)\n", ins->op1, RVM_REG_GETU(r), RVM_REG_GETU(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_BOOLEAN)
		rvm_printf("(UNSIGNED) R%d = %lu(0x%lx)\n", ins->op1, RVM_REG_GETU(r), RVM_REG_GETU(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_POINTER)
		rvm_printf("(POINTER) R%d = %p\n", ins->op1, RVM_REG_GETP(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_LONG)
		rvm_printf("(LONG) R%d = %ld\n", ins->op1, RVM_REG_GETL(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_DOUBLE)
		rvm_printf("(DOUBLE) R%d = %0.2f\n", ins->op1, RVM_REG_GETD(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_STRING)
		rvm_printf("(STRING) R%d = %s\n", ins->op1, ((rstring_t*) RVM_REG_GETP(r))->s.str);
	else if (rvm_reg_gettype(r) == RVM_DTYPE_ARRAY)
		rvm_printf("(ARRAY) R%d = %p\n", ins->op1, RVM_REG_GETP(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_JSOBJECT)
		rvm_printf("(Object) R%d = %p\n", ins->op1, RVM_REG_GETP(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_SWIID)
		rvm_printf("(SWI) R%d = %p\n", ins->op1, RVM_REG_GETP(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_FUNCTION)
		rvm_printf("(FUNCTION) R%d = %p\n", ins->op1, RVM_REG_GETP(r));
	else
		rvm_printf("(UNKNOWN) R%d = ?\n", ins->op1);
}


int rvm_asm_dump_pi_to_str(rvmcpu_t *vm, rvm_asmins_t *pi, char *str, ruint size)
{
	int ret = 0, sz = size;

	if (pi->opcode == RVM_SWI) {
		rchar szSwi[64];
		r_memset(szSwi, 0, sizeof(szSwi));
		if (!vm) {
			rvm_snprintf(szSwi, sizeof(szSwi) - 1, "%s(%x)", stropcalls[pi->opcode], (ruint32)pi->swi);
		} else {
			rvm_switable_t *switable;
			ruint ntable = (ruint) RVM_SWI_TABLE(pi->swi);
			ruint nswi = (ruint) RVM_SWI_NUM(pi->swi);

			if (ntable < r_harray_length(vm->switables)) {
				switable = r_harray_index(vm->switables, ntable, rvm_switable_t*);
				rvm_snprintf(szSwi, sizeof(szSwi) - 1, "(%s)",  switable[nswi].name ? switable[nswi].name : "unknown");
			}
		}
		if ((ret = rvm_snprintf(str, sz, "%16s   ", szSwi)) < 0)
			return ret;
	} else {
		if ((ret = rvm_snprintf(str, sz, "%16s   ", stropcalls[pi->opcode])) < 0)
			return ret;
	}
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


	if (RVM_REG_GETTYPE(&pi->data) == RVM_DTYPE_DOUBLE) {
		if ((ret = rvm_snprintf(str, sz, "%f  ", RVM_REG_GETD(&pi->data))) < 0)
			return ret;
	} else if (RVM_REG_GETTYPE(&pi->data) == RVM_DTYPE_LONG) {
		if ((ret = rvm_snprintf(str, sz, "%ld  ", RVM_REG_GETL(&pi->data))) < 0)
			return ret;
	} else {
		if ((ret = rvm_snprintf(str, sz, "0x%lx  ", (unsigned long)RVM_REG_GETU(&pi->data))) < 0)
			return ret;
	}

	str += ret;
	sz -= ret;

	return size - sz;
}


void rvm_asm_dump(rvm_asmins_t *pi, ruint count)
{
	char buffer[256];
	while (count) {
		rvm_asm_dump_pi_to_str(NULL, pi, buffer, sizeof(buffer));
		rvm_printf("%s\n", buffer);
		++pi;
		--count;
	}
}


void rvm_cpu_dumpregs(rvmcpu_t *vm, rvm_asmins_t *pi)
{
    int ret;
	char buffer[1024];
	
	ret = rvm_asm_dump_pi_to_str(vm, pi, buffer, sizeof(buffer));
	if (ret < 0)
		return;
    ret = rvm_snprintf(buffer + ret, sizeof(buffer) - ret, "                                                                                        ");
	buffer[50] = '\0';
	rvm_printf("%s", buffer);

	rvm_printf("0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, TP=%p, FP=%ld, SP=%ld, LR=0x%lx, PC=0x%lx, DA=0x%lx, S( %c%c%c%c%c )",
   		RVM_CPUREG_GETU(vm, 0), RVM_CPUREG_GETU(vm, 1), RVM_CPUREG_GETU(vm, 2), RVM_CPUREG_GETU(vm, 3),
   		RVM_CPUREG_GETU(vm, 4), RVM_CPUREG_GETU(vm, 5), RVM_CPUREG_GETU(vm, 6), RVM_CPUREG_GETU(vm, 7),
   		RVM_CPUREG_GETU(vm, 8), RVM_CPUREG_GETP(vm, TP), (long int)RVM_CPUREG_GETU(vm, FP), (long int)RVM_CPUREG_GETU(vm, SP),
   		(long int)RVM_CPUREG_GETU(vm, LR), (long int)RVM_CPUREG_GETU(vm, PC), RVM_CPUREG_GETU(vm, DA),
   		vm->status & RVM_STATUS_E ? 'E' : '_',
   		vm->status & RVM_STATUS_V ? 'V' : '_',
   		vm->status & RVM_STATUS_C ? 'C' : '_',
   		vm->status & RVM_STATUS_N ? 'N' : '_',
   		vm->status & RVM_STATUS_Z ? 'Z' : '_');
	rvm_printf("\n");
}


static void rvm_op_cast(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_type_t type = (rvmreg_type_t)RVM_CPUREG_GETU(cpu, ins->op3);
	rvmreg_t tmp;

	RVM_REG_CLEAR(&tmp);
	RVM_REG_SETTYPE(&tmp, type);
	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_CAST, cpu, RVM_CPUREG_PTR(cpu, ins->op1), RVM_CPUREG_PTR(cpu, ins->op2), &tmp);
}


static void rvm_op_type(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_type_t type = (rvmreg_type_t)RVM_CPUREG_GETTYPE(cpu, ins->op2);

	RVM_CPUREG_SETU(cpu, ins->op1, type);
}


static void rvm_op_settype(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_type_t type = (rvmreg_type_t)RVM_CPUREG_GETU(cpu, ins->op2);

	RVM_CPUREG_SETTYPE(cpu, ins->op1, type);
}


static void rvm_op_emov(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvm_op_mov(cpu, ins);
}


static void rvm_op_eadd(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_ADD, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rvm_op_esub(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_SUB, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rvm_op_eneg(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t zero;

	rvm_reg_setunsigned(&zero, 0);
	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_SUB, cpu, RVM_CPUREG_PTR(cpu, ins->op1), &zero, arg2);
}



static void rvm_op_emul(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_MUL, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rvm_op_ediv(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_DIV, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rvm_op_emod(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_MOD, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rvm_op_elsl(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_LSL, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rvm_op_elsr(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_LSR, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rvm_op_elsru(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_LSRU, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rvm_op_eand(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_AND, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rvm_op_eorr(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_OR, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rvm_op_exor(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_XOR, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rvm_op_enot(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);

	rvm_opmap_invoke_unary_handler(cpu->opmap, RVM_OPID_NOT, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2);
}


static void rvm_op_eland(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_LOGICAND, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rvm_op_elor(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_LOGICOR, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rvm_op_elnot(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);

	rvm_opmap_invoke_unary_handler(cpu->opmap, RVM_OPID_LOGICNOT, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2);
}


static void rvm_op_eeq(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_EQ, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rvm_op_enoteq(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_NOTEQ, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rvm_op_egreat(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_GREATER, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rvm_op_egreateq(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_GREATEREQ, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rvm_op_elesseq(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_LESSEQ, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rvm_op_eless(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_LESS, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
}


static void rvm_op_ecmp(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_CMP, cpu, NULL, arg1, arg2);
}


static void rvm_op_ecmn(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_CMN, cpu, NULL, arg1, arg2);
}


static void rvm_op_allocstr(rvmcpu_t *cpu, rvm_asmins_t *ins)
{

	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rstring_t *s = r_string_create_strsize((const rchar*)RVM_CPUREG_GETP(cpu, ins->op2), RVM_CPUREG_GETU(cpu, ins->op3));
	if (!s) {
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	}
	rvm_gc_add(cpu->gc, (robject_t*)s);
	rvm_reg_setstring(arg1, s);
}


static void rvm_op_allocobj(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rjs_object_t *a = rjs_object_create(sizeof(rvmreg_t));
	if (!a) {
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	}
//	r_carray_setlength(a, RVM_CPUREG_GETU(cpu, ins->op2));
	rvm_gc_add(cpu->gc, (robject_t*)a);
	rvm_reg_setjsobject(arg1, (robject_t*)a);
}


static void rvm_op_allocarr(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	ruint size = RVM_CPUREG_GETU(cpu, ins->op2);
	rcarray_t *a = r_carray_create_rvmreg();
	if (!a) {
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	}
	r_carray_setlength(a, size);
	rvm_gc_add(cpu->gc, (robject_t*)a);
	rvm_reg_setarray(arg1, (robject_t*)a);
}


static void rvm_op_addrobjn(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t tmp = rvm_reg_create_long(0);
	rjs_object_t *a;
	rlong index;

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_CAST, cpu, &tmp, RVM_CPUREG_PTR(cpu, ins->op3), &tmp);
	index = RVM_REG_GETL(&tmp);
	if (rvm_reg_gettype(arg2) != RVM_DTYPE_JSOBJECT)
		RVM_ABORT(cpu, RVM_E_NOTOBJECT);
	if (index < 0)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	a = (rjs_object_t*)RVM_REG_GETP(arg2);
	RVM_REG_CLEAR(arg1);
	RVM_REG_SETTYPE(arg1, RVM_DTYPE_POINTER);
	RVM_REG_SETP(arg1, r_carray_slot_expand(a->narray, index));
}


static void rvm_op_ldobjn(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t tmp = rvm_reg_create_long(0);
	rjs_object_t *a = NULL;
	rlong index;

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_CAST, cpu, &tmp, RVM_CPUREG_PTR(cpu, ins->op3), &tmp);
	index = RVM_REG_GETL(&tmp);
	if (rvm_reg_gettype(arg2) != RVM_DTYPE_JSOBJECT)
		RVM_ABORT(cpu, RVM_E_NOTOBJECT);
	if (index < 0)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	a = (rjs_object_t*)RVM_REG_GETP(arg2);
	*arg1 = *((rvmreg_t*)r_carray_slot_expand(a->narray, index));
}


static void rvm_op_stobjn(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t tmp = rvm_reg_create_long(0);
	rjs_object_t *a = NULL;
	rlong index;

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_CAST, cpu, &tmp, RVM_CPUREG_PTR(cpu, ins->op3), &tmp);
	index = RVM_REG_GETL(&tmp);
	if (rvm_reg_gettype(arg2) != RVM_DTYPE_JSOBJECT)
		RVM_ABORT(cpu, RVM_E_NOTOBJECT);
	if (index < 0)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	a = (rjs_object_t*)RVM_REG_GETP(arg2);
	r_carray_replace(a->narray, index, arg1);
}


static void rvm_op_objlookup(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rlong index;
	rjs_object_t *a = (rjs_object_t*)RVM_REG_GETP(arg2);

	if (rvm_reg_gettype(arg2) != RVM_DTYPE_JSOBJECT) {
		RVM_ABORT(cpu, RVM_E_NOTOBJECT);
	}
	index = r_harray_lookup(a->harray, RVM_CPUREG_GETP(cpu, ins->op3), RVM_CPUREG_GETL(cpu, ins->op1));

	RVM_REG_CLEAR(arg1);
	RVM_REG_SETTYPE(arg1, RVM_DTYPE_LONG);
	RVM_REG_SETL(arg1, index);
}


static void rvm_op_objkeyadd(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rlong index;
	rjs_object_t *a = (rjs_object_t*)RVM_REG_GETP(arg2);

	if (rvm_reg_gettype(arg2) != RVM_DTYPE_JSOBJECT) {
		RVM_ABORT(cpu, RVM_E_NOTOBJECT);
	}
	index = r_harray_add(a->harray, RVM_CPUREG_GETP(cpu, ins->op3), RVM_CPUREG_GETL(cpu, ins->op1), NULL);

	RVM_REG_CLEAR(arg1);
	RVM_REG_SETTYPE(arg1, RVM_DTYPE_LONG);
	RVM_REG_SETL(arg1, index);
}


static void rvm_op_objkeylookupadd(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rlong index;
	rjs_object_t *a = (rjs_object_t*)RVM_REG_GETP(arg2);

	if (rvm_reg_gettype(arg2) != RVM_DTYPE_JSOBJECT) {
		RVM_ABORT(cpu, RVM_E_NOTOBJECT);
	}
	index = r_harray_lookup(a->harray, RVM_CPUREG_GETP(cpu, ins->op3), RVM_CPUREG_GETL(cpu, ins->op1));
	if (index < 0)
		index = r_harray_add(a->harray, RVM_CPUREG_GETP(cpu, ins->op3), RVM_CPUREG_GETL(cpu, ins->op1), NULL);

	RVM_REG_CLEAR(arg1);
	RVM_REG_SETTYPE(arg1, RVM_DTYPE_LONG);
	RVM_REG_SETL(arg1, index);
}


static void rvm_op_addrobjh(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t tmp = rvm_reg_create_long(0);
	rjs_object_t *a;
	rlong index;

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_CAST, cpu, &tmp, RVM_CPUREG_PTR(cpu, ins->op3), &tmp);
	index = RVM_REG_GETL(&tmp);
	if (rvm_reg_gettype(arg2) != RVM_DTYPE_JSOBJECT)
		RVM_ABORT(cpu, RVM_E_NOTOBJECT);
	if (index < 0)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	a = (rjs_object_t*)RVM_REG_GETP(arg2);
	RVM_REG_CLEAR(arg1);
	RVM_REG_SETTYPE(arg1, RVM_DTYPE_POINTER);
	RVM_REG_SETP(arg1, r_carray_slot_expand(a->harray->members, index));
}


static void rvm_op_ldobjh(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t tmp = rvm_reg_create_long(0);
	rjs_object_t *a = NULL;
	rlong index;

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_CAST, cpu, &tmp, RVM_CPUREG_PTR(cpu, ins->op3), &tmp);
	index = RVM_REG_GETL(&tmp);
	if (rvm_reg_gettype(arg2) != RVM_DTYPE_JSOBJECT)
		RVM_ABORT(cpu, RVM_E_NOTOBJECT);
	a = (rjs_object_t*)RVM_REG_GETP(arg2);
	if (index < 0) {
		RVM_REG_CLEAR(arg1);
		RVM_REG_SETTYPE(arg1, RVM_DTYPE_UNDEF);
	} else {
		*arg1 = *((rvmreg_t*)r_carray_slot_expand(a->harray->members, index));
	}
}


static void rvm_op_stobjh(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t tmp = rvm_reg_create_long(0);
	rjs_object_t *a = NULL;
	rlong index;

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_CAST, cpu, &tmp, RVM_CPUREG_PTR(cpu, ins->op3), &tmp);
	index = RVM_REG_GETL(&tmp);
	if (rvm_reg_gettype(arg2) != RVM_DTYPE_JSOBJECT)
		RVM_ABORT(cpu, RVM_E_NOTOBJECT);
	if (index < 0)
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	a = (rjs_object_t*)RVM_REG_GETP(arg2);
	r_carray_replace(a->harray->members, index, arg1);
}


static void rvm_op_addra(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t tmp = rvm_reg_create_long(0);
	rcarray_t *a = RVM_REG_GETP(arg2);
	rlong index;

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_CAST, cpu, &tmp, RVM_CPUREG_PTR(cpu, ins->op3), &tmp);
	index = RVM_REG_GETL(&tmp);

	if (rvm_reg_gettype(arg2) != RVM_DTYPE_ARRAY || index < 0) {
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	}
	RVM_REG_CLEAR(arg1);
	RVM_REG_SETTYPE(arg1, RVM_DTYPE_POINTER);
	RVM_REG_SETP(arg1, r_carray_slot_expand(a, index));
}


static void rvm_op_lda(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	ruint index = RVM_CPUREG_GETU(cpu, ins->op3);
	rcarray_t *a = RVM_REG_GETP(arg2);

	if (rvm_reg_gettype(arg2) != RVM_DTYPE_ARRAY) {
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	}
	*arg1 = *((rvmreg_t*)r_carray_slot_expand(a, index));
}


static void rvm_op_sta(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg1 = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	ruint index = RVM_CPUREG_GETU(cpu, ins->op3);
	rcarray_t *a = RVM_REG_GETP(arg2);

	if (rvm_reg_gettype(arg2) != RVM_DTYPE_ARRAY) {
		RVM_ABORT(cpu, RVM_E_ILLEGAL);
	}
	r_carray_replace(a, index, arg1);
}


static void rvm_op_abort(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_ABORT(cpu, RVM_CPUREG_GETU(cpu, ins->op1));
}


static rvm_cpu_op ops[] = {
	rvm_op_exit,		// RVM_EXT
	rvm_op_abort,		// RVM_ABORT
	rvm_op_prn,			// RVM_PRN
	rvm_op_asr,			// RVM_ASR
	rvm_op_swi,			// RVM_SWI
	rvm_op_swiid,		// RVM_SWIID
	rvm_op_call,		// RVM_CALL
	rvm_op_mov,			// RVM_MOV
	rvm_op_movs,		// RVM_MOVS
	rvm_op_swp,			// RVM_SWP
	rvm_op_inc,			// RVM_INC
	rvm_op_dec,			// RVM_DEC
	rvm_op_add,			// RVM_ADD
	rvm_op_adds,		// RVM_ADDS
	rvm_op_adc,			// RVM_ADC
	rvm_op_and,			// RVM_AND
	rvm_op_bic,			// RVM_BIC
	rvm_op_clz,			// RVM_CLZ
	rvm_op_cmn,			// RVM_CMN
	rvm_op_xor,			// RVM_XOR
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
	rvm_op_mod,			// RVM_MOD
	rvm_op_mods,		// RVM_MODS
	rvm_op_bx,			// RVM_BX
	rvm_op_bxeq,		// RVM_BXEQ
	rvm_op_bxneq,		// RVM_BXNEQ
	rvm_op_bxleq,		// RVM_BXLEQ
	rvm_op_bxgeq,		// RVM_BXGEQ
	rvm_op_bxles,		// RVM_BXLES
	rvm_op_bxgre,		// RVM_BXGRE
	rvm_op_bxl,			// RVM_BXL
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
	rvm_op_cflag,		// RVM_CFLAG
	rvm_op_clr,			// RVM_CLR
	rvm_op_clrr,		// RVM_CLRR
	rvm_op_lsl,			// RVM_LSL
	rvm_op_lsr,			// RVM_LSR
	rvm_op_lsrs,		// RVM_LSRS
	rvm_op_stm,			// RVM_STM
	rvm_op_ldm,			// RVM_LDM
	rvm_op_sts,			// RVM_STS
	rvm_op_lds,			// RVM_LDS
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
	rvm_op_addrs, 		// RVM_ADDRS

/* Extended VM instructions */
	rvm_op_cast,		// RVM_CAST
	rvm_op_type,		// RVM_TYPE
	rvm_op_settype,		// RVM_SETTYPE
	rvm_op_emov,		// RVM_EMOV
	rvm_op_eneg,		// RVM_ENEG
	rvm_op_eadd,		// RVM_EADD
	rvm_op_esub,		// RVM_ESUB
	rvm_op_emul,		// RVM_EMUL
	rvm_op_ediv,		// RVM_EDIV
	rvm_op_emod,		// RVM_MOD
	rvm_op_elsl,		// RVM_ELSL
	rvm_op_elsr,		// RVM_ELSR
	rvm_op_elsru,		// RVM_ELSRU
	rvm_op_eand,		// RVM_EAND
	rvm_op_eorr,		// RVM_EORR
	rvm_op_exor,		// RVM_EXOR
	rvm_op_enot,		// RVM_ENOT

	rvm_op_eland,		// RVM_ELAND
	rvm_op_elor,		// RVM_ELOR
	rvm_op_elnot,		// RVM_ELNOT
	rvm_op_eeq,			// RVM_EEQ
	rvm_op_enoteq,		// RVM_ENOTEQ
	rvm_op_egreat,		// RVM_EGREAT
	rvm_op_egreateq,	// RVM_EGREATEQ
	rvm_op_eless,		// RVM_ELESS
	rvm_op_elesseq,		// RVM_ELESSEQ
	rvm_op_ecmp,		// RVM_ECMP
	rvm_op_ecmn,		// RVM_ECMN
	rvm_op_allocstr,	// RVM_ALLOCSTR
	rvm_op_allocarr,	// RVM_ALLOCARR
	rvm_op_addra,		// RVM_ADDRA
	rvm_op_lda,			// RVM_LDA
	rvm_op_sta,			// RVM_STA
	rvm_op_allocobj,	// RVM_ALLOCOBJ
	rvm_op_addrobjn,	// RVM_ADDROBJN,
	rvm_op_addrobjh,	// RVM_ADDROBJH,
	rvm_op_ldobjn,		// RVM_LDOBJN,
	rvm_op_stobjn,		// RVM_STOBJN,
	rvm_op_ldobjh,		// RVM_LDOBJH,
	rvm_op_stobjh,		// RVM_STOBJH,
	rvm_op_objlookup,	// RVM_OBJLKUP,
	rvm_op_objkeyadd,	// RVM_OBJADD,
	rvm_op_objkeylookupadd,	// RVM_OBJLKUPADD,
	(void*) 0,
	(void*) 0,
	(void*) 0,
	(void*) 0,
	(void*) 0,
	(void*) 0,
	(void*) 0,
	(void*) 0,
	
};


rvmcpu_t *rvm_cpu_create(rulong stacksize)
{
	rvmcpu_t *cpu;

	cpu = (rvmcpu_t *)r_malloc(sizeof(*cpu));
	if (!cpu)
		return ((void*)0);
	r_memset(cpu, 0, sizeof(*cpu));
	cpu->stacksize = stacksize;
	cpu->switables = r_harray_create(sizeof(rvm_switable_t*));
	cpu->stack = r_malloc(stacksize * sizeof(rvmreg_t));
	cpu->data = r_carray_create(sizeof(rvmreg_t));
	cpu->opmap = rvm_opmap_create();
	cpu->gc = rvm_gc_create();
	rvm_op_binary_init(cpu->opmap);
	rvm_op_cast_init(cpu->opmap);
	rvm_op_not_init(cpu->opmap);
	rvm_op_logicnot_init(cpu->opmap);

	return cpu;
}


rvmcpu_t *rvm_cpu_create_default()
{
	return rvm_cpu_create(RVM_DEFAULT_STACKSIZE);
}


void rvm_cpu_destroy(rvmcpu_t *cpu)
{
	rvm_gc_deallocate_all(cpu->gc);
	rvm_gc_destroy(cpu->gc);
	r_object_destroy((robject_t*)cpu->switables);
	r_free(cpu->stack);
	r_object_destroy((robject_t*)cpu->data);
	rvm_opmap_destroy(cpu->opmap);
	r_free(cpu);
}


rint rvm_cpu_exec(rvmcpu_t *cpu, rvm_asmins_t *prog, rword off)
{
	rvm_asmins_t *pi;
	rvmreg_t *regda = RVM_CPUREG_PTR(cpu, DA);
	rvmreg_t *regpc = RVM_CPUREG_PTR(cpu, PC);

	RVM_CPUREG_SETIP(cpu, PC, prog + off);
	cpu->abort = 0;
	cpu->error = 0;
	do {
		pi = RVM_REG_GETIP(regpc);
		if (pi->da) {
			*regda = pi->data;
		}
#if RVM_CONDITIONAL_INSTRUCTIONS
		if (pi->cond) {
			switch (pi->cond) {
			case RVM_CEXEC_GRE:
				if (!((cpu->status & RVM_STATUS_N) == 0 && (cpu->status & RVM_STATUS_Z) == 0))
					goto skipexec;
				break;
			case RVM_CEXEC_GEQ:
				if (!((cpu->status & RVM_STATUS_N) == 0 || (cpu->status & RVM_STATUS_Z) == 1))
					goto skipexec;
				break;
			case RVM_CEXEC_EQ:
				if (!((cpu->status & RVM_STATUS_Z)))
					goto skipexec;
				break;
			case RVM_CEXEC_NEQ:
				if (!((cpu->status & RVM_STATUS_Z) == 0))
					goto skipexec;
				break;
			case RVM_CEXEC_LEQ:
				if (!((cpu->status & RVM_STATUS_N) || (cpu->status & RVM_STATUS_Z)))
					goto skipexec;
				break;
			case RVM_CEXEC_LES:
				if (!((cpu->status & RVM_STATUS_N)))
					goto skipexec;
				break;
			default:
				goto skipexec;
			};
		}
#endif
		ops[pi->opcode](cpu, pi);
#if RVM_CONDITIONAL_INSTRUCTIONS
skipexec:
#endif
		RVM_REG_INCIP(regpc, 1);
	} while (!cpu->abort);
	if (cpu->error)
		return -1;
	return 0;
}


rint rvm_cpu_exec_debug(rvmcpu_t *cpu, rvm_asmins_t *prog, rword off)
{
	rlong line = 0;
	rvm_asmins_t *pi;
	rvmreg_t *regda = RVM_CPUREG_PTR(cpu, DA);
	rvmreg_t *regpc = RVM_CPUREG_PTR(cpu, PC);

	RVM_CPUREG_SETIP(cpu, PC, prog + off);
	cpu->abort = 0;
	cpu->error = 0;
	do {
		pi = RVM_REG_GETIP(regpc);
		if (pi->da) {
			*regda = pi->data;
		}
#if RVM_CONDITIONAL_INSTRUCTIONS
		if (pi->cond) {
			switch (pi->cond) {
			case RVM_CEXEC_GRE:
				if (!((cpu->status & RVM_STATUS_N) == 0 && (cpu->status & RVM_STATUS_Z) == 0))
					goto skipexec;
				break;
			case RVM_CEXEC_GEQ:
				if (!((cpu->status & RVM_STATUS_N) == 0 || (cpu->status & RVM_STATUS_Z) == 1))
					goto skipexec;
				break;
			case RVM_CEXEC_EQ:
				if (!((cpu->status & RVM_STATUS_Z)))
					goto skipexec;
				break;
			case RVM_CEXEC_NEQ:
				if (!((cpu->status & RVM_STATUS_Z) == 0))
					goto skipexec;
				break;
			case RVM_CEXEC_LEQ:
				if (!((cpu->status & RVM_STATUS_N) || (cpu->status & RVM_STATUS_Z)))
					goto skipexec;
				break;
			case RVM_CEXEC_LES:
				if (!((cpu->status & RVM_STATUS_N)))
					goto skipexec;
				break;
			default:
				goto skipexec;
			};
		}
#endif
		ops[pi->opcode](cpu, pi);
		r_printf("%7ld :", ++line);
		rvm_cpu_dumpregs(cpu, pi);
#if RVM_CONDITIONAL_INSTRUCTIONS
skipexec:
#endif
		RVM_REG_INCIP(regpc, 1);
	} while (!cpu->abort);
	if (cpu->error)
		return -1;
	return 0;
}


rint rvm_cpu_global_swilookup(rvmcpu_t *cpu, const rchar *swiname, rsize_t size)
{
	rint nswi;
	rvm_switable_t *swientry;
	rlong ntable;

	for (ntable = 0; ntable < r_harray_length(cpu->switables); ntable++) {
		swientry = r_harray_index(cpu->switables, ntable, rvm_switable_t*);
		if (!swientry)
			return -1;
		for (nswi = 0; swientry[nswi].name; nswi++) {
			if (r_strncmp(swientry[nswi].name, swiname, size) == 0 && r_strlen(swientry[nswi].name) == size)
				return (rint)RVM_SWI_ID(ntable, nswi);
		}
	}
	return -1;
}


rint rvm_cpu_table_swilookup(rvmcpu_t *cpu, const rchar *tabname, const rchar *swiname, rsize_t size)
{
	rint nswi;
	rvm_switable_t *swientry;
	rlong ntable = r_harray_lookup_s(cpu->switables, tabname);

	if (ntable < 0)
		return -1;
	swientry = r_harray_index(cpu->switables, ntable, rvm_switable_t*);
	if (!swientry)
		return -1;
	for (nswi = 0; swientry[nswi].name; nswi++) {
		if (r_strncmp(swientry[nswi].name, swiname, size) == 0 && r_strlen(swientry[nswi].name) == size)
			return (rint)RVM_SWI_ID(ntable, nswi);
	}
	return -1;
}


rint rvm_cpu_swilookup(rvmcpu_t *cpu, const rchar *tabname, const rchar *swiname, rsize_t size)
{
	return tabname ? rvm_cpu_table_swilookup(cpu, tabname, swiname, size) : rvm_cpu_global_swilookup(cpu, swiname, size);
}


rint rvm_cpu_swilookup_s(rvmcpu_t *cpu, const rchar *tabname, const rchar *swiname)
{
	return rvm_cpu_swilookup(cpu, tabname, swiname, r_strlen(swiname));
}


rint rvm_cpu_addswitable(rvmcpu_t *cpu, const rchar *tabname, rvm_switable_t *switable)
{
	return r_harray_replace_s(cpu->switables, tabname, &switable);
}


rint rvm_cpu_abort(rvmcpu_t *cpu)
{
	if (!cpu)
		return -1;
	cpu->error = RVM_E_USERABORT;
	cpu->abort = 1;
	return 0;
}


rvm_asmins_t rvm_cond_asma(rword cond, rword opcode, rword op1, rword op2, rword op3, rchar *data, rulong size)
{
	rvm_asmins_t a;

	r_memset(&a, 0, sizeof(a));
	a.opcode = (ruint32) RVM_ASMINS_OPCODE(opcode);
	a.swi = (ruint32) RVM_ASMINS_SWI(opcode);
	a.op1 = (ruint8)op1;
	a.op2 = (ruint8)op2;
	a.op3 = (ruint8)op3;
	a.cond = cond;
	rvm_reg_setstrptr(&a.data, data, size);
	if ((ruint8)op1 == DA || (ruint8)op2 == DA || (ruint8)op3 == DA)
		a.da = 1;
	return a;
}


rvm_asmins_t rvm_asma(rword opcode, rword op1, rword op2, rword op3, rchar *data, rulong size)
{
	return rvm_cond_asma(0, opcode, op1, op2, op3, data, size);
}


rvm_asmins_t rvm_cond_asmp(rword cond, rword opcode, rword op1, rword op2, rword op3, rpointer data)
{
	rvm_asmins_t a;

	r_memset(&a, 0, sizeof(a));
	a.opcode = (ruint32) RVM_ASMINS_OPCODE(opcode);
	a.swi = (ruint32) RVM_ASMINS_SWI(opcode);
	a.op1 = (ruint8)op1;
	a.op2 = (ruint8)op2;
	a.op3 = (ruint8)op3;
	a.cond = cond;
	rvm_reg_setpointer(&a.data, data);
	if ((ruint8)op1 == DA || (ruint8)op2 == DA || (ruint8)op3 == DA)
		a.da = 1;
	return a;
}


rvm_asmins_t rvm_asmp(rword opcode, rword op1, rword op2, rword op3, rpointer data)
{
	return rvm_cond_asmp(0, opcode, op1, op2, op3, data);
}


rvm_asmins_t rvm_cond_asms(rword cond, rword opcode, rword op1, rword op2, rword op3, rword data)
{
	rvm_asmins_t a;

	r_memset(&a, 0, sizeof(a));
	a.opcode = (ruint32) RVM_ASMINS_OPCODE(opcode);
	a.swi = (ruint32) RVM_ASMINS_SWI(opcode);
	a.op1 = (ruint8)op1;
	a.op2 = (ruint8)op2;
	a.op3 = (ruint8)op3;
	a.cond = cond;
	rvm_reg_setunsigned(&a.data, data);
	RVM_REG_SETTYPE(&a.data, RVM_DTYPE_SWIID)
	if ((ruint8)op1 == DA || (ruint8)op2 == DA || (ruint8)op3 == DA)
		a.da = 1;
	return a;
}


rvm_asmins_t rvm_asms(rword opcode, rword op1, rword op2, rword op3, rword data)
{
	return rvm_cond_asms(0, opcode, op1, op2, op3, data);
}


rvm_asmins_t rvm_cond_asmf(rword cond, rword opcode, rword op1, rword op2, rword op3, rword data)
{
	rvm_asmins_t a;

	r_memset(&a, 0, sizeof(a));
	a.opcode = (ruint32) RVM_ASMINS_OPCODE(opcode);
	a.swi = (ruint32) RVM_ASMINS_SWI(opcode);
	a.op1 = (ruint8)op1;
	a.op2 = (ruint8)op2;
	a.op3 = (ruint8)op3;
	a.cond = cond;
	rvm_reg_setunsigned(&a.data, data);
	RVM_REG_SETTYPE(&a.data, RVM_DTYPE_FUNCTION)
	if ((ruint8)op1 == DA || (ruint8)op2 == DA || (ruint8)op3 == DA)
		a.da = 1;
	return a;
}

rvm_asmins_t rvm_asmf(rword opcode, rword op1, rword op2, rword op3, rword data)
{
	return rvm_cond_asmf(0, opcode, op1, op2, op3, data);
}


rvm_asmins_t rvm_cond_asm(rword cond, rword opcode, rword op1, rword op2, rword op3, rword data)
{
	rvm_asmins_t a;

	r_memset(&a, 0, sizeof(a));
	a.opcode = (ruint32) RVM_ASMINS_OPCODE(opcode);
	a.swi = (ruint32) RVM_ASMINS_SWI(opcode);
	a.op1 = (ruint8)op1;
	a.op2 = (ruint8)op2;
	a.op3 = (ruint8)op3;
	a.cond = cond;
	rvm_reg_setunsigned(&a.data, data);
	if ((ruint8)op1 == DA || (ruint8)op2 == DA || (ruint8)op3 == DA)
		a.da = 1;
	return a;
}


rvm_asmins_t rvm_asm(rword opcode, rword op1, rword op2, rword op3, rword data)
{
	return rvm_cond_asm(0, opcode, op1, op2, op3, data);
}



rvm_asmins_t rvm_cond_asml(rword cond, rword opcode, rword op1, rword op2, rword op3, rlong data)
{
	rvm_asmins_t a;

	r_memset(&a, 0, sizeof(a));
	a.opcode = (ruint32) RVM_ASMINS_OPCODE(opcode);
	a.swi = (ruint32) RVM_ASMINS_SWI(opcode);
	a.op1 = (ruint8)op1;
	a.op2 = (ruint8)op2;
	a.op3 = (ruint8)op3;
	a.cond = cond;
	rvm_reg_setlong(&a.data, data);
	if ((ruint8)op1 == DA || (ruint8)op2 == DA || (ruint8)op3 == DA)
		a.da = 1;
	return a;
}


rvm_asmins_t rvm_asml(rword opcode, rword op1, rword op2, rword op3, rlong data)
{
	return rvm_cond_asml(0, opcode, op1, op2, op3, data);
}


rvm_asmins_t rvm_cond_asmb(rword cond, rword opcode, rword op1, rword op2, rword op3, ruint data)
{
	rvm_asmins_t a;

	r_memset(&a, 0, sizeof(a));
	a.opcode = (ruint32) RVM_ASMINS_OPCODE(opcode);
	a.swi = (ruint32) RVM_ASMINS_SWI(opcode);
	a.op1 = (ruint8)op1;
	a.op2 = (ruint8)op2;
	a.op3 = (ruint8)op3;
	a.cond = cond;
	rvm_reg_setboolean(&a.data, data);
	if ((ruint8)op1 == DA || (ruint8)op2 == DA || (ruint8)op3 == DA)
		a.da = 1;
	return a;
}


rvm_asmins_t rvm_asmb(rword opcode, rword op1, rword op2, rword op3, ruint data)
{
	return rvm_cond_asmb(0, opcode, op1, op2, op3, data);
}


rvm_asmins_t rvm_cond_asmd(rword cond, rword opcode, rword op1, rword op2, rword op3, rdouble data)
{
	rvm_asmins_t a;

	r_memset(&a, 0, sizeof(a));
	a.opcode = (ruint32) RVM_ASMINS_OPCODE(opcode);
	a.swi = (ruint32) RVM_ASMINS_SWI(opcode);
	a.op1 = (ruint8)op1;
	a.op2 = (ruint8)op2;
	a.op3 = (ruint8)op3;
	a.cond = cond;
	rvm_reg_setdouble(&a.data, data);
	if ((ruint8)op1 == DA || (ruint8)op2 == DA || (ruint8)op3 == DA)
		a.da = 1;
	return a;
}

rvm_asmins_t rvm_asmd(rword opcode, rword op1, rword op2, rword op3, rdouble data)
{
	return rvm_cond_asmd(0, opcode, op1, op2, op3, data);
}


rvm_asmins_t rvm_cond_asm2(rword cond, rword opcode, rword op1, rword op2, rword op3, ruint32 p1, ruint32 p2)
{
	rvm_asmins_t a;

	r_memset(&a, 0, sizeof(a));
	a.opcode = (ruint32) RVM_ASMINS_OPCODE(opcode);
	a.swi = (ruint32) RVM_ASMINS_SWI(opcode);
	a.op1 = (ruint8)op1;
	a.op2 = (ruint8)op2;
	a.op3 = (ruint8)op3;
	a.cond = cond;
	rvm_reg_setpair(&a.data, p1, p2);
	if ((ruint8)op1 == DA || (ruint8)op2 == DA || (ruint8)op3 == DA)
		a.da = 1;
	return a;
}


rvm_asmins_t rvm_asm2(rword opcode, rword op1, rword op2, rword op3, ruint32 p1, ruint32 p2)
{
	return rvm_cond_asm2(0, opcode, op1, op2, op3, p1, p2);
}

rvmreg_t *rvm_cpu_alloc_global(rvmcpu_t *cpu)
{
	rvmreg_t *global;
	rint index = r_carray_add(cpu->data, NULL);

	global = (rvmreg_t*)r_carray_slot(cpu->data, index);
	RVM_REG_CLEAR(global);
	RVM_REG_SETTYPE(global, RVM_DTYPE_UNDEF);
	return global;
}


int rvm_cpu_setreg(rvmcpu_t *cpu, rword regnum, const rvmreg_t *src)
{
	if (regnum >= RLST)
		return -1;
	RVM_CPUREG_SET(cpu, regnum, *src);
	return 0;
}


rvmreg_t *rvm_cpu_getreg(rvmcpu_t *cpu, rword regnum)
{
	if (regnum >= RLST)
		return NULL;
	return RVM_CPUREG_PTR(cpu, regnum);
}
