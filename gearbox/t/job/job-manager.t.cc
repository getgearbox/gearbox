// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>
#include <gearbox/core/logger.h>
#include <gearbox/core/Json.h>
#include <gearbox/job/JobManager.h>

#include <stub/gearman.hh>

using std::string;
using std::vector;

using namespace Gearbox;

typedef vector<string> jobs;

int main() {
    chdir(TESTDIR);
    TEST_START(36);
    log_init("./unit.conf");
    ConfigFile cfg("./unit.conf");    
    JobManager jm(cfg);
    jm.base_uri("http://foo/");
    std::string content("{\"this\":\"that\"}");
    OK( jm.known_job_name("do_put_trm_zone_v1") );
    OK( jm.known_job_name("do_get_trm_tier_v1") );
    THROWS( JobPtr f = jm.job("totally_bogus_job"), "No such job 'totally_bogus_job' enabled in config." );
    THROWS( jm.job("totally_bogus_job"), "No such job 'totally_bogus_job' enabled in config." );
    THROWS( jm.job("totally_bogus_job"), "No such job 'totally_bogus_job' enabled in config." );
    NOTHROW( jm.job("do_put_trm_zone_v1") );
    NOTHROW( jm.job("do_put_trm_zone_v1") );
    JobPtr a;
    NOTHROW( a = jm.job("do_put_trm_zone_v1") );
    a->content(content);
    a->add_argument("a");
    a->add_argument("b");
    a->resource_name("felix");

    response["do_get_global_status_v1"].parseFile("./jobs_data/do_get_global_status_v1");
    
    NOTHROW( a->run() );
    NOTHROW( JobPtr f = jm.job("do_put_trm_zone_v1") );
    jm.base_uri("");
    JobPtr j = jm.job("do_put_trm_zone_v1");
    THROWS( j->run(), "Uri Error: The string '' is not parseable as a uri.");
    jm.base_uri("http://localhost:4080/trm/v1/");
    j = jm.job("do_put_trm_zone_v1");
    JobResponse resp;
    NOTHROW( resp = j->run() );
    _DEBUG( resp.status()->name() );
    OK( resp.status()->name().size() );
    IS( resp.status()->operation(), "create");
    j->operation("enroll");
    response["do_get_global_status_v1"]["operation"] = "enroll";
    NOTHROW( resp = j->run() );
    IS( resp.status()->operation(), "enroll");
    j = jm.job("do_post_trm_tier_v1");
    IS( j->operation(), "update" );
    
    response["do_get_global_status_v1"]["operation"] = "update";
    jm.base_uri("http://foo/");
    JobPtr j2 = jm.job("do_post_trm_tier_v1");
    j2->add_argument("abc");
    NOTHROW( resp = j2->run() );
    IS( resp.status()->operation(), "update" );

    

    Json conf;
    conf["do_trm_update_opsdb_v1"] = Json::Object();
    conf["do_trm_update_dnsdb_v1"] = Json::Object();
    conf["do_trm_update_roles_v1"] = Json::Object();
    conf["do_trm_update_cloudrouter_v1"] = Json::Object();
    conf["do_trm_update_tiercontroller_v1"]["require"][0] = "do_trm_update_opsdb_v1";
    conf["do_trm_update_tiercontroller_v1"]["require"][0] = "do_trm_update_dnsdb_v1";
    conf["do_trm_update_tiercontroller_v1"]["require"][0] = "do_trm_update_roles_v1";
    conf["do_trm_update_tiercontroller_v1"]["require"][0] = "do_trm_update_cloudrouter_v1";
    JobQueue q;
    NOTHROW( q = jm.job_queue(conf) );
    OK( q.size() == 2 );
    OK( q[0].size() == 4 );
    OK( q[1].size() == 1 );
    IS( q[1][0]->name(), "do_trm_update_tiercontroller_v1" );
    conf["do_trm_update_cloudrouter_v1"]["require"][0] = "do_trm_update_opsdb_v1";

    NOTHROW( q = jm.job_queue(conf) );
    OK( q.size() == 3 );
    OK( q[0].size() == 3 );
    OK( q[1].size() == 1 );
    IS( q[1][0]->name(), "do_trm_update_cloudrouter_v1" );
    OK( q[2].size() == 1 );
    IS( q[2][0]->name(), "do_trm_update_tiercontroller_v1" );
    
    conf.clear();
    NOTHROW( q = jm.job_queue(conf) );

    conf["do_foo_zot_zot_v1"]["require"][0] = "do_bar_zot_zot_v1";
    THROWS( jm.job_queue(conf), "INTERNAL_SERVER_ERROR [500]: Agent: 'do_foo_zot_zot_v1' has undefined dependency 'do_bar_zot_zot_v1'" );

    conf["do_bar_zot_zot_v1"] = Json::Object();
    NOTHROW( jm.job_queue(conf) );

    conf["do_foo_zot_zot_v1"]["require"][0] = "do_foo_zot_zot_v1";
    THROWS( jm.job_queue(conf), "INTERNAL_SERVER_ERROR [500]: Agent: 'do_foo_zot_zot_v1' defines a cyclic dependency!" );

    conf["do_foo_zot_zot_v1"]["require"][0] = "do_bar_zot_zot_v1";
    conf["do_bar_zot_zot_v1"]["require"][0] = "do_foo_zot_zot_v1";
    THROWS( jm.job_queue(conf), "INTERNAL_SERVER_ERROR [500]: Cycle detected in configuration: The graph must be a DAG." );

    TEST_END;
}
