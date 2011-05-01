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
		goto end;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PRN, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));
	rjs_compiler_debugtail(co, records, rec);
	r_array_removelast(co->coctx);
	return 0;

end:
	r_array_removelast(co->coctx);
	return -1;
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
	co->coctx = r_array_create(sizeof(rjs_coctx_t *));
	co->cpu = cpu;
	r_memset(co->handlers, 0, sizeof(co->handlers));

	co->handlers[UID_PROGRAM] = rjs_compiler_rh_program;
	co->handlers[UID_EXPRESSION] = rjs_compiler_rh_expression;
	co->handlers[UID_LEFTHANDSIDEEXPRESSION] = rjs_compiler_rh_lefthandsideexpression;
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
	co->handlers[UID_BITWISEANDOP] = rjs_compiler_rh_expressionop;
	co->handlers[UID_BITWISEANDOP] = rjs_compiler_rh_expressionop;

	return co;
}


void rjs_compiler_destroy(rjs_compiler_t *co)
{
	if (co) {
		rvm_codegen_destroy(co->cg);
		r_array_destroy(co->coctx);
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
