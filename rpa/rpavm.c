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

#include "rpavm.h"
#include "rmem.h"
#include "rstring.h"


static const char *stropcalls[] = {
	"RPA_EXT",
	"RPA_ASR",
	"RPA_XCAL",
	"RPA_MOV",
	"RPA_ADD",
	"RPA_ADDS",
	"RPA_ADC",
	"RPA_AND",
	"RPA_BIC",
	"RPA_CLZ",
	"RPA_CMN",
	"RPA_EOR",
	"RPA_SUB",
	"RPA_SUBS",
	"RPA_SBC",
	"RPA_MUL",
	"RPA_MLS",
	"RPA_MULS",
	"RPA_NOT",
	"RPA_DIV",
	"RPA_DVS",
	"RPA_DIVS",
	"RPA_BL",
	"RPA_B",
	"RPA_STR",
	"RPA_STRB",
	"RPA_STRH",
	"RPA_LDR",
	"RPA_LDRB",
	"RPA_LDRH",
	"RPA_LSL",
	"RPA_LSR",
	"RPA_STM",
	"RPA_LDM",
	"RPA_ORR",
	"RPA_PUSH",
	"RPA_POP",
	"RPA_CMP",
	"RPA_NOP",
	"RPA_BEQ",
	"RPA_BNEQ",
	"RPA_BLEQ",
	"RPA_BGEQ",
	"RPA_BLES",
	"RPA_BGRE",
	"RPA_RET",
	"RPA_ROR",
	"RPA_PUSH,",
	"RPA_POPM",	
	"RPA_TST",	
	"RPA_TEQ",	
};


rpa_asmins_t rpa_asm(rpa_word_t opcode, rpa_word_t op1, rpa_word_t op2, rpa_word_t op3, rpa_word_t data)
{
	rpa_asmins_t a;
	a.opcode = (unsigned char) opcode;
	a.op1 = (unsigned char)op1;
	a.op2 = (unsigned char)op2;
	a.op3 = (unsigned char)op3;
	a.data = (rpa_word_t)data;
	return a;
}


int rpa_asm_dump_reg_to_str(unsigned char reg, char *str, unsigned int size)
{
	int ret = 0;

	if (reg == XX)
		ret = r_snprintf(str, size, "XX ");
	else if (reg == SP)
		ret = r_snprintf(str, size, "SP ");
	else if (reg == LR)
		ret = r_snprintf(str, size, "LR ");
	else if (reg == PC)
		ret = r_snprintf(str, size, "PC ");
	else if (reg == DA)
		ret = r_snprintf(str, size, "DA ");
	else if (reg >= 0 && reg < 10)
		ret = r_snprintf(str, size, "R%d ",  reg);
	else if (reg >= 10)
		ret = r_snprintf(str, size, "R%d",  reg);

	return ret;
}


int rpa_asm_dump_pi_to_str(rpa_asmins_t *pi, char *str, unsigned int size)
{
	int ret = 0, sz = size;

	if ((ret = r_snprintf(str, sz, "%10s   ", stropcalls[pi->opcode])) < 0)
		return ret;
	str += ret;
	sz -= ret;
	
	if ((ret = rpa_asm_dump_reg_to_str(pi->op1, str, sz)) < 0)
		return ret;
	str += ret;
	sz -= ret;
		
	if ((ret = r_snprintf(str, sz, " ")) < 0)
		return ret;
	str += ret;
	sz -= ret;

	if ((ret = rpa_asm_dump_reg_to_str(pi->op2, str, sz)) < 0)
		return ret;
	str += ret;
	sz -= ret;
		
	if ((ret = r_snprintf(str, sz, " ")) < 0)
		return ret;
	str += ret;
	sz -= ret;

	if ((ret = rpa_asm_dump_reg_to_str(pi->op3, str, sz)) < 0)
		return ret;
	str += ret;
	sz -= ret;
		
	if ((ret = r_snprintf(str, sz, " ")) < 0)
		return ret;
	str += ret;
	sz -= ret;

	if ((ret = r_snprintf(str, sz, "0x%lx  ", pi->data)) < 0)
		return ret;
	str += ret;
	sz -= ret;

	return size - sz;
}


void rpa_asm_dump(rpa_asmins_t *pi, unsigned int count)
{
	char buffer[256];
	while (count) {
		rpa_asm_dump_pi_to_str(pi, buffer, sizeof(buffer));
		r_printf("%s\n", buffer);
		++pi;
		--count;
	}
}


static void rpa_vm_dumpregs(rpa_asmins_t *pi, rpa_vm_t *vm)
{
    int ret;
	char buffer[1024];
	
	ret = rpa_asm_dump_pi_to_str(pi, buffer, sizeof(buffer));
	if (ret < 0)
		return;
    ret = r_snprintf(buffer + ret, sizeof(buffer) - ret, "                                                                                        ");
	buffer[50] = '\0';
	r_printf("%s", buffer);

   	r_printf("0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, SP=0x%lx, LR=0x%lx, PC=%ld, S( %c%c%c%c )", 
   		vm->r[0], vm->r[1], vm->r[2], vm->r[3], vm->r[4], vm->r[5], vm->r[6], vm->r[7], vm->r[8], vm->r[SP], vm->r[LR], 
   		(long int)vm->r[PC], 
   		vm->status & RPA_STATUS_V ? 'V' : '_',
   		vm->status & RPA_STATUS_C ? 'C' : '_',
   		vm->status & RPA_STATUS_N ? 'N' : '_',
   		vm->status & RPA_STATUS_Z ? 'Z' : '_');
	r_printf("\n");
}


int rpa_vm_check_space(rpa_vm_t *vm)
{
	rpa_word_t *stack;
	rpa_word_t stacksize;

	if (vm->stacksize - vm->r[SP] <= RPAVM_STACK_CHUNK) {
		stacksize = vm->stacksize + 2 * RPAVM_STACK_CHUNK;
		stack = (rpa_word_t *)r_realloc(vm->stack, (unsigned long)(sizeof(rpa_word_t) * stacksize));
		if (!stack)
			return -1;
		vm->stacksize = stacksize;
		vm->stack = stack;
	}
	return 0;
}


static void rpa_vmop_b(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	vm->r[PC] += vm->r[ins->op1] - 1;
}


static void rpa_vmop_beq(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	if ((vm->status & RPA_STATUS_Z))
		vm->r[PC] += vm->r[ins->op1] - 1;
}


static void rpa_vmop_bneq(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	if ((vm->status & RPA_STATUS_Z) == 0)
		vm->r[PC] += vm->r[ins->op1] - 1;
}


static void rpa_vmop_bleq(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	if ((vm->status & RPA_STATUS_N) || (vm->status & RPA_STATUS_Z))
		vm->r[PC] += vm->r[ins->op1] - 1;
}

static void rpa_vmop_bgeq(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	if ((vm->status & RPA_STATUS_N) == 0 || (vm->status & RPA_STATUS_Z) == 1)
		vm->r[PC] += vm->r[ins->op1] - 1;
}


static void rpa_vmop_bles(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	if ((vm->status & RPA_STATUS_N))
		vm->r[PC] += vm->r[ins->op1] - 1;
}


static void rpa_vmop_bgre(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	if ((vm->status & RPA_STATUS_N) == 0 && (vm->status & RPA_STATUS_Z) == 0)
		vm->r[PC] += vm->r[ins->op1] - 1;
}


static void rpa_vmop_bl(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	vm->r[LR] = vm->r[PC];
	vm->r[PC] += vm->r[ins->op1] - 1;
}


static void rpa_vmop_exit(rpa_vm_t *vm, rpa_asmins_t *ins)
{

}

static void rpa_vmop_mov(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	vm->r[ins->op1] = vm->r[ins->op2];
}


static void rpa_vmop_ldr(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	vm->r[ins->op1] = *((rpa_word_t*)vm->r[ins->op2]);
}


static void rpa_vmop_ldrb(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	vm->r[ins->op1] = *((unsigned char*)((rpa_word_t*)vm->r[ins->op2]));
}


static void rpa_vmop_ldrh(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	vm->r[ins->op1] = *((unsigned short*)((rpa_word_t*)vm->r[ins->op2]));
}


static void rpa_vmop_str(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	*((rpa_word_t*)vm->r[ins->op2]) = vm->r[ins->op1];
}


static void rpa_vmop_strb(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	*((unsigned char*)vm->r[ins->op2]) = (unsigned char)vm->r[ins->op1];
}


static void rpa_vmop_strh(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	*((unsigned short*)vm->r[ins->op2]) = (unsigned short)vm->r[ins->op1];
}


static void rpa_vmop_adds(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	rpa_word_t res, op2 = vm->r[ins->op2], op3 = vm->r[ins->op3];
	
	res = op2 + op3;
	vm->r[ins->op1] = res;
	RPA_STATUS_UPDATE(vm, RPA_STATUS_C, res < op2 || res < op3);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_Z, !res);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_N, res & RPA_SIGN_BIT);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_V, (op2 & RPA_SIGN_BIT) == (op3 & RPA_SIGN_BIT) &&
							(res & RPA_SIGN_BIT) != (op2 & RPA_SIGN_BIT));
}


static void rpa_vmop_adc(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	rpa_word_t res, op2 = vm->r[ins->op2], op3 = vm->r[ins->op3];
	
	res = op2 + op3 + (RPA_STATUS_GETBIT(vm, RPA_STATUS_C) ? 1 : 0);
	vm->r[ins->op1] = res;
	RPA_STATUS_UPDATE(vm, RPA_STATUS_C, res < op2 || res < op3);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_Z, !res);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_N, res & RPA_SIGN_BIT);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_V, (op2 & RPA_SIGN_BIT) == (op3 & RPA_SIGN_BIT) &&
							(res & RPA_SIGN_BIT) != (op2 & RPA_SIGN_BIT));
}


static void rpa_vmop_and(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	rpa_word_t res, op2 = vm->r[ins->op2], op3 = vm->r[ins->op3];
	
	res = op2 & op3;
	vm->r[ins->op1] = res;
	RPA_STATUS_CLRALL(vm);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_Z, !res);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_N, res & RPA_SIGN_BIT);
}


static void rpa_vmop_orr(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	rpa_word_t res, op2 = vm->r[ins->op2], op3 = vm->r[ins->op3];
	
	res = op2 | op3;
	vm->r[ins->op1] = res;
	RPA_STATUS_CLRALL(vm);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_Z, !res);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_N, res & RPA_SIGN_BIT);
}


static void rpa_vmop_lsl(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	rpa_word_t res, op2 = vm->r[ins->op2], op3 = vm->r[ins->op3];
	
	res = op2 << op3;
	vm->r[ins->op1] = res;
	RPA_STATUS_CLRALL(vm);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_Z, !res);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_N, res & RPA_SIGN_BIT);
}


static void rpa_vmop_lsr(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	rpa_word_t res, op2 = vm->r[ins->op2], op3 = vm->r[ins->op3];
	
	res = op2 >> op3;
	vm->r[ins->op1] = res;
	RPA_STATUS_CLRALL(vm);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_Z, !res);
}


static void rpa_vmop_asr(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	rpa_word_t res, op2 = vm->r[ins->op2], op3 = vm->r[ins->op3];
	
	res = op2 >> op3;
	res |= op2 & RPA_SIGN_BIT;
	vm->r[ins->op1] = res;
	RPA_STATUS_CLRALL(vm);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_Z, !res);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_N, res & RPA_SIGN_BIT);
}


static void rpa_vmop_ror(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	unsigned int i;
	rpa_word_t res, op2 = vm->r[ins->op2], op3 = vm->r[ins->op3];
	
	res = op2;
	for (i = 0; i < op3; i++) {
		if (res & 1) {
			res >>= 1;
			res |= RPA_SIGN_BIT;
		} else {
			res >>= 1;
		}
	}
	vm->r[ins->op1] = res;
	RPA_STATUS_CLRALL(vm);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_Z, !res);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_N, res & RPA_SIGN_BIT);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_C, res & RPA_SIGN_BIT);
}


static void rpa_vmop_tst(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	rpa_word_t res, op2 = vm->r[ins->op2], op3 = vm->r[ins->op3];
	
	res = op2 & op3;
	RPA_STATUS_CLRALL(vm);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_Z, !res);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_N, res & RPA_SIGN_BIT);
}


static void rpa_vmop_eor(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	rpa_word_t res, op2 = vm->r[ins->op2], op3 = vm->r[ins->op3];
	
	res = op2 ^ op3;
	vm->r[ins->op1] = res;
	RPA_STATUS_CLRALL(vm);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_Z, !res);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_N, res & RPA_SIGN_BIT);
}


static void rpa_vmop_not(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	rpa_word_t res, op2 = vm->r[ins->op2];
	
	res = ~op2;
	vm->r[ins->op1] = res;
	RPA_STATUS_CLRALL(vm);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_Z, !res);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_N, res & RPA_SIGN_BIT);
}


static void rpa_vmop_teq(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	rpa_word_t res, op2 = vm->r[ins->op2], op3 = vm->r[ins->op3];
	
	res = op2 ^ op3;
	RPA_STATUS_CLRALL(vm);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_Z, !res);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_N, res & RPA_SIGN_BIT);
}


static void rpa_vmop_bic(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	rpa_word_t res, op2 = vm->r[ins->op2], op3 = vm->r[ins->op3];
	
	res = op2 & ~op3;
	vm->r[ins->op1] = res;
	RPA_STATUS_CLRALL(vm);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_Z, !res);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_N, res & RPA_SIGN_BIT);
}


static void rpa_vmop_clz(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	rpa_word_t res, op2 = vm->r[ins->op2];
	
	for (res = RPA_REGISTER_BITS; op2; ) {
		op2 >>= 1;
		res -= 1;
	}
	vm->r[ins->op1] = res;
}


static void rpa_vmop_add(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	vm->r[ins->op1] = vm->r[ins->op2] + vm->r[ins->op3];
}


static void rpa_vmop_xcal(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	rpa_vm_callback *cbt = vm->cbtable[RPAVM_CB_TABLE(vm->r[ins->op1])];
	vm->r[0] = cbt[RPAVM_CB_OFFSET(vm->r[ins->op1])](vm);
}


static void rpa_vmop_sub(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	vm->r[ins->op1] = vm->r[ins->op2] - vm->r[ins->op3];
}


static void rpa_vmop_subs(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	rpa_word_t res, op2 = vm->r[ins->op2], op3 = vm->r[ins->op3];
	
	res = op2 - op3;
	vm->r[ins->op1] = res;
	RPA_STATUS_UPDATE(vm, RPA_STATUS_C, !(res > op2));  /* borrow = !carry */
	RPA_STATUS_UPDATE(vm, RPA_STATUS_Z, !res);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_N, res & RPA_SIGN_BIT);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_V, (op2 & RPA_SIGN_BIT) != (op3 & RPA_SIGN_BIT) &&
							(res & RPA_SIGN_BIT) == (op2 & RPA_SIGN_BIT));
}


static void rpa_vmop_sbc(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	rpa_word_t res, op2 = vm->r[ins->op2], op3 = vm->r[ins->op3];

	res = op2 - op3 + (RPA_STATUS_GETBIT(vm, RPA_STATUS_C) ? 0 : -1);
	vm->r[ins->op1] = res;
	RPA_STATUS_UPDATE(vm, RPA_STATUS_C, !(res > op2));	/* borrow = !carry */
	RPA_STATUS_UPDATE(vm, RPA_STATUS_Z, !res);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_N, res & RPA_SIGN_BIT);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_V, (op2 & RPA_SIGN_BIT) != (op3 & RPA_SIGN_BIT) &&
							(res & RPA_SIGN_BIT) == (op2 & RPA_SIGN_BIT));
}


static void rpa_vmop_mul(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	vm->r[ins->op1] = vm->r[ins->op2] * vm->r[ins->op3];
}


static void rpa_vmop_mls(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	rpa_word_t res;
	rpa_sword_t op2 = (rpa_sword_t)vm->r[ins->op2], op3 = (rpa_sword_t)vm->r[ins->op3];

	res = (rpa_word_t)(op2 * op3);
	vm->r[ins->op1] = res;
	/* TBD: Not sure how to update the RPA_STATUS_C */
	RPA_STATUS_CLRALL(vm);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_Z, !res);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_N, res & RPA_SIGN_BIT);
}


static void rpa_vmop_muls(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	rpa_word_t res, op2 = vm->r[ins->op2], op3 = vm->r[ins->op3];
	
	res = op2 * op3;
	vm->r[ins->op1] = res;
	RPA_STATUS_CLRALL(vm);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_C, op2 && (res / op2) != op3);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_Z, !res);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_N, res & RPA_SIGN_BIT);
}


static void rpa_vmop_div(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	if (!vm->r[ins->op3])
		RPAVM_ABORT(vm, RPAVM_E_DIVZERO);

	if (vm->r[ins->op3])
		vm->r[ins->op1] = vm->r[ins->op2] / vm->r[ins->op3];
}


static void rpa_vmop_divs(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	rpa_word_t res, op2 = vm->r[ins->op2], op3 = vm->r[ins->op3];

	if (!op3)
		RPAVM_ABORT(vm, RPAVM_E_DIVZERO);
	res = op2 / op3;
	vm->r[ins->op1] = res;
	RPA_STATUS_CLRALL(vm);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_Z, !res);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_N, res & RPA_SIGN_BIT);
}


static void rpa_vmop_dvs(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	rpa_word_t res;
	rpa_sword_t op2 = (rpa_sword_t)vm->r[ins->op2], op3 = (rpa_sword_t)vm->r[ins->op3];

	if (!op3)
		RPAVM_ABORT(vm, RPAVM_E_DIVZERO);
	res = (rpa_word_t)(op2 / op3);
	vm->r[ins->op1] = res;
	RPA_STATUS_CLRALL(vm);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_Z, !res);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_N, res & RPA_SIGN_BIT);
}


static void rpa_vmop_pushm(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	int n;
	rpa_word_t bits = vm->r[ins->op1];

	if ((vm->r[SP] % RPAVM_STACK_CHUNK) == 0)
		rpa_vm_check_space(vm);
	for (n = 0; bits && n < RLST; n++) {
		if (((rpa_word_t)(1 << n)) & bits) {
			vm->r[SP] += 1;
			vm->stack[vm->r[SP]] = vm->r[n];
			bits &= ~(1<<n);
		}
	}
}


static void rpa_vmop_popm(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	int n;
	rpa_word_t bits = vm->r[ins->op1];

	for (n = RLST - 1; bits && n >= 0; n--) {
		if (((rpa_word_t)(1 << n)) & bits) {
			vm->r[n] = vm->stack[vm->r[SP]];
			vm->r[SP] -= 1;
			bits &= ~(1<<n);
		}
	}
}


static void rpa_vmop_push(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	if ((vm->r[SP] % RPAVM_STACK_CHUNK) == 0)
		rpa_vm_check_space(vm);
	vm->r[SP] += 1;
	vm->stack[vm->r[SP]] = vm->r[ins->op1];
}


static void rpa_vmop_pop(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	vm->r[ins->op1] = vm->stack[vm->r[SP]];	
	vm->r[SP] -= 1;
	
}

static void rpa_vmop_stm(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	int n;
	rpa_word_t *dst = (rpa_word_t*)vm->r[ins->op1];
	rpa_word_t bits = vm->r[ins->op2];
	
	for (n = 0; bits && n < RLST; n++) {
		if (((rpa_word_t)(1 << n)) & bits) {
			*dst = vm->r[n];
			dst += 1;
			bits &= ~(1<<n);
		}
	}
}


static void rpa_vmop_ldm(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	int n;
	rpa_word_t *src = (rpa_word_t*)vm->r[ins->op1];
	rpa_word_t bits = vm->r[ins->op2];

	for (n = 0; bits && n < RLST; n++) {
		if (((rpa_word_t)(1 << n)) & bits) {
			vm->r[n] = *src;
			src += 1;
			bits &= ~(1<<n);
		}
	}
}


static void rpa_vmop_cmp(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	rpa_word_t op1 = vm->r[ins->op1];
	rpa_word_t op2 = vm->r[ins->op2];
	rpa_word_t res = (rpa_word_t)(op1 - op2);
	
	RPA_STATUS_UPDATE(vm, RPA_STATUS_C, op1 < op2);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_Z, !res);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_N, res & RPA_SIGN_BIT);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_V, (op1 & RPA_SIGN_BIT) != (op2 & RPA_SIGN_BIT) &&
							(res & RPA_SIGN_BIT) == (op1 & RPA_SIGN_BIT));
}


static void rpa_vmop_cmn(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	rpa_word_t op1 = vm->r[ins->op1];
	rpa_word_t op2 = vm->r[ins->op2];
	rpa_word_t res = (rpa_word_t)(op1 + op2);
	
	RPA_STATUS_UPDATE(vm, RPA_STATUS_C, res < op1 || res < op2);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_Z, !res);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_N, res & RPA_SIGN_BIT);
	RPA_STATUS_UPDATE(vm, RPA_STATUS_V, (op1 & RPA_SIGN_BIT) == (op2 & RPA_SIGN_BIT) &&
							(res & RPA_SIGN_BIT) != (op1 & RPA_SIGN_BIT));

}


static void rpa_vmop_nop(rpa_vm_t *vm, rpa_asmins_t *ins)
{
//	fprintf(stdout, "nop\n");
}


static void rpa_vmop_ret(rpa_vm_t *vm, rpa_asmins_t *ins)
{
	vm->r[PC] = vm->r[LR];
}


static rpa_vm_opcall opcalls[] = {
	rpa_vmop_exit,			// RPA_EXT
	rpa_vmop_asr,			// RPA_ASR
	rpa_vmop_xcal,			// RPA_XCAL
	rpa_vmop_mov,			// RPA_MOV
	rpa_vmop_add,			// RPA_ADD
	rpa_vmop_adds,			// RPA_ADDS
	rpa_vmop_adc,			// RPA_ADC
	rpa_vmop_and,			// RPA_AND
	rpa_vmop_bic,			// RPA_BIC
	rpa_vmop_clz,			// RPA_CLZ
	rpa_vmop_cmn,			// RPA_CMN
	rpa_vmop_eor,			// RPA_EOR
	rpa_vmop_sub,			// RPA_SUB
	rpa_vmop_subs,			// RPA_SUBS
	rpa_vmop_sbc,			// RPA_SBC
	rpa_vmop_mul,			// RPA_MUL
	rpa_vmop_mls,			// RPA_MLS
	rpa_vmop_muls,			// RPA_MULS
	rpa_vmop_not,			// RPA_NOT
	rpa_vmop_div,			// RPA_DIV
	rpa_vmop_dvs,			// RPA_DVS
	rpa_vmop_divs,			// RPA_DIVS
	rpa_vmop_bl,			// RPA_BL
	rpa_vmop_b,				// RPA_B
	rpa_vmop_str,			// RPA_STR
	rpa_vmop_strb,			// RPA_STRB
	rpa_vmop_strh,			// RPA_STRH
	rpa_vmop_ldr,			// RPA_LDR
	rpa_vmop_ldrb,			// RPA_LDRB
	rpa_vmop_ldrh,			// RPA_LDRH
	rpa_vmop_lsl,			// RPA_LSL
	rpa_vmop_lsr,			// RPA_LSR
	rpa_vmop_stm,			// RPA_STM
	rpa_vmop_ldm,			// RPA_LDM
	rpa_vmop_orr,			// RPA_ORR
	rpa_vmop_push,			// RPA_PUSH
	rpa_vmop_pop,			// RPA_POP
	rpa_vmop_cmp,			// RPA_CMP
	rpa_vmop_nop,			// RPA_NOP
	rpa_vmop_beq,			// RPA_BEQ
	rpa_vmop_bneq,			// RPA_BNEQ
	rpa_vmop_bleq,			// RPA_BLEQ
	rpa_vmop_bgeq,			// RPA_BGEQ
	rpa_vmop_bles,			// RPA_BLES
	rpa_vmop_bgre,			// RPA_BGRE
	rpa_vmop_ret,			// RPA_RET
	rpa_vmop_ror,			// RPA_ROR
	rpa_vmop_pushm, 		// RPA_PUSHM
	rpa_vmop_popm, 			// RPA_POPM
	rpa_vmop_tst, 			// RPA_TST
	rpa_vmop_teq, 			// RPA_TEQ
	(void*) 0,
	(void*) 0,
	(void*) 0,
	(void*) 0,
	(void*) 0,
	(void*) 0,
	(void*) 0,
	(void*) 0,
	
};
	

rpa_vm_t *rpa_vm_create()
{
	rpa_vm_t *vm;

	vm = (rpa_vm_t *)r_malloc(sizeof(*vm));
	if (!vm)
		return ((void*)0);
	r_memset(vm, 0, sizeof(*vm));	
	return vm;
}


void rpa_vm_destroy(rpa_vm_t * vm)
{
	r_free(vm->stack);
	r_free(vm);
}


int rpa_vm_exec(rpa_vm_t *vm, rpa_asmins_t *prog, rpa_word_t pc)
{
	rpa_asmins_t *pi;

	vm->r[PC] = pc;
	vm->abort = 0;
	vm->error = 0;
	do {
		pi = &prog[vm->r[PC]];
		vm->r[DA] = pi->data;
		opcalls[pi->opcode](vm, pi);
		if (vm->abort)
			return -1;
		vm->r[PC] += 1;
	} while (pi->opcode);
	return 0;
}


int rpa_vm_exec_debug(rpa_vm_t *vm, rpa_asmins_t *prog, rpa_word_t pc)
{
	rpa_asmins_t *pi;

	vm->r[PC] = pc;
	vm->abort = 0;
	vm->error = 0;
	do {
		pi = &prog[vm->r[PC]];
		vm->r[DA] = pi->data;
		opcalls[pi->opcode](vm, pi);
		if (vm->abort)
			return -1;
		rpa_vm_dumpregs(pi, vm);		
		vm->r[PC] += 1;
	} while (pi->opcode);
	return 0;
}


int rpa_vm_cbtable_add(rpa_vm_t *vm, rpa_vm_callback *cbtable)
{
	int ret;

	if (vm->cbtable_count >= RPAVM_MAX_CBTABLES)
		return -1;
	ret = vm->cbtable_count;
	vm->cbtable[vm->cbtable_count++] = cbtable;
	return ret;
}
