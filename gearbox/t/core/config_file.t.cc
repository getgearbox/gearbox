// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>
#include <gearbox/core/ConfigFile.h>

using namespace Gearbox;

int main() {
    chdir(TESTDIR);
    TEST_START(18);

    ConfigFile cfg("cfg/2.cfg");

    Json a;
    NOTHROW( a = cfg.get_json( "json" ) );
    IS( a.empty(), true );
    IS( a.length(), 0 );
    IS( a.typeName(), "array" );
    IS( a.serialize(), "[]" );

    {
        ConfigFile cfg("cfg/3.cfg");
        IS( cfg.get_path("section1", "nonsense"), "" );
        IS( cfg.get_path("section1", "mypath"), "/1/2/3" );
        IS( cfg.get_path("section1", "yourpath"), "/etc/gearbox/4/5/6" );
        IS( cfg.get_path("zaz"), "");
        IS( cfg.get_path("zot"), "/foo");
        IS( cfg.get_path("zap"), "/etc/gearbox/foo");

        setenv("GB_SERVER_ROOT","/altroot",1);

        IS( cfg.get_path("section1", "nonsense"), "" );
        IS( cfg.get_path("section1", "mypath"), "/1/2/3" );
        IS( cfg.get_path("section1", "yourpath"), "/altroot/4/5/6" );
        IS( cfg.get_path("zot"), "/foo");
        IS( cfg.get_path("zap"), "/altroot/foo");

        unsetenv("GB_SERVER_ROOT");
        IS( cfg.get_path("section1", "yourpath"), "/etc/gearbox/4/5/6" );
        IS( cfg.get_path_default("section1", "nonsense", "vodka"), "vodka" );

    }

    TEST_END;
}
