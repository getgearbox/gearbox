// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>
#include <gearbox/core/util.h>
#include <gearbox/core/TempFile.h>
using namespace Gearbox;

#include "log4cxx/propertyconfigurator.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
namespace bfs=boost::filesystem;

#include <unistd.h>    // mkstemp
#include <errno.h>     // errno

#include <string>
#include <iostream>
using namespace std;

int main() {
    chdir(TESTDIR);
    log4cxx::PropertyConfigurator::configure("../../../common/conf/stdout-logger.conf");

    TEST_START(6);

    // create a "small" (1Mb) file of zeros
    TempFile small("/tmp/ffdigest.small");
    small.close();
    small.unlink();
    OK( run("dd bs=1048576 count=1 if=/dev/zero of=" + small.name() + " 2> /dev/null") == 0 );

    // create a "large" (64Mb) file of zeros
    TempFile large("/tmp/ffdigest.large");
    large.close();
    large.unlink();
    OK( run("dd bs=1048576 count=64 if=/dev/zero of=" + large.name() + " 2> /dev/null") == 0 );

    OK( digest_full( small.name() ) == "b6d81b360a5672d80c27430f39153e2c" );
    OK( digest_full( large.name() ) == "7f614da9329cd3aebf59b91aadc30bf0" );
    OK( digest_fast( small.name() ) == "b6d81b360a5672d80c27430f39153e2c" );
    OK( digest_fast( large.name() ) == "0a9156c4e3c48ef827980639c4d1e263" );

    TEST_END;
}
