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


#ifdef __cplusplus
extern "C" {
#endif

#define RVM_MIN_REGSIZE (sizeof(rword)/8)


enum {
	RVM_EXT = 0,
	RVM_ASR,		/* Arithmetic shift right: op1 = op2 >> op3, preserve the signed bit */
	RVM_SWI,
	RVM_MOV,
	RVM_ADD,		/* Add: op1 = op2 + op3 */
	RVM_ADDS,		/* Add: op1 = op2 + op3, update the status register */
	RVM_ADC,		/* Add: op1 = op2 + op3 + C, update the status register */
	RVM_AND,		/* Bitwise AND: op1 = op2 & op3, update status register */
	RVM_BIC,		/* Bit Clear: op1 = op2 & ~op3, update status register */
	RVM_CLZ,		/* Count Leading Zeros: op1 = leading_zeros(op2) */
	RVM_CMN,		/* Compare Negative: status register is updated based on the result: op1 + op2 */
	RVM_EOR,		/* XOR: op1 = op2 ^ op3, update the status register */
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
	RVM_BL,			/* Branch Link */
	RVM_B,			/* Branch */
	RVM_STR,		/* Save: val_at_location(op2) = op1 */
	RVM_STRP,		/* Save pointer: pointer_at_location(op2) = op1 */
	RVM_STRB,		/* Save: byte_at_location(op2) = op1 */
	RVM_STRH,		/* Save: u16_at_location(op2) = op1 */
	RVM_STRW,		/* Save: u32_at_location(op2) = op1 */
	RVM_STRR,		/* Save: rev_reg_t_at_location(op2) = op1 */
	RVM_LDR,		/* Load: op1 = val_at_location(op2) */
	RVM_LDRP,		/* Load pointer: op1 = pointer_at_location(op2) */
	RVM_LDRB,		/* Load Byte: op1 = byte_at_location(op2) */
	RVM_LDRH,		/* Load Half Word: op1 = u16_at_location(op2) */
	RVM_LDRW,		/* Load Word: op1 = u32_at_location(op2) */
	RVM_LDRR,		/* Load rvm_reg_t: op1 = rvm_reg_t_at_location(op2) */
	RVM_LSL,		/* Logical Shift Left: op1 = op2 << op3, update the status register */
	RVM_LSR,		/* Logical Shift Right: op1 = op2 >> op3, update the status register */
	RVM_STM,
	RVM_LDM,
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
};


#define RVM_DTYPE_WORD 0
#define RVM_DTYPE_POINTER 1
#define RVM_DTYPE_LONG 2
#define RVM_DTYPE_DOUBLE 3
#define RVM_DTYPE_BOOLEAN 4
#define RVM_DTYPE_OBJECT 5
#define RVM_DTYPE_STRING 6
#define RVM_DTYPE_ARRAY 7
#define RVM_DTYPE_NAMEDARRAY 8
#define RVM_DTYPE_RELOCPTR 14
#define RVM_DTYPE_RELOCINDEX 15
#define RVM_DTYPE_USER 16
#define RVM_DTYPE_SIZE (1 << 5)
#define RVM_DTYPE_MASK (RVM_DTYPE_SIZE - 1)
#define RVM_DTYPE_MAX (RVM_DTYPE_MASK)
#define RVM_DTYPE_USERDEF(__n__) (RVM_DTYPE_USER + (__n__))

#define RVM_INFOBIT_ALL ((ruint32)-1)
#define RVM_INFOBIT_REFOBJECT (1 << 8)


#define RVM_REGISTER_BITS (8 * sizeof(rword))
#define RVM_SIGN_BIT (1LU << (RVM_REGISTER_BITS - 1))
#define RVM_STATUS_Z (1 << 0)
#define RVM_STATUS_N (1 << 1)
#define RVM_STATUS_C (1 << 2)
#define RVM_STATUS_V (1 << 3)
#define RVM_STATUS_GETBIT(cpu, bit) ((cpu)->status & (bit))
#define RVM_STATUS_SETBIT(cpu, bit) do { (cpu)->status |= (bit); } while (0)
#define RVM_STATUS_CLRBIT(cpu, bit) do { (cpu)->status &= ~(bit); } while (0)
#define RVM_STATUS_CLRALL(cpu) RVM_STATUS_CLRBIT(cpu, (RVM_STATUS_Z | RVM_STATUS_N | RVM_STATUS_C | RVM_STATUS_V))

#define RVM_STATUS_UPDATE(cpu, b, c) \
do { \
    if (c) \
        RVM_STATUS_SETBIT(cpu, b); \
    else \
        RVM_STATUS_CLRBIT(cpu, b); \
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
#define FP R12
#define SP R13
#define LR R14
#define PC R15
#define DA 16
#define RLST 16
#define XX 255

#define RVM_STACK_CHUNK 256
#define RVM_ABORT(__cpu__, __e__) do { __cpu__->error = (__e__); (__cpu__)->abort = 1; return; } while (0)
#define BIT(__shiftby__) (1 << (__shiftby__))
#define RVM_REGU(__r__) (__r__)->v.w
#define RVM_GET_REGU(__cpu__, __r__) (__cpu__)->r[(__r__)].v.w
#define RVM_REG_ASSIGN_UNSIGNED(__r__, val) do { (__r__)->v.w = (rword)(val); rvm_reg_setinfo((__r__), RVM_DTYPE_WORD);} while (0)
#define RVM_SET_REGU(__cpu__, __r__, __val__) RVM_REG_ASSIGN_UNSIGNED(RVM_REG_PTR(__cpu__, __r__), __val__)

#define RVM_REGL(__r__) (__r__)->v.l
#define RVM_GET_REGL(__cpu__, __r__) (__cpu__)->r[(__r__)].v.l
//#define RVM_SET_REGL(__cpu__, __r__, val) do { (__cpu__)->r[(__r__)].v.l = (rlong)(val); rvm_reg_setinfo(&(__cpu__)->r[(__r__)], RVM_DTYPE_LONG);} while (0)
#define RVM_REG_ASSIGN_LONG(__r__, val) do { (__r__)->v.l = (rlong)(val); rvm_reg_setinfo((__r__), RVM_DTYPE_LONG);} while (0)
#define RVM_SET_REGL(__cpu__, __r__, __val__) RVM_REG_ASSIGN_LONG(RVM_REG_PTR(__cpu__, __r__), __val__)

#define RVM_REGP(__r__) (__r__)->v.p
#define RVM_GET_REGP(__cpu__, __r__) (__cpu__)->r[(__r__)].v.p
//#define RVM_SET_REGP(__cpu__, __r__, val) do { (__cpu__)->r[(__r__)].v.p = (rpointer)(val); rvm_reg_setinfo(&(__cpu__)->r[(__r__)], RVM_DTYPE_POINTER);} while (0)
#define RVM_REG_ASSIGN_POINTER(__r__, val) do { (__r__)->v.p = (rpointer)(val); rvm_reg_setinfo((__r__), RVM_DTYPE_POINTER);} while (0)
#define RVM_SET_REGP(__cpu__, __r__, __val__) RVM_REG_ASSIGN_POINTER(RVM_REG_PTR(__cpu__, __r__), __val__)

#define RVM_REGD(__r__) (__r__)->v.d
#define RVM_GET_REGD(__cpu__, __r__) (__cpu__)->r[(__r__)].v.d
#define RVM_SET_REGD(__cpu__, __r__, __val__) do { (__cpu__)->r[(__r__)].v.d = (rdouble)(__val__); rvm_reg_setinfo(&(__cpu__)->r[(__r__)], RVM_DTYPE_DOUBLE);} while (0)
#define RVM_REG_ASSIGN_DOUBLE(__r__, val) do { (__r__)->v.d = (rdouble)(val); rvm_reg_setinfo((__r__), RVM_DTYPE_DOUBLE);} while (0)

#define RVM_REG(__r__) (__r__)
#define RVM_REGM(__r__) (rvm_asmins_t*)((__r__)->v.p)
#define RVM_GET_REGM(__cpu__, __r__) ((rvm_asmins_t*)(__cpu__)->r[(__r__)].v.p)
#define RVM_SET_REGM(__cpu__, __r__, __p__) do { (__cpu__)->r[(__r__)].v.p = ((rpointer)(__p__)); rvm_reg_setinfo(&(__cpu__)->r[(__r__)], RVM_DTYPE_POINTER);} while (0)
#define RVM_INC_REGM(__cpu__, __r__, __val__) do {rvm_asmins_t *p = RVM_GET_REGM(__cpu__, __r__); (__cpu__)->r[(__r__)].v.p = (rpointer)(p + (__val__)); } while (0)
#define RVM_REG_INFO(__r__) (__r__)->info
#define RVM_REG_SIZE(__r__) (__r__)->size
#define RVM_REG_PTR(__cpu__, __r__) (&(__cpu__)->r[(__r__)])
#define RVM_GET_REG(__cpu__, __r__) (__cpu__)->r[(__r__)]
#define RVM_SET_REG(__cpu__, __r__, val) do { (__cpu__)->r[(__r__)] = (rvm_reg_t)(val); } while (0)
#define RVM_REG_ASSIGN_STRING(__r__, val) do { (__r__)->v.p = (rpointer)(val); rvm_reg_setinfo((__r__), RVM_INFOBIT_REFOBJECT|RVM_DTYPE_STRING);} while (0)
#define RVM_REG_REF(__r__) do { if (rvm_reg_flagtst((__r__), RVM_INFOBIT_REFOBJECT)) r_ref_inc((rref_t*)RVM_REGP(__r__));} while (0)
#define RVM_REG_UNREF(__r__) do { if (rvm_reg_flagtst((__r__), RVM_INFOBIT_REFOBJECT)) r_ref_dec((rref_t*)RVM_REGP(__r__));} while (0)
#define RVM_REG_CLEAR(__r__) do { (__r__)->v.w = 0UL;  rvm_reg_setinfo((__r__), 0); } while (0)

#define RVM_SWI_TABLE(__op__) ((__op__) >> 16)
#define RVM_SWI_NUM(__op__) ((__op__) & ((1 << 16) - 1))
#define RVM_SWI_ID(__t__, __o__) ((((__t__) & ((1 << 16) - 1))  << 16) | ((__o__) & ((1 << 16) - 1)))


#define RVM_E_DIVZERO  (1)
#define RVM_E_ILLEGAL  (2)
#define RVM_E_SWINUM   (3)
#define RVM_E_SWITABLE (4)


typedef struct rvm_asmins_s rvm_asmins_t;
typedef struct rvm_cpu_s rvm_cpu_t;
typedef void (*rvm_cpu_swi)(rvm_cpu_t *cpu, rvm_asmins_t *ins);
typedef void (*rvm_cpu_op)(rvm_cpu_t *cpu, rvm_asmins_t *ins);

typedef struct rvm_switable_s {
	const char *name;
	rvm_cpu_swi op;
} rvm_switable_t;


typedef struct rvm_reg_s {
	union {
		rword w;
		rlong l;
		rpointer p;
		rdouble d;
		ruint8 c[RVM_MIN_REGSIZE];
	} v;
	ruint32 info;
	ruint32 size;
} rvm_reg_t;


struct rvm_asmins_s {
	ruint8 opcode;
	ruint8 op1;
	ruint8 op2;
	ruint8 op3;
	rvm_reg_t data;	
};


struct rvm_cpu_s {
	rvm_reg_t r[DA + 1];
	rword status;
	rword error;
	rword abort;
	rarray_t *switables;
	rarray_t *stack;
	void *userdata;
};


rvm_cpu_t *rvm_cpu_create();
void rvm_cpu_destroy(rvm_cpu_t * vm);
rint rvm_cpu_switable_add(rvm_cpu_t * cpu, rvm_switable_t *switalbe);
rint rvm_cpu_exec(rvm_cpu_t *cpu, rvm_asmins_t *prog, rword off);
rint rvm_cpu_exec_debug(rvm_cpu_t *cpu, rvm_asmins_t *prog, rword off);
rint rvm_cpu_getswi(rvm_cpu_t *cpu, const rchar *swiname);
void rvm_relocate(rvm_asmins_t *code, rsize_t size);
rvm_asmins_t rvm_asm(rword opcode, rword op1, rword op2, rword op3, rword data);
rvm_asmins_t rvm_asmi(rword opcode, rword op1, rword op2, rword op3, rint data);
rvm_asmins_t rvm_asml(rword opcode, rword op1, rword op2, rword op3, rlong data);
rvm_asmins_t rvm_asmu(rword opcode, rword op1, rword op2, rword op3, rword data);
rvm_asmins_t rvm_asmp(rword opcode, rword op1, rword op2, rword op3, rpointer data);
rvm_asmins_t rvm_asmr(rword opcode, rword op1, rword op2, rword op3, rpointer pReloc);
rvm_asmins_t rvm_asmx(rword opcode, rword op1, rword op2, rword op3, rpointer pReloc);
rvm_asmins_t rvm_asmd(rword opcode, rword op1, rword op2, rword op3, rdouble data);
void rvm_asm_dump(rvm_asmins_t *pi, ruint count);
void rvm_reg_settype(rvm_reg_t *r, ruint type);
ruint rvm_reg_gettype(rvm_reg_t *r);
void rvm_reg_setinfo(rvm_reg_t *r, ruint info);
ruint rvm_reg_getinfo(rvm_reg_t *r);
rboolean rvm_reg_flagtst(rvm_reg_t *r, ruint32 flag);
void rvm_reg_flagset(rvm_reg_t *r, ruint32 flag);
void rvm_reg_flagclr(rvm_reg_t *r, ruint32 flag);
void rvm_reg_setlong(rvm_reg_t *r, rlong l);
void rvm_reg_setdouble(rvm_reg_t *r, rdouble d);


#ifdef __cplusplus
}
#endif


#endif
