%{
#include <iostream>
#include "zend_closures.h"
#include "zend_interfaces.h"
  class PhpWorker : public SwigWorker {
     typedef SwigWorker super;
   public:
   PhpWorker(const std::string &config) : super(config), self(NULL), called(0) {};
     virtual ~PhpWorker() {}

     void request_wrapper(const Job & job, const std::string & phase) {
       std::string fname = phase+ "_request";

       zval *result, funcname;
       MAKE_STD_ZVAL(result);
       ZVAL_STRING(&funcname, (char *)fname.c_str(), 0);

       zval zjob;
       INIT_ZVAL(zjob);
       SWIG_SetPointerZval(&zjob, (void *)&job, SWIGTYPE_p_Job, 0);
       zval * args[1] = {&zjob};

       call_user_function(EG(function_table), (zval**)&self, &funcname, result, 1, args TSRMLS_CC);
       
       if (EG(exception)) {
         zval * retval;
         long code = exGetCode();
         std::string msg = exGetMessage();
         zend_clear_exception();
         throw_from_code(code,msg);
       }
       
       FREE_ZVAL(result);
       return;
     fail:
       SWIG_FAIL();
     }

     void pre_request(const Job & job) { 
       this->called++;
       try {
         if( this->called == 1 ) {
           this->request_wrapper(job,"pre");
         }
       }
       catch ( const std::exception & err ) {}
       this->called--;
       this->super::pre_request(job);
     }
     
     void post_request(const Job & job) { 
       this->called++;
       try { 
         if( this->called == 1 ) {
           this->request_wrapper(job,"post");
         }
       }
       catch ( const std::exception & err ) {}
       this->called--;
       this->super::post_request(job);
     }

     void register_handler( const std::string & function_name, zval * func = NULL ) {
       const char * fn = function_name.c_str();

       if( func && Z_TYPE_P(func) != IS_NULL ) {
         Z_SET_REFCOUNT_P(func, Z_REFCOUNT_P(func)+1);
         funcs[function_name] = func;
       }
       else {
         zval * funcname;
         MAKE_STD_ZVAL(funcname);
         ZVAL_STRINGL(funcname, function_name.c_str(), function_name.size(), 1);
         funcs[function_name] = funcname;
       }
       this->Worker::register_handler(
         function_name,
          static_cast<Worker::handler_t>(&PhpWorker::redispatcher)
       );
     }

     Worker::response_t redispatcher(const Job & job, JobResponse & resp) {
       zval zjob;
       INIT_ZVAL(zjob);
       SWIG_SetPointerZval(&zjob, (void *)&job, SWIGTYPE_p_Job, 0);

       zval zresp;
       INIT_ZVAL(zresp);
       SWIG_SetPointerZval(&zresp, (void *)&resp, SWIGTYPE_p_JobResponse, 0);
        
       zval * args[2] = { &zjob, &zresp };

       zval * result;
       MAKE_STD_ZVAL(result);

       call_user_function(EG(function_table), (zval**)&self, funcs[job.name()], result, 2, args TSRMLS_CC);
         
       if (EG(exception)) {
         zval * retval;
         long code = exGetCode();
         std::string msg = exGetMessage();
         zend_clear_exception();
         throw_from_code(code,msg);
       }
       
       long retval = Z_LVAL_P(result);
       FREE_ZVAL(result);
       return static_cast<Worker::response_t>(retval);
     }

     void set_self(zval *s) {
       self = s;
     }
       
   private:
     std::map<std::string,zval*> funcs;
     zval *self;
     int called;
   };
%}

class PhpWorker : public SwigWorker {
public:
    PhpWorker(const std::string &config);
    void register_handler( const std::string & function_name, zval * func = NULL );
    virtual ~PhpWorker();
    void set_self(zval *s);
};
