// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include "WorkerTestAgents.h"
#include <gearbox/core/util.h>
#include <gearbox/core/Errors.h>
#include <queue>
#include <glob.h>
#include <boost/filesystem/operations.hpp>
namespace bfs=boost::filesystem;

static const std::string DBDIR(LOCALSTATEDIR "/gearbox/db/test-agents/");

namespace Gearbox {
    WorkerTestAgents::WorkerTestAgents(const std::string & config) : super(config) {

        this->register_handler(
            "do_get_testagents_thing_v1",
            static_cast<Worker::handler_t>(&WorkerTestAgents::thing_handler)
        );
        this->register_handler(
            "do_post_testagents_thing_v1",
            static_cast<Worker::handler_t>(&WorkerTestAgents::thing_handler)
        );
        this->register_handler(
            "do_delete_testagents_thing_v1",
            static_cast<Worker::handler_t>(&WorkerTestAgents::thing_handler)
        );


        this->register_handler(
            "do_reg_testagents_A_v1", 
            static_cast<Worker::handler_t>(&WorkerTestAgents::dummy_handler)
        );
        this->register_handler(
            "do_reg_testagents_B_v1", 
            static_cast<Worker::handler_t>(&WorkerTestAgents::dummy_handler)
        );
        this->register_handler(
            "do_reg_testagents_C_v1", 
            static_cast<Worker::handler_t>(&WorkerTestAgents::dummy_handler)
        );
        this->register_handler(
            "do_reg_testagents_D_v1", 
            static_cast<Worker::handler_t>(&WorkerTestAgents::dummy_handler)
        );
                               

        this->register_handler(
            "do_unreg_testagents_A_v1", 
            static_cast<Worker::handler_t>(&WorkerTestAgents::dummy_handler)
        );
        this->register_handler(
            "do_unreg_testagents_B_v1", 
            static_cast<Worker::handler_t>(&WorkerTestAgents::dummy_handler)
        );
        this->register_handler(
            "do_unreg_testagents_C_v1", 
            static_cast<Worker::handler_t>(&WorkerTestAgents::dummy_handler)
        );
        this->register_handler(
            "do_unreg_testagents_D_v1", 
            static_cast<Worker::handler_t>(&WorkerTestAgents::dummy_handler)
        );
    }

    Worker::response_t
    WorkerTestAgents::thing_handler( const Job & job, JobResponse & resp ) {
        Json content;
        if( bfs::exists( DBDIR + job.resource_name() ) ) {
            content.parseFile(DBDIR + job.resource_name());
        }

        if( job.operation() == "get" ) {
            resp.content( content.serialize() );
            return WORKER_SUCCESS;
        }
            
        Json agents;
        agents.parseFile( server_root() + "/test-agents-agents.conf" );
        
        resp.status()->add_message("calling agents");
        
        if( job.operation() == "create" ) {
            content["id"] = job.resource_name();
            write_file( DBDIR + job.resource_name(), content.serialize());
            // for the create/register precess we will use the
            // global worker for running agent configs
            JobPtr run_agents = this->job_manager().job("do_run_global_agents_v1");
            Json agent_content;
            agent_content["agents"] = agents["register"];
            agent_content["content"] = content.serialize();
            run_agents->content(agent_content.serialize());
            JobResponse r = run_agents->run();
            Status s = *(r.status());
            // poll for agents to be done
            do {
                sleep(1);
                s.sync();
            } while( ! s.has_completed() );

            if ( ! s.is_success() ) {
                throw_from_code( s.code(), s.messages().back() );
            }
        }
        else {
            // job.operation == delete
            JobQueue q = this->job_manager().job_queue( agents["unregister"] );
            JobManager::job_queue_apply(q, &Job::content, content.serialize());
            JobManager::job_queue_run(q);
            bfs::remove( DBDIR + job.resource_name() );
        }
        
        resp.status()->add_message("done");
        return WORKER_SUCCESS;
    }

    Worker::response_t
    WorkerTestAgents::dummy_handler(const Job & job, JobResponse & resp) {
        std::string msg;
        msg = job.resource_type() + " ";
        msg += job.operation() == "reg" ? "registered" : "unregistered";
        msg += " for " + job.json_content()["id"].as<std::string>();

        // give us time from smoke tests to verify the progress of the
        // agents job
        if( job.operation() == "reg" ) {
            sleep(10);
        }

        resp.status()->add_message(msg);
        resp.status()->meta( resp.status()->name(), job.json_content()["id"] );
        return WORKER_SUCCESS;
    }
}
