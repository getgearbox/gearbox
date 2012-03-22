/*
  This source contains modified code from php-5.3.5/ext/json/json.c
  original copyright below:

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
  | Author: Omar Kilani <omar@php.net>                                   |
  +----------------------------------------------------------------------+
*/

#include <boost/lexical_cast.hpp>

#define PHP_JSON_OUTPUT_ARRAY 0
#define PHP_JSON_OUTPUT_OBJECT 1

void php_populate_json(Json & json, zval *val);

static int json_determine_array_type(zval **val)
{
	int i;
	HashTable *myht = HASH_OF(*val);

	i = myht ? zend_hash_num_elements(myht) : 0;
	if (i > 0) {
		char *key;
		ulong index, idx;
		uint key_len;
		HashPosition pos;

		zend_hash_internal_pointer_reset_ex(myht, &pos);
		idx = 0;
		for (;; zend_hash_move_forward_ex(myht, &pos)) {
			i = zend_hash_get_current_key_ex(myht, &key, &key_len, &index, 0, &pos);
			if (i == HASH_KEY_NON_EXISTANT)
				break;

			if (i == HASH_KEY_IS_STRING) {
				return 1;
			} else {
				if (index != idx) {
					return 1;
				}
			}
			idx++;
		}
	}

	return PHP_JSON_OUTPUT_ARRAY;
}
/* }}} */

static void json_encode_array(Json & json, zval **val)
{
	int i, r;
	HashTable *myht;

	if (Z_TYPE_PP(val) == IS_ARRAY) {
		myht = HASH_OF(*val);
		r = json_determine_array_type(val TSRMLS_CC);
	} else {
		myht = Z_OBJPROP_PP(val);
		r = PHP_JSON_OUTPUT_OBJECT;
	}

	if (myht && myht->nApplyCount > 1) {
        json.empty();
		return;
	}

	// if (r == PHP_JSON_OUTPUT_ARRAY) {
	// 	smart_str_appendc(buf, '[');
	// } else {
	// 	smart_str_appendc(buf, '{');
	// }

	i = myht ? zend_hash_num_elements(myht) : 0;

	if (i > 0)
	{
		char *key;
		zval **data;
		ulong index;
		uint key_len;
		HashPosition pos;
		HashTable *tmp_ht;
		// int need_comma = 0;

		zend_hash_internal_pointer_reset_ex(myht, &pos);
		for (;; zend_hash_move_forward_ex(myht, &pos)) {
			i = zend_hash_get_current_key_ex(myht, &key, &key_len, &index, 0, &pos);
			if (i == HASH_KEY_NON_EXISTANT)
				break;

			if (zend_hash_get_current_data_ex(myht, (void **) &data, &pos) == SUCCESS) {
				tmp_ht = HASH_OF(*data);
				if (tmp_ht) {
					tmp_ht->nApplyCount++;
				}

				if (r == PHP_JSON_OUTPUT_ARRAY) {
					// if (need_comma) {
					// 	smart_str_appendc(buf, ',');
					// } else {
					// 	need_comma = 1;
					// }
                    if( json.type() != Json::ARRAY ) {
                        json = Json::Array();
                    }
 
					php_populate_json(json[json.length()], *data);
				} else if (r == PHP_JSON_OUTPUT_OBJECT) {
					if (i == HASH_KEY_IS_STRING) {
						if (key[0] == '\0' && Z_TYPE_PP(val) == IS_OBJECT) {
							/* Skip protected and private members. */
							if (tmp_ht) {
								tmp_ht->nApplyCount--;
							}
							continue;
						}

						// if (need_comma) {
						// 	smart_str_appendc(buf, ',');
						// } else {
						// 	need_comma = 1;
						// }

						// json_escape_string(buf, key, key_len - 1, options TSRMLS_CC);
						// smart_str_appendc(buf, ':');

						php_populate_json(json[std::string(key,key_len-1)], *data);
					} else {
						// if (need_comma) {
						// 	smart_str_appendc(buf, ',');
						// } else {
						// 	need_comma = 1;
						// }

						// smart_str_appendc(buf, '"');
						// smart_str_append_long(buf, (long) index);
						// smart_str_appendc(buf, '"');
						// smart_str_appendc(buf, ':');

						php_populate_json(json[boost::lexical_cast<std::string>(index)], *data);
					}
				}

				if (tmp_ht) {
					tmp_ht->nApplyCount--;
				}
			}
		}
	}

	// if (r == PHP_JSON_OUTPUT_ARRAY) {
	// 	smart_str_appendc(buf, ']');
	// } else {
	// 	smart_str_appendc(buf, '}');
	// }
}
/* }}} */

void php_populate_json(Json & json, zval *val)
{
	switch (Z_TYPE_P(val))
	{
		case IS_NULL:
            json.empty();
			break;

		case IS_BOOL:
			if (Z_BVAL_P(val)) {
                json = true;
			} else {
                json = false;
			}
			break;

		case IS_LONG:
            json = boost::lexical_cast<int64_t>(Z_LVAL_P(val));
			break;

		case IS_DOUBLE:
            json = Z_DVAL_P(val);
			break;

		case IS_STRING:
            json = std::string(Z_STRVAL_P(val), Z_STRLEN_P(val));
			break;

		case IS_ARRAY:
		case IS_OBJECT:
			json_encode_array(json, &val);
			break;

		default:
            json.empty();
			break;
	}

	return;
}
/* }}} */
