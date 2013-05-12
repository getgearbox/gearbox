// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <libgearman/gearman.h>
#include <gearbox/worker/Worker.h>
#define LOGCAT "gearbox.worker"
#include <gearbox/core/logger.h>
#include <log4cxx/ndc.h>
#include <vector>
#include <queue>
#include <gearbox/core/Errors.h>
#include <gearbox/core/Uri.h>
#include <gearbox/core/util.h>

#include <gearbox/core/ConfigFile.h>
#include <gearbox/store/dbconn.h>
#include <soci/soci.h>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <log4cxx/level.h>

// for kill and signal
#include <sys/types.h>
#include <signal.h>

using namespace soci;
using namespace Gearbox::Database;
using namespace Gearbox;
using std::string;

namespace Gearbox {

struct callback_args {
    Worker * obj;
    string name;
    Worker::handler_t handler;
    int in_use;
    ConfigFile cfg;
    callback_args(const ConfigFile & c) : obj(NULL), handler(NULL), in_use(0), cfg(c) {}
};

    struct QueueEntry {
        JobPtr job;
        int delay;
        QueueEntry(JobPtr j, int d = 0) : job(j), delay(d) {}
    };

    class Worker::Private {
    public:
        JobManager jm;
        StatusManager sm;
        ConfigFile cfg;
        gearman_worker_st * worker;
        int max_requests;
        int counter;
        std::map<std::string, callback_args *> argmap;
        std::queue<QueueEntry> afterwards;
        Private(const ConfigFile & c)
            : jm(c),
              sm(c),
              cfg(c),
              worker(new gearman_worker_st()),
              max_requests(0),
              counter(0) {
            try {
                if ( gearman_worker_create( worker ) == NULL ) {
                    gbTHROW( ERR_INTERNAL_SERVER_ERROR("Failed to initialize worker" ) );
                }
                
                std::string gm_host = cfg.get_string_default("gearman", "host", "localhost");
                int gm_port = cfg.get_int_default("gearman", "port", 4730);
                
                _DEBUG("Using gearman connection: " << gm_host << ":" << gm_port);
                if ( gearman_worker_add_server( worker, gm_host.c_str(), gm_port ) != GEARMAN_SUCCESS ) {
                    gbTHROW( ERR_INTERNAL_SERVER_ERROR(string("failed to add server: ") + gearman_worker_error( worker ) ) );
                }
            }
            catch(...) {
                gearman_worker_free(worker);
                delete worker;
                throw;
            }
                
        }
        ~Private() {
            std::map<std::string, callback_args *>::iterator itr = argmap.begin();
            std::map<std::string, callback_args *>::iterator end = argmap.end();
            for( ; itr != end; ++itr ) {
                delete itr->second;
            }
            gearman_worker_free( worker );
            delete worker;
        }
    };

    static void 
    handle_event(
        Job::Event ev,
        const Job & job,
        const Status & status
    ) {
        JobPtr on_event = job.on(ev);
        if( on_event.get() ) {
            on_event->event_status( status );
            on_event->run();
        }
    }

static void*
run_job (
    gearman_job_st* job,
    void* cb_arg,
    size_t* result_size,
    gearman_return_t* ret_ptr
) {
    callback_args * args = static_cast<callback_args*>(cb_arg);
    void* result = NULL;

    int shhh_logging = args->name == "do_get_global_status_v1";
    
    const log4cxx::LevelPtr & rootLevel = log4cxx::Logger::getRootLogger()->getLevel();
    const log4cxx::LevelPtr & accLevel = log4cxx::Logger::getLogger("access")->getLevel();
    
    if( shhh_logging ) {
        log4cxx::Logger::getRootLogger()->setLevel( log4cxx::Level::getError() );
        log4cxx::Logger::getLogger("access")->setLevel( log4cxx::Level::getError() );
    }
    
    _DEBUG("Running job for " << args->name);
    Worker::response_t resp = Worker::WORKER_SUCCESS;
    *ret_ptr = GEARMAN_SUCCESS;

    Json content;

    JobPtr job_ptr;
    JobResponse job_response;
    job_response.code(0);

    int rollback = 0;
    int ranjob = 0;
    StatusPtr status;
    try {
        
        string workload;
        zlib_decompress(std::string(
                   static_cast<const char *>(gearman_job_workload(job)),
                   gearman_job_workload_size(job)
               ), workload);
        
        Json work;
        work.parse(workload);

        args->obj->status_manager().base_uri( work["base_uri"].as<string>() );
        job_ptr = args->obj->job_manager().job(args->name, workload);

        if( job_ptr->type() == Job::JOB_ASYNC && work.hasKey("status") && !work["status"].as<string>().empty()) {
            status = args->obj->status_manager().fetch( work["status"].as<string>() );
        }
        else {
            StatusManager sm("transient", args->cfg);
            Uri rsrc_uri = job_ptr->base_uri();
            string id = work.hasKey("status") && !work["status"].as<string>().empty()
                ? work["status"].as<string>() : JobManager::gen_id("st");
            status = sm.create(id, job_ptr->operation(), job_ptr->resource_uri(), job_ptr->component());
        }

        job_response.status(*status);
        log4cxx::NDC::push(status->name());
        status->starting();

        LOG4CXX_INFO( log4cxx::Logger::getLogger("request"), "ENTER " << args->name << " " << ( work.hasKey("content") ? work["content"].as<string>() : "null" ) );
        
        if( work["arguments"].length() > 0 ) {
            log4cxx::NDC::push(work["arguments"][0].as<string>());
        }
        else if( work["resource"].hasKey("name") ) {
            log4cxx::NDC::push( work["resource"]["name"].as<string>() );
        }
        
        LOG4CXX_INFO( log4cxx::Logger::getLogger("access"), "ENTER " << args->name);

        args->obj->job_manager().parent_uri( status->uri() );

        Status::State state = status->state();
        if( state == Status::STATE_PENDING ) {
            if( ! status->state( Status::STATE_RUNNING ) ) {
                status->sync();
                gbTHROW(
                    ERR_PRECONDITION_FAILED(
                        "Invalid status state transition from \"" 
                        + Status::state2str(status->state())
                        + "\" to \"RUNNING\" for status "
                        + status->name()
                    )
                );
            }
        }

        handle_event(Job::EVENT_STARTED, *job_ptr, *status);
        
        // set job manager parent uri to this uri
        // for any child jobs created
        args->obj->job_manager().parent_uri(
            job_ptr->base_uri() + "/status/" + status->name()
        );
        
        args->obj->pre_request(*job_ptr);
        _TRACE("Dispatching to " << args->name);
        Worker::handler_t h = args->handler;
        ranjob=1;
        // see if we should stop even before starting
        status->checkpoint();
        resp = (args->obj->*h)(*job_ptr, job_response);
        if( resp == Worker::WORKER_ERROR ) {
            gbTHROW( ERR_INTERNAL_SERVER_ERROR(args->name + " returned unknown error") );
        }
        content["message"] = "OK";
        content["status"] = "OK";
        content["code"] = job_response.code();
    }
    catch (const Error & err ) {
        _ERROR(err.what());
        content["message"] = err.what();
        content["status"] = err.name();
        content["code"] = err.code();
        rollback = 1;
    }
    catch (const JsonError & err ) {
        _ERROR(err.what());
        content["message"] = err.what();
        content["status"] = "BAD_REQUEST";
        content["code"] = 400;
        rollback = 1;
    }
    catch ( const soci::soci_error & err ) {
        _ERROR("SOCI Error: " << err.what() << " from statement:\n" << lastsql());
        content["message"] = err.what();
        content["status"] = "INTERNAL_SERVER_ERROR";
        content["code"] = 500;
        rollback = 1;
    }
    catch ( const WorkerStop & err ) {
        resp = Worker::WORKER_ERROR;
        content["message"] = err.what();
        content["status"] = "GONE";
        content["code"] = ERR_GONE().code();
        handle_event(Job::EVENT_STOPPED, *job_ptr, *job_response.status());
        rollback = 1;
    }
    catch ( const std::exception & err ) {
        _ERROR(err.what());
        content["message"] = err.what();
        content["status"] = "INTERNAL_SERVER_ERROR";
        content["code"] = 500;
        rollback = 1;
    }
    
    // if we have not run the job yet there was probably an exception
    // in the setup for the job, so just try to run it again
    if( !ranjob && job_ptr.get() ) {
        args->obj->job_manager().delay(*job_ptr,1);
        resp = Worker::WORKER_CONTINUE;
    }
    
    try {
        if(rollback || resp == Worker::WORKER_RETRY) getconn().rollback();
    }
    catch ( ... ) {
        // dont care, we will likely get "cant rollback, no transaction" errors
    }
    
    Hash::const_iterator it = job_response.headers().begin();
    Hash::const_iterator end = job_response.headers().end();
    for( ; it != end; ++it ) {
        content["headers"][it->first] = it->second;
    }

    content["content"] = job_response.content();

    if( job_response.status().get() ) {
        Status & s = *(job_response.status());
        try {
            if( resp != Worker::WORKER_CONTINUE && resp != Worker::WORKER_RETRY ) {
                // we should double check that the status
                // progress has been set to 100% ... in case
                // an exception was thrown and code never finalized
                if( s.progress() != 100 ) {
                    if( content["code"].as<int>() >= 300 ) {
                        s.add_message( content["message"].as<string>() );
                        s.fail( content["code"].as<int>() );
                    }
                    else {
                        // doesnt look like a failure message
                        // so assuming success
                        s.success();
                    }
                }
                Status::State state = s.state();
                if( job_ptr.get() ) {
                    if( state == Status::STATE_COMPLETED || state == Status::STATE_STOPPED ) {
                        handle_event(Job::EVENT_COMPLETED,*job_ptr,s);
                        if( s.is_success() ) {
                            handle_event(Job::EVENT_SUCCEEDED, *job_ptr, s);
                        }
                        else {
                            handle_event(Job::EVENT_FAILED, *job_ptr, s);
                        }
                    }
                    else if( state == Status::STATE_CANCELLED ) {
                        handle_event(Job::EVENT_CANCELLED, *job_ptr, s);
                    }
                }
            }
        }
        catch( const std::exception & err ) {
            _ERROR("Failed to update Status " << job_response.status()->name() << ": " << err.what()); 
        }
    }
    if( status.get() ) {
        status->stopping();
    }

    if( resp == Worker::WORKER_RETRY && job_ptr.get() ) {
        args->obj->job_manager().retry(*job_ptr);
    }
    
    int sc = content["code"].as<int>();
    if( sc >= 300 ) {
        // dont run "afterwards" jobs if this job has failed
        args->obj->clear_afterwards();
    }

    try {
        if( job_ptr.get() ) args->obj->post_request(*job_ptr);
    }
    catch( const std::exception & e ) {
        // dont care here, calling post request is just a courtesy,
        // if they throw it is not our problem
        _WARN( "exception in post_request: " << e.what());
    }

    string outData = content.serialize();    

    if( sc >=300 ) {
        _ERROR(args->name << " Failed! Returned: [" << outData << ']');
    }
    else {
        _DEBUG(args->name << " Succeeded! Returned: [" << outData << ']');
    }

    LOG4CXX_INFO( log4cxx::Logger::getLogger("request"), "EXIT " << args->name << " " << content.serialize() );
    std::string ending =
        resp == Worker::WORKER_CONTINUE ? "CONTINUE" : 
        resp == Worker::WORKER_RETRY ? "RETRY" :
        content["message"].as<string>();
    LOG4CXX_INFO( log4cxx::Logger::getLogger("access"), "EXIT " << args->name << ": " << ending);
    log4cxx::NDC::clear();
    log4cxx::MDC::remove("request_id");

    *ret_ptr = GEARMAN_SUCCESS;
    // must copy data to be returned ... gearman will free it later
    size_t outDataSize = outData.size();
    *result_size = outDataSize;
    result = malloc(*result_size);
    memcpy(result, outData.c_str(), *result_size);
        
    // has the function been deregistered?
    // if so then we need to free it and remove
    // it from the map
    if( !args->in_use ) {
        delete args;
    }
    if( shhh_logging ) {
        log4cxx::Logger::getRootLogger()->setLevel( rootLevel );
        log4cxx::Logger::getLogger("access")->setLevel( accLevel );
    }
    return result;
}

void
termination_handler (int signum) {
    // reset the signal handler to do the default
    struct sigaction new_action;
    new_action.sa_handler = SIG_DFL;
    sigemptyset (&new_action.sa_mask);
    new_action.sa_flags = 0;
    sigaction(signum, &new_action, NULL);
    // kill everything in my process group
    // ie all child processes
    kill(0,signum);
}

Worker::Worker(const string & config) {
    log_init(config);

    // start a new process group and become the leader
    if( setsid() > 0 ) {
        struct sigaction new_action, old_action;
        
        /* Set up the structure to specify the new action. */
        new_action.sa_handler = termination_handler;
        sigemptyset (&new_action.sa_mask);
        new_action.sa_flags = 0;
        
        // setups INT HUP and TERM signal handlers to 
        // propogate that signal to all child processes
        
        sigaction (SIGINT, NULL, &old_action);
        if (old_action.sa_handler != SIG_IGN)
            sigaction (SIGINT, &new_action, NULL);
        sigaction (SIGHUP, NULL, &old_action);
        if (old_action.sa_handler != SIG_IGN)
            sigaction (SIGHUP, &new_action, NULL);
        sigaction (SIGTERM, NULL, &old_action);
        if (old_action.sa_handler != SIG_IGN)
            sigaction (SIGTERM, &new_action, NULL);
    }

    impl = new Private(config);
}

Worker::~Worker() {
    delete impl;
}

void
Worker::register_handler(
    const string & name,
    handler_t handler
) {
    std::auto_ptr<callback_args> args(new callback_args(impl->cfg));

    args->obj     = this;
    args->name    = name;
    args->handler = handler;
    args->in_use  = 1;

    _DEBUG("registering handler for [" << name << ']');
    if ( gearman_worker_add_function( impl->worker, name.c_str(), 0, run_job, args.get() ) != GEARMAN_SUCCESS ) {
        gbTHROW( ERR_INTERNAL_SERVER_ERROR( string("gearman_worker_add_function: ") +  gearman_worker_error( impl->worker ) ) );
    }

    impl->argmap[name] = args.release();
}

void
Worker::deregister_handler(
    const string & name
) {
    if( impl->argmap.find(name) == impl->argmap.end() ) {
        gbTHROW( ERR_INTERNAL_SERVER_ERROR( "cannot unregister uknown handler: " + name ) );
    }

    _DEBUG("unregistering handler for [" << name << ']');
    if ( gearman_worker_unregister( impl->worker, name.c_str() ) != GEARMAN_SUCCESS ) {
        gbTHROW( ERR_INTERNAL_SERVER_ERROR( string("gearman_worker_unregister: ") + gearman_worker_error( impl->worker ) ) );
    }
    
    impl->argmap[name]->in_use = 0;
    delete impl->argmap[name];
    impl->argmap.erase(name);
}

void
Worker::run() {
    while ( 1 ) {
        if ( gearman_worker_work( impl->worker ) != GEARMAN_SUCCESS ) {
            gbTHROW( ERR_INTERNAL_SERVER_ERROR( string("gearman_worker_work: ") + gearman_worker_error( impl->worker ) ) );
        }
        impl->counter++;
        if( impl->max_requests && impl->counter >= impl->max_requests ) {
            break;
        }
    }
}

void
Worker::pre_request(const Job & job) {
}

void
Worker::post_request(const Job & job) {
    while( !this->impl->afterwards.empty() ) {
        QueueEntry next  = this->impl->afterwards.front();
        this->impl->afterwards.pop();
        if( next.delay ) {
            _INFO("Running delay job for " << next.job->name() << " in " << next.delay << " seconds");
            this->job_manager().delay(*next.job, next.delay);
        }
        else {
            _INFO("Running job: " << next.job->name());
            next.job->run();
        }
    }
}

void
Worker::afterwards(const Job & job, const std::string & name, int delay) {
    JobPtr j(new Job(job));
    j->name(name);
    this->impl->afterwards.push(QueueEntry(j,delay));
}

void
Worker::afterwards(const Job & job, const char * name, int delay) {
    this->afterwards(job,std::string(name),delay);
}

void
Worker::afterwards(const Job & job, int delay) {
    this->impl->afterwards.push(QueueEntry(JobPtr(new Job(job)),delay));
}

void
Worker::clear_afterwards() {
    // reset to empty queue
    this->impl->afterwards = std::queue<QueueEntry>();
}

JobManager &
Worker::job_manager()
{
    return impl->jm;
}

StatusManager &
Worker::status_manager()
{
    return impl->sm;
}

const ConfigFile & Worker::cfg() const {
    return impl->cfg;
}

void Worker::max_requests(int max) {
    impl->max_requests = max;
}

int Worker::request_count() const {
    return impl->counter;
}

} // namespace
