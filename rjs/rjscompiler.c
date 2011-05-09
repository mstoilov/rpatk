#include "rmem.h"
#include "rjscompiler.h"

rint rjs_compiler_playreversechildrecords(rjs_compiler_t *co, rarray_t *records, rlong rec);
static rint rjs_compiler_playrecord(rjs_compiler_t *co, rarray_t *records, rlong rec);
static rint rjs_compiler_playchildrecords(rjs_compiler_t *co, rarray_t *records, rlong rec);


void rjs_compiler_debughead(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	if (co->debug) {
		rpa_record_dump(records, rec);
		co->headoff = rvm_codegen_getcodesize(co->cg);
	}
}


void rjs_compiler_debugtail(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	if (co->debug) {
		rvm_asm_dump(rvm_codegen_getcode(co->cg, co->headoff), rvm_codegen_getcodesize(co->cg) - co->headoff);
	}

}


static const rchar *rjs_compiler_record2str(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec = (rparecord_t *)r_array_slot(records, rpa_recordtree_get(records, rec, RPA_RECORD_END));
	rsize_t size = 16; /* Min size */

	if (prec && prec->inputsiz) {
		size = prec->inputsiz + 1;
	}
	co->temp = r_realloc(co->temp, size);
	r_memset(co->temp, 0, size);
	if (prec->input && prec->inputsiz)
		r_memcpy(co->temp, prec->input, prec->inputsiz);
	return co->temp;
}


static rlong rjs_compiler_record2identifer(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rvm_varmap_t *v = NULL;
	rjs_coctx_t *ctx = NULL;
	rparecord_t *prec = (rparecord_t *)r_array_slot(records, rec);

	/*
	 * First lets find out if we are within a function definition or
	 * this is a global variable.
	 */
	ctx = rjs_compiler_getctx(co, RJS_COCTX_FUNCTION | RJS_COCTX_GLOBAL);
	R_ASSERT(ctx);

	v = rvm_scope_tiplookup(co->scope, prec->input, prec->inputsiz);

	/*
	 * TBD: Temporary here
	 */
	if (v) {
		r_printf("ERROR: Identifier already defined: %s\n", rjs_compiler_record2str(co, records, rec));
		return -1;
	}

	if (ctx->type == RJS_COCTX_FUNCTION) {
		rjs_coctx_function_t *functx = (rjs_coctx_function_t *)ctx;
		functx->allocs += 1;
		rvm_scope_addoffset(co->scope, prec->input, prec->inputsiz, functx->allocs);
	} else {
		rjs_coctx_global_t *functx = (rjs_coctx_global_t *)ctx;
		functx->allocs += 1;
		r_carray_setlength(co->cpu->data, functx->allocs + 1);
		rvm_scope_addpointer(co->scope, prec->input, prec->inputsiz, r_carray_slot(co->cpu->data, functx->allocs));
	}
	v = rvm_scope_tiplookup(co->scope, prec->input, prec->inputsiz);
	if (v->datatype == VARMAP_DATATYPE_OFFSET) {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_ADDRS, R0, FP, DA, v->data.offset));
	} else {
		rvm_codegen_addins(co->cg, rvm_asmp(RVM_MOV, R0, DA, XX, v->data.ptr));
	}

	return 0;
}


rjs_coctx_t *rjs_compiler_getctx(rjs_compiler_t *co, rulong type)
{
	rlong i;
	rjs_coctx_t *ctx;

	for (i = r_array_length(co->coctx) - 1; i >= 0; i--) {
		ctx = r_array_index(co->coctx, i, rjs_coctx_t*);
		if (ctx->type & type)
			return ctx;
	}
	return NULL;
}


rlong rjs_compiler_record2opcode(rparecord_t *prec)
{
	const rchar *input = prec->input;
	rsize_t size = prec->inputsiz;

	if (r_stringncmp("++", input,  size))
		return RVM_EADD;
	else if (r_stringncmp("+", input,  size))
		return RVM_EADD;
	else if (r_stringncmp("+=", input,  size))
		return RVM_EADD;
	else if (r_stringncmp("--", input,  size))
		return RVM_ESUB;
	else if (r_stringncmp("-", input,  size))
		return RVM_ESUB;
	else if (r_stringncmp("-=", input,  size))
		return RVM_ESUB;
	else if (r_stringncmp("*", input,  size))
		return RVM_EMUL;
	else if (r_stringncmp("*=", input,  size))
		return RVM_EMUL;
	else if (r_stringncmp("/", input,  size))
		return RVM_EDIV;
	else if (r_stringncmp("/=", input,  size))
		return RVM_EDIV;
	else if (r_stringncmp("%", input,  size))
		return RVM_EMOD;
	else if (r_stringncmp("%=", input,  size))
		return RVM_EMOD;
	else if (r_stringncmp("&&", input,  size))
		return RVM_ELAND;
	else if (r_stringncmp("||", input,  size))
		return RVM_ELOR;
	else if (r_stringncmp("&", input,  size))
		return RVM_EAND;
	else if (r_stringncmp("&=", input,  size))
		return RVM_EAND;
	else if (r_stringncmp("|", input,  size))
		return RVM_EORR;
	else if (r_stringncmp("|=", input,  size))
		return RVM_EORR;
	else if (r_stringncmp("^", input,  size))
		return RVM_EXOR;
	else if (r_stringncmp("^=", input,  size))
		return RVM_EXOR;
	else if (r_stringncmp(">>", input,  size))
		return RVM_ELSR;
	else if (r_stringncmp(">>=", input,  size))
		return RVM_ELSR;
	else if (r_stringncmp("<<", input,  size))
		return RVM_ELSL;
	else if (r_stringncmp("<<=", input,  size))
		return RVM_ELSL;
	else if (r_stringncmp(">>>", input,  size))
		return RVM_ELSRU;
	else if (r_stringncmp(">>>=", input,  size))
		return RVM_ELSRU;
	else if (r_stringncmp("<=", input,  size))
		return RVM_ELESSEQ;
	else if (r_stringncmp(">=", input,  size))
		return RVM_EGREATEQ;
	else if (r_stringncmp("<", input,  size))
		return RVM_ELESS;
	else if (r_stringncmp(">", input,  size))
		return RVM_EGREAT;
	else if (r_stringncmp("===", input,  size))
		return RVM_EEQ;
	else if (r_stringncmp("==", input,  size))
		return RVM_EEQ;
	else if (r_stringncmp("!==", input,  size))
		return RVM_ENOTEQ;
	else if (r_stringncmp("!=", input,  size))
		return RVM_ENOTEQ;
	else if (r_stringncmp("!", input,  size))
		return RVM_ELNOT;
	else if (r_stringncmp("~", input,  size))
		return RVM_ENOT;
	else if (r_stringncmp("=", input,  size))
		return RVM_NOP;

	return -1;
}


rint rjs_compiler_rh_program(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rjs_coctx_global_t ctx;
	rparecord_t *prec;
	rlong mainidx = rvm_codegen_invalid_add_numlabel_s(co->cg, "__main", 0);
	rlong start;

	r_memset(&ctx, 0, sizeof(ctx));
	ctx.base.type = RJS_COCTX_GLOBAL;
	r_array_push(co->coctx, &ctx, rjs_coctx_t*);

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, mainidx, rvm_asm(RVM_B, DA, XX, XX, 0));
	start = rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		goto error;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_redefinelabel(co->cg, mainidx);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BL, DA, XX, XX, start - rvm_codegen_getcodesize(co->cg)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PRN, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));
	rjs_compiler_debugtail(co, records, rec);
	r_array_removelast(co->coctx);
	return 0;

error:
	r_array_removelast(co->coctx);
	return -1;
}


rint rjs_compiler_rh_varalloc(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	if (rjs_compiler_record2identifer(co, records, rec) < 0)
		return -1;
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rint rjs_compiler_rh_varallocinit(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	if (rjs_compiler_record2identifer(co, records, rec) < 0)
		return -1;
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rint rjs_compiler_rh_initializer(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_STRR, R0, R1, XX, 0));
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rint rjs_compiler_rh_identifier(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rvm_varmap_t *v;
	rlong parrec;
	rparecord_t *prec, *pparrec;
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	v = rvm_scope_lookup(co->scope, prec->input, prec->inputsiz);
	if (!v) {
		r_printf("ERROR: Undefined identifier: %s\n", rjs_compiler_record2str(co, records, rec));
		return -1;
	}

	if ((parrec = rpa_recordtree_parent(records, rec, RPA_RECORD_END)) < 0) {

		return -1;
	}
	pparrec = (rparecord_t *)r_array_slot(records, parrec);
	if (rpa_recordtree_next(records, rec, RPA_RECORD_START) < 0 && pparrec->ruleuid == UID_LEFTHANDSIDEEXPRESSIONADDR) {
		/*
		 * If this is the last child of UID_LEFTHANDSIDEEXPRESSIONADDR
		 */
		if (v->datatype == VARMAP_DATATYPE_OFFSET) {
			rvm_codegen_addins(co->cg, rvm_asm(RVM_ADDRS, R0, FP, DA, v->data.offset));
		} else {
			rvm_codegen_addins(co->cg, rvm_asmp(RVM_MOV, R0, DA, XX, v->data.ptr));
		}
	} else {
		if (v->datatype == VARMAP_DATATYPE_OFFSET) {
			rvm_codegen_addins(co->cg, rvm_asm(RVM_LDS, R0, FP, DA, v->data.offset));
		} else {
			rvm_codegen_addins(co->cg, rvm_asmp(RVM_LDRR, R0, DA, XX, v->data.ptr));
		}
	}

	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rint rjs_compiler_rh_expression(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rint rjs_compiler_rh_lefthandsideexpression(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rint rjs_compiler_rh_lefthandsideexpressionaddr(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asmp(RVM_PUSH, R0, XX, XX, 0));
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rint rjs_compiler_rh_decimalintegerliteral(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, R0, DA, XX, r_strtol(prec->input, NULL, 10)));
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rint rjs_compiler_rh_decimalnonintegerliteral(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asmd(RVM_MOV, R0, DA, XX, r_strtod(prec->input, NULL)));

	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rint rjs_compiler_rh_stringcharacters(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asmp(RVM_MOV, R1, DA, XX, (void*)prec->input));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R2, DA, XX, prec->inputsiz));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ALLOCSTR, R0, R1, R2, 0));
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rint rjs_compiler_rh_expressionop(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rlong opcode = 0;
	rlong opcoderec = -1;

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));
	rjs_compiler_debugtail(co, records, rec);

	if ((opcoderec = rpa_recordtree_firstchild(records, rec, RPA_RECORD_END)) < 0)
		return -1;
	opcode = rjs_compiler_record2opcode((rparecord_t *)r_array_slot(records, opcoderec));

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(opcode, R0, R1, R0, 0));
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rint rjs_compiler_rh_assignmentexpressionop(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rlong opcode = 0;
	rlong opcoderec = -1;

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	if ((opcoderec = rpa_recordtree_next(records, rpa_recordtree_firstchild(records, rec, RPA_RECORD_END), RPA_RECORD_END)) < 0)
		return -1;
	opcode = rjs_compiler_record2opcode((rparecord_t *)r_array_slot(records, opcoderec));

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	if (opcode != RVM_NOP) {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_LDRR, R2, R1, XX, 0));
		rvm_codegen_addins(co->cg, rvm_asm(opcode, R0, R2, R0, 0));
	}
	rvm_codegen_addins(co->cg, rvm_asm(RVM_STRR, R0, R1, XX, 0));
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rint rjs_compiler_rh_newarrayexpression(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ALLOCOBJ, R0, DA, XX, 0));
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rint rjs_compiler_rh_memberexpressiondotop(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	if (rpa_record_getruleuid(records, rpa_recordtree_parent(records, rec, RPA_RECORD_START)) == UID_LEFTHANDSIDEEXPRESSIONADDR &&
		rpa_recordtree_next(records, rec, RPA_RECORD_START) == -1) {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R1, R0, XX, 0)); 	// Supposedly an Array
		rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, prec->inputsiz));
		rvm_codegen_addins(co->cg, rvm_asmp(RVM_MOV, R2, DA, XX, (void*)prec->input));
		rvm_codegen_addins(co->cg, rvm_asm(RVM_OBJLKUPADD, R0, R1, R2, 0));	// Get the offset of the element at offset R0
		rvm_codegen_addins(co->cg, rvm_asm(RVM_ADDROBJH, R0, R1, R0, 0));	// Get the address of the element at offset R0

	} else {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R1, R0, XX, 0)); 	// Supposedly an Array
		rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, prec->inputsiz));
		rvm_codegen_addins(co->cg, rvm_asmp(RVM_MOV, R2, DA, XX, (void*)prec->input));
		rvm_codegen_addins(co->cg, rvm_asm(RVM_OBJLKUP, R0, R1, R2, 0));	// Get the offset of the element at offset R0
		rvm_codegen_addins(co->cg, rvm_asm(RVM_LDOBJH, R0, R1, R0, 0));	// Get the value of the element at offset R0
	}
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rint rjs_compiler_rh_memberexpressionindexop(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0)); 	// Supposedly an Array
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	if (rpa_record_getruleuid(records, rpa_recordtree_parent(records, rec, RPA_RECORD_START)) == UID_LEFTHANDSIDEEXPRESSIONADDR &&
		rpa_recordtree_next(records, rec, RPA_RECORD_START) == -1) {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0)); 	// Supposedly an Array
		rvm_codegen_addins(co->cg, rvm_asm(RVM_ADDROBJN, R0, R1, R0, 0));	// Get the address of the element at offset R0

	} else {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0)); 	// Supposedly an Array
		rvm_codegen_addins(co->cg, rvm_asm(RVM_LDOBJN, R0, R1, R0, 0));	// Get the value of the element at offset R0
	}
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rint rjs_compiler_rh_functiondeclaration(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rjs_coctx_function_t ctx;
	rparecord_t *prec;
	rlong start, execidx, endidx, allocsidx;

	r_memset(&ctx, 0, sizeof(ctx));
	ctx.base.type = RJS_COCTX_FUNCTION;

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	start = rvm_codegen_getcodesize(co->cg);
	endidx = rvm_codegen_invalid_add_numlabel_s(co->cg, "__funend", start);
	execidx = rvm_codegen_invalid_add_numlabel_s(co->cg, "__funexec", start);
	allocsidx = rvm_codegen_invalid_add_numlabel_s(co->cg, "__allocs", start);

	if (rpa_record_getruleuid(records, rpa_recordtree_firstchild(records, rec, RPA_RECORD_START)) != UID_FUNCTIONNAME)
		goto error;
	if (rjs_compiler_record2identifer(co, records, rpa_recordtree_firstchild(records, rec, RPA_RECORD_START)) < 0)
		goto error;

	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R1, R0, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_JUMP, execidx, rvm_asm(RVM_MOV, R0, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SETTYPE, R0, DA, XX, RVM_DTYPE_FUNCTION));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_STRR, R0, R1, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, endidx, rvm_asm(RVM_B, DA, XX, XX, 0));
	rvm_codegen_redefinelabel(co->cg, execidx);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_DEFAULT, allocsidx, rvm_asm(RVM_ADD, SP, FP, DA, 0));

	rjs_compiler_debugtail(co, records, rec);

	r_array_push(co->coctx, &ctx, rjs_coctx_t*);
	rvm_scope_push(co->scope);
	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		goto error;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_redefinelabel(co->cg, endidx);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0xffffffff));
	rvm_codegen_redefinepointer(co->cg, allocsidx, (rpointer)ctx.allocs);
	rjs_compiler_debugtail(co, records, rec);

	rvm_scope_pop(co->scope);
	r_array_removelast(co->coctx);
	return 0;

error:
	rvm_scope_pop(co->scope);
	r_array_removelast(co->coctx);
	return -1;
}


rint rjs_compiler_rh_functionparameter(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rjs_coctx_t *ctx = NULL;
	rjs_coctx_function_t *functx = NULL;
	rparecord_t *prec;
	rvm_varmap_t *v;

	/*
	 * First lets find out if we are within a function definition or
	 * this is a global variable.
	 */
	ctx = rjs_compiler_getctx(co, RJS_COCTX_FUNCTION);
	R_ASSERT(ctx);
	functx = (rjs_coctx_function_t *)ctx;

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	v = rvm_scope_tiplookup(co->scope, prec->input, prec->inputsiz);

	/*
	 * TBD: Temporary here
	 */
	if (v) {
		r_printf("ERROR: Identifier already defined: %s\n", rjs_compiler_record2str(co, records, rec));
		return -1;
	}

	functx->allocs += 1;
	rvm_scope_addoffset(co->scope, prec->input, prec->inputsiz, functx->allocs);

	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rint rjs_compiler_rh_functioncall(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rjs_coctx_functioncall_t ctx;

	r_memset(&ctx, 0, sizeof(ctx));
	ctx.base.type = RJS_COCTX_FUNCTIONCALL;
	r_array_push(co->coctx, &ctx, rjs_coctx_t*);

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(TP)|BIT(FP)|BIT(SP)|BIT(LR)));
	rjs_compiler_debugtail(co, records, rec);

	/*
	 * Important: Function call has two children FunctionCallName, Arguments
	 * We evaluate them in reverse order, first we evaluate the Arguments and push them on the
	 * stack, than we evaluate the FunctionCallName -> R0. When we make the call we assume the
	 * result of the FunctionCallName will be in R0.
	 */
	if (rjs_compiler_playreversechildrecords(co, records, rec) < 0)
		goto error;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUB, FP, SP, DA, ctx.arguments));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BXL, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, SP, FP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BITS(TP,LR)));
	rjs_compiler_debugtail(co, records, rec);
	r_array_removelast(co->coctx);
	return 0;

error:
	r_array_removelast(co->coctx);
	return -1;
}


rint rjs_compiler_rh_argument(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rjs_coctx_functioncall_t *ctx = (rjs_coctx_functioncall_t *)rjs_compiler_getctx(co, RJS_COCTX_FUNCTIONCALL);

	R_ASSERT(ctx);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));
	ctx->arguments += 1;
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rint rjs_compiler_rh_arguments(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rjs_coctx_functioncall_t *ctx = (rjs_coctx_functioncall_t *)rjs_compiler_getctx(co, RJS_COCTX_FUNCTIONCALL);

	R_ASSERT(ctx);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rint rjs_compiler_rh_returnstatement(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rint rjs_compiler_rh_ifstatement(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rjs_coctx_ifstatement_t ctx;

	r_memset(&ctx, 0, sizeof(ctx));
	ctx.base.type = RJS_COCTX_IFSTATEMENT;
	ctx.start = rvm_codegen_getcodesize(co->cg);
	ctx.trueidx = rvm_codegen_invalid_add_numlabel_s(co->cg, "__iftrue", ctx.start);
	ctx.falseidx = rvm_codegen_invalid_add_numlabel_s(co->cg, "__iffalse", ctx.start);
	ctx.endidx = rvm_codegen_invalid_add_numlabel_s(co->cg, "__ifend", ctx.start);
	r_array_push(co->coctx, &ctx, rjs_coctx_t*);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		goto error;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_redefinelabel(co->cg, ctx.endidx);
	if (!rvm_codegen_validlabel(co->cg, ctx.falseidx))
		rvm_codegen_redefinelabel(co->cg, ctx.falseidx);
	rjs_compiler_debugtail(co, records, rec);

	r_array_removelast(co->coctx);
	return 0;

error:
	r_array_removelast(co->coctx);
	return -1;
}


rint rjs_compiler_rh_ifconditionop(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rjs_coctx_ifstatement_t *ctx = (rjs_coctx_ifstatement_t *)rjs_compiler_getctx(co, RJS_COCTX_IFSTATEMENT);

	R_ASSERT(ctx);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);
	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, ctx->falseidx, rvm_asm(RVM_BEQ, DA, XX, XX, 0));
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rint rjs_compiler_rh_iftruestatement(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rjs_coctx_ifstatement_t *ctx = (rjs_coctx_ifstatement_t *)rjs_compiler_getctx(co, RJS_COCTX_IFSTATEMENT);

	R_ASSERT(ctx);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, ctx->endidx, rvm_asm(RVM_B, DA, XX, XX, 0));
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rint rjs_compiler_rh_iffalsestatement(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rjs_coctx_ifstatement_t *ctx = (rjs_coctx_ifstatement_t *)rjs_compiler_getctx(co, RJS_COCTX_IFSTATEMENT);

	R_ASSERT(ctx);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_redefinelabel(co->cg, ctx->falseidx);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rint rjs_compiler_rh_iterationfor(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rlong childrec;
	rparecord_t *prec;
	rjs_coctx_iteration_t ctx;

	r_memset(&ctx, 0, sizeof(ctx));
	ctx.base.type = RJS_COCTX_ITERATION;
	ctx.start = rvm_codegen_getcodesize(co->cg);
	ctx.iterationidx = rvm_codegen_invalid_add_numlabel_s(co->cg, "__iteration", ctx.start);
	ctx.endidx = rvm_codegen_invalid_add_numlabel_s(co->cg, "__end", ctx.start);
	r_array_push(co->coctx, &ctx, rjs_coctx_t*);
	rvm_scope_push(co->scope);

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	for (childrec = rpa_recordtree_firstchild(records, rec, RPA_RECORD_START); childrec >= 0; childrec = rpa_recordtree_next(records, childrec, RPA_RECORD_START)) {
		if (rpa_record_getruleuid(records, childrec) == UID_FOREXPRESSIONINIT) {
			if (rjs_compiler_playrecord(co, records, childrec) < 0)
				goto error;
			break;
		}
	}

	rvm_codegen_redefinelabel(co->cg, ctx.iterationidx);
	for (childrec = rpa_recordtree_firstchild(records, rec, RPA_RECORD_START); childrec >= 0; childrec = rpa_recordtree_next(records, childrec, RPA_RECORD_START)) {
		if (rpa_record_getruleuid(records, childrec) == UID_FOREXPRESSIONCOMPARE) {
			if (rjs_compiler_playrecord(co, records, childrec) < 0)
				goto error;
			break;
		}
	}

	for (childrec = rpa_recordtree_firstchild(records, rec, RPA_RECORD_START); childrec >= 0; childrec = rpa_recordtree_next(records, childrec, RPA_RECORD_START)) {
		if (rpa_record_getruleuid(records, childrec) == UID_FORITERATIONSTATEMENT) {
			if (rjs_compiler_playrecord(co, records, childrec) < 0)
				goto error;
			break;
		}
	}

	for (childrec = rpa_recordtree_firstchild(records, rec, RPA_RECORD_START); childrec >= 0; childrec = rpa_recordtree_next(records, childrec, RPA_RECORD_START)) {
		if (rpa_record_getruleuid(records, childrec) == UID_FOREXPRESSIONINCREMENT) {
			if (rjs_compiler_playrecord(co, records, childrec) < 0)
				goto error;
			break;
		}
	}

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_redefinelabel(co->cg, ctx.endidx);
	rjs_compiler_debugtail(co, records, rec);

	rvm_scope_pop(co->scope);
	r_array_removelast(co->coctx);
	return 0;

error:
	rvm_scope_pop(co->scope);
	r_array_removelast(co->coctx);
	return -1;
}


rint rjs_compiler_rh_forexpressioncompare(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rjs_coctx_iteration_t *ctx = (rjs_coctx_iteration_t *)rjs_compiler_getctx(co, RJS_COCTX_ITERATION);

	R_ASSERT(ctx);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, ctx->endidx, rvm_asm(RVM_BEQ, DA, XX, XX, 0));
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rint rjs_compiler_rh_forexpressionincrement(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rjs_coctx_iteration_t *ctx = (rjs_coctx_iteration_t *)rjs_compiler_getctx(co, RJS_COCTX_ITERATION);

	R_ASSERT(ctx);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, ctx->iterationidx, rvm_asm(RVM_B, DA, XX, XX, 0));
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rint rjs_compiler_rh_postfixexpressionop(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rlong opcode;
	rlong opcoderec;

	if ((opcoderec = rpa_recordtree_lastchild(records, rec, RPA_RECORD_END)) < 0)
		return -1;
	opcode = rjs_compiler_record2opcode((rparecord_t *)r_array_slot(records, opcoderec));
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_LDRR, R0, R1, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(opcode, R2, R0, DA, 1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_STRR, R2, R1, XX, 0));

	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rint rjs_compiler_rh_prefixexpressionop(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rlong opcode;
	rlong opcoderec;

	if ((opcoderec = rpa_recordtree_firstchild(records, rec, RPA_RECORD_END)) < 0)
		return -1;
	opcode = rjs_compiler_record2opcode((rparecord_t *)r_array_slot(records, opcoderec));

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_LDRR, R0, R0, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(opcode, R0, R0, DA, 1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_STRR, R0, R1, XX, 0));

	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rint rjs_compiler_rh_(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rjs_compiler_t *rjs_compiler_create(rvmcpu_t *cpu)
{
	rjs_compiler_t *co = (rjs_compiler_t *) r_zmalloc(sizeof(*co));

	co->cg = rvm_codegen_create();
	co->scope = rvm_scope_create();
	co->coctx = r_array_create(sizeof(rjs_coctx_t *));
	co->cpu = cpu;
	r_memset(co->handlers, 0, sizeof(co->handlers));

	co->handlers[UID_PROGRAM] = rjs_compiler_rh_program;
	co->handlers[UID_EXPRESSION] = rjs_compiler_rh_expression;
	co->handlers[UID_LEFTHANDSIDEEXPRESSION] = rjs_compiler_rh_lefthandsideexpression;
	co->handlers[UID_LEFTHANDSIDEEXPRESSIONADDR] = rjs_compiler_rh_lefthandsideexpressionaddr;
	co->handlers[UID_DECIMALINTEGERLITERAL] = rjs_compiler_rh_decimalintegerliteral;
	co->handlers[UID_DECIMALNONINTEGERLITERAL] = rjs_compiler_rh_decimalnonintegerliteral;
	co->handlers[UID_STRINGCHARACTERS] = rjs_compiler_rh_stringcharacters;
	co->handlers[UID_ADDITIVEEXPRESSIONOP] = rjs_compiler_rh_expressionop;
	co->handlers[UID_MULTIPLICATIVEEXPRESSIONOP] = rjs_compiler_rh_expressionop;
	co->handlers[UID_BITWISEANDOP] = rjs_compiler_rh_expressionop;
	co->handlers[UID_BITWISEXOROP] = rjs_compiler_rh_expressionop;
	co->handlers[UID_BITWISEOROP] = rjs_compiler_rh_expressionop;
	co->handlers[UID_SHIFTEXPRESSIONOP] = rjs_compiler_rh_expressionop;
	co->handlers[UID_EQUALITYEXPRESSIONOP] = rjs_compiler_rh_expressionop;
	co->handlers[UID_RELATIONALEXPRESSIONOP] = rjs_compiler_rh_expressionop;
	co->handlers[UID_LOGICALOROP] = rjs_compiler_rh_expressionop;
	co->handlers[UID_LOGICALANDOP] = rjs_compiler_rh_expressionop;
	co->handlers[UID_VARIABLEALLOCATEANDINIT] = rjs_compiler_rh_varallocinit;
	co->handlers[UID_VARIABLEALLOCATE] = rjs_compiler_rh_varalloc;
	co->handlers[UID_IDENTIFIER] = rjs_compiler_rh_identifier;
	co->handlers[UID_INITIALISER] = rjs_compiler_rh_initializer;
	co->handlers[UID_ASSIGNMENTEXPRESSIONOP] = rjs_compiler_rh_assignmentexpressionop;
	co->handlers[UID_NEWARRAYEXPRESSION] = rjs_compiler_rh_newarrayexpression;
	co->handlers[UID_MEMBEREXPRESSIONDOTOP] = rjs_compiler_rh_memberexpressiondotop;
	co->handlers[UID_MEMBEREXPRESSIONINDEXOP] = rjs_compiler_rh_memberexpressionindexop;
	co->handlers[UID_FUNCTIONDECLARATION] = rjs_compiler_rh_functiondeclaration;
	co->handlers[UID_FUNCTIONPARAMETER] = rjs_compiler_rh_functionparameter;
	co->handlers[UID_FUNCTIONCALL] = rjs_compiler_rh_functioncall;
	co->handlers[UID_ARGUMENT] = rjs_compiler_rh_argument;
	co->handlers[UID_ARGUMENTS] = rjs_compiler_rh_arguments;
	co->handlers[UID_RETURNSTATEMENT] = rjs_compiler_rh_returnstatement;
	co->handlers[UID_STRINGCHARACTERS] = rjs_compiler_rh_stringcharacters;
	co->handlers[UID_IFSTATEMENT] = rjs_compiler_rh_ifstatement;
	co->handlers[UID_IFCONDITIONOP] = rjs_compiler_rh_ifconditionop;
	co->handlers[UID_IFTRUESTATEMENT] = rjs_compiler_rh_iftruestatement;
	co->handlers[UID_IFFALSESTATEMENT] = rjs_compiler_rh_iffalsestatement;
	co->handlers[UID_ITERATIONFOR] = rjs_compiler_rh_iterationfor;
	co->handlers[UID_FOREXPRESSIONCOMPARE] = rjs_compiler_rh_forexpressioncompare;
	co->handlers[UID_FOREXPRESSIONINCREMENT] = rjs_compiler_rh_forexpressionincrement;
	co->handlers[UID_POSTFIXEXPRESSIONOP] = rjs_compiler_rh_postfixexpressionop;
	co->handlers[UID_PREFIXEXPRESSIONOP] = rjs_compiler_rh_prefixexpressionop;

	return co;
}


void rjs_compiler_destroy(rjs_compiler_t *co)
{
	if (co) {
		rvm_codegen_destroy(co->cg);
		rvm_scope_destroy(co->scope);
		r_array_destroy(co->coctx);
		r_free(co->temp);
		co->cpu = NULL;
		r_free(co);
	}
}


static rint rjs_compiler_rh_default(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	return 0;
}


static rint rjs_compiler_playchildrecords(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rlong child;

	for (child = rpa_recordtree_firstchild(records, rec, RPA_RECORD_START); child >= 0; child = rpa_recordtree_next(records, child, RPA_RECORD_START)) {
		if (rjs_compiler_playrecord(co, records, child) < 0)
			return -1;
	}

	return 0;
}


rint rjs_compiler_playreversechildrecords(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rlong child;

	for (child = rpa_recordtree_lastchild(records, rec, RPA_RECORD_START); child >= 0; child = rpa_recordtree_prev(records, child, RPA_RECORD_START)) {
		if (rjs_compiler_playrecord(co, records, child) < 0)
			return -1;
	}

	return 0;
}


static rint rjs_compiler_playrecord(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	prec = (rparecord_t *)r_array_slot(records, rec);
	if (prec->ruleuid >= 0 && prec->ruleuid < RJS_COMPILER_NHANDLERS && co->handlers[prec->ruleuid]) {
		return co->handlers[prec->ruleuid](co, records, rec);
	}
	return rjs_compiler_rh_default(co, records, rec);
}


rint rjs_compiler_compile(rjs_compiler_t *co, rarray_t *records)
{
	rlong i;
	rvm_codelabel_t *labelerr;

	if (!co || !records || r_array_empty(records)) {
		/*
		 * TBD
		 */
		return -1;
	}

	for (i = 0; i >= 0; i = rpa_recordtree_next(records, i, RPA_RECORD_START)) {
		if (rjs_compiler_playrecord(co, records, i) < 0)
			/*
			 * TBD
			 */
			return -1;
	}

	if (rvm_codegen_relocate(co->cg, &labelerr) < 0) {
		/*
		 * TBD
		 */
		return -1;
	}

	return 0;
}
