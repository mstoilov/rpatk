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

#ifndef _RPAVM_H_
#define _RPAVM_H_

#include "rpatypes.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
	RPA_EXT = 0,
	RPA_ASR,		/* Arithmetic shift right: op1 = op2 >> op3, preserve the signed bit */
	RPA_XCAL,
	RPA_MOV,
	RPA_ADD,		/* Add: op1 = op2 + op3 */
	RPA_ADDS,		/* Add: op1 = op2 + op3, update the status register */
	RPA_ADC,		/* Add: op1 = op2 + op3 + C, update the status register */
	RPA_AND,		/* Bitwise AND: op1 = op2 & op3, update status register */
	RPA_BIC,		/* Bit Clear: op1 = op2 & ~op3, update status register */
	RPA_CLZ,		/* Count Leading Zeros: op1 = leading_zeros(op2) */
	RPA_CMN,		/* Compare Negative: status register is updated based on the result: op1 + op2 */
	RPA_EOR,		/* XOR: op1 = op2 ^ op3, update the status register */
	RPA_SUB,
	RPA_SUBS,
	RPA_SBC,
	RPA_MUL,
	RPA_MLS,		/* Signed multiplication: op1 = op2 * op3 */
	RPA_MULS,
	RPA_NOT,		/* Bitwise NOT: op1 = ~op2, Update the status register */
	RPA_DIV,		/* Divide: op1 = op2 / op3 */
	RPA_DVS,		/* Signed division: op1 = op2 / op3 */	
	RPA_DIVS,		/* Divide: op1 = op2 / op3, Update the status register */
	RPA_BL,			/* Branch Link */
	RPA_B,			/* Branch */
	RPA_STR,		/* Save: val_at_location(op2) = op1 */
	RPA_STRB,		/* Save: byte_at_location(op2) = op1 */
	RPA_STRH,		/* Save: halfword_at_location(op2) = op1 */
	RPA_LDR,		/* Load: op1 = val_at_location(op2) */
	RPA_LDRB,		/* Load Byte: op1 = byte_at_location(op2) */
	RPA_LDRH,		/* Load Half Word: op1 = halfword_at_location(op2) */
	RPA_LSL,		/* Logical Shift Left: op1 = op2 << op3, update the status register */
	RPA_LSR,		/* Logical Shift Right: op1 = op2 >> op3, update the status register */
	RPA_STM,
	RPA_LDM,
	RPA_ORR,		/* ORR: op1 = op2 | op3, update the status register */
	RPA_PUSH,
	RPA_POP,
	RPA_CMP,
	RPA_NOP,
	RPA_BEQ,
	RPA_BNEQ,
	RPA_BLEQ,
	RPA_BGEQ,
	RPA_BLES,
	RPA_BGRE,
	RPA_RET,
	RPA_ROR,		/* Rotate right, the last bit rotated out updates the Carry flag */
	RPA_PUSHM,
	RPA_POPM,
	RPA_TST,
	RPA_TEQ,
};


#define RPA_REGISTER_BITS (8 * sizeof(rpa_word_t))
#define RPA_SIGN_BIT ((rpa_word_t)1LU << (RPA_REGISTER_BITS - 1))
#define RPA_STATUS_Z (1 << 0)
#define RPA_STATUS_N (1 << 1)
#define RPA_STATUS_C (1 << 2)
#define RPA_STATUS_V (1 << 3)
#define RPA_STATUS_GETBIT(vm, bit) ((vm)->status & (bit))
#define RPA_STATUS_SETBIT(vm, bit) do { (vm)->status |= (bit); } while (0)
#define RPA_STATUS_CLRBIT(vm, bit) do { (vm)->status &= ~(bit); } while (0)
#define RPA_STATUS_CLRALL(vm) RPA_STATUS_CLRBIT(vm, (RPA_STATUS_Z | RPA_STATUS_N | RPA_STATUS_C | RPA_STATUS_V))

#define RPA_STATUS_UPDATE(vm, b, c) \
do { \
    if (c) \
        RPA_STATUS_SETBIT(vm, b); \
    else \
        RPA_STATUS_CLRBIT(vm, b); \
} while (0)


#define R0 0
#define R1 1
#define R2 2
#define R3 3
#define R4 4
#define R5 5
#define R6 6
#define R7 7
#define R8 8
#define R9 9
#define R10 10
#define R11 11
#define R12 12
#define R13 13
#define R14 14
#define R15 15
#define SP R13
#define LR R14
#define PC R15
#define DA 16
#define RLST 16
#define XX 255

#define RPAVM_STACK_CHUNK 256
#define RPAVM_MAX_CBTABLES 8
#define RPAVM_CB_TABLE(op) (op >> 16)
#define RPAVM_CB_OFFSET(op) (op & ((1 << 16) - 1))
#define RPAVM_CB(table, offset) ((((table) & ((1 << 16) - 1))  << 16) | ((offset) & ((1 << 16) - 1)))
#define RPAVM_ABORT(vm, e) do { vm->error = (e); vm->abort = 1; return; } while (0)
#define BIT(_shiftby_) (1 << (_shiftby_))

#define RPAVM_E_DIVZERO (1)
#define RPAVM_E_ILLEGAL (2)



typedef struct rpa_asmins_s rpa_asmins_t;
typedef struct rpa_vm_s rpa_vm_t;
typedef rpa_word_t (*rpa_vm_callback)(rpa_vm_t *vm);
typedef void (*rpa_vm_opcall)(rpa_vm_t *vm, rpa_asmins_t *ins);


struct rpa_asmins_s {
	unsigned char opcode;
	unsigned char op1;
	unsigned char op2;
	unsigned char op3;
	rpa_word_t data;	
};

struct rpa_vm_s {
	rpa_word_t r[DA + 1];
	rpa_word_t status;
	rpa_word_t error;
	rpa_word_t abort;
	rpa_word_t *stack;
	rpa_word_t stacksize;
	rpa_vm_callback *cbtable[RPAVM_MAX_CBTABLES];
	unsigned int cbtable_count;
	void *userdata;
};


rpa_vm_t *rpa_vm_create();
void rpa_vm_destroy(rpa_vm_t * vm);
int rpa_vm_cbtable_add(rpa_vm_t * vm, rpa_vm_callback *cbtable);
int rpa_vm_exec(rpa_vm_t *vm, rpa_asmins_t *prog, rpa_word_t pc);
int rpa_vm_exec_debug(rpa_vm_t *vm, rpa_asmins_t *prog, rpa_word_t pc);
rpa_asmins_t rpa_asm(rpa_word_t opcode, rpa_word_t op1, rpa_word_t op2, rpa_word_t op3, rpa_word_t data);
void rpa_asm_dump(rpa_asmins_t *pi, unsigned int count);


#ifdef __cplusplus
}
#endif


#endif
