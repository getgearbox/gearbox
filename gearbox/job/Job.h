#ifndef GEARBOX_JOB
#define GEARBOX_JOB

#include <gearbox/core/Json.h>
#include <gearbox/core/Hash.h>
#include <gearbox/job/JobResponse.h>

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>
#include <map>

namespace Gearbox
{
    class JobImpl;
    class JobResponse;

    typedef std::vector<std::string> Array;

    class Job {
    public:
        enum JobType {
            JOB_UNKNOWN,
            JOB_SYNC,
            JOB_ASYNC
        };

        enum Event {
            EVENT_UNKNOWN,
            EVENT_COMPLETED,                                               
            EVENT_FAILED,                                                         
            EVENT_SUCCEEDED,                                                        
            EVENT_STARTED,                                                    
            EVENT_STOPPED,                                                           
            EVENT_CANCELLED,
        };
        
        Job( const Job & );
        Job & operator=(const Job &);
        ~Job();

        Job & content( const std::string & content );
        const std::string & content() const;
        const Json & json_content() const;

        Job & arguments( const Array & args );
        Job & add_argument( const std::string & arg );
        const Array & arguments() const;
        
        Job & matrix_arguments( const Hash & matrix );
        Job & add_matrix_argument( const std::string & name, const std::string & value);
        const Hash & matrix_arguments() const;

        Job & query_params( const Hash & params );
        Job & add_query_param( const std::string & name, const std::string & value);
        const Hash & query_params() const;

        Job & headers( const Hash & head );
        Job & add_header( const std::string & name, const std::string & value);
        const Hash & headers() const;

        Job & environ( const Hash & environ );
        Job & add_environ( const std::string & name, const std::string & value);
        const Hash & environ() const;

        Job & status( const std::string & status_id );
        const std::string & status() const;

        Job & name( const std::string & name );
        const std::string & name() const;

        const std::string & base_uri() const;

        Job & type(JobType t);
        JobType type() const;

        Job & api_version( const std::string & ver );
        const std::string & api_version() const;

        Job & operation( const std::string & op );
        const std::string & operation() const;

        Job & component( const std::string & comp );
        const std::string & component() const;

        Job & resource_type( const std::string & type );
        const std::string & resource_type() const;

        Job & resource_name( const std::string & name );
        const std::string & resource_name() const;

        std::string resource_uri() const;

        Job & remote_ip( const std::string & ip );
        const std::string & remote_ip() const;

        Job & remote_user( const std::string & ip );
        const std::string & remote_user() const;

        Job & timeout( int t );
        int timeout() const;

        void on( Job::Event e, const Job & job );
        JobPtr on(Job::Event e) const;

        static std::string event2str(Job::Event ev);
        static Job::Event  str2event(const std::string & ev);

        void event_status(const Gearbox::Status & s);
        StatusPtr event_status() const;

        // name of job impl
        const char * impltype() const;

        // for debugging
        std::string serialize() const;

        JobResponse run() const;

    private:
        friend class JobManager;
        friend class JobImpl;
        Job( JobImpl * impl );
        JobImpl * impl;
    };
}

#endif
