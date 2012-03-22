// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/job/RestJobImpl.h>
#define LOGCAT "gearbox.job.gearman"
#include <gearbox/core/logger.h>
#include <gearbox/core/Errors.h>
#include <libgearman/gearman.h>
#include <gearbox/job/JobManager.h>

using std::string;
using namespace Gearbox;

RestJobImpl::RestJobImpl( const ConfigFile & c ) : super( c ) {}
RestJobImpl::RestJobImpl( const RestJobImpl & copy ) : super( copy ) {}

JobImpl * RestJobImpl::clone() const {
    return new RestJobImpl(*this);
}

RestJobImpl::~RestJobImpl() {}

JobResponse
RestJobImpl::run() const {

    HttpClient c;
    HttpResponse hr;

    Uri rest_uri( this->name() );

    string response;
    
    Array::const_iterator aitr = this->arguments().begin();
    for( ; aitr != this->arguments().end(); ++aitr ) {
        rest_uri /= *aitr;
    }
    
    Hash::const_iterator hitr = this->headers().begin();
    for( ; hitr != this->headers().end(); ++hitr ) {
        c.set_header( hitr->first, hitr->second );
    }
    
    if( ! this->parent_uri().empty() ) {
        c.set_header("Y-Status-Parent-Uri", this->parent_uri());
    }

    JobResponse jr;
    jr.job( this->job() );

    switch( this->method() ) {
    case HttpClient::METHOD_DELETE:
        hr = c.DELETE( rest_uri.canonical(), response );
        break;
    case HttpClient::METHOD_POST:
        hr = c.POST( rest_uri.canonical(), this->content(), response );
        break;
    case HttpClient::METHOD_PUT:
        hr = c.PUT( rest_uri.canonical(), this->content(), response );
        break;
    case HttpClient::METHOD_GET:
        hr = c.GET( rest_uri.canonical(), response );
        break;
    default:
        gbTHROW( ERR_INTERNAL_SERVER_ERROR("Invalid HTTP method.") );
    }

    jr.content(response);
    
    Json json_response;
    try {
        json_response.parse(response);
    }
    catch(...) {
        gbTHROW(
            ERR_INTERNAL_SERVER_ERROR(
                "Invalid json returned from remote job \"" + this->name() + "\": " + response
            )
        );
    }

    Hash headers;
    hr.get_headers( headers );
    jr.headers(headers);

    if ( hr.code() >= 300 ) {
        jr.status( *(this->create_status()) );
        if( ! jr.status()->state(Status::STATE_RUNNING) ) {
            jr.status()->sync();
            gbTHROW(
                ERR_PRECONDITION_FAILED(
                    "Invalid status state transition from \"" 
                    + Status::state2str(jr.status()->state())
                    + "\" to \"RUNNING\" for status "
                    + jr.status()->name()
                )
            );
        }

        // Async requests should return 202/status unless there is a problem -- throw early if
        // we were unable to dispatch the async operation.
        std::string meth =
            this->method() == HttpClient::METHOD_POST   ? "POST" :
            this->method() == HttpClient::METHOD_DELETE ? "DELETE" :
            this->method() == HttpClient::METHOD_PUT    ? "PUT" :
            this->method() == HttpClient::METHOD_GET    ? "GET" :
                                                          "UNKNOWN";
        if( json_response.hasKey("messages") ) {
            jr.status()->add_message("Failed to " + meth + " " + rest_uri.canonical() + " got: " + json_response["messages"][0].as<std::string>());
        } else {
            jr.status()->add_message("Failed to " + meth + " " + rest_uri.canonical() + " got: invalid response");
        }

        jr.status()->fail(hr.code());
        jr.code(hr.code());
        return jr;
    }

    jr.code(0);
    
    if( hr.code() == 202 ) {
        // update the parent status with the new child status uri
        std::string status_uri = json_response["status_uri"].as<string>();
        if( ! this->parent_uri().empty() ) {
            JobManager jm(this);
            JobPtr job = jm.job("do_update_global_status_v1");
            job->type(Job::JOB_SYNC);
            std::string parent_uri_id = Uri(this->parent_uri()).leaf();
            job->resource_name( parent_uri_id );
            job->add_argument( parent_uri_id );
            Json job_data;
            job_data["children"][0] = status_uri;
            job->content(job_data.serialize());
            job->run();
        }
        jr.status( *(this->create_status(status_uri)) );
    }
    else {
        jr.status( *(this->create_status()) );
        if( ! jr.status()->state(Status::STATE_RUNNING) ) {
            jr.status()->sync();
            gbTHROW(
                ERR_PRECONDITION_FAILED(
                    "Invalid status state transition from \"" 
                    + Status::state2str(jr.status()->state())
                    + "\" to \"RUNNING\" for status "
                    + jr.status()->name()
                )
            );
        }
        jr.status()->success();
    }
    
    return jr;
}

StatusPtr RestJobImpl::create_status(const std::string & status_uri) const {
    if( status_uri.empty() ) {
        std::string id = JobManager::gen_id("st");
        StatusManager sm("transient", this->cfg());
        return sm.create(id, this->operation(), this->resource_uri());
    }
    StatusManager sm(this->cfg());
    Uri status(status_uri);
    status.path_pop(); // chop off name
    status.path_pop(); // chop off status resource type
    sm.base_uri( status.canonical() );
    return sm.fetch( Uri(status_uri) );
}

const char * RestJobImpl::impltype() const {
    return "rest";
}
