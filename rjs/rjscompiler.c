#include "rlib/rmem.h"
#include "rjs/rjscompiler.h"
#include "rjs/rjsparser.h"


rinteger rjs_compiler_playreversechildrecords(rjs_compiler_t *co, rarray_t *records, rlong rec);
static rinteger rjs_compiler_playrecord(rjs_compiler_t *co, rarray_t *records, rlong rec);
static rinteger rjs_compiler_playchildrecords(rjs_compiler_t *co, rarray_t *records, rlong rec);


void rjs_compiler_debughead(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rvm_codegen_setsource(co->cg, rec);
	if (co->debug) {
		rparecord_t *prec = (rparecord_t *) r_array_slot(records, rec);
		co->headoff = rvm_codegen_getcodesize(co->cg);
		if (prec->type & RPA_RECORD_START) {
			rpa_record_dump(records, rec);
		}

	}
}


void rjs_compiler_debugtail(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rvm_codegen_setsource(co->cg, rec);
	if (co->debug) {
		rparecord_t *prec = (rparecord_t *) r_array_slot(records, rec);
		rvm_asm_dump(rvm_codegen_getcode(co->cg, co->headoff), rvm_codegen_getcodesize(co->cg) - co->headoff);
		if (prec->type & RPA_RECORD_END) {
			rpa_record_dump(records, rec);
		}
	}

}


void rjs_compiler_adderror(rjs_compiler_t *co, rlong code, rparecord_t *prec)
{
	co->error->type = RJS_ERRORTYPE_SYNTAX;
	co->error->error = code;
	co->error->script = co->script;
	co->error->offset = prec->input - co->script;
	co->error->size = prec->inputsiz;
	co->error->line = rjs_parser_offset2line(co->script, co->error->offset);
	co->error->lineoffset = rjs_parser_offset2lineoffset(co->script, co->error->offset);

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


static rlong rjs_compiler_record_parentuid(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rlong parent = rpa_recordtree_parent(records, rec, RPA_RECORD_START);

	if (parent < 0)
		return -1;
	return rpa_record_getruleuid(records, parent);
}


static rlong rjs_compiler_record_lastofkind(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rlong uid = rpa_record_getruleuid(records, rec);
	rlong i;

	for (i = rpa_recordtree_next(records, rec, RPA_RECORD_START) ; i >= 0; i = rpa_recordtree_next(records, i, RPA_RECORD_START)) {
		if (rpa_record_getruleuid(records, i) == uid)
			return 0;
	}
	return 1;
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

	if ( ctx->type == RJS_COCTX_FUNCTION ) {
		rjs_coctx_function_t *functx = (rjs_coctx_function_t *)ctx;
		functx->stackallocs += 1;
		rvm_scope_addoffset(co->scope, prec->input, prec->inputsiz, functx->stackallocs);
	} else {
		if (rvm_scope_count(co->scope)) {
			rjs_coctx_global_t *globalctx = (rjs_coctx_global_t *)ctx;
			globalctx->stackallocs += 1;
			rvm_scope_addoffset(co->scope, prec->input, prec->inputsiz, globalctx->stackallocs);
		} else {
			/*
			 * Global Data
			 */
			rvm_scope_addpointer(co->scope, prec->input, prec->inputsiz, rvm_cpu_alloc_global(co->cpu));
		}
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


rjs_coctx_t *rjs_compiler_gettopctx(rjs_compiler_t *co)
{
	rlong len = r_array_length(co->coctx);

	if (len)
		return r_array_index(co->coctx, len - 1, rjs_coctx_t*);
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


rlong rjs_compiler_record2unaryopcode(rparecord_t *prec)
{
	const rchar *input = prec->input;
	rsize_t size = prec->inputsiz;

	if (r_stringncmp("+", input,  size))
		return RVM_NOP;
	else if (r_stringncmp("-", input,  size))
		return RVM_ENEG;
	else if (r_stringncmp("~", input,  size))
		return RVM_ENOT;
	else if (r_stringncmp("!", input,  size))
		return RVM_ELNOT;

	return -1;
}


static rinteger rjs_compiler_internalvars_setup(rjs_compiler_t *co)
{
	rvm_varmap_t *v;

	v = rvm_scope_tiplookup_s(co->scope, "ARGS");
	if (!v) {
		rvm_scope_addpointer_s(co->scope, "ARGS", rvm_cpu_alloc_global(co->cpu));
		v = rvm_scope_tiplookup_s(co->scope, "ARGS");
		R_ASSERT(v);
		return -1;
	}
	rvm_reg_setundef((rvmreg_t*)v->data.ptr);

	return 0;
}


rinteger rjs_compiler_rh_program(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rjs_coctx_global_t ctx;
	rparecord_t *prec;
	rlong mainidx = rvm_codegen_invalid_addlabel_s(co->cg, NULL);
	rlong allocsidx = rvm_codegen_invalid_addlabel_s(co->cg, NULL);
	rlong start;

	r_memset(&ctx, 0, sizeof(ctx));
	ctx.base.type = RJS_COCTX_GLOBAL;
	r_array_push(co->coctx, &ctx, rjs_coctx_t*);

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_internalvars_setup(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, mainidx, rvm_asm(RVM_B, DA, XX, XX, 0));
	start = rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_DEFAULT, allocsidx, rvm_asm(RVM_ADD, SP, FP, DA, 0));
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		goto error;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rvm_codegen_redefinepointer(co->cg, allocsidx, (rpointer)ctx.stackallocs);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_redefinelabel_default(co->cg, mainidx);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BL, DA, XX, XX, start - rvm_codegen_getcodesize(co->cg)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));
	rjs_compiler_debugtail(co, records, rec);
	r_array_removelast(co->coctx);
	return 0;

error:
	r_array_removelast(co->coctx);
	return -1;
}


rinteger rjs_compiler_rh_varalloc(rjs_compiler_t *co, rarray_t *records, rlong rec)
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


rinteger rjs_compiler_rh_varallocinit(rjs_compiler_t *co, rarray_t *records, rlong rec)
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


rinteger rjs_compiler_rh_initializer(rjs_compiler_t *co, rarray_t *records, rlong rec)
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


rinteger rjs_compiler_rh_identifier(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rvm_varmap_t *v;
	rparecord_t *prec;
	rlong swiid = -1;
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
		/*
		 * Let see if this is a swiid
		 */
		if ((swiid = rvm_cpu_swilookup(co->cpu, NULL, prec->input, prec->inputsiz)) >= 0) {
			rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, swiid));
			rvm_codegen_addins(co->cg, rvm_asm(RVM_SETTYPE, R0, DA, XX, RVM_DTYPE_SWIID));
			goto end;
		}

		rjs_compiler_adderror(co, RJS_ERROR_UNDEFINED, prec);
		return -1;
	}

	if (rjs_compiler_record_parentuid(co, records, rec) == UID_LEFTHANDSIDEEXPRESSIONADDR && rpa_recordtree_next(records, rec, RPA_RECORD_START) == -1) {
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
end:
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rinteger rjs_compiler_rh_expression(rjs_compiler_t *co, rarray_t *records, rlong rec)
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


rinteger rjs_compiler_rh_lefthandsideexpression(rjs_compiler_t *co, rarray_t *records, rlong rec)
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


rinteger rjs_compiler_rh_lefthandsideexpressionaddr(rjs_compiler_t *co, rarray_t *records, rlong rec)
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


rinteger rjs_compiler_rh_decimalintegerliteral(rjs_compiler_t *co, rarray_t *records, rlong rec)
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


rinteger rjs_compiler_rh_decimalnonintegerliteral(rjs_compiler_t *co, rarray_t *records, rlong rec)
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


rinteger rjs_compiler_rh_stringcharacters(rjs_compiler_t *co, rarray_t *records, rlong rec)
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
	co->stringcharacters.str = (rchar*)prec->input;
	co->stringcharacters.size = prec->inputsiz;
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rinteger rjs_compiler_rh_stringliteral(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	prec = (rparecord_t *)r_array_slot(records, rec);

	rjs_compiler_debughead(co, records, rec);
	co->stringcharacters.str = NULL;
	co->stringcharacters.size = 0;
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asmp(RVM_MOV, R1, DA, XX, co->stringcharacters.str));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R2, DA, XX, co->stringcharacters.size));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ALLOCSTR, R0, R1, R2, 0));
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rinteger rjs_compiler_rh_binaryexpressionop(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rjs_coctx_operation_t ctx;

	r_memset(&ctx, 0, sizeof(ctx));
	ctx.base.type = RJS_COCTX_OPERATION;
	r_array_push(co->coctx, &ctx, rjs_coctx_t*);

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));
	rjs_compiler_debugtail(co, records, rec);
	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		goto error;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(ctx.opcode, R0, R1, R0, 0));
	rjs_compiler_debugtail(co, records, rec);
	r_array_removelast(co->coctx);
	return 0;

error:
	r_array_removelast(co->coctx);
	return -1;
}


rinteger rjs_compiler_rh_unaryexpressionop(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rjs_coctx_operation_t ctx;

	r_memset(&ctx, 0, sizeof(ctx));
	ctx.base.type = RJS_COCTX_OPERATION;
	r_array_push(co->coctx, &ctx, rjs_coctx_t*);

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);
	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		goto error;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asm(ctx.opcode, R0, R0, XX, 0));
	rjs_compiler_debugtail(co, records, rec);
	r_array_removelast(co->coctx);
	return 0;

error:
	r_array_removelast(co->coctx);
	return -1;
}


rinteger rjs_compiler_rh_assignmentexpressionop(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rjs_coctx_operation_t ctx;

	r_memset(&ctx, 0, sizeof(ctx));
	ctx.base.type = RJS_COCTX_OPERATION;
	r_array_push(co->coctx, &ctx, rjs_coctx_t*);

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);
	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		goto error;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	if (ctx.opcode != RVM_NOP) {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_LDRR, R2, R1, XX, 0));
		rvm_codegen_addins(co->cg, rvm_asm(ctx.opcode, R0, R2, R0, 0));
	}
	rvm_codegen_addins(co->cg, rvm_asm(RVM_STRR, R0, R1, XX, 0));
	rjs_compiler_debugtail(co, records, rec);
	r_array_removelast(co->coctx);
	return 0;
error:
	r_array_removelast(co->coctx);
	return -1;
}


rinteger rjs_compiler_rh_newarrayexpression(rjs_compiler_t *co, rarray_t *records, rlong rec)
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
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MAPALLOC, R0, DA, XX, 0));
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rinteger rjs_compiler_rh_memberexpressiondotop(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rjs_coctx_t *ctx = rjs_compiler_gettopctx(co);
	rparecord_t *prec;

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R1, R0, XX, 0)); 	// Supposedly an Array
	rjs_compiler_debugtail(co, records, rec);
	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;
	if (ctx && ctx->type == RJS_COCTX_FUNCTIONCALL)
		((rjs_coctx_functioncall_t *)ctx)->setthis = 1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	if (rjs_compiler_record_parentuid(co, records, rec) == UID_LEFTHANDSIDEEXPRESSIONADDR && rjs_compiler_record_lastofkind(co, records, rec)) {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_MAPLKUPADD, R0, R1, R2, 0));	// Get the offset of the element at offset R0
		rvm_codegen_addins(co->cg, rvm_asm(RVM_MAPADDR, R0, R1, R0, 0));	// Get the address of the element at offset R0

	} else {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_MAPLKUP, R0, R1, R2, 0));	// Get the offset of the element at offset R0
		rvm_codegen_addins(co->cg, rvm_asm(RVM_MAPLDR, R0, R1, R0, 0));	// Get the value of the element at offset R0
	}
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rinteger rjs_compiler_rh_memberexpressionindexop(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rjs_coctx_t *ctx = rjs_compiler_gettopctx(co);
	rparecord_t *prec;
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0)); 	// Supposedly an Array
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;
	if (ctx && ctx->type == RJS_COCTX_FUNCTIONCALL)
		((rjs_coctx_functioncall_t *)ctx)->setthis = 1;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	if (rjs_compiler_record_parentuid(co, records, rec) == UID_LEFTHANDSIDEEXPRESSIONADDR && rjs_compiler_record_lastofkind(co, records, rec)) {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0)); 	// Supposedly an Array
		rvm_codegen_addins(co->cg, rvm_asm(RVM_MAPLKUPADD, R0, R1, R0, 0));	// R1 Array
		rvm_codegen_addins(co->cg, rvm_asm(RVM_MAPADDR, R0, R1, R0, 0));	// Get the address of the element at offset R0

	} else {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0)); 	// Supposedly an Array
		rvm_codegen_addins(co->cg, rvm_asm(RVM_MAPLKUP, R0, R1, R0, 0));	// R1 Array
		rvm_codegen_addins(co->cg, rvm_asm(RVM_MAPLDR, R0, R1, R0, 0));	// Get the value of the element at offset R0
	}
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rinteger rjs_compiler_rh_functiondeclaration(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rjs_coctx_function_t ctx;
	rparecord_t *prec;
	rlong start, execidx, endidx, allocsidx;

	r_memset(&ctx, 0, sizeof(ctx));
	ctx.base.type = RJS_COCTX_FUNCTION;

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	start = rvm_codegen_getcodesize(co->cg);
	endidx = rvm_codegen_invalid_addlabel_s(co->cg, NULL);
	execidx = rvm_codegen_invalid_addlabel_s(co->cg, NULL);
	allocsidx = rvm_codegen_invalid_addlabel_s(co->cg, NULL);
	if (rpa_record_getruleuid(records, rpa_recordtree_firstchild(records, rec, RPA_RECORD_START)) == UID_FUNCTIONNAME) {
		if (rjs_compiler_record2identifer(co, records, rpa_recordtree_firstchild(records, rec, RPA_RECORD_START)) < 0)
			goto error;
		rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R1, R0, XX, 0));
	}
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_JUMP, execidx, rvm_asm(RVM_MOV, R0, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SETTYPE, R0, DA, XX, RVM_DTYPE_FUNCTION));
	if (rpa_record_getruleuid(records, rpa_recordtree_firstchild(records, rec, RPA_RECORD_START)) == UID_FUNCTIONNAME) {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_STRR, R0, R1, XX, 0));
	}
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, endidx, rvm_asm(RVM_B, DA, XX, XX, 0));
	rvm_codegen_redefinelabel_default(co->cg, execidx);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_DEFAULT, allocsidx, rvm_asm(RVM_ADD, SP, FP, DA, 0));

	rjs_compiler_debugtail(co, records, rec);

	r_array_push(co->coctx, &ctx, rjs_coctx_t*);
	rvm_scope_push(co->scope);
	if (rjs_compiler_playchildrecords(co, records, rec) < 0) {
		rvm_scope_pop(co->scope);
		goto error;
	}
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_redefinelabel_default(co->cg, endidx);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0xffffffff));
	rvm_codegen_redefinepointer(co->cg, allocsidx, (rpointer)ctx.stackallocs);
	rjs_compiler_debugtail(co, records, rec);

	co->cg->userdata = RJS_COMPILER_CODEGENKEEP;
	rvm_scope_pop(co->scope);
	r_array_removelast(co->coctx);
	return 0;

error:
	r_array_removelast(co->coctx);
	return -1;
}


rinteger rjs_compiler_rh_functionparameter(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rjs_coctx_t *ctx = NULL;
	rjs_coctx_function_t *functx = NULL;
	rparecord_t *prec;
	rvm_varmap_t *v;

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	/*
	 * First lets find out if we are within a function definition or
	 * this is a global variable.
	 */
	ctx = rjs_compiler_getctx(co, RJS_COCTX_FUNCTION);
	if (!ctx) {
		rjs_compiler_adderror(co, RJS_ERROR_NOTAFUNCTION, prec);
		return -1;
	}
	R_ASSERT(ctx);
	functx = (rjs_coctx_function_t *)ctx;
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

	functx->stackallocs += 1;
	rvm_scope_addoffset(co->scope, prec->input, prec->inputsiz, functx->stackallocs);

	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rinteger rjs_compiler_rh_functioncall(rjs_compiler_t *co, rarray_t *records, rlong rec)
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
	if (ctx.setthis)
		rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, TP, R1, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUB, FP, SP, DA, ctx.arguments));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_CALL, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, SP, FP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BITS(TP,LR)));
	rjs_compiler_debugtail(co, records, rec);
	r_array_removelast(co->coctx);
	return 0;

error:
	r_array_removelast(co->coctx);
	return -1;
}


rinteger rjs_compiler_rh_argument(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rjs_coctx_functioncall_t *ctx = (rjs_coctx_functioncall_t *)rjs_compiler_getctx(co, RJS_COCTX_FUNCTIONCALL);

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	if (!ctx) {
		rjs_compiler_adderror(co, RJS_ERROR_NOTAFUNCTIONCALL, prec);
		return -1;
	}
	R_ASSERT(ctx);
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


rinteger rjs_compiler_rh_arguments(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rjs_coctx_functioncall_t *ctx = (rjs_coctx_functioncall_t *)rjs_compiler_getctx(co, RJS_COCTX_FUNCTIONCALL);

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	if (!ctx) {
		rjs_compiler_adderror(co, RJS_ERROR_NOTAFUNCTIONCALL, prec);
		return -1;
	}
	R_ASSERT(ctx);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rinteger rjs_compiler_rh_returnstatement(rjs_compiler_t *co, rarray_t *records, rlong rec)
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


rinteger rjs_compiler_rh_ifstatement(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rjs_coctx_ifstatement_t ctx;

	r_memset(&ctx, 0, sizeof(ctx));
	ctx.base.type = RJS_COCTX_IFSTATEMENT;
	ctx.start = rvm_codegen_getcodesize(co->cg);
	ctx.trueidx = rvm_codegen_invalid_addlabel_s(co->cg, NULL);
	ctx.falseidx = rvm_codegen_invalid_addlabel_s(co->cg, NULL);
	ctx.endidx = rvm_codegen_invalid_addlabel_s(co->cg, NULL);
	r_array_push(co->coctx, &ctx, rjs_coctx_t*);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);
	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		goto error;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_redefinelabel_default(co->cg, ctx.endidx);
	if (!rvm_codegen_validlabel(co->cg, ctx.falseidx))
		rvm_codegen_redefinelabel_default(co->cg, ctx.falseidx);
	rjs_compiler_debugtail(co, records, rec);
	r_array_removelast(co->coctx);
	return 0;

error:
	r_array_removelast(co->coctx);
	return -1;
}


rinteger rjs_compiler_rh_ifconditionop(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rjs_coctx_ifstatement_t *ctx = (rjs_coctx_ifstatement_t *)rjs_compiler_getctx(co, RJS_COCTX_IFSTATEMENT);

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	if (!ctx) {
		rjs_compiler_adderror(co, RJS_ERROR_NOTAIFSTATEMENT, prec);
		return -1;
	}
	R_ASSERT(ctx);
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


rinteger rjs_compiler_rh_iftruestatement(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rjs_coctx_ifstatement_t *ctx = (rjs_coctx_ifstatement_t *)rjs_compiler_getctx(co, RJS_COCTX_IFSTATEMENT);

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	if (!ctx) {
		rjs_compiler_adderror(co, RJS_ERROR_NOTAIFSTATEMENT, prec);
		return -1;
	}
	R_ASSERT(ctx);
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


rinteger rjs_compiler_rh_iffalsestatement(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rjs_coctx_ifstatement_t *ctx = (rjs_coctx_ifstatement_t *)rjs_compiler_getctx(co, RJS_COCTX_IFSTATEMENT);

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	if (!ctx) {
		rjs_compiler_adderror(co, RJS_ERROR_NOTAIFSTATEMENT, prec);
		return -1;
	}
	R_ASSERT(ctx);
	rvm_codegen_redefinelabel_default(co->cg, ctx->falseidx);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rinteger rjs_compiler_rh_iterationfor(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rlong childrec;
	rparecord_t *prec;
	rjs_coctx_iteration_t ctx;

	r_memset(&ctx, 0, sizeof(ctx));
	ctx.base.type = RJS_COCTX_ITERATION;
	ctx.start = rvm_codegen_getcodesize(co->cg);
	ctx.iterationidx = rvm_codegen_invalid_addlabel_s(co->cg, NULL);
	ctx.continueidx = rvm_codegen_invalid_addlabel_s(co->cg, NULL);
	ctx.endidx = rvm_codegen_invalid_addlabel_s(co->cg, NULL);
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
	rvm_codegen_redefinelabel_default(co->cg, ctx.iterationidx);
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
	rvm_codegen_redefinelabel_default(co->cg, ctx.continueidx);
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
	rvm_codegen_redefinelabel_default(co->cg, ctx.endidx);
	rjs_compiler_debugtail(co, records, rec);
	rvm_scope_pop(co->scope);
	r_array_removelast(co->coctx);
	return 0;

error:
	rvm_scope_pop(co->scope);
	r_array_removelast(co->coctx);
	return -1;
}


rinteger rjs_compiler_rh_iterationwhile(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rjs_coctx_iteration_t ctx;

	r_memset(&ctx, 0, sizeof(ctx));
	ctx.base.type = RJS_COCTX_ITERATION;
	ctx.start = rvm_codegen_getcodesize(co->cg);
	ctx.iterationidx = rvm_codegen_invalid_addlabel_s(co->cg, NULL);
	ctx.continueidx = rvm_codegen_invalid_addlabel_s(co->cg, NULL);
	ctx.endidx = rvm_codegen_invalid_addlabel_s(co->cg, NULL);
	r_array_push(co->coctx, &ctx, rjs_coctx_t*);
	rvm_scope_push(co->scope);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);
	rvm_codegen_redefinelabel_default(co->cg, ctx.iterationidx);
	rvm_codegen_redefinelabel_default(co->cg, ctx.continueidx);
	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		goto error;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, ctx.iterationidx, rvm_asm(RVM_B, DA, XX, XX, 0));
	rvm_codegen_redefinelabel_default(co->cg, ctx.endidx);
	rjs_compiler_debugtail(co, records, rec);
	rvm_scope_pop(co->scope);
	r_array_removelast(co->coctx);
	return 0;

error:
	rvm_scope_pop(co->scope);
	r_array_removelast(co->coctx);
	return -1;
}


rinteger rjs_compiler_rh_iterationdo(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rjs_coctx_iteration_t ctx;

	r_memset(&ctx, 0, sizeof(ctx));
	ctx.base.type = RJS_COCTX_ITERATION;
	ctx.start = rvm_codegen_getcodesize(co->cg);
	ctx.iterationidx = rvm_codegen_invalid_addlabel_s(co->cg, NULL);
	ctx.continueidx = rvm_codegen_invalid_addlabel_s(co->cg, NULL);
	ctx.endidx = rvm_codegen_invalid_addlabel_s(co->cg, NULL);
	r_array_push(co->coctx, &ctx, rjs_coctx_t*);
	rvm_scope_push(co->scope);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);
	rvm_codegen_redefinelabel_default(co->cg, ctx.iterationidx);
	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		goto error;
	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, ctx.iterationidx, rvm_asm(RVM_B, DA, XX, XX, 0));
	rvm_codegen_redefinelabel_default(co->cg, ctx.endidx);
	rjs_compiler_debugtail(co, records, rec);
	rvm_scope_pop(co->scope);
	r_array_removelast(co->coctx);
	return 0;

error:
	rvm_scope_pop(co->scope);
	r_array_removelast(co->coctx);
	return -1;
}


rinteger rjs_compiler_rh_dowhileexpressioncompare(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rjs_coctx_iteration_t *ctx = (rjs_coctx_iteration_t *)rjs_compiler_getctx(co, RJS_COCTX_ITERATION);

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	if (!ctx) {
		rjs_compiler_adderror(co, RJS_ERROR_NOTALOOP, prec);
		return -1;
	}
	R_ASSERT(ctx);
	rvm_codegen_redefinelabel_default(co->cg, ctx->continueidx);
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


rinteger rjs_compiler_rh_forexpressioncompare(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rjs_coctx_iteration_t *ctx = (rjs_coctx_iteration_t *)rjs_compiler_getctx(co, RJS_COCTX_ITERATION);

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	if (!ctx) {
		rjs_compiler_adderror(co, RJS_ERROR_NOTALOOP, prec);
		return -1;
	}
	R_ASSERT(ctx);
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


rinteger rjs_compiler_rh_forexpressionincrement(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rjs_coctx_iteration_t *ctx = (rjs_coctx_iteration_t *)rjs_compiler_getctx(co, RJS_COCTX_ITERATION);

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	if (!ctx) {
		rjs_compiler_adderror(co, RJS_ERROR_NOTALOOP, prec);
		return -1;
	}
	R_ASSERT(ctx);
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


rinteger rjs_compiler_rh_postfixexpressionop(rjs_compiler_t *co, rarray_t *records, rlong rec)
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


rinteger rjs_compiler_rh_prefixexpressionop(rjs_compiler_t *co, rarray_t *records, rlong rec)
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


rinteger rjs_compiler_rh_newexpressioncall(rjs_compiler_t *co, rarray_t *records, rlong rec)
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
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MAPALLOC, TP, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUB, FP, SP, DA, ctx.arguments));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_CALL, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, SP, FP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, TP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BITS(TP,LR)));
	rjs_compiler_debugtail(co, records, rec);
	r_array_removelast(co->coctx);
	return 0;

error:
	r_array_removelast(co->coctx);
	return -1;
}


rinteger rjs_compiler_rh_this(rjs_compiler_t *co, rarray_t *records, rlong rec)
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
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, TP, XX, 0));
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rinteger rjs_compiler_rh_binaryoperator(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rjs_coctx_t *ctx = rjs_compiler_gettopctx(co);

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	if (ctx && ctx->type == RJS_COCTX_OPERATION)
		((rjs_coctx_operation_t *)ctx)->opcode = rjs_compiler_record2opcode(prec);
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rinteger rjs_compiler_rh_unaryoperator(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rjs_coctx_t *ctx = rjs_compiler_gettopctx(co);

	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	if (ctx && ctx->type == RJS_COCTX_OPERATION)
		((rjs_coctx_operation_t *)ctx)->opcode = rjs_compiler_record2unaryopcode(prec);
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rinteger rjs_compiler_rh_identifiername(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, prec->inputsiz));
	rvm_codegen_addins(co->cg, rvm_asmp(RVM_MOV, R2, DA, XX, (void*)prec->input));
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rinteger rjs_compiler_rh_continue(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rjs_coctx_t *ctx;
	rjs_coctx_iteration_t *iterctx;

	ctx = rjs_compiler_getctx(co, RJS_COCTX_ITERATION);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	if (!ctx) {
		rjs_compiler_adderror(co, RJS_ERROR_NOTALOOP, prec);
		return -1;
	}
	R_ASSERT(ctx);
	iterctx = (rjs_coctx_iteration_t *)ctx;
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, iterctx->continueidx, rvm_asm(RVM_B, DA, XX, XX, 0));
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rinteger rjs_compiler_rh_break(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	rjs_coctx_t *ctx;
	rjs_coctx_iteration_t *iterctx;


	ctx = rjs_compiler_getctx(co, RJS_COCTX_ITERATION);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	if (!ctx) {
		rjs_compiler_adderror(co, RJS_ERROR_NOTALOOP, prec);
		return -1;
	}
	R_ASSERT(ctx);
	iterctx = (rjs_coctx_iteration_t *)ctx;
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0)
		return -1;

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, iterctx->endidx, rvm_asm(RVM_B, DA, XX, XX, 0));
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rinteger rjs_compiler_rh_syntaxerror(rjs_compiler_t *co, rarray_t *records, rlong rec)
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
	rjs_compiler_adderror(co, RJS_ERROR_SYNTAX, prec);
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rinteger rjs_compiler_rh_block(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_scope_push(co->scope);
	rjs_compiler_debugtail(co, records, rec);

	if (rjs_compiler_playchildrecords(co, records, rec) < 0) {
		rvm_scope_pop(co->scope);
		return -1;
	}

	rec = rpa_recordtree_get(records, rec, RPA_RECORD_END);
	prec = (rparecord_t *)r_array_slot(records, rec);
	rjs_compiler_debughead(co, records, rec);
	rvm_scope_pop(co->scope);
	rjs_compiler_debugtail(co, records, rec);
	return 0;
}


rinteger rjs_compiler_rh_(rjs_compiler_t *co, rarray_t *records, rlong rec)
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
	co->handlers[UID_STRINGLITERAL] = rjs_compiler_rh_stringliteral;

	co->handlers[UID_ADDITIVEEXPRESSIONOP] = rjs_compiler_rh_binaryexpressionop;
	co->handlers[UID_MULTIPLICATIVEEXPRESSIONOP] = rjs_compiler_rh_binaryexpressionop;
	co->handlers[UID_BITWISEANDOP] = rjs_compiler_rh_binaryexpressionop;
	co->handlers[UID_BITWISEXOROP] = rjs_compiler_rh_binaryexpressionop;
	co->handlers[UID_BITWISEOROP] = rjs_compiler_rh_binaryexpressionop;
	co->handlers[UID_SHIFTEXPRESSIONOP] = rjs_compiler_rh_binaryexpressionop;
	co->handlers[UID_EQUALITYEXPRESSIONOP] = rjs_compiler_rh_binaryexpressionop;
	co->handlers[UID_RELATIONALEXPRESSIONOP] = rjs_compiler_rh_binaryexpressionop;
	co->handlers[UID_LOGICALOROP] = rjs_compiler_rh_binaryexpressionop;
	co->handlers[UID_LOGICALANDOP] = rjs_compiler_rh_binaryexpressionop;
	co->handlers[UID_VARIABLEALLOCATEANDINIT] = rjs_compiler_rh_varallocinit;
	co->handlers[UID_VARIABLEALLOCATE] = rjs_compiler_rh_varalloc;
	co->handlers[UID_IDENTIFIER] = rjs_compiler_rh_identifier;
	co->handlers[UID_IDENTIFIERNAME] = rjs_compiler_rh_identifiername;
	co->handlers[UID_INITIALISER] = rjs_compiler_rh_initializer;
	co->handlers[UID_ASSIGNMENTEXPRESSIONOP] = rjs_compiler_rh_assignmentexpressionop;
	co->handlers[UID_NEWARRAYEXPRESSION] = rjs_compiler_rh_newarrayexpression;
	co->handlers[UID_MEMBEREXPRESSIONDOTOP] = rjs_compiler_rh_memberexpressiondotop;
	co->handlers[UID_MEMBEREXPRESSIONINDEXOP] = rjs_compiler_rh_memberexpressionindexop;
	co->handlers[UID_FUNCTIONDECLARATION] = rjs_compiler_rh_functiondeclaration;
	co->handlers[UID_FUNCTIONEXPRESSION] = rjs_compiler_rh_functiondeclaration;
	co->handlers[UID_FUNCTIONPARAMETER] = rjs_compiler_rh_functionparameter;
	co->handlers[UID_FUNCTIONCALL] = rjs_compiler_rh_functioncall;
	co->handlers[UID_ARGUMENT] = rjs_compiler_rh_argument;
	co->handlers[UID_ARGUMENTS] = rjs_compiler_rh_arguments;
	co->handlers[UID_RETURNSTATEMENT] = rjs_compiler_rh_returnstatement;
	co->handlers[UID_IFSTATEMENT] = rjs_compiler_rh_ifstatement;
	co->handlers[UID_IFCONDITIONOP] = rjs_compiler_rh_ifconditionop;
	co->handlers[UID_IFTRUESTATEMENT] = rjs_compiler_rh_iftruestatement;
	co->handlers[UID_IFFALSESTATEMENT] = rjs_compiler_rh_iffalsestatement;
	co->handlers[UID_ITERATIONDO] = rjs_compiler_rh_iterationdo;
	co->handlers[UID_ITERATIONWHILE] = rjs_compiler_rh_iterationwhile;
	co->handlers[UID_ITERATIONFOR] = rjs_compiler_rh_iterationfor;
	co->handlers[UID_DOWHILEEXPRESSIONCOMPARE] = rjs_compiler_rh_dowhileexpressioncompare;
	co->handlers[UID_WHILEEXPRESSIONCOMPARE] = rjs_compiler_rh_forexpressioncompare;
	co->handlers[UID_FOREXPRESSIONCOMPARE] = rjs_compiler_rh_forexpressioncompare;
	co->handlers[UID_FOREXPRESSIONINCREMENT] = rjs_compiler_rh_forexpressionincrement;
	co->handlers[UID_POSTFIXEXPRESSIONOP] = rjs_compiler_rh_postfixexpressionop;
	co->handlers[UID_PREFIXEXPRESSIONOP] = rjs_compiler_rh_prefixexpressionop;
	co->handlers[UID_THIS] = rjs_compiler_rh_this;
	co->handlers[UID_NEWEXPRESSIONCALL] = rjs_compiler_rh_newexpressioncall;
	co->handlers[UID_UNARYEXPRESSIONOP] = rjs_compiler_rh_unaryexpressionop;
	co->handlers[UID_BINARYOPERATOR] = rjs_compiler_rh_binaryoperator;
	co->handlers[UID_UNARYOPERATOR] = rjs_compiler_rh_unaryoperator;
	co->handlers[UID_BREAKSTATEMENT] = rjs_compiler_rh_break;
	co->handlers[UID_CONTINUESTATEMENT] = rjs_compiler_rh_continue;
	co->handlers[UID_SYNTAXERROR] = rjs_compiler_rh_syntaxerror;
	co->handlers[UID_BLOCK] = rjs_compiler_rh_block;

	return co;
}


void rjs_compiler_destroy(rjs_compiler_t *co)
{
	if (co) {
		rvm_scope_destroy(co->scope);
		r_array_destroy(co->coctx);
		r_free(co->temp);
		co->cpu = NULL;
		r_free(co);
	}
}


static rinteger rjs_compiler_rh_default(rjs_compiler_t *co, rarray_t *records, rlong rec)
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


static rinteger rjs_compiler_playchildrecords(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rlong child;

	for (child = rpa_recordtree_firstchild(records, rec, RPA_RECORD_START); child >= 0; child = rpa_recordtree_next(records, child, RPA_RECORD_START)) {
		if (rjs_compiler_playrecord(co, records, child) < 0)
			return -1;
	}

	return 0;
}


rinteger rjs_compiler_playreversechildrecords(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rlong child;

	for (child = rpa_recordtree_lastchild(records, rec, RPA_RECORD_START); child >= 0; child = rpa_recordtree_prev(records, child, RPA_RECORD_START)) {
		if (rjs_compiler_playrecord(co, records, child) < 0)
			return -1;
	}

	return 0;
}


static rinteger rjs_compiler_playrecord(rjs_compiler_t *co, rarray_t *records, rlong rec)
{
	rparecord_t *prec;
	prec = (rparecord_t *)r_array_slot(records, rec);
	if (prec->ruleuid >= 0 && prec->ruleuid < RJS_COMPILER_NHANDLERS && co->handlers[prec->ruleuid]) {
		return co->handlers[prec->ruleuid](co, records, rec);
	}
	return rjs_compiler_rh_default(co, records, rec);
}


rinteger rjs_compiler_compile(rjs_compiler_t *co, const rchar *script, rsize_t scriptsize, rarray_t *records, rvm_codegen_t *cg, rjs_error_t *error)
{
	rlong i;
	rvm_codelabel_t *labelerr;

	if (!co || !records || !cg || r_array_empty(records)) {
		/*
		 * TBD
		 */
		return -1;
	}
	co->cg = cg;
	co->error = error;
	co->script = script;
	co->scriptsize = scriptsize;

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
