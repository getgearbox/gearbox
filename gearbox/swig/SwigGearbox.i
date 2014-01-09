#if defined(SWIGPHP)
    %module(directors="1") SwigGearbox
#elif defined(SWIGPERL)
    %module Gearbox
#elif defined(SWIGPYTHON)
    %module(directors="1") gearbox
#endif

%{
#include <gearbox/core/Json.h>
#include <gearbox/core/Hash.h>
#include <gearbox/job/Job.h>
#include <gearbox/worker/Worker.h>
#include <gearbox/core/Errors.h>
#include <gearbox/core/logger.h>
#include <gearbox/swig/SwigWorker.h>

using namespace Gearbox;
%}

%include "std_string.i"

#if defined(SWIGPHP)
  %include "./php/php.i"
#elif defined(SWIGPERL)
  %include "./perl/perl.i"
#elif defined(SWIGPYTHON)
  %include "./python/python.i"
#else
  #error "only php, perl and python languages are supported!"
#endif

// ignore these since they arent needed and conflict with php
%ignore operator =;

#if __WORDSIZE == 64
typedef long int        int64_t;
#else
typedef long long int   int64_t;
#endif
typedef unsigned int    uint32_t;

typedef int time_t;

%include "gearbox/job/Job.h"
%include "gearbox/job/JobResponse.h"
%include "gearbox/job/Status.h"
%include "gearbox/core/ConfigFile.h"

class HttpClient
{
  public:
    enum method_t
    {
        METHOD_GET,
        METHOD_DELETE,
        METHOD_POST,
        METHOD_PUT,
        METHOD_HEAD,
        METHOD_UNKNOWN
    };
};

%ignore JobManager(const JobImpl *);
%ignore getSchema(const JobImpl *, const ConfigFile &);
#if defined(SWIGPERL) || defined(SWIGPHP)
%rename(RealJobManager) JobManager;
#endif
%include "gearbox/job/JobManager.h"
%include "gearbox/job/StatusManager.h"

namespace Gearbox {
    void log_init(const std::string & configFile);
}

class SwigWorker {
    public:
        enum response_t {
            WORKER_SUCCESS,
            WORKER_ERROR,
            WORKER_CONTINUE,
            WORKER_RETRY
        };
        SwigWorker(const std::string &config);
        virtual ~SwigWorker();
        void register_handler( const std::string & function_name );
        virtual response_t do_dispatch( const Gearbox::Job & job, Gearbox::JobResponse & resp );
        void afterwards( const Gearbox::Job & job, const std::string & name, int delay = 0);
        void afterwards( const Gearbox::Job & job, int delay = 0);
        void pre_request( const Gearbox::Job & job );
        void post_request( const Gearbox::Job & job );
        void max_requests( int max );
        int request_count();
        virtual void run();
        Gearbox::ConfigFile & cfg();
        Gearbox::JobManager    & job_manager();
        Gearbox::StatusManager & status_manager();
};

#if defined(SWIGPERL)
  %include "./perl/perlworker.i"
#elif defined(SWIGPHP)
  %include "./php/phpworker.i"
#elif defined(SWIGPYTHON)
  %include "./python/pythonworker.i"
#endif
