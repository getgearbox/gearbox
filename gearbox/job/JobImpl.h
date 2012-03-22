#ifndef GEARBOX_JOB_IMPL
#define GEARBOX_JOB_IMPL

#include <gearbox/core/Json.h>
#include <gearbox/core/ConfigFile.h>
#include <gearbox/core/HttpClient.h>
#include <gearbox/core/Hash.h>
#include <gearbox/job/Job.h>
#include <boost/shared_ptr.hpp>

namespace Gearbox
{
    typedef boost::shared_ptr<Job> JobPtr;
    typedef std::vector<std::string> Array;

    class JobImpl {
    public:
        JobImpl(const ConfigFile & cfg);
        JobImpl( const JobImpl & );
        virtual ~JobImpl();

        // to clone self
        virtual JobPtr job() const;
        virtual JobImpl * clone() const = 0;

        virtual void name( const std::string & name );
        virtual const std::string & name() const;
        
        virtual void type( Job::JobType t );
        virtual Job::JobType type() const;
        
        virtual void method( HttpClient::method_t m );
        virtual HttpClient::method_t method() const;

        virtual void api_version( const std::string & version);
        virtual const std::string & api_version() const;

        virtual void operation( const std::string & operation);
        virtual const std::string & operation() const;

        virtual void component( const std::string & component);
        virtual const std::string & component() const;

        virtual void resource_type( const std::string & resource_type);
        virtual const std::string & resource_type() const;

        virtual void resource_name( const std::string & resource_name);
        virtual const std::string & resource_name() const;

        virtual std::string resource_uri() const;

        virtual void base_uri( const std::string & base_uri);
        virtual const std::string & base_uri() const;

        virtual void parent_uri(const std::string & parent_uri);
        virtual const std::string & parent_uri() const;

        virtual void remote_ip( const std::string & remote_ip);
        virtual const std::string & remote_ip() const;

        virtual void remote_user( const std::string & remote_user);
        virtual const std::string & remote_user() const;

        virtual void content( const std::string & content );
        virtual const std::string & content() const;

        virtual void arguments( const Array & args );
        virtual void add_argument( const std::string & arg );
        virtual const Array & arguments() const;

        virtual void matrix_arguments( const Hash & matrix );
        virtual void add_matrix_argument( const std::string & name, const std::string & value );
        virtual const Hash & matrix_arguments() const;
        
        virtual void query_params( const Hash & params );
        virtual void add_query_param( const std::string & name, const std::string & value );
        virtual const Hash & query_params() const;

        virtual void headers( const Hash & head );
        virtual void add_header( const std::string & name, const std::string & value );
        virtual const Hash & headers() const;
        
        virtual void environ( const Hash & environ );
        virtual void add_environ( const std::string & name, const std::string & value );
        virtual const Hash & environ() const;

        virtual void status( const std::string & status_id );
        virtual const std::string & status() const;

        virtual void timeout( int t );
        virtual int timeout() const;

        virtual void on( Job::Event e, const Job & job );
        virtual JobPtr on(Job::Event e) const;

        virtual void event_status(const Status & s);
        virtual StatusPtr event_status() const;

        virtual const char * impltype() const = 0;

        virtual JobResponse run() const = 0;
        virtual void to_json(Json & job_data) const;
        
        virtual const Json & json_content() const;
        
        virtual const ConfigFile & cfg() const;

    private:
        JobImpl & operator=(const JobImpl &);
        class Private;
        Private * impl;
    };
}

#endif
