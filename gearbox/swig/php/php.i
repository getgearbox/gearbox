%{

#include "php_Json_helper.h"
#include "var_dump.h"
    static inline long exGetCode() {
        zval * codeval = zend_read_property (
            Z_OBJCE_P(EG(exception)),
            EG(exception),
            "code",
            sizeof("code")-1,
            0 TSRMLS_CC
        );
        zval_copy_ctor(codeval);
        INIT_PZVAL(codeval);
        return Z_LVAL_PP(&codeval);
    }
    
    static inline std::string exGetMessage() {
        zval * msgval = zend_read_property(
            Z_OBJCE_P(EG(exception)),
            EG(exception),
            "message",
            sizeof("message")-1,
            0 TSRMLS_CC
        );
        zval_copy_ctor(msgval);
        INIT_PZVAL(msgval);
        return Z_STRVAL_PP(&msgval);
    }

    static void fromJson(Json & json, zval ** out) {
      switch( json.type() ) {
      case Json::UNDEF:
        ZVAL_NULL(*out);
        return;
      case Json::BOOL:
        ZVAL_BOOL(*out, json.as<bool>());
        return;
      case Json::INT:
        ZVAL_LONG(*out, json.as<int64_t>());
        return;
      case Json::DOUBLE:
        ZVAL_DOUBLE(*out, json.as<double>());
        return;
      case Json::STRING: {
        std::string & str = json.as<std::string>();
        ZVAL_STRINGL(*out, str.c_str(), str.size(), 1);
        return;
      }
      case Json::OBJECT: {
        array_init(*out);
        Json::Object & obj = json.as<Json::Object>();
        Json::Object::iterator i = obj.begin();
        Json::Object::iterator e = obj.end();
        for( ; i != e; ++i ) {
          zval * data = 0;
          MAKE_STD_ZVAL(data);
          fromJson(*(i->second), &data);
          zend_hash_add(HASH_OF(*out), (char*)i->first.c_str(), i->first.size()+1, &data, sizeof(zval*), NULL);
        }
        return;
      }
      case Json::ARRAY: {
        array_init(*out);
        for( int i=0; i < json.length(); ++i ) {
          zval * data = 0;
          MAKE_STD_ZVAL(data);
          fromJson(json[i], &data);
          zend_hash_next_index_insert( HASH_OF(*out), &data, sizeof(zval*), NULL );
        }
        return;
      }
      }
    }
%}

%typemap(typecheck) const std::vector<std::string> & %{
  $1 = Z_TYPE_PP($input) == IS_ARRAY ? 1 : 0;
%}

// convert PHP native array to std::vector<std::string>
%typemap(in) const std::vector<std::string> & (int index, Array temp) %{
    index = 0;
    $1 = &temp;
    while(true) {
        zval ** value;
        if( zend_hash_index_find(Z_ARRVAL_P(*$input), index++, (void**)&value) == SUCCESS ) {
            convert_to_string_ex(value);
            $1->push_back(Z_STRVAL_PP(value));
        }
        else {
            // no more indexes in array
            break;
        }
    }
%}

// convert std::vector<std::string> to PHP native array
%typemap(out) const std::vector<std::string> & {
    array_init($result);
    Array::const_iterator i = $1->begin();
    Array::const_iterator e = $1->end();
    for(; i != e; ++i ) {
        zval * data;
        MAKE_STD_ZVAL(data);
        ZVAL_STRINGL(data, (char*)i->c_str(), i->size(), 1);
        zend_hash_next_index_insert( HASH_OF($result), &data, sizeof(zval *), NULL );
    }
}

%typemap(out) const JobPtr & {
  if( $1->get() ) {
    SWIG_SetPointerZval($result, (void *)new Job(*$1->get()), SWIGTYPE_p_Gearbox__Job, 2);
  }
  else {
    ZVAL_NULL($result);
  }
}

%typemap(out) JobPtr {
  if( $1.get() ) {
    SWIG_SetPointerZval($result, (void *)new Job(*$1.get()), SWIGTYPE_p_Gearbox__Job, 2);
  }
  else {
    ZVAL_NULL($result);
  }
}

%typemap(out) Gearbox::JobPtr {
  if( $1.get() ) {
    SWIG_SetPointerZval($result, (void *)new Job(*$1.get()), SWIGTYPE_p_Gearbox__Job, 2);
  }
  else {
    ZVAL_NULL($result);
  }
}

%typemap(out) Gearbox::JobQueue {
  array_init($result);
  Gearbox::JobQueue::const_iterator i = $1.begin();
  Gearbox::JobQueue::const_iterator e = $1.end();
  for( ; i != e; ++i ) {
    zval * level;
    MAKE_STD_ZVAL(level);
    array_init(level);
    zend_hash_next_index_insert( HASH_OF($result), &level, sizeof(zval*), NULL );
    std::vector<JobPtr>::const_iterator li = i->begin();
    std::vector<JobPtr>::const_iterator le = i->end();
    for( ; li != le; ++li ) {
      zval * job;
      MAKE_STD_ZVAL(job);
      Job * j = new Job( *(li->get()) );
      SWIG_SetPointerZval(job, (void *)j,SWIGTYPE_p_Gearbox__Job, 2);
      zend_hash_next_index_insert( HASH_OF(level), &job, sizeof(zval*), NULL );
    }
  }
}

%typemap(in) Gearbox::JobQueue & (JobQueue temp) {
  $1 = &temp;
  for( int i=0; true; i++ ) {
    zval ** level;
    if( zend_hash_index_find(Z_ARRVAL_P(*$input), i, (void**)&level) == SUCCESS ) {
      temp.push_back(std::vector<JobPtr>());
      for( int j=0; true; j++ ) {
        zval ** job;
        if( zend_hash_index_find(Z_ARRVAL_P(*level), j, (void**)&job) == SUCCESS ) {
          Job * jptr;
          SWIG_ConvertPtr(*job,(void**)&jptr,SWIGTYPE_p_Gearbox__Job,0);
          temp[i].push_back(boost::shared_ptr<Job>(jptr));
        }
        else {
          break;
        }
      }
    }
    else {
      break;
    }
  }
}

%typemap(typecheck) const Json & %{
  $1 = 1;
%}

%typemap(typecheck) zval * %{
  $1 = 1;
%}

%typemap(in) zval * %{
  $1 = *$input;
%}

// For functions that take a Hash ... accept a string from PHP
// and construct the Hash object on the fly
%typemap(typecheck) const Hash & %{
  $1 = ( Z_TYPE_PP($input) == IS_ARRAY ) ? 1 : 0;
%}

// convert PHP array/hash into Hash c++ object
%typemap(in) const Hash & (HashTable * arrht, Hash temp) %{
  arrht = HASH_OF(*$input);
  $1 = &temp;
  if( arrht ) {
    for(zend_hash_internal_pointer_reset(arrht);
        zend_hash_has_more_elements(arrht) == SUCCESS;
        zend_hash_move_forward(arrht)) {
      char *key;
      uint keylen;
      ulong idx;
      zval **ppzval;
      
      zend_hash_get_current_key_ex(arrht, &key, &keylen,
                                   &idx, 0, NULL);
      if (zend_hash_get_current_data(arrht, (void**)&ppzval) == FAILURE) {
        /* Should never actually fail
         * since the key is known to exist. */
        continue;
      }
      convert_to_string_ex(ppzval);
      $1->insert(
        std::pair<std::string,std::string>(
          std::string(key,keylen-1),
          std::string(Z_STRVAL_PP(ppzval))
       ) 
      );
    }
  }
%}

// for PHP make Hash response into normal PHP arrays
%typemap(out) const Hash & {
  array_init($result);
  Hash::iterator i = $1->begin();
  Hash::iterator e = $1->end();
  for( ; i != e; ++i ) {
    zval * data;
    MAKE_STD_ZVAL(data);
    ZVAL_STRINGL(data, (char*)i->second.c_str(), i->second.size(), 1);
    zend_hash_add(HASH_OF($result), (char*)i->first.c_str(), i->first.size()+1, &data, sizeof(zval*), NULL);
  }
}

// The generated PHP class is "Status" but the wrapper is looking
// for "Gearbox__Status" which does not exist.  So create a
// new swig_type_info with a hacked name to make the object
// creation work properly
%typemap(out) const StatusPtr & {
    SWIG_SetPointerZval($result, (void *)$1->get(), SWIGTYPE_p_Gearbox__Status, 2);
}

%typemap(out) Gearbox::StatusPtr {
  SWIG_SetPointerZval($result, (void *)new Status(*($1.get())), SWIGTYPE_p_Gearbox__Status, 2);
}

// For functions that take a ConfigFile ... accept a string from PHP
// and construct the ConfigFile object on the fly
%typemap(typecheck) const ConfigFile & %{
  $1 = ( Z_TYPE_PP($input) == IS_STRING ) ? 1 : 0;
%}

%typemap(in) const ConfigFile & (std::string filename, std::auto_ptr<ConfigFile> cfptr) %{
  convert_to_string_ex($input);
  filename.assign(Z_STRVAL_PP($input), Z_STRLEN_PP($input));
  cfptr.reset(new ConfigFile(filename));
  $1 = cfptr.get();
%}

// For functions that take a Uri ... accept a string from PHP
// and construct the Uri object on the fly
%typemap(typecheck,precedence=SWIG_TYPECHECK_POINTER) Uri const & %{
  $1 = ( Z_TYPE_PP($input) == IS_STRING && Z_STRLEN_PP($input) > 4 && strncmp(Z_STRVAL_PP($input),"http",4) == 0 ) ? 1 : 0;
%}

%typemap(in) const Uri & (std::string uri, std::auto_ptr<Uri> cfptr) %{
  convert_to_string_ex($input);
  uri.assign(Z_STRVAL_PP($input), Z_STRLEN_PP($input));
  cfptr.reset(new Uri(uri));
  $1 = cfptr.get();
%}

%typemap(in) const Json & (Json temp) %{
  php_populate_json(temp, *$input);
  $1 = &temp;
%}

%typemap(out) const Json & (std::string temp) %{
  fromJson(*$1, &$result);
%}

%exception %{
  try { $action  }
  catch(const Error & err) {
    _WARN("Translating C++ Exception to PHP: " << err.what()); 
    std::string name("ERR_");
    name += err.name();
    zend_class_entry **ce = NULL;
    zval *classname;
    MAKE_STD_ZVAL(classname);
    ZVAL_STRING(classname, (char*)name.c_str(), 1);
    php_strtolower(Z_STRVAL_PP(&classname), Z_STRLEN_PP(&classname));
    if (zend_lookup_class(Z_STRVAL_P(classname), Z_STRLEN_P(classname), &ce TSRMLS_CC) != SUCCESS) {
      // didn't find class name so throw default exception type
      zend_throw_exception (
        zend_exception_get_default(TSRMLS_C),
        const_cast<char*>(err.what()),
        err.code()
      );
      return;
    }
    else {
      // found class name so throw gearbox ERR_* exception
      zend_throw_exception (
        *ce,
        const_cast<char*>(err.what()),
        err.code()
      );
      return;
    }
  }
  catch(const std::exception & err) {
    _WARN("Translating C++ Exception to PHP: " << err.what()); 
    // fixme need to throw "ERR_".err.name() exceptions
    zend_throw_exception (
      zend_exception_get_default(TSRMLS_C),
      const_cast<char*>(err.what()),
      500
    );
    return;
  }
%}

%feature("director:except") %{
  if (EG(exception)) {
    zval * retval;
    long code = exGetCode();
    std::string msg = exGetMessage();
    zend_clear_exception();
    throw_from_code(code,msg);
  }
%}

// directors allow us to extend C++ classes in target languages
%feature("director") SwigWorker;
