#ifndef VAR_DUMP_H
#define VAR_DUMP_H
#include "Zend/zend.h"
#include "Zend/zend_operators.h"
extern "C" {
// this var_dump was taken from PHP internals as a useful debugging tool for
// our swig binding.  Most of it was taken from php-5.3.5/ext/standard/var.c

/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Jani Lehtimäki <jkl@njet.net>                               |
   |          Thies C. Arntzen <thies@thieso.net>                         |
   |          Sascha Schumann <sascha@schumann.cx>                        |
   +----------------------------------------------------------------------+
*/
#define COMMON ""
//(Z_ISREF_PP(struc) ? "&" : "")

void var_dump(zval **struc, int level TSRMLS_DC);

static int php_array_element_dump(zval **zv TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key) /* {{{ */
{
	int level;

	level = va_arg(args, int);

	if (hash_key->nKeyLength == 0) { /* numeric key */
		fprintf(stderr,"%*c[%ld]=>\n", level + 1, ' ', hash_key->h);
	} else { /* string key */
		fprintf(stderr,"%*c[\"", level + 1, ' ');
		PHPWRITE(hash_key->arKey, hash_key->nKeyLength - 1);
		fprintf(stderr,"\"]=>\n");
	}
	var_dump(zv, level + 2 TSRMLS_CC);
	return 0;
}
/* }}} */

static int php_object_property_dump(zval **zv TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key) /* {{{ */
{
	int level;
	char *prop_name, *class_name;

	level = va_arg(args, int);

	// if (hash_key->nKeyLength == 0) { /* numeric key */
	// 	fprintf(stderr,"%*c[%ld]=>\n", level + 1, ' ', hash_key->h);
	// } else { /* string key */
    //     int unmangle = SUCCESS;
	// 	zend_unmangle_property_name(hash_key->arKey, hash_key->nKeyLength - 1, &class_name, &prop_name);
	// 	fprintf(stderr,"%*c[", level + 1, ' ');

	// 	if (class_name && unmangle == SUCCESS) {
	// 		if (class_name[0] == '*') {
	// 			fprintf(stderr,"\"%s\":protected", prop_name);
	// 		} else {
	// 			fprintf(stderr,"\"%s\":\"%s\":private", prop_name, class_name);
	// 		}
	// 	} else {
	// 		fprintf(stderr,"\"");
	// 		PHPWRITE(hash_key->arKey, hash_key->nKeyLength - 1);
	// 		fprintf(stderr,"\"");
	// 	}
	// 	ZEND_PUTS("]=>\n");
	// }
	var_dump(zv, level + 2 TSRMLS_CC);
	return 0;
}
/* }}} */

PHPAPI void var_dump(zval **struc, int level = 0 TSRMLS_DC) /* {{{ */
{
	HashTable *myht;
	char *class_name;
	zend_uint class_name_len;
	int (*php_element_dump_func)(zval** TSRMLS_DC, int, va_list, zend_hash_key*);
	int is_temp;

	if (level > 1) {
		fprintf(stderr,"%*c", level - 1, ' ');
	}

	switch (Z_TYPE_PP(struc)) {
	case IS_BOOL:
		fprintf(stderr,"%sbool(%s)\n", COMMON, Z_LVAL_PP(struc) ? "true" : "false");
		break;
	case IS_NULL:
		fprintf(stderr,"%sNULL\n", COMMON);
		break;
	case IS_LONG:
		fprintf(stderr,"%sint(%ld)\n", COMMON, Z_LVAL_PP(struc));
		break;
	case IS_DOUBLE:
		fprintf(stderr,"%sfloat(%.*G)\n", COMMON, (int) EG(precision), Z_DVAL_PP(struc));
		break;
	case IS_STRING:
		fprintf(stderr,"%sstring(%d) \"", COMMON, Z_STRLEN_PP(struc));
		PHPWRITE(Z_STRVAL_PP(struc), Z_STRLEN_PP(struc));
		PUTS("\"\n");
		break;
	case IS_ARRAY:
		myht = Z_ARRVAL_PP(struc);
		if (++myht->nApplyCount > 1) {
			PUTS("*RECURSION*\n");
			--myht->nApplyCount;
			return;
		}
		fprintf(stderr,"%sarray(%d) {\n", COMMON, zend_hash_num_elements(myht));
		php_element_dump_func = php_array_element_dump;
		is_temp = 0;
		goto head_done;
	case IS_OBJECT:
		myht = 0; //Z_OBJDEBUG_PP(struc, is_temp);
		if (myht && ++myht->nApplyCount > 1) {
			PUTS("*RECURSION*\n");
			--myht->nApplyCount;
			return;
		}

		Z_OBJ_HANDLER(**struc, get_class_name)(*struc, &class_name, &class_name_len, 0 TSRMLS_CC);
		fprintf(stderr,"%sobject(%s)#%d (%d) {\n", COMMON, class_name, Z_OBJ_HANDLE_PP(struc), myht ? zend_hash_num_elements(myht) : 0);
		efree(class_name);
		php_element_dump_func = php_object_property_dump;
head_done:
		if (myht) {
			zend_hash_apply_with_arguments(myht TSRMLS_CC, (apply_func_args_t) php_element_dump_func, 1, level);
			--myht->nApplyCount;
			if (is_temp) {
				zend_hash_destroy(myht);
				efree(myht);
			}
		}
		if (level > 1) {
			fprintf(stderr,"%*c", level-1, ' ');
		}
		PUTS("}\n");
		break;
	case IS_RESOURCE: {
		char *type_name;

		type_name = zend_rsrc_list_get_rsrc_type(Z_LVAL_PP(struc) TSRMLS_CC);
		fprintf(stderr,"%sresource(%ld) of type (%s)\n", COMMON, Z_LVAL_PP(struc), type_name ? type_name : "Unknown");
		break;
	}
	default:
		fprintf(stderr,"%sUNKNOWN:0\n", COMMON);
		break;
	}
}
}
#endif 
