// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/core/logger.h>
#include <gearbox/job/JobStatusImpl.h>
#include <gearbox/core/Uri.h>
#include <gearbox/core/Errors.h>

namespace Gearbox {
    
    JobStatusImpl::JobStatusImpl(const ConfigFile & c)
        : super(c) {}

    JobStatusImpl::JobStatusImpl( const JobStatusImpl & other ) 
        : super(other), status_job(new Job(*(other.status_job))) {}

    JobStatusImpl::~JobStatusImpl() {}

    StatusImpl * JobStatusImpl::clone() const {
        return new JobStatusImpl(*this);
    }

    void JobStatusImpl::job( const Job & job ) {
        this->status_job.reset(new Job(job));
    }
    
    void JobStatusImpl::load() {
        JobResponse resp;
        while( 1 ) {
            resp = status_job->run();
            if( ! resp.status()->is_success() ) {
                _WARN("failed to fetch status, got: " << resp.status()->serialize());
            }
            else {
                break;
            }
            sleep(3);
        }
        Json json;
        _TRACE("Loading: " << resp.content());
        json.parse(resp.content());
        this->json(json);
        this->super::load();
    }
    
    void JobStatusImpl::insert() {
        gbTHROW( std::runtime_error("job status insert not implemented") );
    }
    
    void JobStatusImpl::sync() {
        this->super::sync();
    }

    const char * JobStatusImpl::impltype() const {
        return "job";
    }

    void JobStatusImpl::on(Status::Event e, const Job & job) {
        gbTHROW( ERR_NOT_IMPLEMENTED("cannot set event handlers for remote job status object") );
    }

    JobPtr JobStatusImpl::on(Status::Event e, const JobManager & jm) const {
        gbTHROW( ERR_NOT_IMPLEMENTED("cannot get event handlers for remote job status object") );
    }
    

}
