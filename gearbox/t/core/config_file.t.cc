// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>
#include <gearbox/core/ConfigFile.h>

using namespace Gearbox;

int main() {
    chdir(TESTDIR);
    TEST_START(5);

    ConfigFile cfg("cfg/2.cfg");

    Json a;
    NOTHROW( a = cfg.get_json( "json" ) );
    IS( a.empty(), true );
    IS( a.length(), 0 );
    IS( a.typeName(), "array" );
    IS( a.serialize(), "[]" );

    TEST_END;
}
