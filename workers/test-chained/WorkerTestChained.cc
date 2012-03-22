// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include "WorkerTestChained.h"

#include <gearbox/core/util.h>
#include <gearbox/core/Errors.h>

#include <queue>

#include <glob.h>

#include <boost/filesystem/operations.hpp>
namespace bfs=boost::filesystem;

static const std::string DBDIR(LOCALSTATEDIR "/gearbox/db/test-chained/");

namespace Gearbox {
    WorkerTestChained::WorkerTestChained(const std::string & config) : super(config) {
        WORKER_REGISTER(WorkerTestChained, do_get_testchained_hello_v1);
        WORKER_REGISTER(WorkerTestChained, do_get_internal_hello1_v1);
        WORKER_REGISTER(WorkerTestChained, do_post_testchained_hello2_v1);

        WORKER_REGISTER(WorkerTestChained, do_get_testchained_goodbye_v1);
        WORKER_REGISTER(WorkerTestChained, do_post_testchained_goodbye_v1);
        WORKER_REGISTER(WorkerTestChained, do_append_internal_goodbye1_v1);
        WORKER_REGISTER(WorkerTestChained, do_append_internal_goodbye2_v1);

        WORKER_REGISTER(WorkerTestChained, do_get_testchained_thing_v1);
        WORKER_REGISTER(WorkerTestChained, do_post_testchained_thing_v1);
        WORKER_REGISTER(WorkerTestChained, do_reg_internal_service1_v1);
        WORKER_REGISTER(WorkerTestChained, do_post_testchained_service2_v1);

        WORKER_REGISTER(WorkerTestChained, do_delete_testchained_thing_v1);
        WORKER_REGISTER(WorkerTestChained, do_unreg_internal_service1_v1);
        WORKER_REGISTER(WorkerTestChained, do_delete_testchained_service2_v1);
    }

    // I am not sure why we would want to do a chained syncronous get, but 
    // you can chain a bunch of sync jobs together
    Worker::response_t
    WorkerTestChained::do_get_testchained_hello_v1( const Job & job, JobResponse & resp ) {

        Json content("Hello from job");
        
        // do internal hello1 which just appends it name to our content
        JobPtr j(new Job(job));
        j->name("do_get_internal_hello1_v1");
        j->type(Job::JOB_SYNC);
        j->content(content.serialize());
        JobResponse r = j->run();

        
        // create sync http rest job back to localhost which takes
        // the output from previous job and adds its own name.
        j = this->job_manager().job(HttpClient::METHOD_POST, job.base_uri() + "/hello2");
        j->content(r.content());
        j->headers(r.headers());
        r = j->run();
        
        resp.content( r.content() );
        resp.headers( r.headers() );
        return WORKER_SUCCESS;
    }

    Worker::response_t
    WorkerTestChained::do_get_internal_hello1_v1( const Job & job, JobResponse & resp ) {
        Json in = job.json_content();
        in = in.as<std::string>() + " and job1";
        resp.add_header("job1-header", "1");
        resp.content(in.serialize());
        return WORKER_SUCCESS;
    }

    // this is a SYNC post call configured via the httpd-test-chained.conf
    Worker::response_t
    WorkerTestChained::do_post_testchained_hello2_v1( const Job & job, JobResponse & resp ) {
        Json in = job.json_content();
        in = in.as<std::string>() + " and job2";
        resp.headers(job.headers());
        resp.add_header("job2-header", "1");
        resp.content(in.serialize());
        return WORKER_SUCCESS;
    }

    Worker::response_t
    WorkerTestChained::do_get_testchained_goodbye_v1( const Job & job, JobResponse & resp ) {
        resp.content( slurp(DBDIR + job.resource_name()) );
        return WORKER_SUCCESS;
    }

    Worker::response_t
    WorkerTestChained::do_post_testchained_goodbye_v1( const Job & job, JobResponse & resp ) {
        resp.status()->add_message("processing from " + job.name());
        Json content("Goodbye from job");
        write_file( DBDIR + job.resource_name(), content.serialize() );
        
        // do internal goodbye1 which just appends its name to our content
        this->afterwards(job, "do_append_internal_goodbye1_v1");
        // don't finalize the status, are going to keep going
        return WORKER_CONTINUE;
    }

    Worker::response_t
    WorkerTestChained::do_append_internal_goodbye1_v1( const Job & job, JobResponse & resp ) {
        resp.status()->add_message("processing from " + job.name());
        Json content;
        content.parseFile( DBDIR + job.resource_name() );
        content = content.as<std::string>() + " and job1";
        write_file( DBDIR + job.resource_name(), content.serialize() );
        
        // do internal goodbye2 which just appends its name to our content
        this->afterwards(job, "do_append_internal_goodbye2_v1");
        // don't finalize the status, are going to keep going
        return WORKER_CONTINUE;
    }

    Worker::response_t
    WorkerTestChained::do_append_internal_goodbye2_v1( const Job & job, JobResponse & resp ) {
        resp.status()->add_message("processing from " + job.name());
        Json content;
        content.parseFile( DBDIR + job.resource_name() );
        content = content.as<std::string>() + " and job2";
        write_file( DBDIR + job.resource_name(), content.serialize() );

        // finally done so dont continue
        return WORKER_SUCCESS;
    }



    Worker::response_t
    WorkerTestChained::do_get_testchained_thing_v1( const Job & job, JobResponse & resp ) {
        resp.content( slurp(DBDIR + job.resource_name()) );
        return WORKER_SUCCESS;
    }

    Worker::response_t
    WorkerTestChained::do_post_testchained_thing_v1( const Job & job, JobResponse & resp ) {
        resp.status()->add_message("processing from " + job.name());
        Json out;
        out["id"] = job.resource_name();
        write_file( DBDIR + job.resource_name(), out.serialize());

        // our new thing needs to be registered with 2 fancy
        // services.  They can both be registered at the same 
        // time in parallel.
        

        JobManager & jm = this->job_manager();
        std::queue<JobResponse> responses;
        
        // service 1 is registered via async local worker
        responses.push(
            jm.job("do_reg_internal_service1_v1")->content(out.serialize()).run()
        );

        // service 2 is registered via async POST (back to localhost)
        responses.push(
            jm.job(HttpClient::METHOD_POST, job.base_uri() + "/service2")->content(out.serialize()).run()
        );

        while( !responses.empty() ) {
            // refresh the status
            StatusPtr s = responses.front().status();
            s->sync();
            if( s->has_completed() ) {
                if( s->is_success() ) {
                    responses.pop();
                }
                else {
                    throw_from_code(s->code(), s->messages()[0]);
                }
                // pause between polling again
                sleep(1);
            }
        }
        return WORKER_SUCCESS;
    }

    Worker::response_t
    WorkerTestChained::do_reg_internal_service1_v1(const Job & job, JobResponse & resp) {
        resp.status()->add_message("service1 registered");
        return WORKER_SUCCESS;
    }

    Worker::response_t
    WorkerTestChained::do_post_testchained_service2_v1(const Job & job, JobResponse & resp) {
        resp.status()->add_message("service2 registered");
        return WORKER_SUCCESS;
    }

    Worker::response_t
    WorkerTestChained::do_delete_testchained_thing_v1( const Job & job, JobResponse & resp ) {
        // our new thing needs to be unregistered with 2 fancy
        // services.  service 1 must be unregistered before service 2
        
        Json content;
        content.parseFile( DBDIR + job.resource_name() );

        JobManager & jm = this->job_manager();
        
        JobQueue jobs;
        jobs.push_back( std::vector<JobPtr>() );
        jobs.push_back( std::vector<JobPtr>() );
        
        // first gen jobs only has service1, unregister happens via local worker
        jobs[0].push_back(jm.job("do_unreg_internal_service1_v1"));

        // second gen jobs only has service 2, unregister happens via DELETE
        // http call on remote worker
        jobs[1].push_back(jm.job(HttpClient::METHOD_DELETE, job.base_uri() + "/service2"));
        
        JobManager::job_queue_apply<std::string>(jobs, &Job::content, content.serialize());

        jm.job_queue_run(jobs);
        return WORKER_SUCCESS;
    }

    Worker::response_t
    WorkerTestChained::do_unreg_internal_service1_v1(const Job & job, JobResponse & resp) {
        resp.status()->add_message("service1 unregistered");
        return WORKER_SUCCESS;
    }

    Worker::response_t
    WorkerTestChained::do_delete_testchained_service2_v1(const Job & job, JobResponse & resp) {
        resp.status()->add_message("service2 unregistered");
        return WORKER_SUCCESS;
    }
}
