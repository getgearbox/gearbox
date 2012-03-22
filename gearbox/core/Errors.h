// -*- Mode: c++ -*-

#ifndef GEARBOX_ERRORS_H
#define GEARBOX_ERRORS_H
#include <gearbox/core/logger.h>

#include <exception>
#include <string>
#include <string.h>

#include <errno.h>

#define gbTHROW(err) {                          \
        _ERROR(err.what());                     \
        Gearbox::print_trace();                 \
        throw err;                              \
    }
 

    

namespace Gearbox {

/*! Base class for exceptions
 *
 *  The Gearbox HTTP-style error classes are subclassed from this class. 
 */

void print_trace();

class Error : public std::exception {
 public:
  /** Default constructor for error object
  *
  */
  Error();
  /** Constructor for error object
  *
  * @param msg Human-readable error message described a specific error occurrence, e.g. "page index.html not found"
  * @param code status code (http mostly)for the message, e.g. 404
  * @param name The text description of generic error category, e.g. NOT_FOUND
  */
  Error(const std::string & msg, int code, const char * name);
  /** Destructor for error object
  */
  virtual ~Error() throw();
  //! accessor for the error message
  virtual const char * what() const throw();    

  //! accessor for the numeric error code
  virtual int code() const throw();             

  //! accesor for the error type
  virtual const char * name() const throw();  
    
 private:
  char buffer[512];
  int _code;
  const char * _name;
};

/** throw the appropriate error based on a numeric error code
 * 
 * throw_from_code() will look up the appropriate error code class
 * for a numeric error code, e.g ERR_NOT_FOUND for code '404'
 * 
 * @param code numeric error code, e.g. 404
 * @param msg user-readable message describing specific error condition, e.g "index.html not found"
 */
void throw_from_code(int code, const std::string & msg = "");  

#define DEFINE_ERROR(TYPE,CODE)                            \
  class ERR_##TYPE : public Error {                        \
  public:                                                  \
    ERR_##TYPE() : Error("", CODE, #TYPE) {};              \
    ERR_##TYPE(const std::string & msg)                    \
      : Error(msg, CODE, #TYPE) {};                        \
  };                                                       \
  class ERR_CODE_##CODE : public ERR_##TYPE {              \
  public:                                                  \
    ERR_CODE_##CODE() : ERR_##TYPE() {};                     \
    ERR_CODE_##CODE(const std::string & msg)               \
      : ERR_##TYPE(msg) {};                                \
  };
  

DEFINE_ERROR(MULTIPLE_CHOICES,300);
DEFINE_ERROR(MOVED_PERMANENTLY,301);
DEFINE_ERROR(MOVED_TEMPORARILY,302);
DEFINE_ERROR(SEE_OTHER,303);
DEFINE_ERROR(NOT_MODIFIED,304);
DEFINE_ERROR(USE_PROXY,305);
DEFINE_ERROR(TEMPORARY_REDIRECT,307);
DEFINE_ERROR(BAD_REQUEST,400);
DEFINE_ERROR(UNAUTHORIZED,401);
DEFINE_ERROR(PAYMENT_REQUIRED,402);
DEFINE_ERROR(FORBIDDEN,403);
DEFINE_ERROR(NOT_FOUND,404);
DEFINE_ERROR(METHOD_NOT_ALLOWED,405);
DEFINE_ERROR(NOT_ACCEPTABLE,406);
DEFINE_ERROR(PROXY_AUTHENTICATION_REQUIRED,407);
DEFINE_ERROR(REQUEST_TIME_OUT,408);
DEFINE_ERROR(CONFLICT,409);
DEFINE_ERROR(GONE,410);
DEFINE_ERROR(LENGTH_REQUIRED,411);
DEFINE_ERROR(PRECONDITION_FAILED,412);
DEFINE_ERROR(REQUEST_ENTITY_TOO_LARGE,413);
DEFINE_ERROR(REQUEST_URI_TOO_LARGE,414);
DEFINE_ERROR(UNSUPPORTED_MEDIA_TYPE,415);
DEFINE_ERROR(RANGE_NOT_SATISFIABLE,416);
DEFINE_ERROR(EXPECTATION_FAILED,417);
DEFINE_ERROR(UNPROCESSABLE_ENTITY,422);
DEFINE_ERROR(LOCKED,423);
DEFINE_ERROR(FAILED_DEPENDENCY,424);
DEFINE_ERROR(UPGRADE_REQUIRED,426);
DEFINE_ERROR(INTERNAL_SERVER_ERROR,500);
DEFINE_ERROR(NOT_IMPLEMENTED,501);
DEFINE_ERROR(BAD_GATEWAY,502);
DEFINE_ERROR(SERVICE_UNAVAILABLE,503);
DEFINE_ERROR(GATEWAY_TIME_OUT,504);
DEFINE_ERROR(VERSION_NOT_SUPPORTED,505);
DEFINE_ERROR(VARIANT_ALSO_VARIES,506);
DEFINE_ERROR(INSUFFICIENT_STORAGE,507);
DEFINE_ERROR(NOT_EXTENDED,510);

/** throw an internal server error for a libc errno value
 *
 * @param why string that will preface the errno error message
 * @param code optional numeric error, if not given, defaults to errno
 */

// would have been nice to use errno instead of errnum as method, but errno is a macro...
class ERR_LIBC: public ERR_INTERNAL_SERVER_ERROR {
    typedef ERR_INTERNAL_SERVER_ERROR super;
  public:
    ERR_LIBC(const std::string & why, int errnum=errno) : super(why + ": " + strerror(errnum)), _errnum(errnum) {};
    int errnum() const { return _errnum; }
  protected:
    int _errnum;
};

} // namespace

#endif
