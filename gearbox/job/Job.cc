// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/job/Job.h>
#include <gearbox/job/JobImpl.h>
#include <gearbox/job/JobManager.h>

#include <gearbox/core/logger.h>

namespace Gearbox
{
    Job::~Job() {
        if( impl ) delete impl;
    }

    Job::Job(const Job & copy) : impl(copy.impl->clone()) {}

    Job & Job::operator=(const Job & copy) {
        if( this == &copy ) return *this;
        if(impl) delete impl;
        impl = copy.impl->clone();
        return *this;
    }

    Job & Job::content( const std::string & c ) {
        impl->content(c);
        return *this;
    }
    
    const std::string & Job::content() const {
        return impl->content();
    }
    
    const Json & Job::json_content() const {
        return impl->json_content();
    }

    Job & Job::arguments( const Array & args ) {
        impl->arguments(args);
        return *this;
    }
    
    Job & Job::add_argument( const std::string & value ) {
        impl->add_argument(value);
        return *this;
    }

    const Array & Job::arguments() const {
        return impl->arguments();
    }

    Job & Job::matrix_arguments( const Hash & matrix ) {
        impl->matrix_arguments(matrix);
        return *this;
    }
    
    Job & Job::add_matrix_argument( const std::string & name, const std::string & value ) {
        impl->add_matrix_argument( name, value );
        return *this;
    }

    const Hash & Job::matrix_arguments() const {
        return impl->matrix_arguments();
    }

    Job & Job::query_params( const Hash & params ) {
        impl->query_params(params);
        return *this;
    }

    Job & Job::add_query_param( const std::string & name, const std::string & value ) {
        impl->add_query_param( name, value );
        return *this;
    }

    const Hash & Job::query_params() const {
        return impl->query_params();
    }

    Job & Job::headers( const Hash & head ) {
        impl->headers(head);
        return *this;
    }

    Job & Job::add_header( const std::string & name, const std::string & value ) {
        impl->add_header( name, value );
        return *this;
    }

    const Hash & Job::headers() const {
        return impl->headers();
    }

    Job & Job::environ( const Hash & environ ) {
        impl->environ( environ );
        return *this;
    }

    Job & Job::add_environ( const std::string & name, const std::string & value ) {
        impl->add_environ( name, value );
        return *this;
    }

    const Hash & Job::environ() const {
        return impl->environ();
    }

    Job & Job::status( const std::string & s ) {
        impl->status(s);
        return *this;
    }

    const std::string & Job::status() const {
        return impl->status();
    }

    Job & Job::timeout( int t ) {
        impl->timeout(t);
        return *this;
    }

    Job & Job::name( const std::string & name ) {
        impl->name(name);
        return *this;
    }

    const std::string & Job::name() const {
        return impl->name();
    }

    const std::string & Job::base_uri() const {
        return impl->base_uri();
    }

    Job & Job::type( Job::JobType t ) {
        impl->type(t);
        return *this;
    }
    
    Job::JobType Job::type() const {
        return impl->type();
    }   

    int Job::timeout() const {
        return impl->timeout();
    }
    
    Job & Job::api_version( const std::string & ver ) {
        impl->api_version(ver);
        return *this;
    }
    
    const std::string & Job::api_version() const {
        return impl->api_version();
    }

    Job & Job::operation( const std::string & op ) {
        impl->operation(op);
        return *this;
    }

    const std::string & Job::operation() const {
        return impl->operation();
    }

    Job & Job::component( const std::string & comp ) {
        impl->component(comp);
        return *this;
    }

    const std::string & Job::component() const {
        return impl->component();
    }

    Job & Job::resource_type( const std::string & type ) {
        impl->resource_type(type);
        return *this;
    }

    const std::string & Job::resource_type() const {
        return impl->resource_type();
    }

    Job & Job::resource_name( const std::string & name ) {
        impl->resource_name(name);
        return *this;
    }

    const std::string & Job::resource_name() const {
        return impl->resource_name();
    }

    std::string Job::resource_uri() const {
        return impl->resource_uri();
    }

    Job & Job::remote_ip( const std::string & ip ) {
        impl->remote_ip(ip);
        return *this;
    }

    const std::string & Job::remote_ip() const {
        return impl->remote_ip();
    }

    Job & Job::remote_user( const std::string & user ) {
        impl->remote_user(user);
        return *this;
    }

    const std::string & Job::remote_user() const {
        return impl->remote_user();
    }

    void Job::on( Job::Event e, const Job & j ) {
        impl->on(e,j);
    }

    JobPtr Job::on( Job::Event e ) const {
        return impl->on(e);
    }

    std::string 
    Job::event2str(Job::Event ev) { 
        switch(ev) {
        case EVENT_COMPLETED:  return "COMPLETED";
        case EVENT_FAILED:     return "FAILED";
        case EVENT_SUCCEEDED:  return "SUCCEEDED";
        case EVENT_STARTED:    return "STARTED";
        case EVENT_STOPPED:    return "STOPPED";
        case EVENT_CANCELLED:  return "CANCELLED";
        default:               return "UNKNOWN";
        }
    }

    Job::Event
    Job::str2event(const std::string & ev) {
        return
            ev == "COMPLETED" ? EVENT_COMPLETED :
            ev == "FAILED"    ? EVENT_FAILED    :
            ev == "SUCCEEDED" ? EVENT_SUCCEEDED :
            ev == "STARTED"   ? EVENT_STARTED   :
            ev == "STOPPED"   ? EVENT_STOPPED   :
            ev == "CANCELLED" ? EVENT_CANCELLED :
                                EVENT_UNKNOWN;
    }

    void
    Job::event_status(const Status & s) {
        impl->event_status(s);
    }

    StatusPtr
    Job::event_status() const {
        return impl->event_status();
    }
    

    const char * Job::impltype() const {
        return impl->impltype();
    }

    std::string Job::serialize() const {
        Json out;
        impl->to_json(out);
        return out.serialize();
    }

    JobResponse Job::run() const {
        // validate the content if schema available
        JsonSchema * s = JobManager::getSchema(impl, impl->cfg());
        if( s ) {
            Json tmp;
            tmp.setSchema(s);
            tmp.parse(impl->content().empty() ? "null" : impl->content());
        }
        JobResponse resp( impl->run() );
        _DEBUG("Ran job " << this->name() << " with status " << resp.status()->name());
        return resp;
    }

    Job::Job( JobImpl * i ) : impl(i) {}
}
