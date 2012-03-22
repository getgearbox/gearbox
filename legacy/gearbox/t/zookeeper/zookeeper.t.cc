// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>
#include <gearbox/zookeeper/ZooKeeper.h>
#include <gearbox/core/logger.h>
using namespace Gearbox;

#include <stub/zookeeper.hh>

struct TestContext {
    std::string path;
    ZooKeeper::EventType type;
    TestContext(const std::string p,ZooKeeper::EventType t) : path(p), type(t) {}
};

void TestCallback(
    ZooKeeper & zook,
    const std::string & path,
    ZooKeeper::EventType type,
    void * context
) {
    TestContext * ctx = (TestContext*)context;
    is( ctx->path, path, "does callback path == " + ctx->path);
    is( ctx->type, type, "does callback type == " + ZooKeeper::eventTypeToString(ctx->type));
    delete ctx;
}

    
int main() {
    TEST_START(104);
    log_init("./unit.conf");
    OK( run("./mkzoo") == 0 );

    ConfigFile cfg("./unit.conf");

    ZooKeeper zook("localhost:2181");
    
    std::string value;
    THROWS( zook.get("/does/not/exist", value), "ZNONODE [1101]: failed to get /does/not/exist: no node" );
    THROWS( zook.create("/missing/parent", value), "ZNONODE [1101]: failed to create /missing/parent: no node" );
    THROWS( zook.set("/does/not/exist", value), "ZNONODE [1101]: failed to set /does/not/exist: no node" );
    THROWS( zook.del("/does/not/exist"), "ZNONODE [1101]: failed to delete /does/not/exist: no node" );
    std::vector<std::string> children;
    THROWS( zook.children("/does/not/exist", children), "ZNONODE [1101]: failed to get children for /does/not/exist: no node" );

    OK( zook.exists("/does/not/exist") == false );

    // does create work?

    NOTHROW( zook.create("/1", "1") );
    OK( zook.exists("/1") == true );
    
    // creating a subdir 
    NOTHROW( zook.create("/1/1", "1") );
    OK( zook.exists("/1/1") == true );

    // verify children, set watch
    NOTHROW( zook.children("/1", children, true) );
    IS( children.size(), 1 );
    
    // create child node to cause event
    NOTHROW( zook.create("/1/2", "2") );

    // collect the events and verify we got
    // the events we expected
    NOTHROW( zook.yield() );
    IS( zook.countEvents(), 1 );
    ZooKeeper::EventPtr evt = zook.nextEvent();
    IS( evt->type, ZooKeeper::EVENT_CHILD );
    IS( evt->path, "/1");
    
    // set watch on content and parent
    NOTHROW( zook.children("/1", children, true) );
    NOTHROW( zook.get("/1/2", value, true) );
    IS( value, "2" );

    // trigger delete & child event
    NOTHROW( zook.del("/1/2") );

    // collect the events and verify we got
    // the events we expected
    NOTHROW( zook.yield(2) );
    IS( zook.countEvents(), 2 );
    evt = zook.nextEvent();
    IS( evt->type, ZooKeeper::EVENT_DELETED );
    IS( evt->path, "/1/2");
    evt = zook.nextEvent();
    IS( evt->type, ZooKeeper::EVENT_CHILD );
    IS( evt->path, "/1");

    // set watch on node and parent
    NOTHROW( zook.children("/1", children, true) );
    NOTHROW( zook.exists("/1/12", true) );

    // trigger delete & child event
    NOTHROW( zook.create("/1/12", "data") );

    // collect the events and verify we got
    // the events we expected
    NOTHROW( zook.yield() );
    IS( zook.countEvents(), 2 );
    evt = zook.nextEvent();
    IS( evt->type, ZooKeeper::EVENT_CREATED );
    IS( evt->path, "/1/12");
    evt = zook.nextEvent();
    IS( evt->type, ZooKeeper::EVENT_CHILD );
    IS( evt->path, "/1");

    NOTHROW( zook.create("/1/2", "hello") );
    NOTHROW( zook.create("/1/3", "goodbye") );
    OK( zook.exists("/1/2") == true );
    // verity that stat w/ no args
    // returns stat for last read operation
    ZooKeeper::Stat stat;
    NOTHROW( stat = zook.stat() );
    IS( stat.dataLength(), 5 );
    
    // use  stat with path, and verity stat updated
    NOTHROW( stat = zook.stat("/1/3") );
    IS( stat.dataLength(), 7 );

    // update node, verity stat is updated also
    NOTHROW( zook.set("/1/2", "hello again") );
    NOTHROW( stat = zook.stat() );
    IS( stat.dataLength(), 11 );
    
    std::string path = "/1/seq";
    NOTHROW( zook.create(path, "value", ZooKeeper::TYPE_SEQUENCE) );
    IS( path, "/1/seq0000000006" );
    NOTHROW( zook.get(path, value) );
    IS( value, "value" );
    
    path = "/1/seq";
    NOTHROW( zook.create(path, "value2", ZooKeeper::TYPE_SEQUENCE) );
    IS( path, "/1/seq0000000007" );
    NOTHROW( zook.get(path, value) );
    IS( value, "value2" );

    // testing callback on delete operation
    NOTHROW( zook.children("/1", children, TestCallback, new TestContext("/1", ZooKeeper::EVENT_CHILD)) );
    NOTHROW( zook.get("/1/2", value, TestCallback, new TestContext("/1/2", ZooKeeper::EVENT_DELETED)) );
    IS( value, "hello again" );
    // trigger delete & child event
    NOTHROW( zook.del("/1/2") );


    // testing callback on create operation
    NOTHROW( zook.children("/1", children, TestCallback, new TestContext("/1", ZooKeeper::EVENT_CHILD)) );
    NOTHROW( zook.exists("/1/42", TestCallback, new TestContext("/1/42", ZooKeeper::EVENT_CREATED)) );
    // trigger create and child event
    NOTHROW( zook.create("/1/42", "meaning of life") );

    // collect the events and verify we got
    // the events we expected
    NOTHROW( zook.yield() );
    // all the events were handled by callbacks
    IS( zook.countEvents(), 0 );

    // test callback on update operation
    NOTHROW( zook.get("/1/42", value, TestCallback, new TestContext("/1/42", ZooKeeper::EVENT_CHANGED)) );
    // trigger changed event
    NOTHROW( zook.set("/1/42", "not the meaning of life") );

    // collect the events and verify we got
    // the events we expected
    NOTHROW( zook.yield() );
    // all the events were handled by callbacks
    IS( zook.countEvents(), 0 );

    // normalize_path tests
    IS( ZooKeeper::normalize_path(""), "/" );
    IS( ZooKeeper::normalize_path("/"), "/" );
    IS( ZooKeeper::normalize_path("//"), "/" );
    IS( ZooKeeper::normalize_path("//foo"), "/foo" );
    IS( ZooKeeper::normalize_path("foo"), "/foo" );
    IS( ZooKeeper::normalize_path("foo/"), "/foo" );
    IS( ZooKeeper::normalize_path("///foo/bar"), "/foo/bar" );
    IS( ZooKeeper::normalize_path("/foo/bar/"), "/foo/bar" );
    IS( ZooKeeper::normalize_path("/foo//bar/"), "/foo/bar" );
    IS( ZooKeeper::normalize_path("/foo/bar//"), "/foo/bar" );

    // join_paths tests
    IS( ZooKeeper::join_paths("b/c", "d"), "/b/c/d" );
    IS( ZooKeeper::join_paths("/b/c", "d"), "/b/c/d" );
    IS( ZooKeeper::join_paths("/b/c", ""), "/b/c" );
    IS( ZooKeeper::join_paths("/b/c/", "d"), "/b/c/d" );
    IS( ZooKeeper::join_paths("/b/c/", "/d"), "/b/c/d" );
    IS( ZooKeeper::join_paths("/b/c/", "/d/"), "/b/c/d" );
    IS( ZooKeeper::join_paths("/b/c/", ""), "/b/c" );
    IS( ZooKeeper::join_paths("/", "d"), "/d" );
    IS( ZooKeeper::join_paths("", "d"), "/d" );
    IS( ZooKeeper::join_paths("", ""), "/" );
    IS( ZooKeeper::join_paths("/", "d/e"), "/d/e" );
    IS( ZooKeeper::join_paths("/", "/d/e"), "/d/e" );
    IS( ZooKeeper::join_paths("/", "d/e/"), "/d/e" );
    IS( ZooKeeper::join_paths("/", "/d/e/"), "/d/e" );
    IS( ZooKeeper::join_paths("", "d/e"), "/d/e" );
    IS( ZooKeeper::join_paths("", "/d/e"), "/d/e" );

    TEST_END;
}

