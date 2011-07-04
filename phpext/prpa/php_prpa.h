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

#ifndef PHP_PRPA_H
#define PHP_PRPA_H

extern zend_module_entry prpa_module_entry;
#define phpext_prpa_ptr &prpa_module_entry

#ifdef PHP_WIN32
#	define PHP_PRPA_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_PRPA_API __attribute__ ((visibility("default")))
#else
#	define PHP_PRPA_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(prpa);
PHP_MSHUTDOWN_FUNCTION(prpa);
PHP_RINIT_FUNCTION(prpa);
PHP_RSHUTDOWN_FUNCTION(prpa);
PHP_MINFO_FUNCTION(prpa);
PHP_FUNCTION(rpaparse);
PHP_FUNCTION(rpascan);
PHP_FUNCTION(rpamatch);
PHP_FUNCTION(rpa_dbex_version);
PHP_FUNCTION(rpa_dbex_create);
PHP_FUNCTION(rpa_dbex_open);
PHP_FUNCTION(rpa_dbex_close);
PHP_FUNCTION(rpa_dbex_load);
PHP_FUNCTION(rpa_dbex_compile);
PHP_FUNCTION(rpa_dbex_lookup);
PHP_FUNCTION(rpa_dbex_first);
PHP_FUNCTION(rpa_dbex_last);
PHP_FUNCTION(rpa_dbex_cfgset);
PHP_FUNCTION(rpa_dbex_dumpproductions);
PHP_FUNCTION(rpa_dbex_dumptree);
PHP_FUNCTION(rpa_dbex_error);
PHP_FUNCTION(rpa_stat_create);
PHP_FUNCTION(rpa_stat_scan);
PHP_FUNCTION(rpa_stat_match);
PHP_FUNCTION(rpa_stat_parse);
PHP_FUNCTION(rpa_stat_error);


/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(prpa)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(prpa)
*/

/* In every utility function you add that needs to use variables 
   in php_prpa_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as PRPA_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define PRPA_G(v) TSRMG(prpa_globals_id, zend_prpa_globals *, v)
#else
#define PRPA_G(v) (prpa_globals.v)
#endif

#endif	/* PHP_PRPA_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
