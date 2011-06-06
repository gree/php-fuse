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
#include "php_fuse.h"

#if HAVE_FUSE

#define MAKE_LONG_ZVAL(name, val)	MAKE_STD_ZVAL(name); ZVAL_LONG(name, val);

/* {{{ fuse_functions[] */
function_entry fuse_functions[] = {
	{ NULL, NULL, NULL }
};
/* }}} */

ZEND_DECLARE_MODULE_GLOBALS(fuse)

/* {{{ fuse_module_entry
 */
zend_module_entry fuse_module_entry = {
	STANDARD_MODULE_HEADER,
	"fuse",
	fuse_functions,
	PHP_MINIT(fuse),     /* Replace with NULL if there is nothing to do at php startup   */ 
	PHP_MSHUTDOWN(fuse), /* Replace with NULL if there is nothing to do at php shutdown  */
	PHP_RINIT(fuse),     /* Replace with NULL if there is nothing to do at request start */
	PHP_RSHUTDOWN(fuse), /* Replace with NULL if there is nothing to do at request end   */
	PHP_MINFO(fuse),
	"0.9.2", 
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_FUSE
ZEND_GET_MODULE(fuse)
#endif

/* {{{ php_fuse_call_method() (copy from zend_call_method - ze takes only 2 parameters...why?) */
#define php_fuse_call_method_with_0_params(obj, obj_ce, fn_proxy, function_name, retval) php_fuse_call_method(obj, obj_ce, fn_proxy, function_name, sizeof(function_name)-1, retval, 0 TSRMLS_CC)
#define php_fuse_call_method_with_1_params(obj, obj_ce, fn_proxy, function_name, retval, arg1) php_fuse_call_method(obj, obj_ce, fn_proxy, function_name, sizeof(function_name)-1, retval, 1 TSRMLS_CC, &arg1)
#define php_fuse_call_method_with_2_params(obj, obj_ce, fn_proxy, function_name, retval, arg1, arg2) php_fuse_call_method(obj, obj_ce, fn_proxy, function_name, sizeof(function_name)-1, retval, 2 TSRMLS_CC, &arg1, &arg2)
#define php_fuse_call_method_with_3_params(obj, obj_ce, fn_proxy, function_name, retval, arg1, arg2, arg3) php_fuse_call_method(obj, obj_ce, fn_proxy, function_name, sizeof(function_name)-1, retval, 3 TSRMLS_CC, &arg1, &arg2, &arg3)
#define php_fuse_call_method_with_4_params(obj, obj_ce, fn_proxy, function_name, retval, arg1, arg2, arg3, arg4) php_fuse_call_method(obj, obj_ce, fn_proxy, function_name, sizeof(function_name)-1, retval, 4 TSRMLS_CC, &arg1, &arg2, &arg3, &arg4)
#define php_fuse_call_method_with_5_params(obj, obj_ce, fn_proxy, function_name, retval, arg1, arg2, arg3, arg4, arg5) php_fuse_call_method(obj, obj_ce, fn_proxy, function_name, sizeof(function_name)-1, retval, 5 TSRMLS_CC, &arg1, &arg2, &arg3, &arg4, &arg5)

static zval* php_fuse_call_method(zval **object_pp, zend_class_entry *obj_ce, zend_function **fn_proxy, char *function_name, int function_name_len, zval **retval_ptr_ptr, int param_count TSRMLS_DC, ...) {
	int result;
	zend_fcall_info fci;
	zval z_fname;
	zval *retval;
	HashTable *function_table;

	int i;
	va_list va_params;
	zval ***params = emalloc(sizeof(zval**) * param_count);

	va_start(va_params, param_count);
	for (i = 0; i < param_count; i++) {
		zval **tmp = va_arg(va_params, zval**);
		*(params+i) = tmp;
	}
	va_end(va_params);

	fci.size = sizeof(fci);
	/*fci.function_table = NULL; will be read form zend_class_entry of object if needed */
#if PHP_VERSION_ID < 50300
	fci.object_pp = object_pp;
#else
	fci.object_ptr = *object_pp;
#endif
	fci.function_name = &z_fname;
	fci.retval_ptr_ptr = retval_ptr_ptr ? retval_ptr_ptr : &retval;
	fci.param_count = param_count;
	fci.params = params;
	fci.no_separation = 1;
	fci.symbol_table = NULL;

	if (!fn_proxy && !obj_ce) {
		/* no interest in caching and no information already present that is
		 * needed later inside zend_call_function. */
		ZVAL_STRINGL(&z_fname, function_name, function_name_len, 0);
		fci.function_table = !object_pp ? EG(function_table) : NULL;
		result = zend_call_function(&fci, NULL TSRMLS_CC);
	} else {
		zend_fcall_info_cache fcic;

		fcic.initialized = 1;
		if (!obj_ce) {
			obj_ce = object_pp ? Z_OBJCE_PP(object_pp) : NULL;
		}
		if (obj_ce) {
			function_table = &obj_ce->function_table;
		} else {
			function_table = EG(function_table);
		}
		if (!fn_proxy || !*fn_proxy) {
			if (zend_hash_find(function_table, function_name, function_name_len+1, (void **) &fcic.function_handler) == FAILURE) {
				/* error at c-level */
				zend_error(E_CORE_ERROR, "Couldn't find implementation for method %s%s%s", obj_ce ? obj_ce->name : "", obj_ce ? "::" : "", function_name);
			}
			if (fn_proxy) {
				*fn_proxy = fcic.function_handler;
			}
		} else {
			fcic.function_handler = *fn_proxy;
		}
		fcic.calling_scope = obj_ce;
#if PHP_VERSION_ID < 50300
		fcic.object_pp = object_pp;
#else
		fcic.object_ptr = *object_pp;
#endif
		result = zend_call_function(&fci, &fcic TSRMLS_CC);
	}
	if (result == FAILURE) {
		/* error at c-level */
		if (!obj_ce) {
			obj_ce = object_pp ? Z_OBJCE_PP(object_pp) : NULL;
		}
		if (!EG(exception)) {
			zend_error(E_CORE_ERROR, "Couldn't execute method %s%s%s", obj_ce ? obj_ce->name : "", obj_ce ? "::" : "", function_name);
		}
	}
	efree(params);
	if (!retval_ptr_ptr) {
		if (retval) {
			zval_ptr_dtor(&retval);
		}
		return NULL;
	}
	return *retval_ptr_ptr;
}
/* }}} */

/* {{{ fuse call back functions */
PHP_FUSE_API int php_fuse_getattr(const char * path, struct stat * st) {
	
	zval *active_object = NULL;
	
	pthread_mutex_lock(&m);
	
	active_object = FUSEG(active_object);
	
	if (active_object == NULL) {
		pthread_mutex_unlock(&m);
		return -ENOENT;
	}

	memset(st, 0, sizeof(struct stat));

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_st;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_st);
	array_init(arg_st);

	php_fuse_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "getattr", &retval, arg_path, arg_st);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);

	if (r < 0) {
		zval_ptr_dtor(&arg_st);
		pthread_mutex_unlock(&m);
		return r;
	}

	/* reference retval handling */
	if (Z_TYPE_P(arg_st) != IS_ARRAY) {
		zval_ptr_dtor(&arg_st);
		pthread_mutex_unlock(&m);
		return -ENOENT;
	}

	HashPosition	pos;
	zval			**entry;
	char			*tmp_s_key;
	uint			tmp_s_key_len;
	ulong			tmp_n_key;

	zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(arg_st), &pos);
	while (zend_hash_get_current_data_ex(Z_ARRVAL_P(arg_st), (void **)&entry, &pos) == SUCCESS) {
		convert_to_long_ex(entry);
		int value = Z_LVAL_PP(entry);

		switch (zend_hash_get_current_key_ex(Z_ARRVAL_P(arg_st), &tmp_s_key, &tmp_s_key_len, &tmp_n_key, 0, &pos)) {	// no duplication
		case HASH_KEY_IS_STRING:
			if (strncmp(tmp_s_key, "dev", tmp_s_key_len) == 0) {
				st->st_dev = (dev_t)value;
			} else if (strncmp(tmp_s_key, "ino", tmp_s_key_len) == 0) {
				st->st_ino = (ino_t)value;
			} else if (strncmp(tmp_s_key, "mode", tmp_s_key_len) == 0) {
				st->st_mode = (mode_t)value;
			} else if (strncmp(tmp_s_key, "nlink", tmp_s_key_len) == 0) {
				st->st_nlink = (nlink_t)value;
			} else if (strncmp(tmp_s_key, "uid", tmp_s_key_len) == 0) {
				st->st_uid = (uid_t)value;
			} else if (strncmp(tmp_s_key, "gid", tmp_s_key_len) == 0) {
				st->st_gid = (gid_t)value;
			} else if (strncmp(tmp_s_key, "rdev", tmp_s_key_len) == 0) {
				st->st_rdev = (dev_t)value;
			} else if (strncmp(tmp_s_key, "size", tmp_s_key_len) == 0) {
				st->st_size = (size_t)value;
			} else if (strncmp(tmp_s_key, "atime", tmp_s_key_len) == 0) {
				st->st_atime = (time_t)value;
			} else if (strncmp(tmp_s_key, "mtime", tmp_s_key_len) == 0) {
				st->st_mtime = (time_t)value;
			} else if (strncmp(tmp_s_key, "ctime", tmp_s_key_len) == 0) {
				st->st_ctime = (time_t)value;
			} else if (strncmp(tmp_s_key, "blksize", tmp_s_key_len) == 0) {
				st->st_blksize = value;			// TODO: why? blksize_t is not typedefed in some environment
			} else if (strncmp(tmp_s_key, "blocks", tmp_s_key_len) == 0) {
				st->st_blocks = (blkcnt_t)value;
			}
			break;
		case HASH_KEY_IS_LONG:
			switch (tmp_n_key) {
			case 0:
				st->st_dev = (dev_t)value;
				break;
			case 1:
				st->st_ino = (ino_t)value;
				break;
			case 2:
				st->st_mode = (mode_t)value;
				break;
			case 3:
				st->st_nlink = (nlink_t)value;
				break;
			case 4:
				st->st_uid = (uid_t)value;
				break;
			case 5:
				st->st_gid = (gid_t)value;
				break;
			case 6:
				st->st_rdev = (dev_t)value;
				break;
			case 7:
				st->st_size = (off_t)value;
				break;
			case 8:
				st->st_atime = (time_t)value;
				break;
			case 9:
				st->st_mtime = (time_t)value;
				break;
			case 10:
				st->st_ctime = (time_t)value;
				break;
			case 11:
				st->st_blksize = value;			// TODO: why? blksize_t is not typedefed in some environment
				break;
			case 12:
				st->st_blocks = (blkcnt_t)value;
				break;
			}
			break;
		}
		zend_hash_move_forward_ex(Z_ARRVAL_P(arg_st), &pos);
	}

	zval_ptr_dtor(&arg_st);

	pthread_mutex_unlock(&m);
	
	return r;
}

PHP_FUSE_API int php_fuse_readlink(const char * path, char * buf, size_t buf_len) {
	
	zval *active_object = NULL;
	
	pthread_mutex_lock(&m);
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&m);
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_buf;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_buf);

	php_fuse_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "readlink", &retval, arg_path, arg_buf);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);

	if (r < 0) {
		zval_ptr_dtor(&arg_buf);
		pthread_mutex_unlock(&m);
		return r;
	}

	convert_to_string_ex(&arg_buf);
	strncpy(buf, Z_STRVAL_P(arg_buf), buf_len);
	zval_ptr_dtor(&arg_buf);

	pthread_mutex_unlock(&m);
	
	return r;
}

PHP_FUSE_API int php_fuse_getdir(const char * path, fuse_dirh_t dh, fuse_dirfil_t df) {
	
	zval *active_object = NULL;
	
	pthread_mutex_lock(&m);
		
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&m);
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_list;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_list);
	array_init(arg_list);

	php_fuse_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "getdir", &retval, arg_path, arg_list);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);

	if (r < 0) {
		zval_ptr_dtor(&arg_list);
		pthread_mutex_unlock(&m);
		return r;
	}

	/* reference retval handling */
	if (Z_TYPE_P(arg_list) != IS_ARRAY) {
		zval_ptr_dtor(&arg_list);
		pthread_mutex_unlock(&m);
		return -ENOENT;
	}

	HashPosition	pos;
	zval			**entry;
	char			*tmp_s_key;
	uint			tmp_s_key_len;
	ulong			tmp_n_key;

	zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(arg_list), &pos);
	while (zend_hash_get_current_data_ex(Z_ARRVAL_P(arg_list), (void **)&entry, &pos) == SUCCESS) {
		int r = zend_hash_get_current_key_ex(Z_ARRVAL_P(arg_list), &tmp_s_key, &tmp_s_key_len, &tmp_n_key, 0, &pos);
		char buf[1024];
		if (r == HASH_KEY_IS_LONG) {
			snprintf(buf, sizeof(buf), "%d", tmp_n_key);
		}
		convert_to_array_ex(entry);

		zval **tmp_type;
		zval **tmp_ino;
		int type = DT_UNKNOWN;
		int ino = 0;
		if (zend_hash_find(Z_ARRVAL_PP(entry), "type", sizeof("type"), (void**)&tmp_type) == SUCCESS) {
			type = Z_LVAL_PP(tmp_type);
			zval_ptr_dtor(tmp_type);
		}
		if (zend_hash_find(Z_ARRVAL_PP(entry), "ino", sizeof("ino"), (void**)&tmp_ino) == SUCCESS) {
			ino = Z_LVAL_PP(tmp_ino);
			zval_ptr_dtor(tmp_ino);
		}

		df(dh, r == HASH_KEY_IS_LONG ? buf : tmp_s_key, type, ino);
		zend_hash_move_forward_ex(Z_ARRVAL_P(arg_list), &pos);
	}
	zval_ptr_dtor(&arg_list);

	pthread_mutex_unlock(&m);
	
	return r;
}

PHP_FUSE_API int php_fuse_mknod(const char * path, mode_t mode, dev_t dev) {
	
	zval *active_object = NULL;
	
	pthread_mutex_lock(&m);
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&m);
		return -EINVAL;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_mode;
	zval *arg_dev;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_mode);
	ZVAL_LONG(arg_mode, mode);

	MAKE_STD_ZVAL(arg_dev);
	ZVAL_LONG(arg_dev, dev);

	php_fuse_call_method_with_3_params(&active_object, Z_OBJCE_P(active_object), NULL, "mknod", &retval, arg_path, arg_mode, arg_dev);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_mode);
	zval_ptr_dtor(&arg_dev);

	pthread_mutex_unlock(&m);
	return r;
}

PHP_FUSE_API int php_fuse_mkdir(const char * path, mode_t mode) {
	
	zval *active_object = NULL;
	
	pthread_mutex_lock(&m);
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&m);
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_mode;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_mode);
	ZVAL_LONG(arg_mode, mode);

	php_fuse_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "mkdir", &retval, arg_path, arg_mode);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_mode);

	pthread_mutex_unlock(&m);
	
	return r;
}

PHP_FUSE_API int php_fuse_unlink(const char * path) {
	
	zval *active_object = NULL;
	
	pthread_mutex_lock(&m);
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&m);
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	php_fuse_call_method_with_1_params(&active_object, Z_OBJCE_P(active_object), NULL, "unlink", &retval, arg_path);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);

	pthread_mutex_unlock(&m);
	
	return r;
}

PHP_FUSE_API int php_fuse_rmdir(const char * path) {
	
	zval *active_object = NULL;
	
	pthread_mutex_lock(&m);
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&m);
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	php_fuse_call_method_with_1_params(&active_object, Z_OBJCE_P(active_object), NULL, "rmdir", &retval, arg_path);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);

	pthread_mutex_unlock(&m);
	
	return r;
}

PHP_FUSE_API int php_fuse_symlink(const char * path_from, const char * path_to) {
	
	zval *active_object = NULL;
	
	pthread_mutex_lock(&m);
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&m);
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path_from;
	zval *arg_path_to;
	char *p = estrdup(path_from);
	char *q = estrdup(path_to);

	MAKE_STD_ZVAL(arg_path_from);
	ZVAL_STRING(arg_path_from, p, 0);

	MAKE_STD_ZVAL(arg_path_to);
	ZVAL_STRING(arg_path_to, q, 0);

	php_fuse_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "symlink", &retval, arg_path_from, arg_path_to);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path_from);
	zval_ptr_dtor(&arg_path_to);
	
	pthread_mutex_unlock(&m);
	
	return r;
}

PHP_FUSE_API int php_fuse_rename(const char * path_from, const char * path_to) {
	
	zval *active_object = NULL;
	
	pthread_mutex_lock(&m);
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&m);
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path_from;
	zval *arg_path_to;
	char *p = estrdup(path_from);
	char *q = estrdup(path_to);

	MAKE_STD_ZVAL(arg_path_from);
	ZVAL_STRING(arg_path_from, p, 0);

	MAKE_STD_ZVAL(arg_path_to);
	ZVAL_STRING(arg_path_to, q, 0);

	php_fuse_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "rename", &retval, arg_path_from, arg_path_to);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path_from);
	zval_ptr_dtor(&arg_path_to);

	pthread_mutex_unlock(&m);
	
	return r;
}

PHP_FUSE_API int php_fuse_link(const char * path_from, const char * path_to) {
	
	zval *active_object = NULL;
	
	pthread_mutex_lock(&m);
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&m);
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path_from;
	zval *arg_path_to;
	char *p = estrdup(path_from);
	char *q = estrdup(path_to);

	MAKE_STD_ZVAL(arg_path_from);
	ZVAL_STRING(arg_path_from, p, 0);

	MAKE_STD_ZVAL(arg_path_to);
	ZVAL_STRING(arg_path_to, q, 0);

	php_fuse_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "link", &retval, arg_path_from, arg_path_to);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path_from);
	zval_ptr_dtor(&arg_path_to);
	
	pthread_mutex_unlock(&m);
	
	return r;
}

PHP_FUSE_API int php_fuse_chmod(const char * path, mode_t mode) {
	
	zval *active_object = NULL;
	
	pthread_mutex_lock(&m);
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&m);
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_mode;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_mode);
	ZVAL_LONG(arg_mode, mode);

	php_fuse_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "chmod", &retval, arg_path, arg_mode);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_mode);

	pthread_mutex_unlock(&m);
	
	return r;
}

PHP_FUSE_API int php_fuse_chown(const char * path, uid_t uid, gid_t gid) {
	
	zval *active_object = NULL;
	
	pthread_mutex_lock(&m);
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&m);
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_uid;
	zval *arg_gid;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_uid);
	ZVAL_LONG(arg_uid, uid);

	MAKE_STD_ZVAL(arg_gid);
	ZVAL_LONG(arg_gid, gid);

	php_fuse_call_method_with_3_params(&active_object, Z_OBJCE_P(active_object), NULL, "chown", &retval, arg_path, arg_uid, arg_gid);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_uid);
	zval_ptr_dtor(&arg_gid);
	
	pthread_mutex_unlock(&m);
	
	return r;
}

PHP_FUSE_API int php_fuse_truncate(const char * path, off_t offset) {
	
	
	zval *active_object = NULL;
	
	pthread_mutex_lock(&m);
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&m);
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_offset;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_offset);
	ZVAL_LONG(arg_offset, offset);

	php_fuse_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "truncate", &retval, arg_path, arg_offset);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_offset);

	pthread_mutex_unlock(&m);
	
	return r;
}

PHP_FUSE_API int php_fuse_utime(const char * path, struct utimbuf * buf) {
	
	zval *active_object = NULL;
	
	pthread_mutex_lock(&m);
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&m);
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_atime;
	zval *arg_mtime;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_atime);
	ZVAL_LONG(arg_atime, buf->actime);

	MAKE_STD_ZVAL(arg_mtime);
	ZVAL_LONG(arg_mtime, buf->modtime);

	php_fuse_call_method_with_3_params(&active_object, Z_OBJCE_P(active_object), NULL, "utime", &retval, arg_path, arg_atime, arg_mtime);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_atime);
	zval_ptr_dtor(&arg_mtime);

	pthread_mutex_unlock(&m);
	
	return r;
}

PHP_FUSE_API int php_fuse_open(const char * path, struct fuse_file_info * fi) {
	
	zval *active_object = NULL;
	
	pthread_mutex_lock(&m);
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&m);
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_mode;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_mode);
	ZVAL_LONG(arg_mode, fi->flags);

	zend_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "open", &retval, arg_path, arg_mode);

	convert_to_long_ex(&retval);
	long r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);
	
	if (r >= 0) {
		fi->fh = (ulong)r;
	}

	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_mode);

	pthread_mutex_unlock(&m);
	
	return r >= 0 ? 0 : r;
}

PHP_FUSE_API int php_fuse_read(const char * path, char * buf, size_t buf_len, off_t offset, struct fuse_file_info * fi) {
	
	zval *active_object = NULL;
	
	pthread_mutex_lock(&m);
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&m);
		return -EBADF;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_fh;
	zval *arg_offset;
	zval *arg_buf_len;
	zval *arg_buf;
	char *p = estrdup(path);
	char *mybuf = estrdup(buf);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_fh);
	ZVAL_LONG(arg_fh, fi ? fi->fh : 0);

	MAKE_STD_ZVAL(arg_offset);
	ZVAL_LONG(arg_offset, offset);

	MAKE_STD_ZVAL(arg_buf_len);
	ZVAL_LONG(arg_buf_len, buf_len);

	MAKE_STD_ZVAL(arg_buf);
	ZVAL_STRING(arg_buf, mybuf, 0);

	php_fuse_call_method_with_5_params(&active_object, Z_OBJCE_P(active_object), NULL, "read", &retval, arg_path, arg_fh, arg_offset, arg_buf_len, arg_buf);

	convert_to_long_ex(&retval);
	long r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);
	
	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_fh);
	zval_ptr_dtor(&arg_offset);
	zval_ptr_dtor(&arg_buf_len);

	if (r < 0) {
		zval_ptr_dtor(&arg_buf);
		pthread_mutex_unlock(&m);
		return r;
	}
	convert_to_string_ex(&arg_buf);
	if (Z_STRVAL_P(arg_buf) == NULL) {
		zval_ptr_dtor(&arg_buf);
		pthread_mutex_unlock(&m);
		return 0;
	}

	r = Z_STRLEN_P(arg_buf);
	memcpy(buf, Z_STRVAL_P(arg_buf), r > buf_len ? buf_len : r);
	zval_ptr_dtor(&arg_buf);

	pthread_mutex_unlock(&m);
	
	return r > buf_len ? buf_len : r;
}

PHP_FUSE_API int php_fuse_write(const char * path, const char * buf, size_t buf_len, off_t offset, struct fuse_file_info * fi) {
	
	zval *active_object = NULL;
	
	pthread_mutex_lock(&m);
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&m);
		return -EBADF;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_fh;
	zval *arg_buf;
	zval *arg_offset;
	char *p = estrdup(path);
	char *q = emalloc(buf_len);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	memcpy(q, buf, buf_len);
	MAKE_STD_ZVAL(arg_buf);
	ZVAL_STRINGL(arg_buf, q, buf_len, 0);

	MAKE_STD_ZVAL(arg_fh);
	if ( fi == 0 )
	{
		ZVAL_LONG(arg_fh, 0);
	}
	else
	{
		ZVAL_LONG(arg_fh, fi->fh);
	}

	MAKE_STD_ZVAL(arg_offset);
	ZVAL_LONG(arg_offset, offset);

	php_fuse_call_method_with_4_params(&active_object, Z_OBJCE_P(active_object), NULL, "write", &retval, arg_path, arg_fh, arg_offset, arg_buf);

	convert_to_long_ex(&retval);
	long r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);
	
	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_fh);
	zval_ptr_dtor(&arg_offset);
	zval_ptr_dtor(&arg_buf);

	pthread_mutex_unlock(&m);
	
	return r;
}

PHP_FUSE_API int php_fuse_statfs(const char * path, struct statfs * st) {
	
	zval *active_object = NULL;
	
	pthread_mutex_lock(&m);
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&m);
		return -EFAULT;
	}

	memset(st, 0, sizeof(struct statfs));

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_st;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_st);
	array_init(arg_st);

	php_fuse_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "statfs", &retval, arg_path, arg_st);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);

	if (r < 0) {
		zval_ptr_dtor(&arg_st);
		pthread_mutex_unlock(&m);
		return r;
	}

	/* reference retval handling */
	if (Z_TYPE_P(arg_st) != IS_ARRAY) {
		zval_ptr_dtor(&arg_st);
		pthread_mutex_unlock(&m);
		return -EFAULT;
	}

	HashPosition	pos;
	zval			**entry;
	char			*tmp_s_key;
	uint			tmp_s_key_len;
	ulong			tmp_n_key;

	zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(arg_st), &pos);
	while (zend_hash_get_current_data_ex(Z_ARRVAL_P(arg_st), (void **)&entry, &pos) == SUCCESS) {
		convert_to_long_ex(entry);
		int value = Z_LVAL_PP(entry);

		switch (zend_hash_get_current_key_ex(Z_ARRVAL_P(arg_st), &tmp_s_key, &tmp_s_key_len, &tmp_n_key, 0, &pos)) {	// no duplication
		case HASH_KEY_IS_STRING:
			if (strncmp(tmp_s_key, "type", tmp_s_key_len) == 0) {
				st->f_type = value;
			} else if (strncmp(tmp_s_key, "bsize", tmp_s_key_len) == 0) {
				st->f_bsize = value;
			} else if (strncmp(tmp_s_key, "blocks", tmp_s_key_len) == 0) {
				st->f_blocks = value;
			} else if (strncmp(tmp_s_key, "bfree", tmp_s_key_len) == 0) {
				st->f_bfree = value;
			} else if (strncmp(tmp_s_key, "bavail", tmp_s_key_len) == 0) {
				st->f_bavail = value;
			} else if (strncmp(tmp_s_key, "files", tmp_s_key_len) == 0) {
				st->f_files = value;
			} else if (strncmp(tmp_s_key, "ffree", tmp_s_key_len) == 0) {
				st->f_ffree = value;
			} else if (strncmp(tmp_s_key, "fsid", tmp_s_key_len) == 0) {
				// not yet supported
			} else if (strncmp(tmp_s_key, "namelen", tmp_s_key_len) == 0) {
				st->f_namelen = value;
			}
			break;
		case HASH_KEY_IS_LONG:
			break;
		}
		zend_hash_move_forward_ex(Z_ARRVAL_P(arg_st), &pos);
	}

	zval_ptr_dtor(&arg_st);

	pthread_mutex_unlock(&m);
	
	return r;
}

PHP_FUSE_API int php_fuse_flush(const char * path, struct fuse_file_info * fi) {
	
	zval *active_object = NULL;
	
	pthread_mutex_lock(&m);
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&m);
		return -EBADF;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_fh;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_fh);
	ZVAL_LONG(arg_fh, fi->fh);

	php_fuse_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "flush", &retval, arg_path, arg_fh);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_fh);

	pthread_mutex_unlock(&m);
	
	return r;
}

PHP_FUSE_API int php_fuse_release(const char * path, struct fuse_file_info * fi) {
	
	zval *active_object = NULL;
	
	pthread_mutex_lock(&m);
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&m);
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_fh;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_fh);
	ZVAL_LONG(arg_fh, fi->fh);

	php_fuse_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "release", &retval, arg_path, arg_fh);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_fh);

	pthread_mutex_unlock(&m);
	
	return r;
}

PHP_FUSE_API int php_fuse_fsync(const char * path, int mode, struct fuse_file_info * fi) {
	
	zval *active_object = NULL;
	
	pthread_mutex_lock(&m);
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&m);
		return -EBADF;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_fh;
	zval *arg_mode;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_fh);
	ZVAL_LONG(arg_fh, fi->fh);

	MAKE_STD_ZVAL(arg_mode);
	ZVAL_LONG(arg_mode, mode);

	php_fuse_call_method_with_3_params(&active_object, Z_OBJCE_P(active_object), NULL, "fsync", &retval, arg_path, arg_fh, arg_mode);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_fh);
	zval_ptr_dtor(&arg_mode);

	pthread_mutex_unlock(&m);
	
	return r;
}

PHP_FUSE_API int php_fuse_setxattr(const char * path, const char * name, const char * value, size_t value_len, int flag) {
	
	zval *active_object = NULL;
	
	pthread_mutex_lock(&m);
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&m);
		return -ENOSPC;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_name;
	zval *arg_value;
	zval *arg_mode;
	char *p = estrdup(path);
	char *q = estrdup(name);
	char *v = emalloc(value_len);
	memcpy(v, value, value_len);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_name);
	ZVAL_STRING(arg_name, q, 0);

	MAKE_STD_ZVAL(arg_value);
	ZVAL_STRINGL(arg_value, v, value_len, 0);

	MAKE_STD_ZVAL(arg_mode);
	ZVAL_LONG(arg_mode, flag);

	php_fuse_call_method_with_4_params(&active_object, Z_OBJCE_P(active_object), NULL, "setxattr", &retval, arg_path, arg_name, arg_value, arg_mode);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_name);
	zval_ptr_dtor(&arg_value);
	zval_ptr_dtor(&arg_mode);

	pthread_mutex_unlock(&m);
	
	return r;
}

PHP_FUSE_API int php_fuse_getxattr(const char * path, const char * name, char * value, size_t value_len) {
	
	zval *active_object = NULL;
	
	pthread_mutex_lock(&m);
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&m);
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_name;
	zval *arg_value;
	char *p = estrdup(path);
	char *q = estrdup(name);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_name);
	ZVAL_STRING(arg_name, q, 0);

	MAKE_STD_ZVAL(arg_value);

	php_fuse_call_method_with_3_params(&active_object, Z_OBJCE_P(active_object), NULL, "getxattr", &retval, arg_path, arg_name, arg_value);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);
	zval_ptr_dtor(&arg_name);

	if (r < 0) {
		zval_ptr_dtor(&arg_value);
		pthread_mutex_unlock(&m);
		return r;
	}

	convert_to_string_ex(&arg_value);
	memcpy(value, Z_STRVAL_P(arg_value), r > value_len ? value_len : r);
	zval_ptr_dtor(&arg_value);

	pthread_mutex_unlock(&m);
	
	return r > value_len ? value_len : r;
}

PHP_FUSE_API int php_fuse_listxattr(const char * path, char * list, size_t list_len) {
	
	zval *active_object = NULL;
	
	pthread_mutex_lock(&m);
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&m);
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_list;
	char *p = estrdup(path);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_list);
	array_init(arg_list);

	php_fuse_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "listxattr", &retval, arg_path, arg_list);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_path);

	if (r < 0) {
		zval_ptr_dtor(&arg_list);
		pthread_mutex_unlock(&m);
		return r;
	}

	if (Z_TYPE_P(arg_list) != IS_ARRAY) {
		convert_to_string_ex(&arg_list);
		memcpy(list, Z_STRVAL_P(arg_list), r > list_len ? list_len : r);
		zval_ptr_dtor(&arg_list);
	} else {
		HashPosition	pos;
		zval			**entry;

		zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(arg_list), &pos);
		char *q = list;
		memset(list, 0, list_len);
		while (zend_hash_get_current_data_ex(Z_ARRVAL_P(arg_list), (void **)&entry, &pos) == SUCCESS) {
			convert_to_string_ex(entry);
			if ((q-list) + Z_STRLEN_PP(entry) >= list_len) {
				break;
			}
			memcpy(q, Z_STRVAL_PP(entry), Z_STRLEN_PP(entry));
			q += Z_STRLEN_PP(entry);
			if ((q-list) + 1 < list_len) {
				break;
			}
			*q++ = ' ';

			zend_hash_move_forward_ex(Z_ARRVAL_P(arg_list), &pos);
		}
	}

	pthread_mutex_unlock(&m);
	
	return r;
}

PHP_FUSE_API int php_fuse_removexattr(const char * path, const char * name) {
	
	zval *active_object = NULL;
	
	pthread_mutex_lock(&m);
	
	active_object = FUSEG(active_object);
	if (active_object == NULL) {
		pthread_mutex_unlock(&m);
		return -ENOENT;
	}

	/* handler call */
	zval *retval;
	zval *arg_path;
	zval *arg_name;
	char *p = estrdup(path);
	char *q = estrdup(name);

	MAKE_STD_ZVAL(arg_path);
	ZVAL_STRING(arg_path, p, 0);

	MAKE_STD_ZVAL(arg_name);
	ZVAL_STRING(arg_name, q, 0);

	php_fuse_call_method_with_2_params(&active_object, Z_OBJCE_P(active_object), NULL, "removexattr", &retval, arg_path, arg_name);

	convert_to_long_ex(&retval);
	int r = Z_LVAL_P(retval);
	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&arg_name);

	pthread_mutex_unlock(&m);
	
	return r;
}

/*
PHP_FUSE_API int php_fuse_opendir(const char *, struct fuse_file_info *);
PHP_FUSE_API int php_fuse_readdir(const char *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info *);
PHP_FUSE_API int php_fuse_releasedir(const char *, struct fuse_file_info *);
PHP_FUSE_API int php_fuse_fsyncdir(const char *, int, struct fuse_file_info *);
PHP_FUSE_API void* php_fuse_init(struct fuse_conn_info *conn);
PHP_FUSE_API void php_fuse_destroy(void *);
PHP_FUSE_API int php_fuse_access(const char *, int);
PHP_FUSE_API int php_fuse_create(const char *, mode_t, struct fuse_file_info *);
PHP_FUSE_API int php_fuse_ftruncate(const char *, off_t, struct fuse_file_info *);
PHP_FUSE_API int php_fuse_fgetattr(const char *, struct stat *, struct fuse_file_info *);
PHP_FUSE_API int php_fuse_lock(const char *, struct fuse_file_info *, int cmd, struct flock *);
PHP_FUSE_API int php_fuse_utimens(const char *, const struct timespec tv[2]);
PHP_FUSE_API int php_fuse_bmap(const char *, size_t blocksize, uint64_t *idx);
*/
/* }}} */

/* {{{ fuse static vars */
static char * php_fuse_method_list[] = {
	"getattr",
	"readlink",
	"getdir",
	"mknod",
	"mkdir",
	"unlink",
	"rmdir",
	"symlink",
	"rename",
	"link",
	"chmod",
	"chown",
	"truncate",
	"utime",
	"open",
	"read",
	"write",
	"statfs",
	"flush",
	"release",
	"fsync",
	"setxattr",
	"getxattr",
	"listxattr",
	"removexattr",
};
static struct fuse_operations php_fuse_operations = {
	.getattr		= php_fuse_getattr,
	.readlink		= php_fuse_readlink,
	.getdir			= php_fuse_getdir,
	.mknod			= php_fuse_mknod,
	.mkdir			= php_fuse_mkdir,
	.unlink			= php_fuse_unlink,
	.rmdir			= php_fuse_rmdir,
	.symlink		= php_fuse_symlink,
	.rename			= php_fuse_rename,
	.link			= php_fuse_link,
	.chmod			= php_fuse_chmod,
	.chown			= php_fuse_chown,
	.truncate		= php_fuse_truncate,
	.utime			= php_fuse_utime,
	.open			= php_fuse_open,
	.read			= php_fuse_read,
	.write			= php_fuse_write,
	.statfs			= php_fuse_statfs,
	.flush			= php_fuse_flush,
	.release		= php_fuse_release,
	.fsync			= php_fuse_fsync,
	.setxattr		= php_fuse_setxattr,
	.getxattr		= php_fuse_getxattr,
	.listxattr		= php_fuse_listxattr,
	.removexattr	= php_fuse_removexattr,
};

static zend_class_entry *php_fuse_ce;
static zend_object_handlers php_fuse_object_handlers;
static zend_object_handlers php_fuse_object_handlers_ze1;
/* }}} */

/* {{{ fuse object handlers */
static void php_fuse_object_free_storage(void *object TSRMLS_DC) {
	php_fuse_object *intern = (php_fuse_object*)object;
	
	zend_object_std_dtor(&intern->zo TSRMLS_CC);
	efree(object);
}

static zend_object_value php_fuse_object_handler_new(zend_class_entry *ce TSRMLS_DC) {
	zend_object_value retval;
	php_fuse_object *intern;
	zval *tmp;

	intern = emalloc(sizeof(php_fuse_object));
	memset(intern, 0, sizeof(php_fuse_object));

	zend_object_std_init(&intern->zo, ce TSRMLS_CC);
	zend_hash_copy(intern->zo.properties, &ce->default_properties, (copy_ctor_func_t)zval_add_ref, (void*)&tmp, sizeof(zval*));

	retval.handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t)php_fuse_object_free_storage, NULL TSRMLS_CC);
#if PHP_VERSION_ID < 50300
	retval.handlers = EG(ze1_compatibility_mode) ? &php_fuse_object_handlers_ze1 : &php_fuse_object_handlers;
#else
	retval.handlers = &php_fuse_object_handlers;
#endif

	return retval;
}
/* }}} */

/* {{{ fuse method */
static PHP_METHOD(Fuse, fuse_constructor) {
	zval *object = getThis();

	return;
}

static PHP_METHOD(Fuse, mount) {
	zval *object = getThis();

	const char *path = NULL;
	int path_len = 0;
	const char *option = NULL;
	int option_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &path, &path_len, &option, &option_len) == FAILURE) {
		return;
	}

	FUSEG(active_object) = object;

	// currently no mount options are supported:(
	int argc = 2;
	if (option && option_len > 0) {
		argc += 2;
	}
	char **argv = emalloc(sizeof(char*)*argc);
	char *p = estrdup(path);
	char *q = estrdup(option);

	int i = 0;
	*(argv + i++) = "php_fuse";
	if (option && option_len > 0) {
		*(argv + i++) = "-o";
		*(argv + i++) = q;
	}
	*(argv + i++) = p;

	struct fuse_operations op;
	memset(&op, 0, sizeof(struct fuse_operations));
	zend_class_entry *object_ce = Z_OBJCE_P(object);
	if (!object_ce) {
		RETURN_FALSE;
	}
	HashTable *object_ft = &object_ce->function_table;

	for (i = 0; i < sizeof(php_fuse_method_list) / sizeof(char*); i++) {
		void *tmp;
		if (zend_hash_find(object_ft, php_fuse_method_list[i], strlen(php_fuse_method_list[i])+1, &tmp) == FAILURE) {
			continue;
		}

		/* void** way could cause corruption...:( */
		switch (i) {
		case 0:
			op.getattr = php_fuse_operations.getattr; break;
		case 1:
			op.readlink = php_fuse_operations.readlink; break;
		case 2:
			op.getdir = php_fuse_operations.getdir; break;
		case 3:
			op.mknod = php_fuse_operations.mknod; break;
		case 4:
			op.mkdir = php_fuse_operations.mkdir; break;
		case 5:
			op.unlink = php_fuse_operations.unlink; break;
		case 6:
			op.rmdir = php_fuse_operations.rmdir; break;
		case 7:
			op.symlink = php_fuse_operations.symlink; break;
		case 8:
			op.rename = php_fuse_operations.rename; break;
		case 9:
			op.link = php_fuse_operations.link; break;
		case 10:
			op.chmod = php_fuse_operations.chmod; break;
		case 11:
			op.chown = php_fuse_operations.chown; break;
		case 12:
			op.truncate = php_fuse_operations.truncate; break;
		case 13:
			op.utime = php_fuse_operations.utime; break;
		case 14:
			op.open = php_fuse_operations.open; break;
		case 15:
			op.read = php_fuse_operations.read; break;
		case 16:
			op.write = php_fuse_operations.write; break;
		case 17:
			op.statfs = php_fuse_operations.statfs; break;
		case 18:
			op.flush = php_fuse_operations.flush; break;
		case 19:
			op.release = php_fuse_operations.release; break;
		case 20:
			op.fsync = php_fuse_operations.fsync; break;
		case 21:
			op.setxattr = php_fuse_operations.setxattr; break;
		case 22:
			op.getxattr = php_fuse_operations.getxattr; break;
		case 23:
			op.listxattr = php_fuse_operations.listxattr; break;
		case 24:
			op.removexattr = php_fuse_operations.removexattr; break;
		}
	}

	add_property_string(object, "_mount_point", p, 1);

	fuse_main(argc, argv, &op);

	efree(q);
	efree(p);
	efree(argv);

	FUSEG(active_object) = NULL;

	return;
}
/* }}} */

/* {{{ fuse method entries */
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_mount, 0, 0, 1)
	ZEND_ARG_INFO(0, path)				// string
ZEND_END_ARG_INFO()

/*
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_getattr, 1, 0, 2)
	ZEND_ARG_INFO(0, path)				// string
	ZEND_ARG_INFO(1, list_retval)		// [ref] array
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_readlink, 1, 0, 2)
	ZEND_ARG_INFO(0, path)				// string
	ZEND_ARG_INFO(1, str_retval)		// [ref] string
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_getdir, 1, 0, 2)
	ZEND_ARG_INFO(0, path)				// string
	ZEND_ARG_INFO(1, list_retval)		// [ref] array
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_mknod, 0, 0, 2)
	ZEND_ARG_INFO(0, path)				// string
	ZEND_ARG_INFO(0, mode)				// int
	ZEND_ARG_INFO(0, dev)				// int
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_mkdir, 0, 0, 2)
	ZEND_ARG_INFO(0, path)				// string
	ZEND_ARG_INFO(0, mode)				// int
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_unlink, 0, 0, 1)
	ZEND_ARG_INFO(0, path)				// string
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_rmdir, 0, 0, 1)
	ZEND_ARG_INFO(0, path)				// string
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_symlink, 0, 0, 2)
	ZEND_ARG_INFO(0, path_from)			// string
	ZEND_ARG_INFO(0, path_to)			// string
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_rename, 0, 0, 2)
	ZEND_ARG_INFO(0, path_from)			// string
	ZEND_ARG_INFO(0, path_to)			// string
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_link, 0, 0, 2)
	ZEND_ARG_INFO(0, path_from)			// string
	ZEND_ARG_INFO(0, path_to)			// string
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_chmod, 0, 0, 2)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(0, mode)			// int
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_chown, 0, 0, 3)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(0, uid)			// int
	ZEND_ARG_INFO(0, gid)			// int
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_truncate, 0, 0, 2)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(0, len)			// int
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_utime, 0, 0, 2)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(0, buf)			// array
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_open, 0, 0, 2)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(1, mode)			// int
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_read, 1, 0, 5)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(0, handle)		// int
	ZEND_ARG_INFO(0, offset)		// int
	ZEND_ARG_INFO(0, buf_len)			// int
	ZEND_ARG_INFO(1, str_retval)	// string
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_write, 0, 0, 4)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(0, handle)		// int
	ZEND_ARG_INFO(0, offset)		// int
	ZEND_ARG_INFO(0, buf)			// string
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_statfs, 1, 0, 2)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(1, list_retval)	// array
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_flush, 0, 0, 2)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(0, handle)		// int
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_release, 0, 0, 3)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(0, handle)		// int
	ZEND_ARG_INFO(0, flag)			// int
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_fsync, 0, 0, 4)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(0, handle)		// int
	ZEND_ARG_INFO(0, mode)			// int
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_setxattr, 0, 0, 4)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(0, name)			// string
	ZEND_ARG_INFO(0, value)			// string
	ZEND_ARG_INFO(0, mode)			// int
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_getxattr, 1, 0, 3)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(0, name)			// string
	ZEND_ARG_INFO(1, str_retval)	// string
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_listxattr, 1, 0, 2)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(1, list_retval)	// array
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_fuse_removexattr, 0, 0, 2)
	ZEND_ARG_INFO(0, path)			// string
	ZEND_ARG_INFO(0, name)			// string
ZEND_END_ARG_INFO()
*/

zend_function_entry php_fuse_methods[] = {
	ZEND_MALIAS(Fuse,	__construct,	fuse_constructor,	NULL,	ZEND_ACC_PUBLIC)
	PHP_ME(Fuse,	mount,			arginfo_fuse_mount,			ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)

	/*
    PHP_ME(Fuse,	getattr,		arginfo_fuse_getattr,		ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	readlink,		arginfo_fuse_readlink,		ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	getdir,			arginfo_fuse_getdir,		ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	mknod,			arginfo_fuse_mknod,			ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	mkdir,			arginfo_fuse_mkdir,			ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	unlink,			arginfo_fuse_unlink,		ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	rmdir,			arginfo_fuse_rmdir,			ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	symlink,		arginfo_fuse_symlink,		ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	rename,			arginfo_fuse_rename,		ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	link,			arginfo_fuse_link,			ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	chmod,			arginfo_fuse_chmod,			ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	chown,			arginfo_fuse_chown,			ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	truncate,		arginfo_fuse_truncate,		ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	utime,			arginfo_fuse_utime,			ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	open,			arginfo_fuse_open,			ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	read,			arginfo_fuse_read,			ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	write,			arginfo_fuse_write,			ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	statfs,			arginfo_fuse_statfs,		ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	flush,			arginfo_fuse_flush,			ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	release,		arginfo_fuse_release,		ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	fsync,			arginfo_fuse_fsync,			ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	setxattr,		arginfo_fuse_setxattr,		ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	getxattr,		arginfo_fuse_getxattr,		ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	listxattr,		arginfo_fuse_listxattr,		ZEND_ACC_PUBLIC)
    PHP_ME(Fuse,	removexattr,	arginfo_fuse_removexattr,	ZEND_ACC_PUBLIC)
	*/

	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(fuse) {
	zend_class_entry ce;
	pthread_mutexattr_t mutexattr;

	INIT_CLASS_ENTRY(ce, "Fuse", php_fuse_methods); 
	php_fuse_ce = zend_register_internal_class(&ce TSRMLS_CC);
	php_fuse_ce->create_object = php_fuse_object_handler_new;

	memcpy(&php_fuse_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	memcpy(&php_fuse_object_handlers_ze1, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

	/* constants */
	REGISTER_LONG_CONSTANT("FUSE_S_IFMT", S_IFMT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IFSOCK", S_IFSOCK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IFLNK", S_IFLNK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IFREG", S_IFREG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IFBLK", S_IFBLK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IFDIR", S_IFDIR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IFCHR", S_IFCHR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IFIFO", S_IFIFO, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_ISUID", S_ISUID, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_ISGID", S_ISGID, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_ISVTX", S_ISVTX, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IRWXU", S_IRWXU, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IRUSR", S_IRUSR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IWUSR", S_IWUSR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IXUSR", S_IXUSR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IRWXG", S_IRWXG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IRGRP", S_IRGRP, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IWGRP", S_IWGRP, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IXGRP", S_IXGRP, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IRWXO", S_IRWXO, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IROTH", S_IROTH, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IWOTH", S_IWOTH, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_S_IXOTH", S_IXOTH, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("FUSE_EPERM", EPERM, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ENOENT", ENOENT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ESRCH", ESRCH, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EINTR", EINTR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EIO", EIO, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ENXIO", ENXIO, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_E2BIG", E2BIG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ENOEXEC", ENOEXEC, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EBADF", EBADF, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ECHILD", ECHILD, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EAGAIN", EAGAIN, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ENOMEM", ENOMEM, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EACCES", EACCES, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EFAULT", EFAULT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ENOTBLK", ENOTBLK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EBUSY", EBUSY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EEXIST", EEXIST, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EXDEV", EXDEV, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ENODEV", ENODEV, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ENOTDIR", ENOTDIR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EISDIR", EISDIR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EINVAL", EINVAL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ENFILE", ENFILE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EMFILE", EMFILE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ENOTTY", ENOTTY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ETXTBSY", ETXTBSY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EFBIG", EFBIG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ENOSPC", ENOSPC, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ESPIPE", ESPIPE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EROFS", EROFS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EMLINK", EMLINK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EPIPE", EPIPE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_EDOM", EDOM, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_ERANGE", ERANGE, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("FUSE_DT_UKNOWN", DT_UNKNOWN, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_DT_REG", DT_REG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_DT_DIR", DT_DIR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_DT_FIFO", DT_FIFO, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_DT_SOCK", DT_SOCK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_DT_CHR", DT_CHR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_DT_BLK", DT_BLK, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("FUSE_O_RDONLY", O_RDONLY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_O_WRONLY", O_WRONLY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_O_RDWR", O_RDWR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_O_CREAT", O_CREAT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_O_EXCL", O_EXCL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_O_NOCTTY", O_NOCTTY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_O_TRUNC", O_TRUNC, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_O_APPEND", O_APPEND, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_O_NONBLOCK", O_NONBLOCK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_O_NDELAY", O_NDELAY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_O_SYNC", O_SYNC, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_O_FSYNC", O_FSYNC, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("FUSE_XATTR_CREATE", XATTR_CREATE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FUSE_XATTR_REPLACE", XATTR_REPLACE, CONST_CS | CONST_PERSISTENT);

	pthread_mutexattr_settype( &mutexattr, PTHREAD_MUTEX_NORMAL);
	pthread_mutex_init(&m, &mutexattr);
	
	
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION */
PHP_MSHUTDOWN_FUNCTION(fuse) {
	pthread_mutex_destroy(&m);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION */
PHP_RINIT_FUNCTION(fuse) {
	FUSEG(active_object) = NULL;

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION */
PHP_RSHUTDOWN_FUNCTION(fuse) {
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(fuse) {
	php_info_print_box_start(0);
	php_printf("<p>FUSE(File system in USEr space) Bindings for PHP</p>\n");
	php_printf("<p>Version 0.9.2 (2009-10-15)</p>\n");
	php_info_print_box_end();
}
/* }}} */

#endif /* HAVE_FUSE */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
