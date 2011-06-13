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
#include "rpadbex.h"


typedef struct _php_rpa_dbex {
	rpa_dbex_handle hDbex;
	zval *zinput;
#ifdef ZTS
	TSRMLS_D;	
#endif
} php_rpa_dbex;

#define PHP_RPA_DBEX_RES_NAME "php rpa dbex"


typedef struct _php_rpa_pattern {
	rpa_pattern_handle hPattern;
} php_rpa_pattern;

#define PHP_RPA_PATTERN_RES_NAME "php rpa pattern"


typedef struct _php_cbinfo {
//	rpa_dbex_handle hDbex;
	php_rpa_dbex *pPhpDbex;
	char *php_callback;
	zval *userdata;
} php_cbinfo;


static int le_rpa;
static int le_rpa_dbex;
static int le_rpa_pattern;
static void php_rpa_dbex_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC);
static void php_rpa_pattern_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC);

/* If you declare any globals in php_prpa.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(prpa)
*/

/* True global resources - no need for thread safety here */
static int le_prpa;

/* {{{ prpa_functions[]
 *
 * Every user visible function must have an entry in prpa_functions[].
 */
zend_function_entry prpa_functions[] = {
	PHP_FE(confirm_prpa_compiled,	NULL)		/* For testing, remove later. */
    PHP_FE(rpa_dbex_strmatch, NULL)
    PHP_FE(rpa_dbex_create, NULL)
    PHP_FE(rpa_dbex_load_string, NULL)
    PHP_FE(rpa_dbex_load, NULL)
    PHP_FE(rpa_dbex_open, NULL)
    PHP_FE(rpa_dbex_close, NULL)
    PHP_FE(rpa_dbex_set_encoding, NULL)
    PHP_FE(rpa_dbex_match, NULL)
    PHP_FE(rpa_dbex_parse, NULL)
    PHP_FE(rpa_dbex_scan, NULL)
    PHP_FE(rpa_dbex_get_pattern, NULL)
    PHP_FE(rpa_dbex_default_pattern, NULL)
    PHP_FE(rpa_dbex_first_pattern, NULL)
    PHP_FE(rpa_dbex_last_pattern, NULL)
    PHP_FE(rpa_dbex_next_pattern, NULL)
    PHP_FE(rpa_dbex_prev_pattern, NULL)
    PHP_FE(rpa_dbex_pattern_name, NULL)
    PHP_FE(rpa_dbex_pattern_regex, NULL)
    PHP_FE(rpa_dbex_add_callback, NULL)
    PHP_FE(rpa_dbex_version, NULL)
    PHP_FE(rpa_dbex_seversion, NULL)
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
    STD_PHP_INI_ENTRY("prpa.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_prpa_globals, prpa_globals)
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
    le_rpa_pattern = zend_register_list_destructors_ex(php_rpa_pattern_dtor, NULL, PHP_RPA_PATTERN_RES_NAME, module_number);
    REGISTER_LONG_CONSTANT("RPA_ENCODING_BYTE", RPA_ENCODING_BYTE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("RPA_ENCODING_UTF8", RPA_ENCODING_UTF8, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("RPA_ENCODING_ICASE_UTF8", RPA_ENCODING_ICASE_UTF8, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("RPA_ENCODING_UTF16LE", RPA_ENCODING_UTF16LE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("RPA_ENCODING_ICASE_UTF16LE", RPA_ENCODING_ICASE_UTF16LE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("RPA_PATTERN_SINGLE", RPA_PATTERN_SINGLE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("RPA_PATTERN_BEST", RPA_PATTERN_BEST, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("RPA_PATTERN_OR", RPA_PATTERN_OR, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("RPA_REASON_START", RPA_REASON_START, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("RPA_REASON_END", RPA_REASON_END, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("RPA_REASON_MATCHED", RPA_REASON_MATCHED, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("RPA_REASON_ALL", RPA_REASON_ALL, CONST_CS | CONST_PERSISTENT);

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


/* Remove the following function when you have succesfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_prpa_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_prpa_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "prpa", arg);
	RETURN_STRINGL(strg, len, 0);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/




static php_cbinfo *php_cbinfo_create(php_rpa_dbex *pPhpDbex, const char *php_callback, zval *userdata)
{
	php_cbinfo *pCbInfo;

	if ((pCbInfo = emalloc(sizeof(php_cbinfo))) == NULL)
		return NULL;

	pCbInfo->pPhpDbex = pPhpDbex;
	pCbInfo->php_callback = estrdup(php_callback);
	pCbInfo->userdata = userdata;
	if (userdata) {
		ZVAL_ADDREF(userdata);
//		Z_SET_ISREF_P(userdata);
	}
	return pCbInfo;
}


static void php_cbinfo_destroy(php_cbinfo *pCbInfo)
{
	if (!pCbInfo)
		return;
	if (pCbInfo->php_callback)
		efree(pCbInfo->php_callback);
	if (pCbInfo->userdata) {
		if (ZVAL_REFCOUNT(pCbInfo->userdata) == 1) {
			zval_ptr_dtor(&pCbInfo->userdata);
		} else {
			ZVAL_DELREF(pCbInfo->userdata);
		}
	}
	efree(pCbInfo);
}


static void php_do_rpa_dbex_strmatch(INTERNAL_FUNCTION_PARAMETERS, int global) /* {{{ */
{
    /* parameters */
    char             *subject;          /* String to match against */
    char             *regex;            /* Regular expression */
    int               subject_len;
    int               regex_len;
	int	 	  		  ret = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &subject, 
							  &subject_len,
							  &regex,
							  &regex_len) == FAILURE) {
		RETURN_LONG(-1);
    }
/*
	ret = emalloc(subject_len + regex_len + 100);
	php_sprintf(ret, "Hello from rpalib, subject: %s, regex: %s", subject, regex);
    RETURN_STRING(ret, 0);
*/
	ret = rpa_dbex_strmatch(subject, regex);
	RETURN_LONG(ret);
}


/* {{{ proto int rpa_dbex_strmatch(string subject, string pattern)
   Perform a regular expression match */
PHP_FUNCTION(rpa_dbex_strmatch)
{
    php_do_rpa_dbex_strmatch(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}


PHP_FUNCTION(rpa_dbex_greetme)
{
    zval *zname;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &zname) == FAILURE) {
        RETURN_NULL();
    }

    convert_to_string(zname);

    php_printf("Hello ");
    PHPWRITE(Z_STRVAL_P(zname), Z_STRLEN_P(zname));
    php_printf(" ");

    RETURN_TRUE;
}


static void php_rpa_pattern_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    php_rpa_pattern *pPhpPattern = (php_rpa_pattern*)rsrc->ptr;

    if (pPhpPattern) {
        efree(pPhpPattern);
    }
}


static void php_rpa_dbex_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    php_rpa_dbex *pPhpDbex = (php_rpa_dbex*)rsrc->ptr;
	rpa_callback_handle hCallback;
	php_cbinfo *pCbInfo;

    if (pPhpDbex) {
		if (pPhpDbex->hDbex) {
			for (hCallback = rpa_dbex_first_callback(pPhpDbex->hDbex); hCallback; hCallback = rpa_dbex_next_callback(pPhpDbex->hDbex, hCallback)) {
				pCbInfo = (php_cbinfo*)rpa_dbex_callback_userdata(pPhpDbex->hDbex, hCallback);
				php_cbinfo_destroy(pCbInfo);
			}
		}

		rpa_dbex_destroy(pPhpDbex->hDbex);
/*
		zval_dtor(pPhpDbex->zcallbacks);
		Z_DELREF_P(pPhpDbex->zcallbacks);
		Z_DELREF_P(pPhpDbex->zcallbacks);
		php_printf("refcnt: %d\n", Z_REFCOUNT_P(pPhpDbex->zcallbacks));
		efree(pPhpDbex->zcallbacks);
*/
		efree(pPhpDbex);
    }
}


PHP_FUNCTION(rpa_dbex_create)
{
    php_rpa_dbex *pPhpDbex;

/*
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl", &name, &name_len, &age) == FAILURE) {
		RETURN_FALSE;
    }
*/

    pPhpDbex = emalloc(sizeof(php_rpa_dbex));
    pPhpDbex->hDbex = rpa_dbex_create();

/*	
	ALLOC_INIT_ZVAL(pPhpDbex->zcallbacks);
	Z_ADDREF_P(pPhpDbex->zcallbacks);
	array_init(pPhpDbex->zcallbacks);
*/
    ZEND_REGISTER_RESOURCE(return_value, pPhpDbex, le_rpa_dbex);
}


PHP_FUNCTION(rpa_dbex_open)
{
	zval *zres;
    php_rpa_dbex *pPhpDbex;
	int ret;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zres) == FAILURE) {
		RETURN_LONG(-1);
    }

    ZEND_FETCH_RESOURCE(pPhpDbex, php_rpa_dbex*, &zres, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);

	ret = rpa_dbex_open(pPhpDbex->hDbex);
	RETURN_LONG(ret);
}


PHP_FUNCTION(rpa_dbex_close)
{
	zval *zres;
    php_rpa_dbex *pPhpDbex;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zres) == FAILURE) {
		RETURN_LONG(-1);
    }

    ZEND_FETCH_RESOURCE(pPhpDbex, php_rpa_dbex*, &zres, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);

	rpa_dbex_close(pPhpDbex->hDbex);
	RETURN_LONG(0);
}


PHP_FUNCTION(rpa_dbex_load)
{
	zval *zres;
    php_rpa_dbex *pPhpDbex;
	char *patterns;
	int patterns_len;
	int ret;


    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &zres, &patterns, &patterns_len) == FAILURE) {
		RETURN_LONG(-1);
    }

    ZEND_FETCH_RESOURCE(pPhpDbex, php_rpa_dbex*, &zres, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);

    ret = rpa_dbex_load(pPhpDbex->hDbex, patterns, patterns_len);
	RETURN_LONG(ret);
}


PHP_FUNCTION(rpa_dbex_load_string)
{
	zval *zres;
    php_rpa_dbex *pPhpDbex;
	char *patterns;
	int patterns_len;
	int ret;


    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &zres, &patterns, &patterns_len) == FAILURE) {
		RETURN_LONG(-1);
    }

    ZEND_FETCH_RESOURCE(pPhpDbex, php_rpa_dbex*, &zres, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);

    ret = rpa_dbex_load_string(pPhpDbex->hDbex, patterns);
	RETURN_LONG(ret);
}


PHP_FUNCTION(rpa_dbex_set_encoding)
{
	zval *zres;
    php_rpa_dbex *pPhpDbex;
	char *patterns;
	long encoding;
	int patterns_len;
	int ret;


    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &zres, &encoding) == FAILURE) {
		RETURN_LONG(-1);
    }

    ZEND_FETCH_RESOURCE(pPhpDbex, php_rpa_dbex*, &zres, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);

    ret = rpa_dbex_set_encoding(pPhpDbex->hDbex, encoding);
	RETURN_LONG(ret);
}


PHP_FUNCTION(rpa_dbex_match)
{
	zval *zdbex;
	zval *zpattern;
	zval *zinput;
    php_rpa_dbex *pPhpDbex;
    php_rpa_pattern *pPhpPattern;
	char *input;
	int input_len;
	int ret;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rrz", &zdbex, &zpattern, &zinput) == FAILURE) {
		RETURN_LONG(-1);
    }
	input = Z_STRVAL_P(zinput);
	input_len = Z_STRLEN_P(zinput);
    ZEND_FETCH_RESOURCE(pPhpDbex, php_rpa_dbex*, &zdbex, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);
    ZEND_FETCH_RESOURCE(pPhpPattern, php_rpa_pattern*, &zpattern, -1, PHP_RPA_PATTERN_RES_NAME, le_rpa_pattern);
#ifdef ZTS
	pPhpDbex->tsrm_ls = TSRMLS_C;
#endif
	pPhpDbex->zinput = zinput;
    ret = rpa_dbex_match(pPhpDbex->hDbex, pPhpPattern->hPattern, input, input, input + input_len);
	RETURN_LONG(ret);
}


PHP_FUNCTION(rpa_dbex_parse)
{
	zval *zdbex;
	zval *zpattern;
	zval *zinput;
    php_rpa_dbex *pPhpDbex;
    php_rpa_pattern *pPhpPattern;
	char *input;
	int input_len;
	int ret;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rrz", &zdbex, &zpattern, &zinput) == FAILURE) {
		RETURN_LONG(-1);
    }
	input = Z_STRVAL_P(zinput);
	input_len = Z_STRLEN_P(zinput);
    ZEND_FETCH_RESOURCE(pPhpDbex, php_rpa_dbex*, &zdbex, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);
    ZEND_FETCH_RESOURCE(pPhpPattern, php_rpa_pattern*, &zpattern, -1, PHP_RPA_PATTERN_RES_NAME, le_rpa_pattern);
#ifdef ZTS
	pPhpDbex->tsrm_ls = TSRMLS_C;
#endif
	pPhpDbex->zinput = zinput;
    ret = rpa_dbex_parse(pPhpDbex->hDbex, pPhpPattern->hPattern, input, input, input + input_len);
	RETURN_LONG(ret);
}


PHP_FUNCTION(rpa_dbex_scan)
{
	zval *zdbex;
	zval *zpattern;
	zval *zinput;
	zval *zwhere;
    php_rpa_dbex *pPhpDbex;
    php_rpa_pattern *pPhpPattern;
	char *input;
	const char *where = NULL;
	int input_len;
	int ret;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rrzz", &zdbex, &zpattern, &zinput, &zwhere) == FAILURE) {
		RETURN_LONG(-1);
    }
	input = Z_STRVAL_P(zinput);
	input_len = Z_STRLEN_P(zinput);
    ZEND_FETCH_RESOURCE(pPhpDbex, php_rpa_dbex*, &zdbex, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);
    ZEND_FETCH_RESOURCE(pPhpPattern, php_rpa_pattern*, &zpattern, -1, PHP_RPA_PATTERN_RES_NAME, le_rpa_pattern);
#ifdef ZTS
	pPhpDbex->tsrm_ls = TSRMLS_C;
#endif
	pPhpDbex->zinput = zinput;
    ret = rpa_dbex_scan(pPhpDbex->hDbex, pPhpPattern->hPattern, input, input, input + input_len, &where);
	if (ret) {
		ZVAL_LONG(zwhere, where - input);
	} else {
		ZVAL_NULL(zwhere);
	}

	RETURN_LONG(ret);
}


PHP_FUNCTION(rpa_dbex_get_pattern)
{
	zval *zres;
	php_rpa_dbex *pPhpDbex;
	php_rpa_pattern *pPhpPattern;
	char *name;
	int name_len;
	rpa_pattern_handle hPattern;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &zres, &name, &name_len) == FAILURE) {
		RETURN_FALSE;
	}
	ZEND_FETCH_RESOURCE(pPhpDbex, php_rpa_dbex*, &zres, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);
	hPattern = rpa_dbex_get_pattern(pPhpDbex->hDbex, name);
	if (!hPattern)
		RETURN_FALSE;
	pPhpPattern = emalloc(sizeof(php_rpa_pattern));
	pPhpPattern->hPattern = hPattern;
	ZEND_REGISTER_RESOURCE(return_value, pPhpPattern, le_rpa_pattern);
}


PHP_FUNCTION(rpa_dbex_default_pattern)
{
	zval *zres;
	php_rpa_dbex *pPhpDbex;
	php_rpa_pattern *pPhpPattern;
	rpa_pattern_handle hPattern;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zres) == FAILURE) {
		RETURN_FALSE;
	}
	ZEND_FETCH_RESOURCE(pPhpDbex, php_rpa_dbex*, &zres, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);
	hPattern = rpa_dbex_default_pattern(pPhpDbex->hDbex);
	if (!hPattern)
		RETURN_FALSE;
	pPhpPattern = emalloc(sizeof(php_rpa_pattern));
	pPhpPattern->hPattern = hPattern;
	ZEND_REGISTER_RESOURCE(return_value, pPhpPattern, le_rpa_pattern);
}


PHP_FUNCTION(rpa_dbex_first_pattern)
{
	zval *zres;
	php_rpa_dbex *pPhpDbex;
	php_rpa_pattern *pPhpPattern;
	rpa_pattern_handle hPattern;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zres) == FAILURE) {
		RETURN_FALSE;
	}
	ZEND_FETCH_RESOURCE(pPhpDbex, php_rpa_dbex*, &zres, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);
	hPattern = rpa_dbex_first_pattern(pPhpDbex->hDbex);
	if (!hPattern)
		RETURN_FALSE;
	pPhpPattern = emalloc(sizeof(php_rpa_pattern));
	pPhpPattern->hPattern = hPattern;
	ZEND_REGISTER_RESOURCE(return_value, pPhpPattern, le_rpa_pattern);
}


PHP_FUNCTION(rpa_dbex_last_pattern)
{
	zval *zres;
	php_rpa_dbex *pPhpDbex;
	php_rpa_pattern *pPhpPattern;
	rpa_pattern_handle hPattern;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zres) == FAILURE) {
		RETURN_FALSE;
	}
	ZEND_FETCH_RESOURCE(pPhpDbex, php_rpa_dbex*, &zres, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);
	hPattern = rpa_dbex_last_pattern(pPhpDbex->hDbex);
	if (!hPattern)
		RETURN_FALSE;
	pPhpPattern = emalloc(sizeof(php_rpa_pattern));
	pPhpPattern->hPattern = hPattern;
	ZEND_REGISTER_RESOURCE(return_value, pPhpPattern, le_rpa_pattern);
}


PHP_FUNCTION(rpa_dbex_next_pattern)
{
	zval *zresFirst, *zresSecond;
	php_rpa_dbex *pPhpDbex;
	php_rpa_pattern *pPhpPatternCur;
	php_rpa_pattern *pPhpPattern;
	rpa_pattern_handle hPattern;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr", &zresFirst, &zresSecond) == FAILURE) {
		RETURN_FALSE;
	}
	ZEND_FETCH_RESOURCE(pPhpDbex, php_rpa_dbex*, &zresFirst, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);
    ZEND_FETCH_RESOURCE(pPhpPatternCur, php_rpa_pattern*, &zresSecond, -1, PHP_RPA_PATTERN_RES_NAME, le_rpa_pattern);
		
	hPattern = rpa_dbex_next_pattern(pPhpDbex ? pPhpDbex->hDbex : NULL, pPhpPatternCur ? pPhpPatternCur->hPattern : NULL);
	if (!hPattern)
		RETURN_FALSE;
	pPhpPattern = emalloc(sizeof(php_rpa_pattern));
	pPhpPattern->hPattern = hPattern;
	ZEND_REGISTER_RESOURCE(return_value, pPhpPattern, le_rpa_pattern);
}


PHP_FUNCTION(rpa_dbex_prev_pattern)
{
	zval *zresFirst, *zresSecond;
	php_rpa_dbex *pPhpDbex;
	php_rpa_pattern *pPhpPatternCur;
	php_rpa_pattern *pPhpPattern;
	rpa_pattern_handle hPattern;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr", &zresFirst, &zresSecond) == FAILURE) {
		RETURN_FALSE;
	}
	ZEND_FETCH_RESOURCE(pPhpDbex, php_rpa_dbex*, &zresFirst, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);
    ZEND_FETCH_RESOURCE(pPhpPatternCur, php_rpa_pattern*, &zresSecond, -1, PHP_RPA_PATTERN_RES_NAME, le_rpa_pattern);
		
	hPattern = rpa_dbex_prev_pattern(pPhpDbex ? pPhpDbex->hDbex : NULL, pPhpPatternCur ? pPhpPatternCur->hPattern : NULL);
	if (!hPattern)
		RETURN_FALSE;
	pPhpPattern = emalloc(sizeof(php_rpa_pattern));
	pPhpPattern->hPattern = hPattern;
	ZEND_REGISTER_RESOURCE(return_value, pPhpPattern, le_rpa_pattern);
}


PHP_FUNCTION(rpa_dbex_pattern_name)
{
	zval *zresFirst, *zresSecond;
	php_rpa_dbex *pPhpDbex;
	php_rpa_pattern *pPhpPattern;
	const char *name;
	char *ret;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr", &zresFirst, &zresSecond) == FAILURE) {
		RETURN_FALSE;
	}
	ZEND_FETCH_RESOURCE(pPhpDbex, php_rpa_dbex*, &zresFirst, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);
    ZEND_FETCH_RESOURCE(pPhpPattern, php_rpa_pattern*, &zresSecond, -1, PHP_RPA_PATTERN_RES_NAME, le_rpa_pattern);
		
	name = rpa_dbex_pattern_name(pPhpDbex ? pPhpDbex->hDbex : NULL, pPhpPattern ? pPhpPattern->hPattern : NULL);
	if (!name)
		RETURN_FALSE;
	ret = estrdup(name);
	RETURN_STRING(ret, 0);
}


PHP_FUNCTION(rpa_dbex_pattern_regex)
{
	zval *zresFirst, *zresSecond;
	php_rpa_dbex *pPhpDbex;
	php_rpa_pattern *pPhpPattern;
	long int seq = 0;
	const char *regex;
	char *ret;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr|l", &zresFirst, &zresSecond, &seq) == FAILURE) {
		RETURN_FALSE;
	}
	ZEND_FETCH_RESOURCE(pPhpDbex, php_rpa_dbex*, &zresFirst, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);
    ZEND_FETCH_RESOURCE(pPhpPattern, php_rpa_pattern*, &zresSecond, -1, PHP_RPA_PATTERN_RES_NAME, le_rpa_pattern);
		
	regex = rpa_dbex_pattern_regex(pPhpDbex ? pPhpDbex->hDbex : NULL, pPhpPattern ? pPhpPattern->hPattern : NULL, seq);
	if (!regex)
		RETURN_FALSE;
	ret = estrdup(regex);
	RETURN_STRING(ret, 0);
}


static int php_rpa_callback_proxy(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	php_cbinfo *pCbInfo = (php_cbinfo*)userdata;
	zval *retval_ptr = NULL;  /* Function return value */
    zval **args[7];           /* Argument to pass to function */
	zval *zcallback;
	zval *zname;
	zval *zoffset;
	zval *zsize;
	zval *zreason;
	zval *zuserdata;
	zval *zempty;
	int ret = size;
#ifdef ZTS
	TSRMLS_D = pCbInfo->pPhpDbex->tsrm_ls;
#endif

	MAKE_STD_ZVAL(zcallback);
	MAKE_STD_ZVAL(zname);
	MAKE_STD_ZVAL(zoffset);
	MAKE_STD_ZVAL(zsize);
	MAKE_STD_ZVAL(zreason);
	MAKE_STD_ZVAL(zuserdata);
	MAKE_STD_ZVAL(zempty);

	ZVAL_STRING(zcallback, pCbInfo->php_callback, 1);
	ZVAL_LONG(zsize, size);
	ZVAL_LONG(zreason, reason);
	ZVAL_LONG(zempty, 0);
	ZVAL_LONG(zoffset, input - start);
	ZVAL_STRING(zname, name, 1);

	args[0] = &zname;
	if (pCbInfo->userdata) {
		args[1] = &pCbInfo->userdata;
	} else {
		args[1] = &zempty;
	}


	args[2] = &zoffset;
	args[3] = &zsize;
	args[4] = &zreason;
	args[5] = &pCbInfo->pPhpDbex->zinput;


	if (call_user_function_ex(EG(function_table), NULL, zcallback, &retval_ptr, 6, args, 0, NULL TSRMLS_CC) == SUCCESS && retval_ptr) {
		convert_to_long(retval_ptr);
		ret = (int) Z_LVAL_P(retval_ptr);
		zval_ptr_dtor(&retval_ptr);
	} else {
		if (!EG(exception)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to call function: %s()", Z_STRVAL_P(zcallback));
		}
	}

/*
	zval_ptr_dtor(&zcallback);
	zval_ptr_dtor(&zname);
	zval_ptr_dtor(&zsize);
	zval_ptr_dtor(&zreason);
	zval_ptr_dtor(&zuserdata);
	zval_ptr_dtor(&zoffset);
	zval_ptr_dtor(&zempty);
*/

	return ret;
}


PHP_FUNCTION(rpa_dbex_add_callback)
{
	zval *zresDbex, *zresCallback, *zresUserData = NULL;
	php_rpa_dbex *pPhpDbex;
	php_rpa_pattern *pPhpPattern;
	char *name;
	long reason;
	int name_len = 0;
	char *callback;
	int callback_len = 0;
	unsigned long ret = 0;
	php_cbinfo *pCbInfo;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsls|z", &zresDbex, &name, &name_len, &reason, 
							  &callback, &callback_len, &zresUserData) == FAILURE) {
		RETURN_LONG(-1);
	}
	ZEND_FETCH_RESOURCE(pPhpDbex, php_rpa_dbex*, &zresDbex, -1, PHP_RPA_DBEX_RES_NAME, le_rpa_dbex);

	pCbInfo = php_cbinfo_create(pPhpDbex, callback, zresUserData);
	ret = rpa_dbex_add_callback(pPhpDbex ? pPhpDbex->hDbex : NULL, name, reason, php_rpa_callback_proxy, pCbInfo);
	RETURN_LONG(ret);
}


PHP_FUNCTION(rpa_dbex_version)
{
    RETURN_STRING(rpa_dbex_version(), 1);
}


PHP_FUNCTION(rpa_dbex_seversion)
{
    RETURN_STRING(rpa_dbex_seversion(), 1);
}
