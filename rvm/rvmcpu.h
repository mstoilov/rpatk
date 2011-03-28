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

#ifndef _RVMCPU_H_
#define _RVMCPU_H_

#include "rtypes.h"
#include "rarray.h"
#include "rcarray.h"
#include "rvmreg.h"
#include "rvmgc.h"


#ifdef __cplusplus
extern "C" {
#endif


enum {
	RVM_EXT = 0,
	RVM_ABORT,		/* Abort: set the error code, op1: error code */
	RVM_PRN,
	RVM_ASR,		/* Arithmetic shift right: op1 = op2 >> op3, preserve the signed bit */
	RVM_SWI,
	RVM_SWIID,
	RVM_CALL,		/* if op1 is of type RVM_DTYPE_SWIID, then invoke RVM_SWIID, otherwise invoke RVM_BL */
	RVM_MOV,
	RVM_MOVS,		/* op1 <-- op2, compare to 0 and set the status accordingly */
	RVM_SWP,		/* op1 <--> op2 */
	RVM_INC,		/* INC: op1 = op1 + 1 */
	RVM_DEC,		/* DEC: op1 = op1 - 1 */
	RVM_ADD,		/* Add: op1 = op2 + op3 */
	RVM_ADDS,		/* Add: op1 = op2 + op3, update the status register */
	RVM_ADC,		/* Add: op1 = op2 + op3 + C, update the status register */
	RVM_AND,		/* Bitwise AND: op1 = op2 & op3, update status register */
	RVM_BIC,		/* Bit Clear: op1 = op2 & ~op3, update status register */
	RVM_CLZ,		/* Count rvm_reg_settypeLeading Zeros: op1 = leading_zeros(op2) */
	RVM_CMN,		/* Compare Negative: status register is updated based on the result: op1 + op2 */
	RVM_XOR,		/* XOR: op1 = op2 ^ op3, update the status register */
	RVM_SUB,
	RVM_SUBS,
	RVM_SBC,
	RVM_MUL,
	RVM_MLS,		/* Signed multiplication: op1 = op2 * op3 */
	RVM_MULS,
	RVM_NOT,		/* Bitwise NOT: op1 = ~op2, Update the status register */
	RVM_DIV,		/* Divide: op1 = op2 / op3 */
	RVM_DVS,		/* Signed division: op1 = op2 / op3 */	
	RVM_DIVS,		/* Divide: op1 = op2 / op3, Update the status register */
	RVM_MOD,		/* Modulo: op1 = op2 % op3 */
	RVM_MODS,		/* Modulo: op1 = op2 % op3, Update the status register */
	RVM_BX,			/* Jump to op1 */
	RVM_BXEQ,		/* Jump to op1, if equal */
	RVM_BXNEQ,		/* Jump to op1, if not equal */

	RVM_BXLEQ,		/* Branch if less or equal */
	RVM_BXGEQ,		/* Branch if greater or equal */
	RVM_BXLES,		/* Branch if less */
	RVM_BXGRE,		/* Branch if greater */


	RVM_BXL,		/* Jump to op1, link */
	RVM_BL,			/* Branch Link */
	RVM_B,			/* Branch */
	RVM_STR,		/* Save: val_at_location(op2) = op1 */
	RVM_STRP,		/* Save pointer: pointer_at_location(op2) = op1 */
	RVM_STRB,		/* Save: byte_at_location(op2) = op1 */
	RVM_STRH,		/* Save: u16_at_location(op2) = op1 */
	RVM_STRW,		/* Save: u32_at_location(op2) = op1 */
	RVM_STRR,		/* Save: rvmreg_t_at_location(op2) = op1 */
	RVM_LDR,		/* Load: op1 = val_at_location(op2) */
	RVM_LDRP,		/* Load pointer: op1 = pointer_at_location(op2) */
	RVM_LDRB,		/* Load Byte: op1 = byte_at_location(op2) */
	RVM_LDRH,		/* Load Half Word: op1 = u16_at_location(op2) */
	RVM_LDRW,		/* Load Word: op1 = u32_at_location(op2) */
	RVM_LDRR,		/* Load rvmreg_t: op1 = rvmreg_t_at_location(op2) */
	RVM_CFLAG,		/* Clear flag */
	RVM_CLR,		/* Clear op1 */
	RVM_CLRR,		/* Clear: rvmreg_t at memory location op1 */
	RVM_LSL,		/* Logical Shift Left: op1 = op2 << op3, update the status register */
	RVM_LSR,		/* Logical Shift Right: op1 = op2 >> op3, update the status register */
	RVM_LSRS,		/* Signed Logical Shift Right: op1 = op2 >> op3, update the status register */
	RVM_STM,		/* Store multiple */
	RVM_LDM,		/* Load multiple */
	RVM_STS,		/* Store op1 on the stack at position op2 + op3, i.e. stack[op2 + op3] = op1 */
	RVM_LDS,		/* Load op1 from thes stack at position op2 + op3, i.e. op1 = stack[op2 + op3] */
	RVM_ORR,		/* ORR: op1 = op2 | op3, update the status register */
	RVM_PUSH,
	RVM_POP,
	RVM_CMP,
	RVM_NOP,
	RVM_BEQ,		/* Branch if equal */
	RVM_BNEQ,		/* Branch if not equal */
	RVM_BLEQ,		/* Branch if less or equal */
	RVM_BGEQ,		/* Branch if greater or equal */
	RVM_BLES,		/* Branch if less */
	RVM_BGRE,		/* Branch if greater */
	RVM_RET,
	RVM_ROR,		/* Rotate right, the last bit rotated out updates the Carry flag */
	RVM_PUSHM,
	RVM_POPM,
	RVM_TST,
	RVM_TEQ,
	RVM_ADDRS,		/* Memory location of the stack at offset op2 + op3 */

/* Extended VM opcodes, */
	RVM_CAST,		/* Cast: op1 = (op3)op2 */
	RVM_TYPE,		/* Type: op1 = typeof(op2) */
	RVM_SETTYPE,	/* Type: op1.type = op2 */
	RVM_EMOV,
	RVM_EADD,		/* Add: op1 = op2 + op3, update the status register */
	RVM_ESUB,		/* Subtract: op1 = op2 - op3, update the status register */
	RVM_EMUL,		/* Multiply: op1 = op2 * op3, update the status register */
	RVM_EDIV,		/* Divide: op1 = op2 / op3, update the status register */
	RVM_EMOD,		/* Modulo: op1 = op2 % op3, update the status register */
	RVM_ELSL,		/* Logical Shift Left: op1 = op2 << op3, update the status register */
	RVM_ELSR,		/* Logical Shift Right: op1 = op2 >> op3, update the status register */
	RVM_ELSRU,		/* Logical Unsigned Shift Right: op1 = op2 >>> op3, update the status register */
	RVM_EAND,		/* Bitwise AND: op1 = op2 & op3, update status register */
	RVM_EORR,		/* Bitwise OR: op1 = op2 | op3, update the status register */
	RVM_EXOR,		/* Bitwise XOR: op1 = op2 ^ op3, update the status register */
	RVM_ENOT,		/* Bitwise NOT: op1 = ~op2, Update the status register */
	RVM_ELAND,		/* Logical AND: op1 = op2 && op3, update status register */
	RVM_ELOR,		/* Logical OR: op1 = op2 || op3, update the status register */
	RVM_ELNOT,		/* Logical NOT: op1 = !op2, update the status register */
	RVM_EEQ,		/* op1 = op2 == op3 ? 1 : 0, update the status register */
	RVM_ENOTEQ,		/* op1 = op2 != op3 ? 1 : 0, update the status register */
	RVM_EGREAT,		/* op1 = op2 > op3 ? 1 : 0, update the status register */
	RVM_EGREATEQ,	/* op1 = op2 >= op3 ? 1 : 0, update the status register */
	RVM_ELESS,		/* op1 = op2 < op3 ? 1 : 0, update the status register */
	RVM_ELESSEQ,	/* op1 = op2 <= op3 ? 1 : 0, update the status register */

	RVM_ECMP,		/* Compare: status register is updated based on the result: op1 - op2 */
	RVM_ECMN,		/* Compare Negative: status register is updated based on the result: op1 + op2 */
	RVM_ALLOCSTR,	/* Allocate string in op1, op2 is pointer (char*) to string, op3 is the size */
	RVM_ALLOCARR,	/* Allocate array in op1, op2 is the size */
	RVM_ADDRA,		/* op1 is the destination memory, op2 is the array, op3 is the offset */
	RVM_LDA,		/* op1 is the destination, op2 is the array, op3 is the offset */
	RVM_STA,		/* op1 is the source, op2 is the array, op3 is the offset */
	RVM_ALLOCOBJ,
	RVM_ADDROBJN,
	RVM_ADDROBJH,
	RVM_LDOBJN,
	RVM_STOBJN,
	RVM_LDOBJH,
	RVM_STOBJH,
	RVM_OBJLKUP,
	RVM_OBJADD,
	RVM_OBJLKUPADD,
};


#define RVM_REGISTER_BITS (8 * sizeof(rword))
#define RVM_SIGN_BIT (1LU << (RVM_REGISTER_BITS - 1))
#define RVM_STATUS_Z (1 << 0)
#define RVM_STATUS_N (1 << 1)
#define RVM_STATUS_C (1 << 2)
#define RVM_STATUS_V (1 << 3)
#define RVM_STATUS_E (1 << 4)
#define RVM_STATUS_ALL (RVM_STATUS_Z | RVM_STATUS_N | RVM_STATUS_C | RVM_STATUS_V | RVM_STATUS_E)
#define RVM_STATUS_GETBIT(cpu, bit) ((cpu)->status & (bit))
#define RVM_STATUS_SETBIT(cpu, bit) do { (cpu)->status |= (bit); } while (0)
#define RVM_STATUS_CLRBIT(cpu, bit) do { (cpu)->status &= ~(bit); } while (0)
#define RVM_STATUS_CLRALL(cpu) RVM_STATUS_CLRBIT(cpu, RVM_STATUS_ALL)

#define RVM_STATUS_UPDATE(cpu, b, c) \
do { \
    if (c) \
        RVM_STATUS_SETBIT(cpu, b); \
    else \
        RVM_STATUS_CLRBIT(cpu, b); \
} while (0)

#define RVM_OPERAND_BITS 4
#define RVM_REGS_NUM (1 << RVM_OPERAND_BITS)
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
#define R16 16
#define R17 17
#define R18 18
#define R19 19
#define R20 20
#define R21 21
#define R22 22
#define R23 23
#define R24 24
#define R25 25
#define R26 26
#define R27 27
#define R28 28
#define R29 29
#define R30 30
#define R31 31

#define RLST (RVM_REGS_NUM - 1)
#define IP (RLST - 7)
#define TP (RLST - 6)
#define FP (RLST - 5)
#define SP (RLST - 4)
#define LR (RLST - 3)
#define PC (RLST - 2)
#define DA (RLST - 1)				/* The DA register should never be modified manually, otherwise the result is undefined */
#define XX (RLST)


#define RVM_STACK_CHUNK 256
#define RVM_ABORT(__cpu__, __e__) do { __cpu__->error = (__e__); (__cpu__)->abort = 1; R_ASSERT(0); return; } while (0)
#define BIT(__shiftby__) (1 << (__shiftby__))
#define BITR(__f__, __l__, __r__) (((__r__) >= (__f__) && (__r__) <= (__l__)) ? BIT(__r__) : 0)
#define BITS(__f__, __l__)  (BITR(__f__, __l__, R0)) | (BITR(__f__, __l__, R1)) | (BITR(__f__, __l__, R2)) | (BITR(__f__, __l__, R3)) | \
							(BITR(__f__, __l__, R4)) | (BITR(__f__, __l__, R5)) | (BITR(__f__, __l__, R6)) | (BITR(__f__, __l__, R7)) | \
							(BITR(__f__, __l__, R8)) | (BITR(__f__, __l__, R9)) | (BITR(__f__, __l__, R10)) | (BITR(__f__, __l__, R11)) | \
							(BITR(__f__, __l__, R12)) | (BITR(__f__, __l__, R13)) | (BITR(__f__, __l__, R14)) | (BITR(__f__, __l__, R15)) | \
							(BITR(__f__, __l__, R16)) | (BITR(__f__, __l__, R17)) | (BITR(__f__, __l__, R18)) | (BITR(__f__, __l__, R19)) | \
							(BITR(__f__, __l__, R20)) | (BITR(__f__, __l__, R21)) | (BITR(__f__, __l__, R22)) | (BITR(__f__, __l__, R23)) | \
							(BITR(__f__, __l__, R24)) | (BITR(__f__, __l__, R25)) | (BITR(__f__, __l__, R26)) | (BITR(__f__, __l__, R27)) | \
							(BITR(__f__, __l__, R28)) | (BITR(__f__, __l__, R29)) | (BITR(__f__, __l__, R30)) | (BITR(__f__, __l__, R31))

#define RVM_OPCODE_BITS 8
#define RVM_SWI_TABLE_BITS 8
#define RVM_SWI_NUM_BITS 8
#define RVM_SWI_TABLE(__op__) ((__op__) >> RVM_SWI_NUM_BITS)
#define RVM_SWI_NUM(__op__) ((__op__) & ((1 << RVM_SWI_NUM_BITS) - 1))
#define RVM_SWI_ID(__t__, __o__) ((((__t__) & ((1 << RVM_SWI_TABLE_BITS) - 1))  << RVM_SWI_NUM_BITS) | ((__o__) & ((1 << RVM_SWI_NUM_BITS) - 1)))
#define RVM_ASMINS_OPCODE(__opcode__) ((__opcode__) & ((1 << RVM_OPCODE_BITS) - 1))
#define RVM_ASMINS_SWI(__opcode__) ((__opcode__) >> RVM_OPCODE_BITS)
#define RVM_OPSWI(__id__) (((__id__) << RVM_OPCODE_BITS) | RVM_SWI)

//#define RVM_STACK_WRITE(__s__, __sp__, __p__) r_carray_replace((__s__), (__sp__), (rconstpointer)(__p__));
//#define RVM_STACK_READ(__s__, __sp__) r_carray_index((__s__), (__sp__), rvmreg_t)
//#define RVM_STACK_ADDR(__s__, __sp__) r_carray_slot_expand((__s__), (__sp__))


#define RVM_STACK_WRITE(__s__, __sp__, __p__) do { r_memcpy(((rvmreg_t*)(__s__)) + (__sp__), (__p__), sizeof(rvmreg_t)); } while(0)
#define RVM_STACK_READ(__s__, __sp__) *RVM_STACK_ADDR(__s__, __sp__)
#define RVM_STACK_ADDR(__s__, __sp__) (((rvmreg_t*)(__s__)) + (__sp__))
#define RVM_STACK_CHECKSIZE(__cpu__, __s__, __sp__)  ((__sp__) < (__cpu__)->stacksize ? 1 : 0)
#define RVM_CODE2BYTE_OFFSET(__codeoff__) ((rword)(((rsword)(__codeoff__)) * ((rsword)sizeof(rvm_asmins_t))))
#define RVM_BYTE2CODE_OFFSET(__byteoff__) ((rword)(((rsword)(__byteoff__)) / ((rsword)sizeof(rvm_asmins_t))))

#define RVM_E_DIVZERO		(1)
#define RVM_E_ILLEGAL		(2)
#define RVM_E_CAST			(3)
#define RVM_E_SWINUM		(4)
#define RVM_E_SWITABLE		(5)
#define RVM_E_LVALUE		(6)
#define RVM_E_NOMEM			(7)
#define RVM_E_NOTFUNCTION	(8)
#define RVM_E_NOTOBJECT		(9)

typedef struct rvm_asmins_s rvm_asmins_t;
typedef struct rvmcpu_s rvmcpu_t;
typedef void (*rvmcpu_swi)(rvmcpu_t *cpu, rvm_asmins_t *ins);
typedef void (*rvm_cpu_op)(rvmcpu_t *cpu, rvm_asmins_t *ins);

typedef struct rvm_switable_s {
	const char *name;
	rvmcpu_swi op;
} rvm_switable_t;


#define RVM_ASMINS_RELOC (1 << 0)

struct rvm_asmins_s {
	rvmreg_t data;
	ruint16 op1:RVM_OPERAND_BITS;
	ruint16 op2:RVM_OPERAND_BITS;
	ruint16 op3:RVM_OPERAND_BITS;
	ruint16 da:1;
	ruint16 swi;
	ruint8 flags;
	ruint8 opcode;
};


struct rvm_opmap_s;

struct rvmcpu_s {
	rvmreg_t r[RVM_REGS_NUM];
	rword status;
	rword error;
	rword abort;
	rharray_t *switables;
	rulong stacksize;
	void *stack;
	rcarray_t *data;
	struct rvm_opmap_s *opmap;
	rvmreg_t *thisptr;
	rvm_gc_t *gc;
	void *userdata1;
	void *userdata2;
	void *userdata3;
	void *userdata4;
	void *userdata5;
	void *userdata6;
	void *userdata7;
};


rvmcpu_t *rvm_cpu_create_default();
rvmcpu_t *rvm_cpu_create(rulong stacksize);
void rvm_cpu_destroy(rvmcpu_t * vm);
rint rvm_cpu_exec(rvmcpu_t *cpu, rvm_asmins_t *prog, rword off);
rint rvm_cpu_exec_debug(rvmcpu_t *cpu, rvm_asmins_t *prog, rword off);
rint rvm_cpu_swilookup(rvmcpu_t *cpu, const rchar *tabname, const rchar *swiname, rsize_t size);
rint rvm_cpu_swilookup_s(rvmcpu_t *cpu, const rchar *tabname, const rchar *swiname);
rint rvm_cpu_addswitable(rvmcpu_t * cpu, const rchar *tabname, rvm_switable_t *switalbe);
rvmreg_t *rvm_cpu_alloc_global(rvmcpu_t *cpu);
int rvm_cpu_setreg(rvmcpu_t *cpu, rword regnum, const rvmreg_t *src);
rvmreg_t *rvm_cpu_getreg(rvmcpu_t *cpu, rword regnum);
rvm_asmins_t rvm_asm(rword opcode, rword op1, rword op2, rword op3, rword data);
rvm_asmins_t rvm_asma(rword opcode, rword op1, rword op2, rword op3, rchar *data, rulong size);
rvm_asmins_t rvm_asml(rword opcode, rword op1, rword op2, rword op3, rlong data);
rvm_asmins_t rvm_asmb(rword opcode, rword op1, rword op2, rword op3, ruint data);
rvm_asmins_t rvm_asmd(rword opcode, rword op1, rword op2, rword op3, rdouble data);
rvm_asmins_t rvm_asmp(rword opcode, rword op1, rword op2, rword op3, rpointer data);
rvm_asmins_t rvm_asms(rword opcode, rword op1, rword op2, rword op3, rword data);
rvm_asmins_t rvm_asmf(rword opcode, rword op1, rword op2, rword op3, rword data);
rvm_asmins_t rvm_asm2(rword opcode, rword op1, rword op2, rword op3, ruint32 p1, ruint32 p2);
rvm_asmins_t rvm_asmr(rword opcode, rword op1, rword op2, rword op3, rpointer pReloc);
rvm_asmins_t rvm_asmx(rword opcode, rword op1, rword op2, rword op3, rpointer pReloc);
void rvm_asm_dump(rvm_asmins_t *pi, ruint count);


#ifdef __cplusplus
}
#endif


#endif
