%{
  #include <sstream>
  #include "traceback.h"
  #include "frameobject.h"

  class PythonWorker : public Gearbox::Worker {
     typedef Gearbox::Worker super;
   public:
     PythonWorker(const std::string &config) : super(config), self(NULL) {};
     virtual ~PythonWorker() {
       if ( self )
         Py_DECREF( self );
     }
     void register_handler(const std::string & function_name, PyObject * func=NULL) {
       if (func) {
         if (!PyCallable_Check(func))
             lookup_and_throw("register_handler failed to check if given function is callable");
       } else {
         if ( !self )
           gbTHROW( std::runtime_error("internal error: self must be set when using single arg register_handler()") );
         if ( !PyObject_HasAttrString( self, function_name.c_str() ) )
           gbTHROW( std::runtime_error(function_name + " does not exist in class") );

         func = PyObject_GetAttrString( self, function_name.c_str() );
         if ( !func )
           gbTHROW( std::runtime_error("failed to get " + function_name + " from class") );
       }

       if (funcs.count( function_name ) != 0)
         Py_DECREF(funcs[function_name]);

       funcs[function_name] = func;
       Py_INCREF(func);

       this->Worker::register_handler(
         function_name,
         static_cast<Worker::handler_t>(&PythonWorker::redispatcher)
       );
     }

     void request_wrapper(const Job & job, const std::string & phase) {
       std::string fname = phase+ "_request";
       PyObject *result = NULL;
       PyObject *pymethod_name = NULL;
       PyObject *j = NULL;

       // if there is no self, or no method named pre_request on self
       // there is nothing to do
       if ( !self )
         return;

       pymethod_name = Py_BuildValue("s", fname.c_str());
       if (!pymethod_name)
         gbTHROW( std::runtime_error("internal error: failed to convert string to PyObject") );

       if ( !PyObject_HasAttr( self, pymethod_name ) ) {
         Py_DECREF(pymethod_name);
         return;
       }

       j = SWIG_NewPointerObj(SWIG_as_voidptr(&job), SWIGTYPE_p_Gearbox__Job, SWIG_POINTER_OWN);
       result = PyObject_CallMethodObjArgs(self, pymethod_name, j, NULL);
       Py_DECREF(pymethod_name);
       if (!result)
         lookup_and_throw(fname + " failed");

       Py_DECREF(result);
     }

     void pre_request(const Job & job) {
       this->request_wrapper(job,"pre");
       this->super::pre_request(job);
     }

     void post_request(const Job & job) {
       this->request_wrapper(job,"post");
       this->super::post_request(job);
     }


     Worker::response_t redispatcher(const Job & job, JobResponse & resp) {
       PyObject *j = SWIG_NewPointerObj(SWIG_as_voidptr(&job), SWIGTYPE_p_Gearbox__Job, SWIG_POINTER_OWN);
       PyObject *r = SWIG_NewPointerObj(SWIG_as_voidptr(&resp), SWIGTYPE_p_Gearbox__JobResponse, SWIG_POINTER_OWN);
       PyObject *result = NULL;
       PyObject *func_args = NULL;

       if (PyMethod_Self(funcs[job.name()]) )
         func_args = Py_BuildValue("(O,O)", j, r);
       else if (self)
         func_args = Py_BuildValue("(O,O,O)", self, j, r);
       else
         gbTHROW( std::runtime_error("self not defined to dispatch method " + job.name()));

       if (!func_args)
         lookup_and_throw("Unknown error occured while building args to call " + job.name());

       result = PyObject_CallObject(funcs[job.name()], func_args);
       Py_DECREF( func_args );

       if (!result || PyErr_Occurred())
         lookup_and_throw("Unknown error occured while calling " + job.name());

       long int response = PyInt_AsLong(result);
       Py_DECREF(result);

       return static_cast<Worker::response_t>( response );
     }

     void set_self(PyObject* s) {
       PyObject *module, *mod_dict, *cls;
       module = PyImport_ImportModule("gearbox");
       if (!module)
         lookup_and_throw("internal error: failed to import module gearbox");

       mod_dict = PyModule_GetDict(module);
       Py_DECREF(module);
       if (!mod_dict)
         lookup_and_throw("internal error: failed to get gearbox module dictionary");

       cls = PyDict_GetItemString(mod_dict, "Worker");
       if (!cls)
         lookup_and_throw("internal error: failed to get Worker item from gearbox module");

      if ( !s || !PyObject_IsInstance(s, cls) )
        gbTHROW( std::runtime_error("set_self() arg 1 must be an instance of Worker") );

      Py_INCREF(s);
      self = s;
    }

    void lookup_and_throw( const std::string &default_msg ) {
      if (!PyErr_Occurred())
        gbTHROW( std::runtime_error( default_msg ) );

      // get the error that was thrown
      PyObject *errtype = NULL, *errvalue = NULL, *errtb = NULL;
      PyErr_Fetch(&errtype, &errvalue, &errtb);
      PyErr_NormalizeException(&errtype, &errvalue, &errtb);

      // print a traceback to stderr
      // it'd be nice to get the traceback as a string the send it to _WARN but
      // that wasn't straight forward.  The best I could find was the
      // PyTraceback_AsString method here:
      // http://lxr.mozilla.org/seamonkey/source/extensions/python/xpcom/src/ErrorUtils.cpp
      if ( errtb ) {
        char *cstr = new char [6+1];
        strcpy(cstr, "stderr");
        PyObject *f = PySys_GetObject(cstr);
        delete[] cstr;

        PyTraceBack_Print(errtb, f);
        Py_DECREF(errtb);
      }

      std::string errmsg = default_msg;
      if (errvalue) {
         PyObject *pymsgstr = PyObject_Str(errvalue);
         if (pymsgstr) {
             errmsg = std::string(PyString_AsString(pymsgstr));
             Py_DECREF(pymsgstr);
         }
      }

      // if the exception that is a python gearbox.Error then rethrow it as
      // a c++ Gearbox::Error
      PyObject *module, *mod_dict, *cls;
      module = PyImport_ImportModule("gearbox");
      if (module) {
        mod_dict = PyModule_GetDict(module);
        Py_DECREF(module);
        if (mod_dict) {
          cls = PyDict_GetItemString(mod_dict, "Error");
          if (errvalue && cls && PyObject_IsInstance(errvalue, cls) ) {
            PyObject *pycode = PyObject_GetAttrString(errvalue, "code");
            Py_DECREF(errvalue);
            if ( pycode ) {
              Py_DECREF(pycode);
              long int code = PyInt_AsLong(pycode);
              throw_from_code( code, errmsg );
            }
          }
        }
      }
      Py_XDECREF(errvalue);

      // string-ify errtype and build the message to throw
      std::ostringstream err;
      PyObject *pyerrtypestr = PyObject_Str( errtype );
      if ( pyerrtypestr ) {
        err << PyString_AsString( pyerrtypestr ) << ": ";
        Py_DECREF(pyerrtypestr);
      }

      err << errmsg;
      if ( PyTraceBack_Check(errtb) ) {
        err << " (File " << PyString_AsString(((PyTracebackObject *)errtb)->tb_frame->f_code->co_filename);
        err << " line " << ((PyTracebackObject *)errtb)->tb_lineno << ")";
      }
      gbTHROW( std::runtime_error( err.str() ) );
    }

    std::map<std::string,PyObject*> funcs;
    PyObject *self;
  };
%}

class PythonWorker : public SwigWorker {
public:
    PythonWorker(const std::string &config);
    void register_handler(const std::string & function_name, PyObject * func=NULL);
    void set_self(PyObject * s);
};

%pythoncode %{

from os.path import isfile
import signal

class Worker(PythonWorker):
    def __init__(self, config):
        # Allow ctrl-C to work
        # See https://mail.python.org/pipermail/cplusplus-sig/2012-December/016858.html
        signal.signal(signal.SIGINT, signal.SIG_DFL)
        if not isfile(config):
            raise IOError("config file '%s' is not a file" % config)
        super(Worker, self).__init__(config)
        self.set_self(self)

    # we don't want the PythonWorker's {post,pre}_request to get called
    # because that would lead to infinite recursion since it is called
    # called by redispatcher
    def post_request(self, job):
        return 0

    def pre_request(self, job):
        return 0
%}

%extend Gearbox::JobManager {
    %pythoncode %{
        def job_queue_apply(self, queue, function, data):
            for level in queue:
                for job in level:
                    getattr(job, function)(data)
    %}
}
