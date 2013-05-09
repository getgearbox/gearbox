// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>
#include <gearbox/core/logger.h>
#include <gearbox/core/TempDir.h>
using namespace Gearbox;


#include <boost/lexical_cast.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
namespace bfs=boost::filesystem;

using namespace Gearbox;

int main(int argc, char **argv) {
    chdir(TESTDIR);
    TEST_START(9);
    log_init("./unit.conf");

    bfs::path path;

    // test that tmp dirs disappear
    path.clear();
    {
        TempDir tmp_path;
        path = tmp_path.name();
        OK( !path.empty() );
        OK( bfs::exists(path) );
        OK( bfs::is_directory(path) );
    }
    OK( !bfs::exists(path) );

    // test that tmp dirs disappear
    path.clear();
    const std::string prefix = "/tmp/test-" + boost::lexical_cast<std::string>(getpid()) + "-";
    {
        TempDir tmp_path(prefix);
        path = tmp_path.name();
        OK( !path.empty() );
        OK( path.string().find(prefix) == 0 );
        OK( bfs::exists(path) );
        OK( bfs::is_directory(path) );
    }
    OK( !bfs::exists(path) );

    TEST_END;
}
