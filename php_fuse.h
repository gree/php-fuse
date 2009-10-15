/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2007 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.0 of the PHP license,       |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_0.txt.                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Masaki Fujimoto <fujimoto@php.net>                           |
  +----------------------------------------------------------------------+
*/
#ifndef PHP_FUSE_H
#define PHP_FUSE_H

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <php.h>

#ifdef HAVE_FUSE

#include <php_ini.h>
#include <SAPI.h>
#include <ext/standard/info.h>
#include <Zend/zend_extensions.h>
#include <Zend/zend_interfaces.h>
#ifdef  __cplusplus
} // extern "C" 
#endif

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <fcntl.h>
#define	FUSE_USE_VERSION	22
#define	_FILE_OFFSET_BITS	64
#include <pthread.h>
#include <fuse.h>

#ifdef  __cplusplus
extern "C" {
#endif

/* zend_object for php_fuse */
typedef struct _php_fuse_object {
	zend_object		zo;
} php_fuse_object;

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

ZEND_BEGIN_MODULE_GLOBALS(fuse)
	zval			*active_object;
ZEND_END_MODULE_GLOBALS(fuse)

extern zend_module_entry fuse_module_entry;
#define phpext_fuse_ptr &fuse_module_entry

#ifdef PHP_WIN32
#define PHP_FUSE_API __declspec(dllexport)
#else
#define PHP_FUSE_API
#endif

PHP_MINIT_FUNCTION(fuse);
PHP_MSHUTDOWN_FUNCTION(fuse);
PHP_RINIT_FUNCTION(fuse);
PHP_RSHUTDOWN_FUNCTION(fuse);
PHP_MINFO_FUNCTION(fuse);

#ifdef ZTS
#include "TSRM.h"
#endif

#define FREE_RESOURCE(resource) zend_list_delete(Z_LVAL_P(resource))

#define PROP_GET_LONG(name)    Z_LVAL_P(zend_read_property(_this_ce, _this_zval, #name, strlen(#name), 1 TSRMLS_CC))
#define PROP_SET_LONG(name, l) zend_update_property_long(_this_ce, _this_zval, #name, strlen(#name), l TSRMLS_CC)

#define PROP_GET_DOUBLE(name)    Z_DVAL_P(zend_read_property(_this_ce, _this_zval, #name, strlen(#name), 1 TSRMLS_CC))
#define PROP_SET_DOUBLE(name, d) zend_update_property_double(_this_ce, _this_zval, #name, strlen(#name), d TSRMLS_CC)

#define PROP_GET_STRING(name)    Z_STRVAL_P(zend_read_property(_this_ce, _this_zval, #name, strlen(#name), 1 TSRMLS_CC))
#define PROP_GET_STRLEN(name)    Z_STRLEN_P(zend_read_property(_this_ce, _this_zval, #name, strlen(#name), 1 TSRMLS_CC))
#define PROP_SET_STRING(name, s) zend_update_property_string(_this_ce, _this_zval, #name, strlen(#name), s TSRMLS_CC)
#define PROP_SET_STRINGL(name, s, l) zend_update_property_string(_this_ce, _this_zval, #name, strlen(#name), s, l TSRMLS_CC)

#ifdef ZTS
#define FUSEG(v) TSRMG(fuse_globals_id, zend_fuse_globals *, v)
#else
#define FUSEG(v) (fuse_globals.v)
#endif

#ifdef  __cplusplus
} // extern "C" 
#endif

#endif /* PHP_HAVE_FUSE */

#endif /* PHP_FUSE_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
