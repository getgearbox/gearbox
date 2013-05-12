// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>
#include <gearbox/core/logger.h>
#include <gearbox/core/util.h>
#include <gearbox/store/dbconn.h>
using namespace Gearbox;
using namespace Gearbox::Database;

int main() {
    chdir(TESTDIR);
    TEST_START(23);
    OK( run("./mkdb") == 0 );
    log_init("./unit.conf");
    db_init( "./unit.conf" );
    NOTHROW( Database::LoggedStatement( getconn() ) );
    std::string value;
    Database::LoggedStatement ls( getconn() );
    NOTHROW( ls << "SELECT value FROM bar WHERE id = :id" );
    NOTHROW( ls.prepare() );
    NOTHROW( ls.into( value ) );
    NOTHROW( ls.use( 1, "id" ) );
    ls.execute();
    NOTHROW( ls.fetch() );
    IS( value, "palindrome" );
    NOTHROW( ls.reset() );
    NOTHROW( ls << "SELECT value FROM bar WHERE id = :id" );
    NOTHROW( ls.prepare().into( value ).use( 2, "id" ).execute().fetch() );
    IS( value, "emordnilap" );
    ls.reset();
    ls << "INSERT INTO bar ( id, value ) VALUES ( :id, 'anagram' )";
    NOTHROW( ls.prepare().use(3, "id").execute(true) );
    ls.reset();
    NOTHROW( ls << "SELECT value FROM bar WHERE id = :id" );
    NOTHROW( ls.prepare().into( value ).use( 3, "id" ).execute().fetch() );
    IS( value, "anagram" );
    value.clear();
    ls.reset();

    unsigned int number;
    NOTHROW( ls << "SELECT id FROM bar WHERE value = :value" );
    NOTHROW( ls.prepare().into( number ).use( "anagram" ).execute().fetch() );
    IS( number, 3 );
    ls.reset();

    std::string nada;
    soci::indicator null = soci::i_null;
    ls << "INSERT INTO bar ( id, value ) VALUES ( :id, :nada )";
    ls.prepare();
    ls.use(4, "id");
    ls.use(nada, null, "nada");
    ls.execute(true);
    ls.reset();

    NOTHROW( ls << "SELECT value FROM bar WHERE id = :id" );
    ls.prepare();
    NOTHROW( ls.into( value, null ) );
    ls.use( 4, "id" );
    ls.execute().fetch();
    IS( value, "" );
    IS( null, soci::i_null );

    TEST_END;
}
