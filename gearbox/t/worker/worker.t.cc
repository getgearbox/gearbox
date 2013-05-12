// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/core/logger.h>
#include "stub/gearman.hh"

#include <tap/trivial.h>
#include <gearbox/core/Json.h>
#include <gearbox/worker/Worker.h>
#include <gearbox/core/Errors.h>
#include <gearbox/job/StatusManager.h>
using namespace Gearbox;

#include <gearbox/store/dbconn.h>
using namespace soci;

#include <memory>
using namespace std;

static std::string test_error;

struct TestWorker : public Gearbox::Worker {
    TestWorker( const std::string & config ) : Gearbox::Worker(config) {}
    using Gearbox::Worker::register_handler;
    using Gearbox::Worker::deregister_handler;
    using Gearbox::Worker::max_requests;

    Gearbox::Worker::response_t test( const Job & job, JobResponse & resp ) {
        if( test_error == "Error" ) {
            gbTHROW( ERR_NOT_FOUND("testing Error") );
        }
        else if( test_error == "Json" ) {
            Json out;
            out.parse("this is garbage");
        }
        else if( test_error == "soci" ) {
            gbTHROW( soci_error("soci error") );
        }
        else if( test_error == "std" ) {
            gbTHROW( runtime_error("runtime error") );
        }
        else if( test_error == "continue" ) {
            return Gearbox::Worker::WORKER_CONTINUE;
        }
        return Gearbox::Worker::WORKER_SUCCESS;
    }
};


int main() {
    chdir(TESTDIR);
    TEST_START(41);
    log_init("./unit.conf");

    ConfigFile cfg("./unit.conf");
    StatusManager sm( cfg );
    
    auto_ptr<TestWorker> w;
    error["gearman_worker_create"] = true;
    THROWS( new TestWorker("./unit.conf"), "INTERNAL_SERVER_ERROR [500]: Failed to initialize worker" );
    error.clear();

    error["gearman_worker_add_server"] = true;
    THROWS( new TestWorker("./unit.conf"), "INTERNAL_SERVER_ERROR [500]: failed to add server: gearmand error" );
    error.clear();
    
    NOTHROW( w.reset( new TestWorker("./unit.conf") ) );

    error["gearman_worker_add_function"] = true;
    THROWS( w->register_handler("do_test_component_resource_v1", static_cast<Gearbox::Worker::handler_t>(&TestWorker::test)), 
            "INTERNAL_SERVER_ERROR [500]: gearman_worker_add_function: gearmand error" );
    error.clear();
    NOTHROW( w->register_handler("do_test_component_resource_v1", static_cast<Gearbox::Worker::handler_t>(&TestWorker::test)) );

    THROWS( w->deregister_handler("bogus"), "INTERNAL_SERVER_ERROR [500]: cannot unregister uknown handler: bogus" );

    error["gearman_worker_unregister"] = true;
    THROWS( w->deregister_handler("do_test_component_resource_v1"), "INTERNAL_SERVER_ERROR [500]: gearman_worker_unregister: gearmand error" );
    error.clear();

    NOTHROW( w->deregister_handler("do_test_component_resource_v1") );
    NOTHROW( w->register_handler("do_test_component_resource_v1", static_cast<Gearbox::Worker::handler_t>(&TestWorker::test)) );

    error["gearman_worker_work"] = true;
    THROWS( w->run(), "INTERNAL_SERVER_ERROR [500]: gearman_worker_work: gearmand error" );
    error.clear();

    w->max_requests(1);

    w->job_manager().base_uri("http://localhost:4080/foo/v1");
    JobPtr job = w->job_manager().job("do_test_component_resource_v1");
    
    // does it update status?
    // workload is global static variable used in gearman.hh stub
    std::string status;
#define MKSTATUS(s) {                             \
        status = s;                               \
        sm.create(status, "TESTING")->state(Status::STATE_PENDING); \
        job->status(s);                           \
        job->run();                               \
    }

    MKSTATUS("s-1234567891");
    test_error = "Error";
    NOTHROW( w->run() );

    StatusPtr s = sm.fetch(status);
    IS( s->progress(), 100 );
    IS( s->code(), 404 );
    IS( s->messages().size(), 1 );
    IS( s->messages()[0], "NOT_FOUND [404]: testing Error" );

    MKSTATUS("s-1234567892");
    test_error = "Json";
    NOTHROW( w->run() );    

    s = sm.fetch(status);
    IS( s->progress(), 100 );
    IS( s->code(), 400 );
    IS( s->messages().size(), 1 );
    IS( s->messages()[0], "Json Exception: parse error: lexical error: invalid string in json text.\n"
        "                                       this is garbage\n"
        "                     (right here) ------^\n");
    MKSTATUS("s-1234567893");
    test_error = "soci";
    NOTHROW( w->run() );
    
    s = sm.fetch(status);
    IS( s->progress(), 100 );
    IS( s->code(), 500 );
    IS( s->messages().size(), 1 );
    IS( s->messages()[0], "soci error" );

    MKSTATUS("s-1234567894");
    test_error = "std";
    NOTHROW( w->run() );

    s = sm.fetch(status);
    IS( s->progress(), 100 );
    IS( s->code(), 500 );
    IS( s->messages().size(), 1 );
    IS( s->messages()[0], "runtime error" );

    MKSTATUS("s-1234567895");
    // basic no content
    test_error = "";
    NOTHROW( w->run() );    

    s = sm.fetch(status);
    IS( s->progress(), 100 );
    IS( s->code(), 0 );
    IS( s->messages().size(), 0 );

    MKSTATUS("s-31415926");
    test_error = "continue";
    NOTHROW( w->run() );

    s = sm.fetch(status);
    IS( s->progress(), 0 );
    IS( s->messages().size(), 0 );

    // we really have no way to check what is going on here 
    // w->run() cannot throw.  We are really just adding
    // coverage checks and verifying that we dont core dump

    Json work;
    work["arguments"][0] = "n-1234567891";
    work.clear();
    gWorkload = work.serialize();
    NOTHROW( w->run() );    
    
    work["request"]["_content"] = "{\"key\":\"value\"}";
    gWorkload = work.serialize();
    NOTHROW( w->run() );    
    
    work.clear();
    work["request"]["_content"] = "{\"name\":\"t-1234\"}";
    gWorkload = work.serialize();
    NOTHROW( w->run() );    

    work.clear();
    work["resource"]["name"] = "t-1234";
    gWorkload = work.serialize();
    NOTHROW( w->run() );    

    TEST_END;
}
