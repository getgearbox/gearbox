// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>

#include <gearbox/core/logger.h>
#include <gearbox/core/util.h>
#include <gearbox/store/dbconn.h>
#include <gearbox/job/StatusManager.h>
#include <gearbox/core/Errors.h>
#include <stub/GET.hh>
#include <libgen.h>
#include <unistd.h>

using namespace Gearbox;
int main(int argc, char *argv[]) {
    TEST_START(125);

    std::string basedir = std::string(dirname(argv[0])) + "/../";
    chdir(basedir.c_str());

    log_init("unit.conf");
    OK( run("./mkdb") == 0 );
    ConfigFile cfg("unit.conf");
    db_init( cfg.get_json("status"), "status");

    // SQL backed status tests
    StatusManager sm( cfg );
    sm.base_uri( "http://stuff/here" );

    NOTHROW( StatusPtr s = sm.create("s-123456789", "STUFF", "http://stuff/here") );
    StatusPtr s = sm.fetch("s-123456789");
    IS( s->name(), "s-123456789" );
    IS( s->operation(), "STUFF" );
    IS( s->progress(), 0 );
    NOTHROW( s->add_child("http://stuff/there") );
    NOTHROW( s->meta("foo", "bar") );

    Json meta;
    meta["a"][1]["b"] = 123;
    meta["z"]["x"][0] = 987;
    
    NOTHROW( s->meta("foo", "bar") );
    IS( s->meta()["foo"].as<std::string>(), "bar" );

    NOTHROW( s->meta("complex", meta) );
    IS( s->meta()["complex"]["a"][1]["b"].as<int>(), 123 );
    IS( s->meta()["complex"]["z"]["x"][0].as<int>(), 987 );

    // store this so we can verify it does not change
    int mtime = s->mtime();

    sleep(1);

    StatusPtr ss = sm.fetch("s-123456789");
    IS( ss->mtime(), mtime );

    StatusCollectionPtr coll;
    NOTHROW( coll = sm.collection() );
    NOTHROW( coll->filter_progress(0,99) );

    _DEBUG("empty: " << coll->empty() );

    while( !coll->empty() ) {
        StatusPtr s = coll->pop();
        s->state(Status::STATE_RUNNING);
        _DEBUG("got status: " << s->name());
        s->fail(408);
        const Error & err(ERR_REQUEST_TIME_OUT("Request failed to finish in 0 seconds"));
        s->add_message(err.what());
    }

    ss->sync();

    IS( ss->name(), "s-123456789" );
    IS( ss->operation(), "STUFF" );
    IS( ss->progress(), 100 );
    IS( ss->code(), 408 );
    IS( ss->messages().size(), 1 );
    IS( ss->messages()[0], "REQUEST_TIME_OUT [408]: Request failed to finish in 0 seconds" );
    IS( ss->children().size(), 1 );
    IS( ss->children()[0], "http://stuff/there" );
    
    IS( ss->meta()["foo"].as<std::string>(), "bar" );
    IS( ss->meta()["complex"]["a"][1]["b"].as<int>(), 123 );
    IS( ss->meta()["complex"]["z"]["x"][0].as<int>(), 987 );

    // try the meta version that overwrites everything
    meta["abc"] = "zyx";
    NOTHROW( ss->meta( meta ) );
    ss->sync();
    IS( ss->meta(), meta );
    IS( ss->meta()["a"][1]["b"].as<int>(), 123 );
    IS( ss->meta()["z"]["x"][0].as<int>(), 987 );
    IS( ss->meta()["abc"].as<std::string>(), "zyx" );


    NOTHROW( sm.create("s-123456789a", "OP1", "http://stuff/here/rPATTERN01") );
    NOTHROW( sm.create("s-123456789b", "OP1", "http://stuff/here/rPATTERN02") );
    NOTHROW( sm.create("s-123456789c", "OP1", "http://stuff/here/rPATTERN03") );

    NOTHROW( sm.create("s-123456789d", "OP2", "http://stuff/here/rPATT3RN04") );
    NOTHROW( sm.create("s-123456789e", "OP2", "http://stuff/here/rPATT3RN05") );
    NOTHROW( sm.create("s-123456789f", "OP2", "http://stuff/here/rPATT3RN06") );

    NOTHROW( sm.create("s-123456789g", "OP3", "http://stuff/here/rPATT8RN07") );
    NOTHROW( sm.create("s-123456789h", "OP3", "http://stuff/here/rPATT8RN08") );
    NOTHROW( sm.create("s-123456789i", "OP3", "http://stuff/here/rPATT8RN09") );

    NOTHROW( sm.create("s-123456789j", "OP4", "http://stuff/here/rPATT9RN00") );
    NOTHROW( sm.create("s-123456789k", "OP4", "http://stuff/here/rPATT9RN01") );
    NOTHROW( sm.create("s-123456789l", "OP4", "http://stuff/here/rPATT9RN02") );

    sleep(1);

    // test for operation filter
    NOTHROW( coll = sm.collection() );
    NOTHROW( coll->filter_operation( "OP1" ) );

    int count = 0;
    while( !coll->empty() ) {
        StatusPtr s = coll->pop();
        _DEBUG("got status: " << s->name());
        count++;
    }
    _DEBUG("Count: " << count);
    IS ( count, 3 );

    // test for uri filter
    NOTHROW( coll = sm.collection() );
    NOTHROW( coll->filter_uri( "PATT" ) );

    count = 0;
    while( !coll->empty() ) {
        StatusPtr s = coll->pop();
        _DEBUG("got status: " << s->name());
        count++;
    }
    _DEBUG("Count: " << count);
    IS ( count, 10 );

    // test for default limit
    NOTHROW( coll = sm.collection() );
    NOTHROW( coll->filter_uri( "PATT" ) );

    count = 0;
    while( !coll->empty() ) {
        StatusPtr s = coll->pop();
        _DEBUG("got status: " << s->name());
        count++;
    }
    _DEBUG("Count: " << count);
    IS ( count, 10 );

    // test for user set limit
    NOTHROW( coll = sm.collection() );
    NOTHROW( coll->filter_uri( "PATT" ) );
    NOTHROW( coll->limit( 5 ) );

    count = 0;
    while( !coll->empty() ) {
        StatusPtr s = coll->pop();
        _DEBUG("got status: " << s->name());
        count++;
    }
    _DEBUG("Count: " << count);
    IS ( count, 5 );

    // test for user set limit (0)
    NOTHROW( coll = sm.collection() );
    NOTHROW( coll->filter_uri( "PATT" ) );
    NOTHROW( coll->limit( 0 ) );

    count = 0;
    while( !coll->empty() ) {
        StatusPtr s = coll->pop();
        _DEBUG("got status: " << s->name());
        count++;
    }
    _DEBUG("Count: " << count);
    IS ( count, 12 );

    // test for progress filter
    NOTHROW( coll = sm.collection() );
    NOTHROW( coll->filter_uri( "rPATT9RN02" ) );
    NOTHROW( coll->filter_progress( 0, 0 ) );

    s = coll->pop();
    _DEBUG("got status: " << s->name());
    IS ( s->name(),  "s-123456789l" );

    // test for code filter
    NOTHROW( coll = sm.collection() );
    NOTHROW( coll->filter_code( 408, 408 ) );

    s = coll->pop();
    _DEBUG("got status: " << s->name());
    IS ( s->name(),  "s-123456789" );

    // test for code filter
    NOTHROW( coll = sm.collection() );
    NOTHROW( coll->filter_code( 400, 500 ) );

    s = coll->pop();
    _DEBUG("got status: " << s->name());
    IS ( s->name(),  "s-123456789" );

    // test for component
    NOTHROW( coll = sm.collection() );
    NOTHROW( coll->filter_uri( "rPATT3RN04" ) );
    NOTHROW( coll->filter_component( "internal" ) );

    s = coll->pop();
    _DEBUG("got status: " << s->name());
    IS ( s->name(),  "s-123456789d" );

    // test for state
    NOTHROW( coll = sm.collection() );
    NOTHROW( coll->filter_state( "COMPLETED" ) );

    s = coll->pop();
    _DEBUG("got status: " << s->name());
    IS ( s->name(),  "s-123456789" );

    time_t now = time (NULL);

    // test for mtime
    NOTHROW( coll = sm.collection() );
    NOTHROW( coll->filter_mtime( 0, now ) );

    count = 0;
    while( !coll->empty() ) {
        StatusPtr s = coll->pop();
        _DEBUG("got status: " << s->name());
        count++;
    }
    _DEBUG("Count: " << count);
    IS ( count, 10 );

    // test for ctime
    NOTHROW( coll = sm.collection() );
    NOTHROW( coll->filter_ctime( 0, now ) );

    count = 0;
    while( !coll->empty() ) {
        StatusPtr s = coll->pop();
        _DEBUG("got status: " << s->name());
        count++;
    }
    _DEBUG("Count: " << count);
    IS ( count, 10 );

    NOTHROW( StatusPtr s2 = sm.create("s-123456790", "STUFF", "http://stuff/here") );
    StatusPtr s2 = sm.fetch("s-123456790");
    s2->parent_uri( "http://stuff/here" + s->uri() );

    StatusPtr s3 = s2->parent();
    IS( s3->name(), "s-123456789" );
    IS( s3->operation(), "STUFF" );
    IS( s3->progress(), 100 );

    sm.base_uri( "http://newstuff/here" );
    NOTHROW( StatusPtr s4 = sm.create("s-123456791", "STUFF", "http://stuff/here") );
    StatusPtr s4 = sm.fetch("s-123456791");
    s4->parent_uri( "http://10.0.0.1:4080" + s->uri() );

    StatusPtr s5 = s4->parent();
    IS( s5->name(), "s-123456789" );
    IS( s5->operation(), "STUFF" );
    IS( s5->progress(), 100 );

    sm.base_uri( "http://newstuff/here" );
    NOTHROW( StatusPtr s6 = sm.create("s-123456792", "STUFF", "http://stuff/here") );
    StatusPtr s6 = sm.fetch("s-123456792");

    mtime = s6->mtime();
    sleep( 1 );

    NOTHROW( s6->progress(20) );
    OK ( mtime < s6->mtime() );
    IS ( s6->progress(), 20 );

    NOTHROW( s6->cancel() );
    OK ( mtime < s6->mtime() );
    IS ( s6->progress(), 100 );
    IS ( s6->code(), ERR_CONFLICT().code() );

    mtime = s6->mtime();
    sleep( 1 );

    NOTHROW( s6->cancel() );
    IS ( s6->mtime(), mtime );
    IS ( s6->progress(), 100 );
    IS ( s6->code(), ERR_CONFLICT().code() );

    // verify that cancel on a completed status does not
    // update the mtime
    StatusPtr s7;
    NOTHROW(  s7 = sm.create("s-123456793", "STUFF", "http://stuff/here") );
    OK ( s7->state(Status::STATE_RUNNING) );
    NOTHROW( s7->success() );
    
    mtime = s7->mtime();
    sleep(1);
    
    OK ( s7->state(Status::STATE_CANCELLING) );
    NOTHROW( s7->cancel() );
    s7->sync();
    IS ( s7->mtime(), mtime );
    IS ( s7->progress(), 100 );
    IS ( s7->code(), 0 );

    // verify that cancel on a completed status does not
    // update the mtime
    StatusPtr s8, s9;
    NOTHROW( s8 = sm.create("s-123456794", "STUFF", "http://stuff/here") );
    NOTHROW( s9 = sm.fetch("s-123456794") );
    NOTHROW( s8->starting() );
    IS( s8->concurrency(), 1 );
    IS( s9->concurrency(), 1 );

    NOTHROW( s9->starting() );
    IS( s8->concurrency(), 2 );
    IS( s9->concurrency(), 2 );
    
    NOTHROW( s8->stopping() );
    IS( s8->concurrency(), 1 );
    IS( s8->concurrency(), 1 );

    NOTHROW( s9->stopping() );
    IS( s8->concurrency(), 0 );
    IS( s9->concurrency(), 0 );

    TEST_END;
}
