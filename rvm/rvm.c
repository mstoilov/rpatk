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
#include "rvm.h"


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
	"RVM_LDR",
	"RVM_LDRP",
	"RVM_LDRB",
	"RVM_LDRH",
	"RVM_LSL",
	"RVM_LSR",
	"RVM_STM",
	"RVM_LDM",
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
	"RVM_PUSH,",
	"RVM_POPM",	
	"RVM_TST",	
	"RVM_TEQ",	
};


static void *rvm_malloc(unsigned long size)
{
	return malloc((size_t)size);
}


static void rvm_free(void *ptr)
{
	free(ptr);
}


static void *rvm_realloc(void *ptr, unsigned long size)
{
	return realloc(ptr, (size_t)size);
}


static void *rvm_memset(void *s, int c, unsigned long n)
{
	return memset(s, c, (size_t)n);
}


static int rvm_cpu_check_space(rvm_cpu_t *cpu)
{
	rvm_reg_t *stack;
	rword stacksize;

	if (cpu->stacksize - RVM_GET_REGU(cpu, SP) <= RVM_STACK_CHUNK) {
		stacksize = cpu->stacksize + 2 * RVM_STACK_CHUNK;
		stack = (rvm_reg_t *)rvm_realloc(cpu->stack, (unsigned long)(sizeof(rvm_reg_t) * stacksize));
		if (!stack)
			return -1;
		cpu->stacksize = stacksize;
		cpu->stack = stack;
#if 0
		fprintf(stdout, "%s, size: %ld\n", __FUNCTION__, cpu->stacksize);
#endif
	}
	return 0;
}


static void rvm_op_b(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_SET_REGU(cpu, PC, RVM_GET_REGU(cpu, PC) + RVM_GET_REGU(cpu, ins->op1) - 1);
}


static void rvm_op_beq(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	if ((cpu->status & RVM_STATUS_Z))
		RVM_SET_REGU(cpu, PC, RVM_GET_REGU(cpu, PC) + RVM_GET_REGU(cpu, ins->op1) - 1);
}


static void rvm_op_bneq(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	if ((cpu->status & RVM_STATUS_Z) == 0)
		RVM_SET_REGU(cpu, PC, RVM_GET_REGU(cpu, PC) + RVM_GET_REGU(cpu, ins->op1) - 1);
}


static void rvm_op_bleq(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	if ((cpu->status & RVM_STATUS_N) || (cpu->status & RVM_STATUS_Z))
		RVM_SET_REGU(cpu, PC, RVM_GET_REGU(cpu, PC) + RVM_GET_REGU(cpu, ins->op1) - 1);
}

static void rvm_op_bgeq(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	if ((cpu->status & RVM_STATUS_N) == 0 || (cpu->status & RVM_STATUS_Z) == 1)
		RVM_SET_REGU(cpu, PC, RVM_GET_REGU(cpu, PC) + RVM_GET_REGU(cpu, ins->op1) - 1);
}


static void rvm_op_bles(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	if ((cpu->status & RVM_STATUS_N))
		RVM_SET_REGU(cpu, PC, RVM_GET_REGU(cpu, PC) + RVM_GET_REGU(cpu, ins->op1) - 1);
}


static void rvm_op_bgre(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	if ((cpu->status & RVM_STATUS_N) == 0 && (cpu->status & RVM_STATUS_Z) == 0)
		RVM_SET_REGU(cpu, PC, RVM_GET_REGU(cpu, PC) + RVM_GET_REGU(cpu, ins->op1) - 1);
}


static void rvm_op_bl(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_SET_REGU(cpu, LR, RVM_GET_REGU(cpu, PC));
	RVM_SET_REGU(cpu, PC, RVM_GET_REGU(cpu, PC) + RVM_GET_REGU(cpu, ins->op1) - 1);
}


static void rvm_op_exit(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{

}


static void rvm_op_mov(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_SET_REG(cpu, ins->op1, RVM_GET_REG(cpu, ins->op2));
}


static void rvm_op_ldr(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_SET_REGU(cpu, ins->op1, *((rword*)RVM_GET_REGU(cpu, ins->op2)));
	
}


static void rvm_op_ldrp(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_SET_REGU(cpu, ins->op1, *((rpointer*)RVM_GET_REGU(cpu, ins->op2)));
}


static void rvm_op_ldrb(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_SET_REGU(cpu, ins->op1, *((ruint8*)RVM_GET_REGU(cpu, ins->op2)));
}


static void rvm_op_ldrh(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_SET_REGU(cpu, ins->op1, *((ruint16*)RVM_GET_REGU(cpu, ins->op2)));
}


static void rvm_op_str(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	*((rword*)RVM_GET_REGP(cpu, ins->op2)) = RVM_GET_REGU(cpu, ins->op1);
}


static void rvm_op_strp(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	*((rpointer*)RVM_GET_REGP(cpu, ins->op2)) = (rpointer)RVM_GET_REGP(cpu, ins->op1);
}


static void rvm_op_strb(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	*((ruint8*)RVM_GET_REGP(cpu, ins->op2)) = (ruint8)RVM_GET_REGU(cpu, ins->op1);
}


static void rvm_op_strh(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	*((ruint16*)RVM_GET_REGP(cpu, ins->op2)) = (ruint16)RVM_GET_REGU(cpu, ins->op1);
	
}


static void rvm_op_adds(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_GET_REGU(cpu, ins->op2), op3 = RVM_GET_REGU(cpu, ins->op3);
	
	res = op2 + op3;
	RVM_SET_REGU(cpu, ins->op1, res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_C, res < op2 || res < op3);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_V, (op2 & RVM_SIGN_BIT) == (op3 & RVM_SIGN_BIT) &&
							(res & RVM_SIGN_BIT) != (op2 & RVM_SIGN_BIT));
}


static void rvm_op_adc(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_GET_REGU(cpu, ins->op2), op3 = RVM_GET_REGU(cpu, ins->op3);
	
	res = op2 + op3 + (RVM_STATUS_GETBIT(cpu, RVM_STATUS_C) ? 1 : 0);
	RVM_SET_REGU(cpu, ins->op1, res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_C, res < op2 || res < op3);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_V, (op2 & RVM_SIGN_BIT) == (op3 & RVM_SIGN_BIT) &&
							(res & RVM_SIGN_BIT) != (op2 & RVM_SIGN_BIT));
}


static void rvm_op_and(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_GET_REGU(cpu, ins->op2), op3 = RVM_GET_REGU(cpu, ins->op3);
	
	res = op2 & op3;
	RVM_SET_REGU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void rvm_op_orr(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_GET_REGU(cpu, ins->op2), op3 = RVM_GET_REGU(cpu, ins->op3);
	
	res = op2 | op3;
	RVM_SET_REGU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void rvm_op_lsl(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_GET_REGU(cpu, ins->op2), op3 = RVM_GET_REGU(cpu, ins->op3);
	
	res = op2 << op3;
	RVM_SET_REGU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void rvm_op_lsr(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_GET_REGU(cpu, ins->op2), op3 = RVM_GET_REGU(cpu, ins->op3);
	
	res = op2 >> op3;
	RVM_SET_REGU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
}


static void rvm_op_asr(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_GET_REGU(cpu, ins->op2), op3 = RVM_GET_REGU(cpu, ins->op3);
	
	res = op2 >> op3;
	res |= op2 & RVM_SIGN_BIT;
	RVM_SET_REGU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void rvm_op_ror(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	unsigned int i;
	rword res, op2 = RVM_GET_REGU(cpu, ins->op2), op3 = RVM_GET_REGU(cpu, ins->op3);
	
	res = op2;
	for (i = 0; i < op3; i++) {
		if (res & 1) {
			res >>= 1;
			res |= RVM_SIGN_BIT;
		} else {
			res >>= 1;
		}
	}
	RVM_SET_REGU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_C, res & RVM_SIGN_BIT);
}


static void rvm_op_tst(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_GET_REGU(cpu, ins->op2), op3 = RVM_GET_REGU(cpu, ins->op3);
	
	res = op2 & op3;
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void rvm_op_eor(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_GET_REGU(cpu, ins->op2), op3 = RVM_GET_REGU(cpu, ins->op3);
	
	res = op2 ^ op3;
	RVM_SET_REGU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void rvm_op_not(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_GET_REGU(cpu, ins->op2);
	
	res = ~op2;
	RVM_SET_REGU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void rvm_op_teq(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_GET_REGU(cpu, ins->op2), op3 = RVM_GET_REGU(cpu, ins->op3);
	
	res = op2 ^ op3;
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void rvm_op_bic(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_GET_REGU(cpu, ins->op2), op3 = RVM_GET_REGU(cpu, ins->op3);
	
	res = op2 & ~op3;
	RVM_SET_REGU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void rvm_op_clz(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_GET_REGU(cpu, ins->op2);
	
	for (res = RVM_REGISTER_BITS; op2; ) {
		op2 >>= 1;
		res -= 1;
	}
	RVM_SET_REGU(cpu, ins->op1, res);
}


static void rvm_op_add(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_SET_REGU(cpu, ins->op1, RVM_GET_REGU(cpu, ins->op2) + RVM_GET_REGU(cpu, ins->op3));
}


static void rvm_op_swi(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
//	rvm_cpu_swi *cbt = cpu->switable[RVM_CB_TABLE(RVM_GET_REGU(cpu, ins->op1))];
//	cbt[RVM_CB_OFFSET(RVM_GET_REGU(cpu, ins->op1))](cpu);

	rvm_cpu_swi swi;
	ruint swinum = (ruint) RVM_GET_REGU(cpu, ins->op1);

	if (r_array_length(cpu->switable) <= swinum)
		RVM_ABORT(cpu, RVM_E_SWINUM);
	swi = r_array_index(cpu->switable, swinum, rvm_cpu_swi);
	swi(cpu);
}


static void rvm_op_sub(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_SET_REGU(cpu, ins->op1, RVM_GET_REGU(cpu, ins->op2) - RVM_GET_REGU(cpu, ins->op3));
}


static void rvm_op_subs(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_GET_REGU(cpu, ins->op2), op3 = RVM_GET_REGU(cpu, ins->op3);
	
	res = op2 - op3;
	RVM_SET_REGU(cpu, ins->op1, res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_C, !(res > op2));  /* borrow = !carry */
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_V, (op2 & RVM_SIGN_BIT) != (op3 & RVM_SIGN_BIT) &&
							(res & RVM_SIGN_BIT) == (op2 & RVM_SIGN_BIT));
}


static void rvm_op_sbc(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_GET_REGU(cpu, ins->op2), op3 = RVM_GET_REGU(cpu, ins->op3);

	res = op2 - op3 + (RVM_STATUS_GETBIT(cpu, RVM_STATUS_C) ? 0 : -1);
	RVM_SET_REGU(cpu, ins->op1, res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_C, !(res > op2));	/* borrow = !carry */
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_V, (op2 & RVM_SIGN_BIT) != (op3 & RVM_SIGN_BIT) &&
							(res & RVM_SIGN_BIT) == (op2 & RVM_SIGN_BIT));
}


static void rvm_op_mul(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_SET_REGU(cpu, ins->op1, RVM_GET_REGU(cpu, ins->op2) * RVM_GET_REGU(cpu, ins->op3));
}


static void rvm_op_mls(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	rsword res;
	rsword op2 = RVM_GET_REGU(cpu, ins->op2), op3 = RVM_GET_REGU(cpu, ins->op3);

	res = (rsword)(op2 * op3);
	RVM_SET_REGU(cpu, ins->op1, res);
	/* TBD: Not sure how to update the RVM_STATUS_C */
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void rvm_op_muls(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_GET_REGU(cpu, ins->op2), op3 = RVM_GET_REGU(cpu, ins->op3);
	
	res = op2 * op3;
	RVM_SET_REGU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_C, op2 && (res / op2) != op3);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void rvm_op_div(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	if (!RVM_GET_REGU(cpu, ins->op3))
		RVM_ABORT(cpu, RVM_E_DIVZERO);

	RVM_SET_REGU(cpu, ins->op1, RVM_GET_REGU(cpu, ins->op2) / RVM_GET_REGU(cpu, ins->op3));
}


static void rvm_op_divs(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	rword res, op2 = RVM_GET_REGU(cpu, ins->op2), op3 = RVM_GET_REGU(cpu, ins->op3);

	if (!op3)
		RVM_ABORT(cpu, RVM_E_DIVZERO);
	res = op2 / op3;
	RVM_SET_REGU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void rvm_op_dvs(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	rsword res;
	rsword op2 = RVM_GET_REGU(cpu, ins->op2), op3 = RVM_GET_REGU(cpu, ins->op3);

	if (!op3)
		RVM_ABORT(cpu, RVM_E_DIVZERO);
	res = (rsword)(op2 / op3);
	RVM_SET_REGU(cpu, ins->op1, res);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
}


static void rvm_op_pushm(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	int n;
	rword bits = RVM_GET_REGU(cpu, ins->op1);

	if ((RVM_GET_REGU(cpu, SP) % RVM_STACK_CHUNK) == 0)
		rvm_cpu_check_space(cpu);
	for (n = 0; bits && n < RLST; n++) {
		if (((rword)(1 << n)) & bits) {
			RVM_SET_REGU(cpu, SP, RVM_GET_REGU(cpu, SP) + 1);
			cpu->stack[RVM_GET_REGU(cpu, SP)] = RVM_GET_REG(cpu, n);
			bits &= ~(1<<n);
		}
	}
}


static void rvm_op_popm(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	int n;
	rword bits = RVM_GET_REGU(cpu, ins->op1);

	for (n = RLST - 1; bits && n >= 0; n--) {
		if (((rword)(1 << n)) & bits) {
			RVM_SET_REG(cpu, n, cpu->stack[RVM_GET_REGU(cpu, SP)]);
			RVM_SET_REGU(cpu, SP, RVM_GET_REGU(cpu, SP) - 1);
			bits &= ~(1<<n);
		}
	}
}


static void rvm_op_push(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	if ((RVM_GET_REGU(cpu, SP) % RVM_STACK_CHUNK) == 0)
		rvm_cpu_check_space(cpu);
	RVM_SET_REGU(cpu, SP, RVM_GET_REGU(cpu, SP) + 1);
	cpu->stack[RVM_GET_REGU(cpu, SP)] = RVM_GET_REG(cpu, ins->op1);
}


static void rvm_op_pop(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_SET_REG(cpu, ins->op1, cpu->stack[RVM_GET_REGU(cpu, SP)]);
	RVM_SET_REGU(cpu, SP, RVM_GET_REGU(cpu, SP) - 1);
}

static void rvm_op_stm(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	int n;
	rword *dst = (rword*)	RVM_GET_REGU(cpu, ins->op1);
	rword bits = RVM_GET_REGU(cpu, ins->op2);
	
	for (n = 0; bits && n < RLST; n++) {
		if (((rword)(1 << n)) & bits) {
			*dst = RVM_GET_REGU(cpu, n);
			dst += 1;
			bits &= ~(1<<n);
		}
	}
}


static void rvm_op_ldm(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	int n;
	rword *src = (rword*)RVM_GET_REGU(cpu, ins->op1);
	rword bits = RVM_GET_REGU(cpu, ins->op2);

	for (n = 0; bits && n < RLST; n++) {
		if (((rword)(1 << n)) & bits) {
			RVM_SET_REGU(cpu, n, *src);
			src += 1;
			bits &= ~(1<<n);
		}
	}
}


static void rvm_op_cmp(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	rword op1 = RVM_GET_REGU(cpu, ins->op1), op2 = RVM_GET_REGU(cpu, ins->op2);
	rword res = (rword)(op1 - op2);
	
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_C, op1 < op2);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_V, (op1 & RVM_SIGN_BIT) != (op2 & RVM_SIGN_BIT) &&
							(res & RVM_SIGN_BIT) == (op1 & RVM_SIGN_BIT));
}


static void rvm_op_cmn(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	rword op1 = RVM_GET_REGU(cpu, ins->op1), op2 = RVM_GET_REGU(cpu, ins->op2);
	rword res = (rword)(op1 + op2);
	
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_C, res < op1 || res < op2);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !res);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, res & RVM_SIGN_BIT);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_V, (op1 & RVM_SIGN_BIT) == (op2 & RVM_SIGN_BIT) &&
							(res & RVM_SIGN_BIT) != (op1 & RVM_SIGN_BIT));

}


static void rvm_op_nop(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
//	fprintf(stdout, "nop\n");
}


static void rvm_op_ret(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_SET_REGU(cpu, PC, RVM_GET_REGU(cpu, LR));	
}



static int rvm_vsnprintf(char *str, unsigned int size, const char *format, va_list ap)
{
	return vsnprintf(str, size, format, ap);
}


static int rvm_snprintf(char *str, unsigned int size, const char *format, ...)
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


int rvm_asm_dump_reg_to_str(unsigned char reg, char *str, unsigned int size)
{
	int ret = 0;

	if (reg == XX)
		ret = rvm_snprintf(str, size, "XX ");
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


int rvm_asm_dump_pi_to_str(rvm_asmins_t *pi, char *str, unsigned int size)
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

	if ((ret = rvm_snprintf(str, sz, "0x%lx  ", pi->data)) < 0)
		return ret;
	str += ret;
	sz -= ret;

	return size - sz;
}


void rvm_asm_dump(rvm_asmins_t *pi, unsigned int count)
{
	char buffer[256];
	while (count) {
		rvm_asm_dump_pi_to_str(pi, buffer, sizeof(buffer));
		rvm_printf("%s\n", buffer);
		++pi;
		--count;
	}
}


static void rvm_cpu_dumpregs(rvm_asmins_t *pi, rvm_cpu_t *vm)
{
    int ret;
	char buffer[1024];
	
	ret = rvm_asm_dump_pi_to_str(pi, buffer, sizeof(buffer));
	if (ret < 0)
		return;
    ret = rvm_snprintf(buffer + ret, sizeof(buffer) - ret, "                                                                                        ");
	buffer[50] = '\0';
	rvm_printf("%s", buffer);

   	rvm_printf("0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, SP=0x%lx, LR=0x%lx, PC=%ld, S( %c%c%c%c )",
   		RVM_GET_REGU(vm, 0), RVM_GET_REGU(vm, 1), RVM_GET_REGU(vm, 2), RVM_GET_REGU(vm, 3),
   		RVM_GET_REGU(vm, 4), RVM_GET_REGU(vm, 5), RVM_GET_REGU(vm, 6), RVM_GET_REGU(vm, 7),
   		RVM_GET_REGU(vm, 8), RVM_GET_REGU(vm, SP), RVM_GET_REGU(vm, LR), (long int)RVM_GET_REGU(vm, PC),
   		vm->status & RVM_STATUS_V ? 'V' : '_',
   		vm->status & RVM_STATUS_C ? 'C' : '_',
   		vm->status & RVM_STATUS_N ? 'N' : '_',
   		vm->status & RVM_STATUS_Z ? 'Z' : '_');
	rvm_printf("\n");
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
	rvm_op_ldr,			// RVM_LDR
	rvm_op_ldrp,		// RVM_LDRP
	rvm_op_ldrb,		// RVM_LDRB
	rvm_op_ldrh,		// RVM_LDRH
	rvm_op_lsl,			// RVM_LSL
	rvm_op_lsr,			// RVM_LSR
	rvm_op_stm,			// RVM_STM
	rvm_op_ldm,			// RVM_LDM
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
	(void*) 0,
	(void*) 0,
	(void*) 0,
	(void*) 0,
	(void*) 0,
	(void*) 0,
	(void*) 0,
	(void*) 0,
	
};
	

rvm_cpu_t *rvm_cpu_create()
{
	rvm_cpu_t *cpu;

	cpu = (rvm_cpu_t *)rvm_malloc(sizeof(*cpu));
	if (!cpu)
		return ((void*)0);
	rvm_memset(cpu, 0, sizeof(*cpu));
	cpu->switable = r_array_create(sizeof(rvm_cpu_swi));
	return cpu;
}


void rvm_cpu_destroy(rvm_cpu_t *cpu)
{
	rvm_free(cpu->stack);
	rvm_free(cpu);
}


int rvm_cpu_exec(rvm_cpu_t *cpu, rvm_asmins_t *prog, rword pc)
{
	rvm_asmins_t *pi;

	RVM_SET_REGU(cpu, PC, pc);
	cpu->abort = 0;
	cpu->error = 0;
	do {
		pi = &prog[RVM_GET_REGU(cpu, PC)];
		cpu->r[DA] = pi->data;
		ops[pi->opcode](cpu, pi);
		if (cpu->abort)
			return -1;
		RVM_SET_REGU(cpu, PC, RVM_GET_REGU(cpu, PC) + 1);
	} while (pi->opcode);
	return 0;
}


int rvm_cpu_exec_debug(rvm_cpu_t *cpu, rvm_asmins_t *prog, rword pc)
{
	rvm_asmins_t *pi;

	RVM_SET_REGU(cpu, PC, pc);
	cpu->abort = 0;
	cpu->error = 0;
	do {
		pi = &prog[RVM_GET_REGU(cpu, PC)];
		cpu->r[DA] = pi->data;
		ops[pi->opcode](cpu, pi);
		if (cpu->abort)
			return -1;
		rvm_cpu_dumpregs(pi, cpu);		
		RVM_SET_REGU(cpu, PC, RVM_GET_REGU(cpu, PC) + 1);
	} while (pi->opcode);
	return 0;
}


int rvm_cpu_switable_add(rvm_cpu_t *cpu, rvm_cpu_swi swi)
{
	return r_array_add(cpu->switable, &swi);
}


rvm_asmins_t rvm_asmp(rword opcode, rword op1, rword op2, rword op3, rpointer data)
{
	rvm_asmins_t a;
	a.opcode = (unsigned char) opcode;
	a.op1 = (unsigned char)op1;
	a.op2 = (unsigned char)op2;
	a.op3 = (unsigned char)op3;
	RVM_REGP(&a.data) = data;
	return a;
}


rvm_asmins_t rvm_asmu(rword opcode, rword op1, rword op2, rword op3, rword data)
{
	rvm_asmins_t a;
	a.opcode = (unsigned char) opcode;
	a.op1 = (unsigned char)op1;
	a.op2 = (unsigned char)op2;
	a.op3 = (unsigned char)op3;
	RVM_REGU(&a.data) = data;
	return a;
}

rvm_asmins_t rvm_asm(rword opcode, rword op1, rword op2, rword op3, rword data)
{
	return rvm_asmu(opcode, op1, op2, op3, data);
}

