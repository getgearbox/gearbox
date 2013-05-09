// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>
#include <gearbox/core/logger.h>
#include <gearbox/core/util.h>
#include <gearbox/core/Plugin.h>
#include <algorithm>
#include "plugins/TestPlugin.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
namespace bfs = boost::filesystem;
using namespace Gearbox;

int main() {
    chdir(TESTDIR);
    TEST_START(17);
    log_init("./unit.conf");
    
    bfs::path pluginDir(TESTPLUGINDIR);
    
    Plugin * p = Plugin::load(pluginDir, "hello" );
    IS( p->name(), "hello" );
    
    TestPlugin * t = p->create<TestPlugin>();
    IS( t->get(), "hello" );
    std::string data;
    NOTHROW( t->set(data) );
    IS( data, "hello" );

    OK( p->can("greet") );
    typedef std::string (*greet_func)(const std::string & name);
    greet_func greet = (greet_func)p->getFunc("greet");
    IS( greet("Daniel"), "Hello Daniel!" );

    p->destroy(t);

    p = Plugin::load(pluginDir, "goodbye");
    IS( p->name(), "goodbye" );

    t = p->create<TestPlugin>();
    IS( t->get(), "goodbye" );
    NOTHROW( t->set(data) );
    IS( data, "goodbye" );
    OK( !p->can("greet") );
    p->destroy(t);

    THROWS_LIKE( Plugin::load(pluginDir, "does-not-exist"), "^plugin \"does-not-exist\" does not exist$" );

    if( ! bfs::exists(pluginDir/"bogus.so") ) {
        std::string stdout;
        run("echo bogus > " + (pluginDir/"bogus.so").string(), stdout);
    }
    THROWS_LIKE( Plugin::load(pluginDir, "bogus" ), ".*/bogus.so: file too short" );

    p = Plugin::load(pluginDir, "hello");
    THROWS_LIKE( p->getFunc("bogus"), "(undefined symbol|symbol not found)" );
    
    // verifh that fetch "hello" module again
    // will work to make sure it has not been
    // unloaded

    t = p->create<TestPlugin>();
    IS( t->get(), "hello" );
    NOTHROW( t->set(data) );
    IS( data, "hello" );
    p->destroy(t);

    TEST_END;
}
