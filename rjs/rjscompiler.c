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

	r_memset(&ctx, 0, sizeof(ctx));
	ctx.base.type = RJS_COCTX_GLOBAL;
	r_array_push(co->coctx, &ctx, rjs_coctx_t*);

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		goto error;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
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
	rjs_coctx_t *ctx = NULL;
	rvm_varmap_t *v = NULL;
	rlong i;

	R_ASSERT(r_array_length(co->coctx));

	/*
	 * First lets find out if we are within a function definition or
	 * this is a global variable.
	 */
	for (i = r_array_length(co->coctx) - 1; i >= 0; i--) {
		ctx = r_array_index(co->coctx, i, rjs_coctx_t*);
		if (ctx->type == RJS_COCTX_FUNCTION || ctx->type == RJS_COCTX_GLOBAL)
			break;
	}
	R_ASSERT(ctx);

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		goto error;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	v = rvm_scope_tiplookup(co->scope, prec->input, prec->inputsiz);

	/*
	 * TBD: Temporary here
	 */
	if (v) {
		r_printf("ERROR: variable already defined: %s\n", rjs_compiler_record2str(co, records, rec));
		goto error;
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
		rvm_codegen_addins(co->cg, rvm_asm(RVM_ADDRS, R1, FP, DA, v->data.offset));
	} else {
		rvm_codegen_addins(co->cg, rvm_asmp(RVM_MOV, R1, DA, XX, v->data.ptr));
	}
	rvm_codegen_addins(co->cg, rvm_asm(RVM_CLRR, R1, XX, XX, 0));

	rjs_compiler_debugtail(co, records, rec);
	return 0;

error:
	return -1;
}


rint rjs_compiler_rh_varallocinit(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rjs_coctx_t *ctx = NULL;
	rvm_varmap_t *v = NULL;
	rlong i;

	R_ASSERT(r_array_length(co->coctx));

	/*
	 * First lets find out if we are within a function definition or
	 * this is a global variable.
	 */
	for (i = r_array_length(co->coctx) - 1; i >= 0; i--) {
		ctx = r_array_index(co->coctx, i, rjs_coctx_t*);
		if (ctx->type == RJS_COCTX_FUNCTION || ctx->type == RJS_COCTX_GLOBAL)
			break;
	}
	R_ASSERT(ctx);

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		goto error;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	v = rvm_scope_tiplookup(co->scope, prec->input, prec->inputsiz);

	/*
	 * TBD: Temporary here
	 */
	if (v) {
		r_printf("ERROR: variable already defined: %s\n", rjs_compiler_record2str(co, records, rec));
		goto error;
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
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));
	rjs_compiler_debugtail(co, records, rec);
	return 0;

error:
	return -1;
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
			rvm_codegen_addins(co->cg, rvm_asm(RVM_ADDRS, R1, FP, DA, v->data.offset));
		} else {
			rvm_codegen_addins(co->cg, rvm_asmp(RVM_MOV, R1, DA, XX, v->data.ptr));
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
	rvm_codegen_addins(co->cg, rvm_asmp(RVM_PUSH, R1, XX, XX, 0));
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

	if (!co || !records || r_array_empty(records)) {

		return -1;
	}

	for (i = 0; i >= 0; i = rpa_recordtree_next(records, i, RPA_RECORD_START)) {
		if (rjs_compiler_playrecord(co, records, i) < 0)
			return -1;
	}
	return 0;
}
