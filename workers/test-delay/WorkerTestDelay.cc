// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include "WorkerTestDelay.h"

#include <gearbox/core/util.h>
#include <gearbox/core/logger.h>

#include <glob.h>

#include <boost/filesystem/operations.hpp>
namespace bfs=boost::filesystem;

static const std::string DBDIR(LOCALSTATEDIR "/gearbox/db/test-delay/");

namespace Gearbox {
    WorkerTestDelay::WorkerTestDelay(const std::string & config)
        : super(config) {
        WORKER_REGISTER(WorkerTestDelay, do_get_testdelay_counter_v1);
        WORKER_REGISTER(WorkerTestDelay, do_post_testdelay_counter_v1);
        WORKER_REGISTER(WorkerTestDelay, do_delete_testdelay_counter_v1);
        WORKER_REGISTER(WorkerTestDelay, do_increment_testdelay_counter_v1);
    }

    Worker::response_t
    WorkerTestDelay::do_get_testdelay_counter_v1( const Job & job, JobResponse & resp ) {
        resp.content( slurp( DBDIR + job.resource_name() ) );
        return WORKER_SUCCESS;
    }

    Worker::response_t
    WorkerTestDelay::do_post_testdelay_counter_v1( const Job & job, JobResponse & resp ) {
        const std::string & start = job.matrix_arguments().get_default("start", std::string("0"));
        write_file( DBDIR + job.resource_name(), start );

        this->afterwards(job, "do_increment_testdelay_counter_v1", job.matrix_arguments().get_default("delay", 1));
        return WORKER_CONTINUE;
    }

    Worker::response_t
    WorkerTestDelay::do_increment_testdelay_counter_v1( const Job & job, JobResponse & resp ) {

        int newval = 1 + boost::lexical_cast<int>(
            slurp( DBDIR + job.resource_name() )
        );
        
        write_file(
            DBDIR + job.resource_name(),
            boost::lexical_cast<std::string>(newval)
        );
        
        int start = job.matrix_arguments().get_default("start", 0);
        int end   = job.matrix_arguments().get_default("end", 10);
        
        // increment progress for each stage
        resp.status()->add_message("set to " + boost::lexical_cast<std::string>(newval));
        
        if( newval >= end ) {
            return WORKER_SUCCESS;
        }
        else {
            resp.status()->progress( resp.status()->progress() + (end - start) );
        }
        
        const std::string & retry = job.matrix_arguments().get_default("retry", std::string(""));
        if ( !retry.empty() ) {
            resp.status()->add_message("retry attempt number " + boost::lexical_cast<std::string>(resp.status()->failures()+1));
            return WORKER_RETRY;
        } else {
            // run this same job again one second from now
            this->afterwards(job, job.matrix_arguments().get_default("delay", 1));
        }
        return WORKER_CONTINUE;
    }
    
    Worker::response_t
    WorkerTestDelay::do_delete_testdelay_counter_v1( const Job & job, JobResponse & resp ) {
        bfs::remove( DBDIR + job.resource_name());
        return WORKER_SUCCESS;
    }
}
