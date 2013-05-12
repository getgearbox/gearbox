// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>

#include <gearbox/core/logger.h>
#include <gearbox/core/util.h>
#include <gearbox/store/dbconn.h>
using namespace Gearbox;
using namespace Gearbox::Database;

struct TestConnection : public Connection {
    TestConnection(const std::string & type) : Connection(type) {}
};

int main() {
    chdir(TESTDIR);
    TEST_START(25);
    log_init("./unit.conf");
    
    THROWS( TestConnection("bogus").get_session(),
            "Failed to find shared library for backend bogus" );

    THROWS( Connection::get("bogus"), 
            "Database connection bogus has not been registered!" );
    
    std::auto_ptr<TestConnection> c;
    c.reset(new TestConnection("sqlite3"));
    NOTHROW( c->set_dbname("db") );
    IS( c->connection_string(), "dbname=db timeout=10" );

    c.reset(new TestConnection("mysql"));
    NOTHROW( c->set_port("1234") );
    NOTHROW( c->set_pass("secret") );
    NOTHROW( c->set_host("host") );
    NOTHROW( c->set_user("user") );
    NOTHROW( c->set_sock("sock") );
    NOTHROW( c->set_dbname("db") );
    IS( c->connection_string(), 
        "dbname=db user=user password=secret host=host port=1234 unix_socket=sock" );
    
    OK( run("./mkdb") == 0 );
    db_init("./unit.conf", "myconnname");
    
    IS( lastsql(), "" );

    soci::session * sess = &getconn();
    NOTHROW( sess->close() );

    // we do not auto connect
    THROWS( sess = &getconn(), "Session is not connected." );

    // testing ability to reconnect
    NOTHROW( sess = &getconn() );

    int x = 0;
    *sess << "SELECT 1", soci::into(x);
    IS( x, 1 );

    NOTHROW( dblock() );
    NOTHROW( dbcommit() );
    NOTHROW( dblock() );
    NOTHROW( dbrollback() );
    // this will warn because no transaction active, but no exception
    NOTHROW( dbrollback() );

    IS( lastsql(), "SELECT 1" );

    THROWS( getconn("foobar"), "Database connection foobar has not been registered!" );
    NOTHROW( getconn("myconnname") );

    TEST_END;
}
