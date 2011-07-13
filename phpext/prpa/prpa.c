/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2008 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id: header,v 1.16.2.1.2.1.2.1 2008/02/07 19:39:50 iliaa Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_prpa.h"
#include "TSRM/TSRM.h"
#include "rpa/rpadbex.h"
#include "rpa/rpastat.h"
#include "rpa/rpastatpriv.h"
#include "rpa/rpaerror.h"
#include "rpa/rparecord.h"

typedef struct _php_rpa_dbex {
	rpadbex_t* dbex;
#ifdef ZTS
	TSRMLS_D;	
#endif
} php_rpa_dbex;

#define PHP_RPA_DBEX_RES_NAME "php rpa dbex"


typedef struct _php_rpa_stat {
	rpastat_t* stat;
#ifdef ZTS
	TSRMLS_D;
#endif
} php_rpa_stat;

#define PHP_RPA_STAT_RES_NAME "php rpa stat"

static int le_rpa_dbex;
static int le_rpa_stat;
static void php_rpa_dbex_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC);
static void php_rpa_stat_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC);


/* If you declare any globals in php_prpa/rpa.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(prpa)
*/

/* True global resources - no need for thread safety here */
static int le_prpa;

/* {{{ prpa_functions[]
 *
 * Every user visible function must have an entry in prpa_functions[].
 */
zend_function_entry prpa_functions[] = {
	PHP_FE(rpaparse, NULL)
	PHP_FE(rpascan, NULL)
	PHP_FE(rpamatch, NULL)
	PHP_FE(rpa_dbex_version, NULL)
	PHP_FE(rpa_dbex_create, NULL)
	PHP_FE(rpa_dbex_open, NULL)
	PHP_FE(rpa_dbex_close, NULL)
	PHP_FE(rpa_dbex_load, NULL)
	PHP_FE(rpa_dbex_compile, NULL)
	PHP_FE(rpa_dbex_lookup, NULL)
	PHP_FE(rpa_dbex_first, NULL)
	PHP_FE(rpa_dbex_last, NULL)
	PHP_FE(rpa_dbex_cfgset, NULL)
	PHP_FE(rpa_dbex_dumpproductions, NULL)
	PHP_FE(rpa_dbex_dumptree, NULL)
	PHP_FE(rpa_dbex_error, NULL)
	PHP_FE(rpa_stat_create, NULL)
	PHP_FE(rpa_stat_match, NULL)
	PHP_FE(rpa_stat_scan, NULL)
	PHP_FE(rpa_stat_parse, NULL)
	PHP_FE(rpa_stat_error, NULL)
	{NULL, NULL, NULL}	/* Must be the last line in prpa_functions[] */
};
/* }}} */

/* {{{ prpa_module_entry
 */
zend_module_entry prpa_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"prpa",
	prpa_functions,
	PHP_MINIT(prpa),
	PHP_MSHUTDOWN(prpa),
	PHP_RINIT(prpa),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(prpa),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(prpa),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_PRPA
ZEND_GET_MODULE(prpa)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("prpa.global_value",		"42", PHP_INI_ALL, OnUpdateLong, global_value, zend_prpa_globals, prpa_globals)
	STD_PHP_INI_ENTRY("prpa.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_prpa_globals, prpa_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_prpa_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_prpa_init_globals(zend_prpa_globals *prpa_globals)
{
	prpa_globals->global_value = 0;
	prpa_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(prpa)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
	le_rpa_dbex = zend_register_list_destructors_ex(php_rpa_dbex_dtor, NULL, PHP_RPA_DBEX_RES_NAME, module_number);
	le_rpa_stat = zend_register_list_destructors_ex(php_rpa_stat_dtor, NULL, PHP_RPA_STAT_RES_NAME, module_number);
	REGISTER_LONG_CONSTANT("RPA_ENCODING_BYTE", RPA_ENCODING_BYTE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RPA_ENCODING_UTF8", RPA_ENCODING_UTF8, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RPA_ENCODING_ICASE_UTF8", RPA_ENCODING_ICASE_UTF8, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RPA_ENCODING_UTF16LE", RPA_ENCODING_UTF16LE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RPA_ENCODING_ICASE_UTF16LE", RPA_ENCODING_ICASE_UTF16LE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RPA_RECORD_START", RPA_RECORD_START, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RPA_RECORD_END", RPA_RECORD_END, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RPA_DBEXCFG_DEBUG", RPA_DBEXCFG_DEBUG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RPA_DBEXCFG_OPTIMIZATIONS", RPA_DBEXCFG_OPTIMIZATIONS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RPA_E_NONE", RPA_E_NONE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RPA_E_OUTOFMEM", RPA_E_OUTOFMEM, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RPA_E_NOTCOMPILED", RPA_E_NOTCOMPILED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RPA_E_NOTOPEN", RPA_E_NOTOPEN, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RPA_E_NOTCLOSED", RPA_E_NOTCLOSED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RPA_E_NOTFOUND", RPA_E_NOTFOUND, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RPA_E_SYNTAXERROR", RPA_E_SYNTAXERROR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RPA_E_UNRESOLVEDSYMBOL", RPA_E_UNRESOLVEDSYMBOL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RPA_E_PARAM", RPA_E_PARAM, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RPA_E_COMPILE", RPA_E_COMPILE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RPA_E_EXECUTION", RPA_E_EXECUTION, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RPA_E_USERABORT", RPA_E_USERABORT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RPA_E_RULEABORT", RPA_E_RULEABORT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("RPA_E_INVALIDINPUT", RPA_E_INVALIDINPUT, CONST_CS | CONST_PERSISTENT);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(prpa)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(prpa)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(prpa)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(prpa)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "prpa support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


PHP_FUNCTION(rpa_dbex_version)
{
	RETURN_STRING((char*)rpa_dbex_version(), 1);
}


static void php_rpa_stat_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	php_rpa_stat *phpstat = (php_rpa_stat*)rsrc->ptr;

	if (phpstat) {
		rpa_stat_destroy(phpstat->stat);
		efree(phpstat);
	}
}


static void php_rpa_dbex_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	php_rpa_dbex *phpdbex = (php_rpa_dbex*)rsrc->ptr;

	if (phpdbex) {
		rpa_dbex_destroy(phpdbex->dbex);
		efree(phpdbex);
	}
}


PHP_FUNCTION(rpa_dbex_create)
{
	php_rpa_dbex *phpdbex;

	phpdbex = emalloc(sizeof(php_rpa_dbex));
	phpdbex->dbex = rpa_dbex_create();
	ZEND_REGISTER_RESOURCE(return_value, phpdbex, le_rpa_dbex);
}


PHP_FUNCTION(rpa_dbex_open)
{
	zval *zres;
	php_rpa_dbex *phpdbex;
	int ret;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zres) == FAILURE) {
		RETURN_LONG(-1);
	}

	ZEND_FETCH_RESOURCE(phpdbex, php_rpa_dbex*, &zres, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);

	ret = rpa_dbex_open(phpdbex->dbex);
	RETURN_LONG(ret);
}


PHP_FUNCTION(rpa_dbex_close)
{
	zval *zres;
	php_rpa_dbex *phpdbex;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zres) == FAILURE) {
		RETURN_LONG(-1);
	}

	ZEND_FETCH_RESOURCE(phpdbex, php_rpa_dbex*, &zres, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);

	rpa_dbex_close(phpdbex->dbex);
	RETURN_LONG(0);
}


PHP_FUNCTION(rpa_dbex_load)
{
	zval *zres;
	php_rpa_dbex *phpdbex;
	char *patterns;
	int patterns_len;
	int ret;


	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &zres, &patterns, &patterns_len) == FAILURE) {
		RETURN_LONG(-1);
	}

	ZEND_FETCH_RESOURCE(phpdbex, php_rpa_dbex*, &zres, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);

	ret = rpa_dbex_load_s(phpdbex->dbex, patterns);
	RETURN_LONG(ret);
}


PHP_FUNCTION(rpa_dbex_lookup)
{
	zval *zres;
	php_rpa_dbex *phpdbex;
	char *name;
	int name_len;
	long ret;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &zres, &name, &name_len) == FAILURE) {
		RETURN_LONG(-1);
	}

	ZEND_FETCH_RESOURCE(phpdbex, php_rpa_dbex*, &zres, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);

	ret = rpa_dbex_lookup(phpdbex->dbex, name, name_len);
	RETURN_LONG(ret);
}


PHP_FUNCTION(rpa_dbex_first)
{
	zval *zres;
	php_rpa_dbex *phpdbex;
	long ret;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zres) == FAILURE) {
		RETURN_LONG(-1);
	}

	ZEND_FETCH_RESOURCE(phpdbex, php_rpa_dbex*, &zres, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);

	ret = rpa_dbex_first(phpdbex->dbex);
	RETURN_LONG(ret);
}


PHP_FUNCTION(rpa_dbex_last)
{
	zval *zres;
	php_rpa_dbex *phpdbex;
	long ret;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zres) == FAILURE) {
		RETURN_LONG(-1);
	}
	ZEND_FETCH_RESOURCE(phpdbex, php_rpa_dbex*, &zres, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);
	ret = rpa_dbex_last(phpdbex->dbex);
	RETURN_LONG(ret);
}


PHP_FUNCTION(rpa_dbex_compile)
{
	zval *zres;
	php_rpa_dbex *phpdbex;
	char *patterns;
	int patterns_len;
	int ret;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zres) == FAILURE) {
		RETURN_LONG(-1);
	}
	ZEND_FETCH_RESOURCE(phpdbex, php_rpa_dbex*, &zres, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);
	ret = rpa_dbex_compile(phpdbex->dbex);
	RETURN_LONG(ret);
}


PHP_FUNCTION(rpa_dbex_dumpproductions)
{
	zval *zres;
	php_rpa_dbex *phpdbex;
	long ret;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zres) == FAILURE) {
		RETURN_LONG(-1);
	}
	ZEND_FETCH_RESOURCE(phpdbex, php_rpa_dbex*, &zres, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);
	ret = rpa_dbex_dumpproductions(phpdbex->dbex);
	RETURN_LONG(ret);
}


PHP_FUNCTION(rpa_dbex_dumptree)
{
	zval *zres;
	php_rpa_dbex *phpdbex;
	long rid = 0;
	long ret;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &zres, &rid) == FAILURE) {
		RETURN_LONG(-1);
	}
	ZEND_FETCH_RESOURCE(phpdbex, php_rpa_dbex*, &zres, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);
	ret = rpa_dbex_dumptree(phpdbex->dbex, rid);
	RETURN_LONG(ret);
}


PHP_FUNCTION(rpa_dbex_cfgset)
{
	zval *zres;
	php_rpa_dbex *phpdbex;
	long cfg, val;
	long ret;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rll", &zres, cfg, val) == FAILURE) {
		RETURN_LONG(-1);
	}
	ZEND_FETCH_RESOURCE(phpdbex, php_rpa_dbex*, &zres, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);
	ret = rpa_dbex_cfgset(phpdbex->dbex, cfg, val);
	RETURN_LONG(ret);
}


static char *dbexmsg[] = {
	"OK",
	"Out of memory",
	"Invalid input.",
	"Expression database is not open.",
	"Expression database is not closed.",
	"Expression name is not found.",
	"Syntax error.",
	"Unresolved expression name.",
	"Invalid parameter.",
	"Failed to compile rule.",
	"Database is not compiled.",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
};


static char *statmsg[] = {
	"OK",
	"Execution error.",
	"Execution aborted by user.",
	"Aborted on expression.",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
};


static zval *rpa_zvalerror(rpa_errinfo_t *errorinfo)
{
	long n = 0;
	char buffer[1000];
	char *ptr = buffer;
	size_t size = sizeof(buffer) - 1;
	zval *zerror = NULL;

	memset(buffer, 0, sizeof(buffer));
	ALLOC_INIT_ZVAL(zerror);
	array_init(zerror);
	if (errorinfo->mask & RPA_ERRINFO_CODE) {
		if (errorinfo->code >= 1000 && errorinfo->code < 1010)
			n += snprintf(ptr + n, size - n, "%s Code: %ld. ", dbexmsg[errorinfo->code - 1000], errorinfo->code);
		else if (errorinfo->code >= 2000 && errorinfo->code < 2010)
			n += snprintf(ptr + n, size - n, "%s Code: %ld. ", statmsg[errorinfo->code - 2000], errorinfo->code);
		else
			n += snprintf(ptr + n, size - n, "Error Code: %ld. ", errorinfo->code);
		add_assoc_long(zerror, "code", errorinfo->code);
	}
	if (errorinfo->mask & RPA_ERRINFO_LINE) {
		n += snprintf(ptr + n, size - n, "Line: %ld. ", errorinfo->line);
		add_assoc_long(zerror, "line", errorinfo->line);
	}
	if (errorinfo->mask & RPA_ERRINFO_OFFSET) {
		n += snprintf(ptr + n, size - n, "Offset: %ld. ", errorinfo->offset);
		add_assoc_long(zerror, "offset", errorinfo->offset);
	}
	if (errorinfo->mask & RPA_ERRINFO_RULEUID) {
		n += snprintf(ptr + n, size - n, "RuleId: %ld. ", errorinfo->ruleuid);
		add_assoc_long(zerror, "rid", errorinfo->ruleuid);
	}
	if (errorinfo->mask & RPA_ERRINFO_NAME) {
		n += snprintf(ptr + n, size - n, "Name: %s. ", errorinfo->name);
		add_assoc_string(zerror, "name", errorinfo->name, 1);
	}
	add_assoc_stringl(zerror, "message", buffer, n, 1);
	return zerror;
}


PHP_FUNCTION(rpa_dbex_error)
{
	zval *zres;
	php_rpa_dbex *phpdbex;
	zval *zerror = NULL;
	rpa_errinfo_t errorinfo;

	memset(&errorinfo, 0, sizeof(errorinfo));
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zres) == FAILURE) {
		RETURN_LONG(-1);
	}
	ZEND_FETCH_RESOURCE(phpdbex, php_rpa_dbex*, &zres, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);
	rpa_dbex_lasterrorinfo(phpdbex->dbex, &errorinfo);
	zerror = rpa_zvalerror(&errorinfo);
	RETURN_ZVAL(zerror, 1, 0);
}


PHP_FUNCTION(rpa_stat_error)
{
	zval *zres;
	char buffer[1000];
	php_rpa_stat *phpstat;
	zval *zerror = NULL;
	rpa_errinfo_t errorinfo;

	memset(&errorinfo, 0, sizeof(errorinfo));
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zres) == FAILURE) {
		RETURN_LONG(-1);
	}
	ZEND_FETCH_RESOURCE(phpstat, php_rpa_stat*, &zres, -1, PHP_RPA_STAT_RES_NAME, le_rpa_stat);
	rpa_stat_lasterrorinfo(phpstat->stat, &errorinfo);
	zerror = rpa_zvalerror(&errorinfo);
	RETURN_ZVAL(zerror, 1, 0);
}


static void rpa_records2array(const char *input, rarray_t *records, zval *zrecords)
{
	long i;

	rparecord_t *record;
	array_init(zrecords);
	for (i = 0; i < rpa_records_length(records); i++) {
		zval *zrecord;
		record = rpa_records_slot(records, i);
		ALLOC_INIT_ZVAL(zrecord);
		array_init(zrecord);
		add_assoc_stringl(zrecord, "input", (char*)record->input, record->inputsiz, 1);
		add_assoc_string(zrecord, "rule", (char*)record->rule, 1);
		add_assoc_long(zrecord, "type", record->type);
		add_assoc_long(zrecord, "uid", record->ruleuid);
		add_assoc_long(zrecord, "offset", record->input - input);
		add_assoc_long(zrecord, "size", record->inputsiz);
		add_next_index_zval(zrecords, zrecord);
	}
}


PHP_FUNCTION(rpa_stat_create)
{
	zval *zres;
	php_rpa_dbex *phpdbex;
	php_rpa_stat *phpstat;
	int ret;
	long stackSize = 0L;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|l", &zres, &stackSize) == FAILURE) {
		RETURN_LONG(-1);
	}
	ZEND_FETCH_RESOURCE(phpdbex, php_rpa_dbex*, &zres, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);
	phpstat = emalloc(sizeof(php_rpa_stat));
	phpstat->stat = rpa_stat_create(phpdbex->dbex, stackSize);
	ZEND_REGISTER_RESOURCE(return_value, phpstat, le_rpa_stat);
}


PHP_FUNCTION(rpa_stat_match)
{
	zval *zstat;
	php_rpa_stat *phpstat;
	long rid;
	long encoding;
	long ret;
	char *input;
	int input_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rlls", &zstat, &rid, &encoding, &input, &input_len) == FAILURE) {
		RETURN_LONG(-1);
	}
	ZEND_FETCH_RESOURCE(phpstat, php_rpa_stat*, &zstat, -1, PHP_RPA_STAT_RES_NAME, le_rpa_stat);
	ret = rpa_stat_match(phpstat->stat, rid, encoding, input, input, input + input_len);
	RETURN_LONG(ret);
}


PHP_FUNCTION(rpa_stat_scan)
{
	zval *zstat;
	php_rpa_stat *phpstat;
	long rid;
	long encoding;
	long ret;
	char *input;
	int input_len;
	zval *zwhere;
	const char *where = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rllsz", &zstat, &rid, &encoding, &input, &input_len, &zwhere) == FAILURE) {
		RETURN_LONG(-1);
	}
	ZEND_FETCH_RESOURCE(phpstat, php_rpa_stat*, &zstat, -1, PHP_RPA_STAT_RES_NAME, le_rpa_stat);
	ret = rpa_stat_scan(phpstat->stat, rid, encoding, input, input, input + input_len, &where);
	if (ret > 0) {
		ZVAL_LONG(zwhere, where - input);
	} else {
		ZVAL_NULL(zwhere);
	}
	RETURN_LONG(ret);
}


PHP_FUNCTION(rpa_stat_parse)
{
	zval *zstat;
	php_rpa_stat *phpstat;
	long rid;
	long encoding;
	long ret, i;
	char *input;
	int input_len;
	zval *zrecords = NULL;
	rarray_t *records = rpa_records_create();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rlls|z", &zstat, &rid, &encoding, &input, &input_len, &zrecords) == FAILURE) {
		RETURN_LONG(-1);
	}
	ZEND_FETCH_RESOURCE(phpstat, php_rpa_stat*, &zstat, -1, PHP_RPA_STAT_RES_NAME, le_rpa_stat);
	ret = rpa_stat_parse(phpstat->stat, rid, encoding, input, input, input + input_len, records);
	if (ret <= 0) {
		if (zrecords)
			ZVAL_NULL(zrecords);
		rpa_records_destroy(records);
		RETURN_LONG(ret);
	}
	if (zrecords) {
		rpa_records2array(input, records, zrecords);
	}
	rpa_records_destroy(records);
	RETURN_LONG(ret);
}


PHP_FUNCTION(rpaparse)
{
	rpadbex_t *dbex = NULL;
	rpastat_t *stat = NULL;
	long rid;
	long encoding;
	long ret, i;
	char *bnf;
	int bnf_len;
	char *input;
	int input_len;
	zval *zerror = NULL;
	zval *zrecords = NULL;
	rarray_t *records = rpa_records_create();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sls|zz", &bnf, &bnf_len, &encoding, &input, &input_len, &zrecords, &zerror) == FAILURE) {
		RETURN_LONG(-1);
	}
	dbex = rpa_dbex_create();
	if (rpa_dbex_open(dbex) < 0)
		goto dbexerror;
	if (rpa_dbex_load(dbex, bnf, bnf_len) < 0)
		goto dbexerror;
	rpa_dbex_close(dbex);
	if (rpa_dbex_compile(dbex) < 0)
		goto dbexerror;
	stat = rpa_stat_create(dbex, 16000);
	ret = rpa_stat_parse(stat, rpa_dbex_last(dbex), encoding, input, input, input + input_len, records);
	if (ret < 0)
		goto staterror;
	if (ret > 0 && zrecords) {
		rpa_records2array(input, records, zrecords);
	}

	if (zerror) {
		ZVAL_NULL(zerror);
	}
	rpa_records_destroy(records);
	rpa_stat_destroy(stat);
	rpa_dbex_destroy(dbex);
	RETURN_LONG(ret);

dbexerror:
	if (zerror) {
		rpa_errinfo_t errorinfo;
		memset(&errorinfo, 0, sizeof(errorinfo));
		rpa_dbex_lasterrorinfo(dbex, &errorinfo);
		zval *temp = rpa_zvalerror(&errorinfo);
		ZVAL_ZVAL(zerror, temp, 0, 1);
	}
//	if (zrecords)
//		ZVAL_NULL(zrecords);
	rpa_records_destroy(records);
	rpa_stat_destroy(stat);
	rpa_dbex_destroy(dbex);
	RETURN_LONG(-1);

staterror:
	if (zerror) {
		rpa_errinfo_t errorinfo;
		memset(&errorinfo, 0, sizeof(errorinfo));
		rpa_stat_lasterrorinfo(stat, &errorinfo);
		zval *temp = rpa_zvalerror(&errorinfo);
		ZVAL_ZVAL(zerror, temp, 0, 1);
	}
//	if (zrecords)
//		ZVAL_NULL(zrecords);
	rpa_records_destroy(records);
	rpa_stat_destroy(stat);
	rpa_dbex_destroy(dbex);
	RETURN_LONG(-1);
}


PHP_FUNCTION(rpascan)
{
	rpadbex_t *dbex = NULL;
	rpastat_t *stat = NULL;
	long rid;
	long encoding;
	long ret, i;
	char *bnf;
	int bnf_len;
	char *input;
	const char *where = NULL;
	int input_len;
	zval *zerror = NULL;
	zval *zwhere = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sls|zz", &bnf, &bnf_len, &encoding, &input, &input_len, &zwhere, &zerror) == FAILURE) {
		RETURN_LONG(-1);
	}
	dbex = rpa_dbex_create();
	if (rpa_dbex_open(dbex) < 0)
		goto dbexerror;
	if (rpa_dbex_load(dbex, bnf, bnf_len) < 0)
		goto dbexerror;
	rpa_dbex_close(dbex);
	if (rpa_dbex_compile(dbex) < 0)
		goto dbexerror;
	stat = rpa_stat_create(dbex, 16000);
	ret = rpa_stat_scan(stat, rpa_dbex_last(dbex), encoding, input, input, input + input_len, &where);
	if (ret < 0)
		goto staterror;
	if (zwhere) {
		if (ret > 0 && where) {
			ZVAL_LONG(zwhere, where - input);
		} else {
			ZVAL_NULL(zwhere);
		}
	}
	if (zerror) {
		ZVAL_NULL(zerror);
	}
	rpa_stat_destroy(stat);
	rpa_dbex_destroy(dbex);
	RETURN_LONG(ret);

dbexerror:
	if (zerror) {
		rpa_errinfo_t errorinfo;
		memset(&errorinfo, 0, sizeof(errorinfo));
		rpa_dbex_lasterrorinfo(dbex, &errorinfo);
		zval *temp = rpa_zvalerror(&errorinfo);
		ZVAL_ZVAL(zerror, temp, 0, 1);
	}
	rpa_stat_destroy(stat);
	rpa_dbex_destroy(dbex);
	RETURN_LONG(-1);

staterror:
	if (zerror) {
		rpa_errinfo_t errorinfo;
		memset(&errorinfo, 0, sizeof(errorinfo));
		rpa_stat_lasterrorinfo(stat, &errorinfo);
		zval *temp = rpa_zvalerror(&errorinfo);
		ZVAL_ZVAL(zerror, temp, 0, 1);
	}
	rpa_stat_destroy(stat);
	rpa_dbex_destroy(dbex);
	RETURN_LONG(-1);
}


PHP_FUNCTION(rpamatch)
{
	rpadbex_t *dbex = NULL;
	rpastat_t *stat = NULL;
	long rid;
	long encoding;
	long ret, i;
	char *bnf;
	int bnf_len;
	char *input;
	int input_len;
	zval *zerror = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sls|z", &bnf, &bnf_len, &encoding, &input, &input_len, &zerror) == FAILURE) {
		RETURN_LONG(-1);
	}
	dbex = rpa_dbex_create();
	if (rpa_dbex_open(dbex) < 0)
		goto dbexerror;
	if (rpa_dbex_load(dbex, bnf, bnf_len) < 0)
		goto dbexerror;
	rpa_dbex_close(dbex);
	if (rpa_dbex_compile(dbex) < 0)
		goto dbexerror;
	stat = rpa_stat_create(dbex, 16000);
	ret = rpa_stat_match(stat, rpa_dbex_last(dbex), encoding, input, input, input + input_len);
	if (ret < 0)
		goto staterror;
	if (zerror) {
		ZVAL_NULL(zerror);
	}
	rpa_stat_destroy(stat);
	rpa_dbex_destroy(dbex);
	RETURN_LONG(ret);

dbexerror:
	if (zerror) {
		rpa_errinfo_t errorinfo;
		memset(&errorinfo, 0, sizeof(errorinfo));
		rpa_dbex_lasterrorinfo(dbex, &errorinfo);
		zval *temp = rpa_zvalerror(&errorinfo);
		ZVAL_ZVAL(zerror, temp, 0, 1);
	}
	rpa_stat_destroy(stat);
	rpa_dbex_destroy(dbex);
	RETURN_LONG(-1);

staterror:
	if (zerror) {
		rpa_errinfo_t errorinfo;
		memset(&errorinfo, 0, sizeof(errorinfo));
		rpa_stat_lasterrorinfo(stat, &errorinfo);
		zval *temp = rpa_zvalerror(&errorinfo);
		ZVAL_ZVAL(zerror, temp, 0, 1);
	}
	rpa_stat_destroy(stat);
	rpa_dbex_destroy(dbex);
	RETURN_LONG(-1);
}

