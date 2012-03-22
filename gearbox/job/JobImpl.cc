// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/job/JobImpl.h>
#include <gearbox/core/Errors.h>
#include <gearbox/job/JobManager.h>

namespace Gearbox
{
    static void
    set_job_from_name(JobImpl * job, const std::string & name) {
        // do_<op>_<component>_<resource>_<version>
        
        size_t pos = name.find('_');
        if( pos == std::string::npos || name.substr(0, 3) != "do_" )
            gbTHROW( std::invalid_argument("The string '"+name+"' is not a valid job name.") );

        size_t next = name.find('_', pos+1);
        if( next == std::string::npos )
            gbTHROW( std::invalid_argument("The string '"+name+"' is not a valid job name.") );

        std::string version = name.substr( name.rfind('_') + 1 );
        if( version[0] != 'v' || !isdigit(version[1]) )
            gbTHROW( std::invalid_argument("The string '"+name+"' is missing a version.") );
        job->api_version(version);

        std::string op = name.substr(pos+1,next-pos-1);
        job->operation(op);

        pos = next;
        next = name.find('_', pos+1);
        if( next == std::string::npos )
            gbTHROW( std::invalid_argument("The string '"+name+"' is missing a valid component.") );
        std::string component = name.substr(pos+1, next-pos-1);
        job->component(component);

        pos = next;
        next = name.find('_', pos+1);
        if( next == std::string::npos )
            gbTHROW( std::invalid_argument("The string '"+name+"' is missing a valid resource type.") );
        std::string resource = name.substr(pos+1, next-pos-1);
        job->resource_type(resource);
    }
    
    typedef std::map<Job::Event,JobPtr> EventHandlers;
    class JobImpl::Private {
    public:
        std::string name;

        std::string version;
        std::string operation;
        std::string component;
        std::string resource_type;
        std::string resource_name;
        std::string base_uri;
        std::string parent_uri;
        std::string remote_ip;
        std::string remote_user;

        Job::JobType type;
        HttpClient::method_t method;
        std::string content;
        Array arguments;
        Hash matrix_arguments;
        Hash query_params;
        Hash headers;
        Hash environ;
        std::string status_id;
        int timeout;
        
        Json * json_content;

        ConfigFile cfg;
        EventHandlers events;
        StatusPtr event_status;
        Private(const ConfigFile & c) 
            : base_uri("http://localhost:4080/internal"),
              type(Job::JOB_ASYNC), 
              method(HttpClient::METHOD_UNKNOWN), 
              timeout(600), 
              json_content(NULL),
              cfg(c) {}
        Private(const Private & copy)
            : name(copy.name),
              version(copy.version),
              operation(copy.operation),
              component(copy.component),
              resource_type(copy.resource_type),
              resource_name(copy.resource_name),
              base_uri(copy.base_uri),
              parent_uri(copy.parent_uri),
              remote_ip(copy.remote_ip),
              remote_user(copy.remote_user),
              type(copy.type),
              method(copy.method),
              content(copy.content),
              arguments(copy.arguments),
              matrix_arguments(copy.matrix_arguments),
              query_params(copy.query_params),
              headers(copy.headers),
              environ(copy.environ),
              status_id(copy.status_id),
              timeout(copy.timeout),
              json_content(NULL),
              cfg(copy.cfg),
              events(copy.events),
              event_status(copy.event_status) {}
        ~Private() {
            if(json_content) delete json_content;
        }
    };

    JobImpl::JobImpl(const ConfigFile & cfg) : impl(new Private(cfg)) {}
    JobImpl::JobImpl( const JobImpl & other ) : impl(new Private(*(other.impl))) {}
    JobImpl::~JobImpl() {
        if( impl ) delete impl;
    }
    
    JobPtr JobImpl::job() const {
        return JobPtr( new Job( this->clone() ) );
    }

    void JobImpl::name( const std::string & name ) {
        impl->name = name;
        // if name is not a uri then set up
        // the default properties from job name
        if( name.find("://") == std::string::npos ) {
            set_job_from_name(this, name);
        }
    }

    const std::string & JobImpl::name() const {
        return impl->name;
    }
    
    void JobImpl::type( Job::JobType t ) {
        impl->type = t;
    }

    Job::JobType JobImpl::type() const {
        return impl->type;
    }
    
    void JobImpl::method( HttpClient::method_t m ) {
        impl->method = m;
    }

    HttpClient::method_t JobImpl::method() const {
        return impl->method;
    }
    
    void JobImpl::api_version( const std::string & version) {
        impl->version = version;
    }

    const std::string & JobImpl::api_version() const {
        return impl->version;
    }

    void JobImpl::operation( const std::string & op) {
        if ( op == "put" ) {
            impl->operation = "create";
        }
        else if ( op == "post" ) {
            impl->operation = "update";
        }
        else {
            impl->operation = op;
        }
    }

    const std::string & JobImpl::operation() const {
        return impl->operation;
    }

    void JobImpl::component( const std::string & component) {
        impl->component = component;
    }

    const std::string & JobImpl::component() const {
        return impl->component;
    }

    void JobImpl::resource_type( const std::string & resource_type) {
        impl->resource_type = resource_type;
    }

    const std::string & JobImpl::resource_type() const {
        return impl->resource_type;
    }

    void JobImpl::resource_name( const std::string & resource_name) {
        impl->resource_name = resource_name;
    }

    const std::string & JobImpl::resource_name() const {
        return impl->resource_name;
    }

    std::string JobImpl::resource_uri() const {
        Uri uri(this->base_uri());
        uri /= this->resource_type();
        uri /= this->resource_name();
        return uri.canonical();
    }

    void JobImpl::base_uri( const std::string & base_uri) {
        impl->base_uri = base_uri;
    }

    const std::string & JobImpl::base_uri() const {
        return impl->base_uri;
    }

    void JobImpl::parent_uri( const std::string & parent_uri) {
        impl->parent_uri = parent_uri;
    }

    const std::string & JobImpl::parent_uri() const {
        return impl->parent_uri;
    }

    void JobImpl::remote_ip( const std::string & remote_ip) {
        impl->remote_ip = remote_ip;
    }

    const std::string & JobImpl::remote_ip() const {
        return impl->remote_ip;
    }

    void JobImpl::remote_user( const std::string & remote_user) {
        impl->remote_user = remote_user;
    }

    const std::string & JobImpl::remote_user() const {
        return impl->remote_user;
    }

    void JobImpl::content( const std::string & content ) {
        // delete json_content cache if they reset the content
        if( impl->json_content ) {
            delete impl->json_content;
            impl->json_content = NULL;
        }
        impl->content = content;
    }
    
    const std::string & JobImpl::content() const {
        return impl->content;
    }

    void JobImpl::arguments( const Array & args ) {
        impl->arguments = args;
    }
    void JobImpl::add_argument( const std::string & value ) {
        impl->arguments.push_back(value);
    }
    const Array & JobImpl::arguments() const {
        return impl->arguments;
    }
    
    void JobImpl::matrix_arguments( const Hash & matrix ) {
        impl->matrix_arguments = matrix;
    }
    void JobImpl::add_matrix_argument( const std::string & name, const std::string & value ) {
        impl->matrix_arguments[name] = value;
    }
    const Hash & JobImpl::matrix_arguments() const {
        return impl->matrix_arguments;
    }
    
    void JobImpl::query_params( const Hash & params ) {
        impl->query_params = params;
    }
    void JobImpl::add_query_param( const std::string & name, const std::string & value ) {
        impl->query_params[name] = value;
    }
    const Hash & JobImpl::query_params() const {
        return impl->query_params;
    }
    
    void JobImpl::headers( const Hash & head ) {
        impl->headers = head;
    }
    void JobImpl::add_header( const std::string & name, const std::string & value ) {
        impl->headers[name] = value;
    }
    const Hash & JobImpl::headers() const {
        return impl->headers;
    }
    
    void JobImpl::environ( const Hash & environ ) {
        impl->environ = environ;
    }
    void JobImpl::add_environ( const std::string & name, const std::string & value ) {
        impl->environ[name] = value;
    }
    const Hash & JobImpl::environ() const {
        return impl->environ;
    }
    
    void JobImpl::status( const std::string & status_id ) {
        impl->status_id = status_id;
    }
    const std::string & JobImpl::status() const {
        return impl->status_id; 
    }
    
    void JobImpl::timeout( int t ) {
        impl->timeout = t;
    }
    int JobImpl::timeout() const {
        return impl->timeout;
    }

    void JobImpl::on( Job::Event e, const Job & j ) {
        impl->events[e] = JobPtr(new Job(j));
    }

    JobPtr JobImpl::on( Job::Event e ) const {
        return impl->events[e];
    }

    void JobImpl::event_status(const Status & s) {
        impl->event_status.reset(new Status(s));
    }
    
    StatusPtr JobImpl::event_status() const {
        return impl->event_status;
    }

    void JobImpl::to_json(Json & job_data) const {
        job_data["version"] = this->api_version();
        if( this->type() == Job::JOB_SYNC ) {
            job_data["job_type"] = "sync";
        }
        else {
            job_data["job_type"] = "async";
        }
        job_data["operation"] = this->operation();
        job_data["component"] = this->component();
        job_data["resource"]["type"] = this->resource_type();
        if( ! this->resource_name().empty() ) {
            job_data["resource"]["name"] = this->resource_name();
        }
        if( ! this->remote_ip().empty() ) {
            job_data["remote"]["ip"] = this->remote_ip();
        }
        if( ! this->remote_user().empty() ) {
            job_data["remote"]["user"] = this->remote_user();
        }
        if( ! this->base_uri().empty() ) {
            job_data["base_uri"] = this->base_uri();
        }

        if ( ! this->parent_uri().empty() ) {
            job_data["parent_uri"] = this->parent_uri();
        }

        if( ! this->status().empty() ) {
            job_data["status"] = this->status();
        }

        job_data["arguments"] = Json::Array();
        for ( unsigned int i = 0; i < this->arguments().size(); i++ ) {
            job_data["arguments"][i] = this->arguments()[i];
        }
        
        job_data["matrix_arguments"] = Json::Object();
        Hash::const_iterator itr = this->matrix_arguments().begin();
        Hash::const_iterator end = this->matrix_arguments().end();
        for( ; itr != end; ++itr ) {
            job_data["matrix_arguments"][itr->first] = itr->second;
        }

        job_data["headers"] = Json::Object();
        Hash::const_iterator sr = this->headers().begin();
        for (; sr != this->headers().end(); ++sr) {
            job_data["headers"][(*sr).first] = (*sr).second;
        }

        job_data["environ"] = Json::Object();
        const Hash & env = this->environ();
        itr = env.begin();
        end = env.end();
        for ( ; itr != end; ++itr ) {
            job_data["environ"][itr->first] = itr->second;
        }
    
        job_data["query_params"] = Json::Object();
        const Hash & params = this->query_params();
        itr = params.begin();
        end = params.end();
        for ( ; itr != end; ++itr ) {
            job_data["query_params"][itr->first] = itr->second;
        }

        if ( impl->events.size() ) {
            EventHandlers::const_iterator it = impl->events.begin();
            EventHandlers::const_iterator end = impl->events.end();
            for( ; it != end; ++it ) {
                JobPtr j = this->on(it->first);
                if( ! j.get() ) continue;
                job_data["on"][ Job::event2str(it->first) ]["name"] = j->name();
                job_data["on"][ Job::event2str(it->first) ]["envelope"].parse(j->serialize());
            }
        }
        
        if ( impl->event_status.get() ) {
            job_data["event_status"].parse(impl->event_status->serialize());
        }

        if ( ! this->content().empty() ) {
            job_data["content"] = this->content();
        }
    }
    
    const Json & JobImpl::json_content() const {
        if( !impl->json_content ) {
            impl->json_content = new Json();
            JsonSchema * s = JobManager::getSchema(this, impl->cfg);
            impl->json_content->setSchema(s);
            try {
                impl->json_content->parse(impl->content.empty() ? "null" : impl->content);
            }
            catch( const JsonError & e ) {
                _WARN("Job " << this->name() << " json content parse error: " << e.what() << "\nContent: " << impl->content);
                throw;
            }
        } 
        return (*impl->json_content);
    }

    const ConfigFile & JobImpl::cfg() const {
        return impl->cfg;
    }
}
