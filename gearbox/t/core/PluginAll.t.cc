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

bool sorter (Plugin* a,Plugin*b) { return (a->name() < b->name()); }

int main() {
    chdir(TESTDIR);
    TEST_START(19);
    log_init("./unit.conf");

    bfs::path pluginDir(TESTPLUGINDIR);

    if( ! bfs::exists(pluginDir/"bogus.so") ) {
        std::string stdout;
        run("echo bogus > " + (pluginDir/"bogus.so").string(), stdout);
    }
    THROWS_LIKE(Plugin::loadAll(pluginDir), ".*/bogus.so: file too short" );

    bfs::remove(pluginDir/"bogus.so");

    Plugins p = Plugin::loadAll(pluginDir);
    std::sort(p.begin(), p.end(), sorter);
    IS( p.size(), 2 );
    IS( p[0]->name(), "goodbye" );
    IS( p[1]->name(), "hello" );
    TestPlugin * t = p[0]->create<TestPlugin>();
    IS( t->get(), "goodbye" );
    std::string data;
    NOTHROW( t->set(data) );
    IS( data, "goodbye" );
    p[0]->destroy(t);
    
    t = p[1]->create<TestPlugin>();
    IS( t->get(), "hello" );
    NOTHROW( t->set(data) );
    IS( data, "hello" );
    p[1]->destroy(t);

    // do it all again to see if libraries in cache
    // got unloaded
    
    p = Plugin::loadAll(pluginDir);
    std::sort(p.begin(), p.end(), sorter);
    IS( p.size(), 2 );
    IS( p[0]->name(), "goodbye" );
    IS( p[1]->name(), "hello" );
    t = p[0]->create<TestPlugin>();
    IS( t->get(), "goodbye" );
    NOTHROW( t->set(data) );
    IS( data, "goodbye" );
    p[0]->destroy(t);
    
    t = p[1]->create<TestPlugin>();
    IS( t->get(), "hello" );
    NOTHROW( t->set(data) );
    IS( data, "hello" );
    p[1]->destroy(t);

    TEST_END;
}
