// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/job/GearmanJobImpl.h>
#define LOGCAT "gearbox.job.gearman"
#include <gearbox/core/logger.h>
#include <gearbox/core/Errors.h>
#include <libgearman/gearman.h>
#include <gearbox/job/JobManager.h>
#include <gearbox/core/util.h>

using std::string;
using namespace Gearbox;

GearmanJobImpl::GearmanJobImpl( const ConfigFile & c ) 
    : super( c ), gm_host("localhost"), gm_port(4730), gm(NULL)
{
    gm_host = this->cfg().get_string_default("gearman", "host", "localhost" );
    gm_port = this->cfg().get_int_default("gearman", "port", 4730 );
    _DEBUG( "Using gearman connection: " << gm_host << ":" << gm_port );
    this->init();
}

GearmanJobImpl::GearmanJobImpl( const GearmanJobImpl & copy  ) 
    : super( copy ), gm_host(copy.gm_host), gm_port(copy.gm_port), gm(NULL)
{
    this->init();
}

void GearmanJobImpl::init() {
    gm = new gearman_client_st();
    if ( ! gearman_client_create(gm)) {
        // according to docs this should never happen
        gbTHROW( ERR_INTERNAL_SERVER_ERROR("gearman_client_create failed") );
    }
    if ( gearman_client_add_server( gm, gm_host.c_str(), gm_port ) != GEARMAN_SUCCESS ) {
        // ? need to cleanup gm?
        gbTHROW( ERR_INTERNAL_SERVER_ERROR( std::string("gearman_client_add_server: ")
                                         + gearman_client_error(gm) ) );
    }
}

JobImpl * GearmanJobImpl::clone() const {
    return new GearmanJobImpl(*this);
}

GearmanJobImpl::~GearmanJobImpl() {
    if(gm) {
        gearman_client_free(gm);
        delete gm;
    }
}

JobResponse
GearmanJobImpl::run() const {
    this->check_connection();

    Json job_data;
    this->to_json(job_data);

    if( this->type() == Job::JOB_ASYNC ) {
        return this->run_async(job_data);
    }
    return this->run_sync(job_data);
}

JobResponse GearmanJobImpl::run_async(Json & job_data) const {
    JobResponse resp;
    resp.job( this->job() );
    resp.status( *(this->create_status()) );

    job_data["status"] = resp.status()->name();
    
    string encoded;
    zlib_compress(job_data.serialize(),encoded);

    gearman_return_t ret;
    char jobid[GEARMAN_JOB_HANDLE_SIZE];
    
    while( true ) {
        ret = gearman_client_do_background(
            gm,
            this->name().c_str(),
            NULL,
            static_cast<const void*>(encoded.c_str()),
            static_cast<size_t>(encoded.size()),
            jobid
        );
        if ( ret != GEARMAN_SUCCESS ) {
            const char * err = gearman_client_error(gm);
            resp.status()->add_message( "Failed to run async job" + this->name() + ": " + err );
            sleep(1);
        }
        break;
    }
    resp.code(202);
    Json resp_content;
    resp_content["operation"] = this->operation();
    resp_content["uri"] = this->resource_uri();
    Uri uri( this->base_uri() );
    uri /= "status";
    uri /= resp.status()->name();
    resp_content["status_uri"] = uri.canonical();
    resp_content["progress"] = 0;
    _DEBUG("RESPONSE: " << resp_content.serialize() );
    resp.content(resp_content.serialize());
    return resp;
}

JobResponse GearmanJobImpl::run_sync(Json & job_data) const {
    JobResponse resp;
    resp.job( this->job() );
    resp.status( *(this->create_status()) );
    job_data["status"] = resp.status()->name();
    string encoded;
    zlib_compress(job_data.serialize(), encoded);
    
    if( ! resp.status()->state(Status::STATE_RUNNING) ) {
        resp.status()->sync();
        gbTHROW(
            ERR_PRECONDITION_FAILED(
                "Invalid status state transition from \"" 
                + Status::state2str(resp.status()->state())
                + "\" to \"RUNNING\" for status "
                + resp.status()->name()
            )
        );
    }
    size_t result_size;
    gearman_return_t ret;
    
    char * response;
    std::string respstr;

    while( true ) {
        response = (char*)gearman_client_do(
            gm,
            this->name().c_str(),
            NULL,
            static_cast<const void*>(encoded.c_str()),
            static_cast<size_t>(encoded.size()),
            &result_size,
            &ret
        );
        if( response ) {
            respstr = std::string(response,result_size);
            free(response);
        }
        
        // worker base class always returns success even in error
        // cases (in which you need to check the status to find
        // out about the error). So anytime success is not returned
        // means some network error or internal gearman server/client
        // error, in which case we need to retry
        if ( ret != GEARMAN_SUCCESS ) {
            const char * err = gearman_client_error(gm);
            resp.status()->add_message( "Failed to run sync job " + this->name() + ": " + err );
            sleep(1);
        }
        break;
    }
    
    Json resp_json;
    resp_json.parse( respstr );
    if( resp_json.hasKey("content") )
        resp.content(resp_json["content"].as<string>());
    
    int code = resp_json["code"].as<int>();
    resp.code(code);
    if( resp_json.hasKey("headers") ) {
        const Json::Object & o = resp_json["headers"].as<Json::Object>();
        Json::Object::const_iterator it = o.begin();
        for( ; it != o.end(); ++it ) {
            resp.add_header(it->first, it->second->as<string>());
        }
    }
    
    if( resp.status()->impltype() == string("transient") ) {
        if( code >= 300 ) {
            resp.status()->add_message(resp_json["message"].as<string>());
            resp.status()->fail( code );
        }
        else {
            resp.status()->success();
        }
    }
    return resp;
}

StatusPtr GearmanJobImpl::create_status() const {
    if( this->status().empty() ) {
        // no status, so create one
        if( this->type() == Job::JOB_ASYNC ) {
            std::string id = JobManager::gen_id("s");
            JobManager jm(this);
            JobPtr job = jm.job("do_create_global_status_v1");
            job->type(Job::JOB_SYNC);
            job->resource_name(id);
            
            Json content;
            content["name"] = id;
            content["operation"] = this->operation();
            content["component"] = this->component();
            
            content["uri"] = this->resource_uri();
            if( ! this->parent_uri().empty() ) {
                content["parent_uri"] = this->parent_uri();
            }
            job->content( content.serialize() );
            JobResponse resp = job->run();
            if( resp.status()->code() > 0 ) {
                throw_from_code(resp.status()->code(), resp.status()->messages()[0]);
            }
            StatusManager sm(this->cfg());
            sm.base_uri(this->base_uri());

            job = jm.job("do_get_global_status_v1");
            job->type(Job::JOB_SYNC);
            job->resource_name(id);
            job->add_argument(id);
            if( ! this->parent_uri().empty() ) {
                JobPtr j = jm.job("do_update_global_status_v1");
                j->type(Job::JOB_SYNC);
                std::string parent_id = Uri(this->parent_uri()).leaf();
                j->resource_name(parent_id);
                j->add_argument(parent_id);
                Json content;
                content["children"][0] = this->base_uri() + "/status/" + id;
                j->content( content.serialize() );
                j->run();
            }
            return sm.fetch(*job);
        }
        
        std::string id = JobManager::gen_id("st");
        // create Transient status object
        StatusManager sm("transient", this->cfg());
        return sm.create(id, this->operation(), this->resource_uri(), this->component());
    }
    if( this->type() == Job::JOB_ASYNC ) {
        StatusManager sm(this->cfg());
        return sm.fetch(this->status());
    }
    else {
        std::string id = JobManager::gen_id("st");
        // create Transient status object
        StatusManager sm("transient", this->cfg());
        return sm.create(id, this->operation(), this->resource_uri(), this->component());
    }
}

void
GearmanJobImpl::check_connection() const {
    int timeout = this->cfg().get_int_default("gearman_timeout", 60 );
    for ( int i = 1; i <= timeout; i++ ) {
        if( gearman_client_echo(gm, "RUOK", 4) == GEARMAN_SUCCESS ) {
            break;
        }
        _WARN("Gearman connection lost?  Trying again ...");
        if( i == timeout ) {
            gbTHROW( ERR_INTERNAL_SERVER_ERROR("Lost connection to backend queue") );
        }
        sleep(1);
    }
}

const char * GearmanJobImpl::impltype() const {
    return "gearman";
}
