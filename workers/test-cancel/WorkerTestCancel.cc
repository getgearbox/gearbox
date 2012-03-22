// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include "WorkerTestCancel.h"

#define LOGCAT "gearbox.worker.test-cancel"
#include <gearbox/core/logger.h>

#include <glob.h>

#include <boost/filesystem/operations.hpp>
namespace bfs=boost::filesystem;

static const std::string DBDIR(LOCALSTATEDIR "/gearbox/db/test-cancel/");

namespace Gearbox {
    WorkerTestCancel::WorkerTestCancel(const std::string & config) : super(config) {
        WORKER_REGISTER(WorkerTestCancel, do_post_testcancel_thing_v1);
        WORKER_REGISTER(WorkerTestCancel, do_cancel_testcancel_thing_v1);

        WORKER_REGISTER(WorkerTestCancel, do_post_testcancel_continuation_v1);
        WORKER_REGISTER(WorkerTestCancel, do_run_testcancel_continuation_v1);
        WORKER_REGISTER(WorkerTestCancel, do_finish_testcancel_continuation_v1);
    }
    
    Worker::response_t
    WorkerTestCancel::do_post_testcancel_thing_v1( const Job & job, JobResponse & resp ) {
        JobPtr onCancel = this->job_manager().job("do_cancel_testcancel_thing_v1");
        resp.status()->on(Status::EVENT_CANCEL, *onCancel);

        int stop = time(NULL) + 30;
        while( stop >= time(NULL) ) {
            int p = resp.status()->progress();
            if ( p < 100 ) {
                p = p + 10;
                resp.status()->progress( p );
                resp.status()->checkpoint();                 
                sleep(5);
                _DEBUG(resp.status()->serialize());
            }
            else {
                return WORKER_SUCCESS;
            }
        }
        return WORKER_ERROR;
    }

    Worker::response_t
    WorkerTestCancel::do_cancel_testcancel_thing_v1( const Job & job, JobResponse & resp ) {
        resp.status()->add_message("on cancel callback called");
        return WORKER_SUCCESS;
    }

    Worker::response_t
    WorkerTestCancel::do_post_testcancel_continuation_v1( const Job & job, JobResponse & resp ) {
        // we have do_post with a continuation of do_finish.  The do_finish
        // is only called via on-completion handler for do_run
        JobPtr run = this->job_manager().job("do_run_testcancel_continuation_v1");
        Job finish(job);
        finish.name("do_finish_testcancel_continuation_v1");
        run->on(Job::EVENT_COMPLETED, finish);
        this->afterwards(*run);
        return WORKER_CONTINUE;
    }

    Worker::response_t
    WorkerTestCancel::do_run_testcancel_continuation_v1( const Job & job, JobResponse & resp ) {
        resp.status()->add_message("run called");
        // this will retry indefinately, we want to test the cancellation of an
        // status in progress that is suspended waiting upon child completion events
        return WORKER_RETRY;
    }

    Worker::response_t
    WorkerTestCancel::do_finish_testcancel_continuation_v1( const Job & job, JobResponse & resp ) {
        // this will never get called since do_run will never complete
        return WORKER_SUCCESS;
    }
}
