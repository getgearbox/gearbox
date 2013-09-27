// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>
#include <gearbox/job/StatusManager.h>
#include <gearbox/core/util.h>
#include <gearbox/core/logger.h>
#include <workers/gearbox/WorkerGearbox.h>
#include <gearbox/core/JsonSchema.h>
#include <libgen.h>
#include <unistd.h>

using namespace Gearbox;

using std::string;

int main(int argc, char *argv[]) {
    TEST_START(71);

    string basedir = string(dirname(argv[0])) + "/../";
    chdir(basedir.c_str());

    log_init("./unit.conf");
    OK( run("./mkdb") == 0 );

    ConfigFile cfg("./unit.conf");
    StatusManager sm( cfg );
    JobManager jm(cfg);

    WorkerGearbox w("./unit.conf");

    Json in;
    in["arguments"] = Json::Array();
    Json out;
    JsonSchema createSchema, getSchema;
    createSchema.parseFile("../../gearbox/schemas/create-global-status-v1.js");
    getSchema.parseFile("../../gearbox/schemas/get-global-status-response-v1.js");

    JobResponse resp;
    JobPtr job = jm.job("do_get_global_status_v1");
    job->add_argument("s-bogus");

    THROWS( w.do_get_global_status_v1(*job,resp), "NOT_FOUND [404]: status s-bogus not found" );

    StatusPtr s = sm.create("s-testing", "CREATE", "http://host:4080/cc/v1/node/n-1234");
    s->state(Status::STATE_RUNNING);
    s->progress(10);
    s->add_message("first message");

    job = jm.job("do_get_global_status_v1");
    job->add_argument("s-testing");
    NOTHROW( w.do_get_global_status_v1(*job,resp) );
    NOTHROW( out.parse(resp.content()) );
    NOTHROW( out.validate(&getSchema) );

    IS( out["operation"].as<string>(), "CREATE" );
    IS( out["uri"].as<string>(), "http://host:4080/cc/v1/node/n-1234" );
    IS( out["status_uri"].as<string>(), "http://localhost:4080/transient/status/s-testing" );
    IS( out["progress"].as<int>(), 10 );
    OK( !out.hasKey("code") );
    IS( out["messages"].length(), 1 );
    IS( out["messages"][0].as<string>(), "first message" );
    IS( out["state"], "RUNNING" );

    s->progress(80);
    s->add_message("second message");

    out.clear();

    NOTHROW( w.do_get_global_status_v1(*job,resp) );
    NOTHROW( out.parse(resp.content()) );
    NOTHROW( out.validate(&getSchema) );

    IS( out["operation"].as<string>(), "CREATE" );
    IS( out["uri"].as<string>(), "http://host:4080/cc/v1/node/n-1234" );
    IS( out["status_uri"].as<string>(), "http://localhost:4080/transient/status/s-testing" );
    IS( out["progress"].as<int>(), 80 );
    OK( !out.hasKey("code") );
    IS( out["messages"].length(), 2 );
    IS( out["messages"][0].as<string>(), "first message" );
    IS( out["messages"][1].as<string>(), "second message" );

    s->success();
    s->add_message("final message");
    IS( s->state(), Status::STATE_COMPLETED );

    out.clear();
    NOTHROW( w.do_get_global_status_v1(*job,resp) );
    NOTHROW( out.parse(resp.content()) );
    NOTHROW( out.validate(&getSchema) );

    IS( out["operation"].as<string>(), "CREATE" );
    IS( out["uri"].as<string>(), "http://host:4080/cc/v1/node/n-1234" );
    IS( out["status_uri"].as<string>(), "http://localhost:4080/transient/status/s-testing" );
    IS( out["progress"].as<int>(), 100 );
    OK( out.hasKey("code") );
    IS( out["code"].as<int>(), 0 );
    IS( out["messages"].length(), 3 );
    IS( out["messages"][0].as<string>(), "first message" );
    IS( out["messages"][1].as<string>(), "second message" );
    IS( out["messages"][2].as<string>(), "final message" );

    s = sm.create("s-12345", "CREATE", "http://host:4080/cc/v1/node/n-12345");
    job->arguments(Array());
    job->add_argument("s-12345");
    s->progress(60);
    out.clear();
    NOTHROW( w.do_get_global_status_v1(*job, resp) );
    NOTHROW( out.parse(resp.content()) );
    NOTHROW( out.validate(&getSchema) );

    IS( out["progress"].as<int>(), 60 );
    OK( !out.hasKey("code") );

    s->fail(123);
    IS( s->state(), Status::STATE_COMPLETED );

    out.clear();
    NOTHROW( w.do_get_global_status_v1(*job,resp) );
    NOTHROW( out.parse(resp.content()) );
    NOTHROW( out.validate(&getSchema) );

    IS( out["operation"].as<string>(), "CREATE" );
    IS( out["uri"].as<string>(), "http://host:4080/cc/v1/node/n-12345" );
    IS( out["status_uri"].as<string>(), "http://localhost:4080/transient/status/s-12345" );
    IS( out["progress"].as<int>(), 100 );
    OK( out.hasKey("code") );
    IS( out["code"].as<int>(), 123 );
    IS( out["messages"].length(), 0 );

    job = jm.job("do_create_global_status_v1");
    Json r;
    r["uri"] = "http://www.sgi.com/";
    r["operation"] = "DELETE";
    r["name"] = "s-1234567890";
    r["component"] = "testing";
    NOTHROW( r.validate(&createSchema) );
    job->content(r.serialize());
    IS ( w.do_create_global_status_v1(*job, resp), Worker::WORKER_SUCCESS );

    job = jm.job("do_get_global_status_v1");
    job->add_argument("s-1234567890");

    out.clear();
    NOTHROW( w.do_get_global_status_v1(*job, resp) );
    NOTHROW( out.parse(resp.content()) );
    NOTHROW( out.validate(&getSchema) );

    IS( out["operation"].as<string>(), "DELETE" );
    IS( out["component"].as<string>(), "testing");
    IS( out["uri"].as<string>(), "http://www.sgi.com/" );
    IS( out["status_uri"].as<string>(), "http://localhost:4080/transient/status/s-1234567890" );
    IS( out["progress"].as<int>(), 0 );

    StatusPtr s2 = sm.create("s-test-cancel", "CREATE", "http://host:4080/cc/v1/node/n-1234");
    IS( s2->state(), Status::STATE_PENDING );
    NOTHROW( s2->state( Status::STATE_RUNNING ) );
    IS( s2->state(), Status::STATE_RUNNING );
    NOTHROW( s2->checkpoint() );
    NOTHROW( s2->state( Status::STATE_STOPPING ) );
    THROWS( s2->checkpoint(), "Worker Stop" );
    IS( s2->state(), Status::STATE_STOPPED );

    TEST_END;
}
