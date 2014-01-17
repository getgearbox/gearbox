%include "std_vector.i"

namespace std {
   %template(stringVector) vector<std::string>;
}

%typemap(out) const JobPtr & {
  $result = $1->get() ? SWIG_NewPointerObj(SWIG_as_voidptr($1->get()), SWIGTYPE_p_Gearbox__Job, 0) : Py_BuildValue("");
}

%typemap(out) JobPtr {
  $result = $1.get() ? SWIG_NewPointerObj(SWIG_as_voidptr(new Job(*($1.get()))), SWIGTYPE_p_Gearbox__Job, SWIG_POINTER_OWN) : Py_BuildValue("");
}

%typemap(out) Gearbox::JobPtr {
  $result = $1.get() ? SWIG_NewPointerObj(SWIG_as_voidptr(new Job(*($1.get()))), SWIGTYPE_p_Gearbox__Job, SWIG_POINTER_OWN) : Py_BuildValue("");
}

%typemap(out) const StatusPtr & {
  $result = $1->get() ? SWIG_NewPointerObj(SWIG_as_voidptr($1->get()), SWIGTYPE_p_Gearbox__Status, 0) : Py_BuildValue("");
}

%typemap(out) Gearbox::StatusPtr {
  $result = $1.get() ? SWIG_NewPointerObj(SWIG_as_voidptr(new Status(*($1.get()))), SWIGTYPE_p_Gearbox__Status, SWIG_POINTER_OWN) : Py_BuildValue("");
}

%typemap(out) Gearbox::JobQueue {
  int len = $1.size();
  $result = PyList_New(len);

  for(int i=0 ; i < len ; i++) {
    int level_len = $1.at(i).size();
    PyObject * level = PyList_New(level_len);
    PyList_SetItem($result, i, level);

    for(int k=0 ; k < level_len ; k++) {
      PyObject * job = Py_BuildValue("");
      std::vector<JobPtr> job_ptrs = $1.at(i);

      if( job_ptrs.at(k) ) {
        Job * j = new Job( *(job_ptrs.at(k)) );
        job = SWIG_NewPointerObj(SWIG_as_voidptr(j), SWIGTYPE_p_Gearbox__Job, SWIG_POINTER_OWN);
      }

      PyList_SetItem(level, k, job);
    }
  }
}

%typemap(in) Gearbox::JobQueue & (JobQueue temp) {
  {
    PyObject * jq = $input;

    if (PyList_Check(jq)) {
      $1 = &temp;
      Py_ssize_t len = PyList_Size(jq);

      for( int i=0; i < len; i++ ) {
        PyObject * level = PyList_GetItem(jq, i);
        Py_ssize_t level_len = PyList_Size(level);

        temp.push_back(std::vector<JobPtr>());
        for( int j=0; j < level_len; j++ ) {
          PyObject * job = PyList_GetItem(level, j);
          Job * jptr;
          SWIG_ConvertPtr(job,(void*)&jptr,SWIGTYPE_p_Gearbox__Job,0);
          temp[i].push_back(boost::shared_ptr<Job>(new Job(*jptr)));
        }
      }
    } else {
        SWIG_exception(SWIG_TypeError, "list expected");
    }
  }
}

%typemap(typecheck) Gearbox::JobQueue & %{
   $1 = PyList_Check($input) ? 1 : 0;
%}

%typemap(out) const Json & (PyObject *json_module, PyObject *json_mod_dict, PyObject *func, char format[] = "s") %{
  json_module = PyImport_ImportModule("json");
  if (json_module == NULL) {
     PyErr_SetString(PyExc_ImportError, "Failed to import the json module");
     return NULL;
  }

  json_mod_dict = PyModule_GetDict(json_module);
  Py_DECREF(json_module);
  if (json_mod_dict == NULL) {
     PyErr_SetString(PyExc_ImportError, "Failed to get module dictionary for module 'json'");
     return NULL;
  }

  func = PyDict_GetItemString(json_mod_dict, "loads");
  if (func == NULL) {
     PyErr_SetString(PyExc_AttributeError, "'json' module has no attribute 'loads'");
     return NULL;
  }

  $result = PyObject_CallFunction(func, format, $1->serialize().c_str() );
%}

// For functions that take a Uri ... accept a string from Python
// and construct the Uri object on the fly
%typemap(typecheck) const Uri & %{
  $1 = PyString_Check($input);
%}

// if a C++ method takes const Json & then allow a python object (string, int, dict, tuple, etc) to be passed in
%typemap(typecheck) const Json & %{
  $1 = 1;
%}

// convert python object into Gearbox::Json object
%typemap(in) const Json & (Json temp, PyObject *json_module, PyObject *json_mod_dict, PyObject *func, PyObject *ret) %{
  $1 = &temp;

  json_module = PyImport_ImportModule("json");
  if (json_module == NULL) {
     PyErr_SetString(PyExc_ImportError, "Failed to import the json module");
     return NULL;
  }

  json_mod_dict = PyModule_GetDict(json_module);
  Py_DECREF(json_module);
  if (json_mod_dict == NULL) {
     PyErr_SetString(PyExc_ImportError, "Failed to get module dictionary for module 'json'");
     return NULL;
  }

  func = PyDict_GetItemString(json_mod_dict, "dumps");
  if (func == NULL) {
     PyErr_SetString(PyExc_AttributeError, "'json' module has no attribute 'dumps'");
     return NULL;
  }

  ret = PyObject_CallFunctionObjArgs(func, $input, NULL);
  if (ret == NULL) {
     PyErr_SetString(PyExc_RuntimeError, "Failed to call json.dumps to convert python dictionary to Gearbox::Json object");
     return NULL;
  }

  temp.parse( PyString_AsString( ret ) );
  Py_DECREF(ret);
%}

%typemap(in) const Uri & (std::string uri, std::auto_ptr<Uri> cfptr) %{
  {
    uri.assign(PyString_AsString($input));
    cfptr.reset(new Uri(uri));
    $1 = cfptr.get();
  }
%}

// make Hash into python dict
%typemap(out) const Hash & {
  PyObject* dict = PyDict_New();

  Hash::iterator i = $1->begin();
  Hash::iterator e = $1->end();
  for( ; i != e; ++i ) {
      PyDict_SetItem(dict,
                     PyString_FromString(i->first.c_str()),
                     PyString_FromString(i->second.c_str()));
  }

  $result = dict;
}

// convert a python dict to C++ Gearbox::Hash object
%typemap(typecheck) const Hash & %{
  $1 = PyDict_Check($input);
%}

// turn python dict into Gearbox::Hash
%typemap(in) const Hash & (Hash temp, PyObject *pykey, PyObject *pyvalue, Py_ssize_t pos) %{
  $1 = &temp;
  pos = 0;

  while (PyDict_Next($input, &pos, &pykey, &pyvalue)) {
    char *key = PyString_AsString(pykey);
    if (key == NULL) {
       PyErr_SetString(PyExc_ValueError,"Dictionary keys can only be strings.");
       return NULL;
    }

    char *value = PyString_AsString(pyvalue);
    if (value == NULL) {
       PyErr_SetString(PyExc_ValueError,"Dictionary values can only be strings.");
       return NULL;
    }

    temp[std::string(key)] = std::string(value);
  }
%}

%exception {
  try { $action }
  catch(const Error & err) {
    std::string pkg("gearbox.ERR_");
    pkg += err.name();
    char * cstr = new char [pkg.length()+1];
    strcpy (cstr, pkg.c_str());
    PyObject *exc = PyErr_NewException( cstr, NULL, NULL );
    delete[] cstr;
    PyErr_SetString( exc, err.what() );
    return NULL;
  }
  catch(const std::exception & err) {
    PyErr_SetString(PyExc_Exception , err.what());
    return NULL;
  }
}

# errors.i is generated by the Makefile
%include "./errors.i"

// directors allow us to extend C++ classes in target languages
%feature("director") Worker;
