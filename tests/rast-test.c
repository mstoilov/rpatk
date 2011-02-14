#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "rmem.h"
#include "rjsobject.h"
#include "rvmcpu.h"
#include "rpadbex.h"
#include "rpaerror.h"
#include "rvmcodegen.h"
#include "rvmcodemap.h"
#include "rvmscope.h"
#include "rvmoperator.h"
#include "rgc.h"
#include "rastnode.h"


typedef struct rastcompiler_s {
	rpa_dbex_handle dbex;
	rgc_t *gc;
	rastnode_t *root;
	rint optimized;
} rastcompiler_t;

void r_astcompiler_loadrules(rastcompiler_t *aco);


static int debuginfo = 0;
static int parseinfo = 0;
static int verboseinfo = 0;
static int compileonly = 0;


void codegen_unmap_file(rstr_t *buf)
{
	if (buf) {
		munmap(buf->str, buf->size);
		r_free(buf);
	}
}


rstr_t *codegen_map_file(const char *filename)
{
	struct stat st;
	rstr_t *str;
	char *buffer;


	int fd = open(filename, O_RDONLY);
	if (fd < 0) {
		return (void*)0;
	}
	if (fstat(fd, &st) < 0) {
		close(fd);
		return (void*)0;
	}
	buffer = (char*)mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (buffer == (void*)-1) {
		close(fd);
		return (void*)0;
	}
	str = (rstr_t *)r_malloc(sizeof(*str));
	if (!str)
		goto error;
	r_memset(str, 0, sizeof(*str));
	str->str = buffer;
	str->size = st.st_size;
	close(fd);
	return str;

error:
	munmap(buffer, st.st_size);
	close(fd);
	return str;
}


rastcompiler_t *r_astcompiler_create()
{
	rastcompiler_t *aco;

	aco = r_malloc(sizeof(*aco));
	r_memset(aco, 0, sizeof(*aco));
	aco->gc = r_gc_create();
	aco->root = r_astnode_create();
	r_gc_add(aco->gc, (robject_t*)aco->root);
	return aco;
}


void r_astcompiler_destroy(rastcompiler_t *aco)
{
	if (aco) {
		r_object_destroy((robject_t*)aco->gc);
	}
	r_free(aco);
}


void r_astnode_dump(rastnode_t *node, ruint level)
{
	ruint i;
	rlink_t *pos;

	for (i = 0; i < level; i++)
		fprintf(stdout, "  ");

	if (node->val.type == R_ASTVAL_HEAD) {
		if (node->name) {
			fprintf(stdout, "%s: ", node->name);
			fwrite(node->src.ptr, sizeof(char), node->src.size, stdout);
			fprintf(stdout, "\n");
		}

		r_list_foreach(pos, &node->val.v.h) {
			r_astnode_dump(r_list_entry(pos, rastnode_t, lnk), level + 1);
		}
//		for (i = 0; i < r_carray_length(node->val.v.arr); i++) {
//			rastval_t *val = (rastval_t *)r_carray_slot(node->val.v.arr, i);
//			r_astnode_dump(val->v.node, level + 1);
//		}

	} else {
		if (node->name) {
			fprintf(stdout, "%s: ", node->name);
		}
		fwrite(node->src.ptr, sizeof(char), node->src.size, stdout);
		fprintf(stdout, "\n");
	}

}


void r_astnode_propery_set_string(rastnode_t *node, const char *key, rstring_t *str)
{
	rastval_t val;
	R_ASTVAL_SET_STRING(&val, str);
//	if (!node->props)
//		node->props = r_harray_create(sizeof(rastval_t));
//	r_harray_add_s(node->props, key, &val);
}

void r_astcompiler_dumptree(rastcompiler_t *aco)
{
	r_astnode_dump(aco->root, 0);
}


int r_astcompiler_dumpnotification(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	if (reason & RPA_REASON_START)
		fprintf(stdout, "START ");
	if (reason & RPA_REASON_MATCHED)
		fprintf(stdout, "MATCHED ");
	if (reason & RPA_REASON_END)
		fprintf(stdout, "END ");
	fprintf(stdout, "%s: ", name);
	fprintf(stdout, "\n");
	return size;
}


int r_astcompiler_notify(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rastcompiler_t *aco = (rastcompiler_t *)userdata;

	if (parseinfo)
		r_astcompiler_dumpnotification(stat, name, userdata, input, size, reason);

	if (compileonly)
		return size;

	if (reason & RPA_REASON_START) {
		rastnode_t *node = r_astnode_create();
		r_gc_add(aco->gc, (robject_t*)node);
		node->parent = aco->root;
		aco->root = node;
	} else if (reason & RPA_REASON_MATCHED) {
		rastnode_t *node = aco->root;
		node->name = name;
		aco->root = node->parent;
		r_astnode_addchild(aco->root, node);
		node->src.ptr = (rpointer)input;
		node->src.size = size;
	} else {
		aco->root = aco->root->parent;
	}

	return size;
}


int main(int argc, char *argv[])
{
	int res, i;
	rstr_t *script = NULL, *unmapscript = NULL;
	rastcompiler_t *aco = r_astcompiler_create();

	aco->dbex = rpa_dbex_create();

	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-L") == 0) {
		} else if (r_strcmp(argv[i], "-d") == 0) {
			debuginfo = 1;
		} else if (r_strcmp(argv[i], "-p") == 0) {
			parseinfo = 1;
		} else if (r_strcmp(argv[i], "-P") == 0) {
			parseinfo = 1;
			verboseinfo = 1;
		} else if (r_strcmp(argv[i], "-c") == 0) {
			compileonly = 1;
		} else if (r_strcmp(argv[i], "-o") == 0) {
			aco->optimized = 1;
		} else if (r_strcmp(argv[i], "-m") == 0) {

		}
	}

//	for (i = 0; i < 500000; i++) {
//		node = r_astnode_create();
//		r_gc_add(aco->gc, (robject_t*)node);
//	}


	r_astcompiler_loadrules(aco);

	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-e") == 0) {
			if (++i < argc) {
				rstr_t script = { argv[i], r_strlen(argv[i]) };
				res = rpa_dbex_parse(aco->dbex, rpa_dbex_default_pattern(aco->dbex), script.str, script.str, script.str + script.size);
				if (res <= 0)
					goto end;
			}
			goto exec;
		}
	}

	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-f") == 0) {

			if (++i < argc) {
				script = codegen_map_file(argv[i]);
				if (script) {
					res = rpa_dbex_parse(aco->dbex, rpa_dbex_default_pattern(aco->dbex), script->str, script->str, script->str + script->size);
					unmapscript = script;
					if (res <= 0)
						goto end;
				}

			}
			goto exec;
		}
	}


exec:
	r_astcompiler_dumptree(aco);


end:
	if (unmapscript)
		codegen_unmap_file(unmapscript);
	if (aco->dbex)
		rpa_dbex_destroy(aco->dbex);
	r_astcompiler_destroy(aco);

	if (debuginfo) {
		r_printf("Max alloc mem: %ld\n", r_debug_get_maxmem());
		r_printf("Leaked mem: %ld\n", r_debug_get_allocmem());
	}
	return 0;
}



extern char _binary_____________tests_astecma262_rpa_start[];
extern char _binary_____________tests_astecma262_rpa_end[];
extern unsigned long *_binary_____________tests_astecma262_rpa_size;

void r_astcompiler_loadrules(rastcompiler_t *aco)
{
	int ret, line;
	int inputsize = _binary_____________tests_astecma262_rpa_end - _binary_____________tests_astecma262_rpa_start;
	const char *buffer = _binary_____________tests_astecma262_rpa_start;
	const char *pattern = buffer;

	rpa_dbex_open(aco->dbex);

	rpa_dbex_add_callback_exact(aco->dbex, "BitwiseANDOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "BitwiseXOROp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "BitwiseOROp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "AdditiveExpressionOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "MultiplicativeExpressionOp", RPA_REASON_ALL, r_astcompiler_notify, aco);

	rpa_dbex_add_callback_exact(aco->dbex, "ShiftExpressionOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "EqualityExpressionOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "RelationalExpressionOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "LogicalOROp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "LogicalANDOp", RPA_REASON_ALL, r_astcompiler_notify, aco);

	rpa_dbex_add_callback_exact(aco->dbex, "AssignmentOperator", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "EqualityOperator", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "RelationalOperator", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "AdditiveOperator", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "MultiplicativeOperator", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "ShiftOperator", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "BitwiseANDOperator", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "BitwiseXOROperator", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "BitwiseOROperator", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "LogicalANDOperator", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "LogicalOROperator", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "LogicalNotOperator", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "BitwiseNotOperator", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "UnaryOperatorOpcode", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "PrintOp", RPA_REASON_ALL, r_astcompiler_notify, aco);


	rpa_dbex_add_callback_exact(aco->dbex, "PostfixOperator", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "PostfixExpressionOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "PrefixExpressionOp", RPA_REASON_ALL, r_astcompiler_notify, aco);


	rpa_dbex_add_callback_exact(aco->dbex, "UnaryExpressionOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "LogicalNotExpressionOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "BitwiseNotExpressionOp", RPA_REASON_ALL, r_astcompiler_notify, aco);

	rpa_dbex_add_callback_exact(aco->dbex, "DecimalIntegerLiteral", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "DecimalNonIntegerLiteral", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "BlockBegin", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "BlockEnd", RPA_REASON_ALL, r_astcompiler_notify, aco);

	rpa_dbex_add_callback_exact(aco->dbex, "DoKeyword", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "IterationDo", RPA_REASON_ALL, r_astcompiler_notify, aco);

	rpa_dbex_add_callback_exact(aco->dbex, "sqstring", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "dqstring", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "DoubleStringCharacters", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "SingleStringCharacters", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "Program", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "Initialiser", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "AssignmentExpressionOp", RPA_REASON_ALL, r_astcompiler_notify, aco);

	rpa_dbex_add_callback_exact(aco->dbex, "VariableAllocate", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "VariableAllocateAndInit", RPA_REASON_ALL, r_astcompiler_notify, aco);

	rpa_dbex_add_callback_exact(aco->dbex, "ReturnOp", RPA_REASON_ALL, r_astcompiler_notify, aco);

//	rpa_dbex_add_callback_exact(aco->dbex, "SwiId", RPA_REASON_ALL, r_astcompiler_notify, aco);
//	rpa_dbex_add_callback_exact(aco->dbex, "SwiIdExist", RPA_REASON_ALL, r_astcompiler_notify, aco);

	rpa_dbex_add_callback_exact(aco->dbex, "PostfixExpressionValOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "LeftHandSideExpressionPush", RPA_REASON_ALL, r_astcompiler_notify, aco);

	////////////////

	rpa_dbex_add_callback_exact(aco->dbex, "ThisOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "IdentifierOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "MemberIdentifierNameOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "MemberExpressionBaseOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "MemberExpressionIndexOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "MemberExpressionNameOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "CallExpressionNameOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "CallExpressionBaseOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "CallExpressionIndexOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "AddressLeftHandSideExpression", RPA_REASON_ALL, r_astcompiler_notify, aco);

	////////////////


	rpa_dbex_add_callback_exact(aco->dbex, "ConditionalExpression", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "NewArrayExpressionOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "compile_error", RPA_REASON_ALL, r_astcompiler_notify, aco);

	rpa_dbex_add_callback_exact(aco->dbex, "ArgumentsOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "BracketExpressionOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "ValLeftHandSideExpression", RPA_REASON_ALL, r_astcompiler_notify, aco);

	rpa_dbex_add_callback_exact(aco->dbex, "FunctionNameLookupAlloc", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "FunctionName", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "FunctionDeclaration", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "FunctionParameter", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "FunctionCallParameter", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "FunctionCallName", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "CallExpressionOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "NewKeyword", RPA_REASON_ALL, r_astcompiler_notify, aco);

	rpa_dbex_add_callback_exact(aco->dbex, "NewExpressionCallName", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "NewExpressionCallOp", RPA_REASON_ALL, r_astcompiler_notify, aco);


	rpa_dbex_add_callback_exact(aco->dbex, "IfConditionOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "IfOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "ElseOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "IfElseOp", RPA_REASON_ALL, r_astcompiler_notify, aco);

	rpa_dbex_add_callback_exact(aco->dbex, "WhileConditionOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "IterationWhileOp", RPA_REASON_ALL, r_astcompiler_notify, aco);

	rpa_dbex_add_callback_exact(aco->dbex, "QuestionMarkOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "AssignmentExpressionIfTrueOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "AssignmentExpressionIfFalseOp", RPA_REASON_ALL, r_astcompiler_notify, aco);

	rpa_dbex_add_callback_exact(aco->dbex, "ForKeyword", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "ForExpressionInitOp", RPA_REASON_END, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "ForExpressionCompareOp", RPA_REASON_END, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "ForExpressionIncrementOp", RPA_REASON_END, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "IterationForOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "BreakOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "ContinueOp", RPA_REASON_ALL, r_astcompiler_notify, aco);

	rpa_dbex_add_callback_exact(aco->dbex, "SwitchExpressionOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "SwitchStatementOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "CaseExpressionOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "CaseClauseOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "DefaultKeywordOp", RPA_REASON_ALL, r_astcompiler_notify, aco);
	rpa_dbex_add_callback_exact(aco->dbex, "DefaultClauseOp", RPA_REASON_ALL, r_astcompiler_notify, aco);



//	rpa_dbex_add_callback(aco->dbex, ".*", RPA_REASON_ALL, r_astcompiler_notify, aco);

	while ((ret = rpa_dbex_load(aco->dbex, pattern, inputsize)) > 0) {
		inputsize -= ret;
		pattern += ret;
	}
	if (ret < 0) {
		for (line = 1; pattern >= buffer; --pattern) {
			if (*pattern == '\n')
				line += 1;
		}
		fprintf(stdout, "Line: %d, RPA LOAD ERROR: %s\n", line, (rpa_dbex_get_error(aco->dbex) == RPA_E_SYNTAX_ERROR) ? "Syntax Error." : "Pattern Loading failed.");
		goto error;
	}

error:
	rpa_dbex_close(aco->dbex);
}
