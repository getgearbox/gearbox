// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
namespace bfs=boost::filesystem;

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

#include <gearbox/core/logger.h>
#include <gearbox/core/util.h>
#include <gearbox/core/JsonSchema.h>
#include <gearbox/job/Job.h>
#include <gearbox/job/JobManager.h>
#include <gearbox/job/StatusManager.h>
using namespace Gearbox;

#include <stub/gearman.hh>

int main() {
    chdir(TESTDIR);
    TEST_START(80);
    log_init("./unit.conf");
    ConfigFile cfg("./unit.conf");

    JsonSchema envSchema;
    envSchema.parseFile("../../worker/schemas/job-envelope.js");

    JobManager jm(cfg);
    jm.base_uri("http://localhost:4080/testing");
    // sync job tests
    THROWS( jm.job("bogus_jobname"), "The string 'bogus_jobname' is not a valid job name." );
    THROWS( jm.job("do_foopledoop"), "The string 'do_foopledoop' is not a valid job name." );
    THROWS( jm.job("do_foo_v1"), "The string 'do_foo_v1' is missing a valid component." );
    THROWS( jm.job("do_get_foo_v1"), "The string 'do_get_foo_v1' is missing a valid resource type." );

    JobPtr j1 = jm.job("do_get_trm_tier_v1");
    j1->type(Job::JOB_SYNC);
    IS( j1->name(), "do_get_trm_tier_v1" );
    IS( j1->api_version(), "v1" );
    IS( j1->operation(), "get" );
    IS( j1->component(), "trm" );
    IS( j1->resource_type(), "tier" );
    IS( j1->serialize(), slurp("jobs_data/do_get_trm_tier_v1") );

    Json json;
    OK( json.parse(j1->serialize()).validate(&envSchema) );
    JobResponse resp;
    NOTHROW( resp = j1->run() );
    OK( resp.status()->is_success() );
    IS( resp.code(), 200 );

    Json sd;
    sd["name"] = "s-123456";
    sd["operation"] = "CREATE";
    sd["uri"] = "http://localhost/foo/v1/beep";

    JobPtr j2 = jm.job("do_create_global_status_v1");
    j2->type(Job::JOB_SYNC);
    j2->content( sd.serialize() );
    IS( j2->name(), "do_create_global_status_v1" );
    IS( j2->api_version(), "v1" );
    IS( j2->operation(), "create" );
    IS( j2->component(), "global" );
    IS( j2->resource_type(), "status" );
    IS( j2->serialize(), slurp("jobs_data/do_create_global_status_v1") );
    OK( json.parse(j2->serialize()).validate(&envSchema) );

    NOTHROW( resp = j2->run() );
    OK( resp.status()->is_success() );
    IS( resp.code(), 200 );

    JobPtr j3 = jm.job("do_audit_nc_image_v1");
    j3->type(Job::JOB_SYNC);
    IS( j3->name(), "do_audit_nc_image_v1" );
    IS( j3->api_version(), "v1" );
    IS( j3->operation(), "audit" );
    IS( j3->component(), "nc" );
    IS( j3->resource_type(), "image" );
    IS( j3->serialize(), slurp("jobs_data/do_audit_images_v1") );
    OK( json.parse(j3->serialize()).validate(&envSchema) );
    NOTHROW( resp = j3->run() );
    OK( resp.status()->is_success() );
    IS( resp.code(), 200 );

    // test for updated mtime on fetch
    StatusManager sm(cfg);
    StatusPtr s = sm.fetch( resp.status()->name() );
    time_t mtime1 = s->mtime();
    sleep(2);
    s = sm.fetch( resp.status()->name() );
    time_t mtime2 = s->mtime();
    IS( mtime1, mtime2 );
    sleep(2);
    s = sm.fetch( resp.status()->name() );
    time_t mtime3 = s->mtime();
    IS( mtime2, mtime3 );

    response["do_get_global_status_v1"].parseFile("./jobs_data/do_get_global_status_v1");

    JobPtr j4 = jm.job("do_update_trm_opsdb_v1");
    OK( json.parse(j4->serialize()).validate(&envSchema) );
    NOTHROW( resp = j4->run() );
    IS( resp.status()->progress(), 0 );
    IS( resp.status()->has_completed(), false );
    IS( resp.status()->code(), -1 );

    response["do_get_global_status_v1"]["progress"] = 100;
    response["do_get_global_status_v1"]["code"] = 0;

    resp.status()->sync();
    OK( resp.status()->has_completed() );
    OK( resp.status()->is_success() );
    IS( resp.status()->progress(), 100 );
    IS( resp.status()->code(), 0 );

    response["do_get_global_status_v1"].parseFile("./jobs_data/do_get_global_status_v1");
    JobPtr j5 = jm.job("do_update_trm_roles_v1");
    OK( json.parse(j5->serialize()).validate(&envSchema) );

    NOTHROW( resp = j5->run() );
    IS( resp.status()->messages().size(), 0 );
    OK( ! resp.status()->has_completed() );
    IS( resp.status()->code(), -1 );

    response["do_get_global_status_v1"]["progress"] = 25;
    NOTHROW( resp.status()->sync() );

    IS( resp.status()->progress(), 25 );
    IS( resp.status()->code(), -1 );

    response["do_get_global_status_v1"]["progress"] = 50;
    NOTHROW( resp.status()->sync() );

    OK( ! resp.status()->has_completed() );
    IS( resp.status()->progress(), 50 );
    IS( resp.status()->code(), -1 );

    response["do_get_global_status_v1"]["progress"] = 75;
    NOTHROW( resp.status()->sync() );

    OK( ! resp.status()->has_completed() );
    IS( resp.status()->progress(), 75 );
    IS( resp.status()->code(), -1 );

    response["do_get_global_status_v1"]["progress"] = 100;
    response["do_get_global_status_v1"]["code"] = 0;
    response["do_get_global_status_v1"]["messages"][0] = "Baa!";
    NOTHROW( resp.status()->sync() );

    OK( resp.status()->has_completed() );
    OK( resp.status()->is_success() );
    IS( resp.status()->progress(), 100 );
    IS( resp.status()->code(), 0 );
    Json meta_exp;
    IS( resp.status()->meta(), meta_exp );
    _DEBUG(resp.status()->serialize());
    const std::vector<std::string> & messages = resp.status()->messages();
    IS( messages.size(), 1 );
    IS( messages[0], "Baa!" );

    Json j;
    j.parse( "{\"children\":[],\"meta\":{\"a\":[1,2,\"b\"],\"zyx\":\"c\"},\"code\":0,\"component\":\"nc\",\"ctime\":1307739168,\"failures\":0,\"messages\":[],\"mtime\":1307739168,\"operation\":\"audit\",\"progress\":100,\"state\":\"COMPLETED\",\"status_uri\":\"http://localhost:4080/transient/status/st-6wxkjpx0rx4w9xn7v5eg8dzj2a\",\"uri\":\"http://localhost:4080/testing/image/\"}" );
    s = sm.fetch( j );

    IS( s->mtime(), 1307739168 );
    IS( s->ctime(), 1307739168 );
    meta_exp.parse( "{\"a\":[1,2,\"b\"],\"zyx\":\"c\"}" );
    IS( s->meta(), meta_exp );

    IS( s->name(), "st-6wxkjpx0rx4w9xn7v5eg8dzj2a" );
    IS( s->children().size(), 0 );
    IS( s->code(), 0 );
    IS( s->component(), "nc" );
    IS( s->messages().size(), 0 );
    IS( s->operation(), "audit");
    IS( s->progress(), 100 );
    IS( s->state(), Status::STATE_COMPLETED );

    TEST_END;
}
