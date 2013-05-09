// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include "config.h"
#include <string>

#include "tap/trivial.h"

#include "gearbox/core/logger.h"
#include "gearbox/core/util.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

using std::string;
using namespace Gearbox;

int
main(int argc, char *argv[])
{
    chdir(TESTDIR);
    TEST_START(4);
    log_init("./unit.conf");

    const string filename("/bin/sh");
    string digest;
    NOTHROW(digest = sha1sum(filename));

    
    // verify generated digest with sha1sum command line util

    std::vector<std::string> args;
    args << SHASUM_BIN << filename;
    string out;
    int rv = run(args, out);
    OK( rv == 0 );     // error

    std::vector<string> parts;
    boost::algorithm::split(parts, out, boost::algorithm::is_any_of(" "));
    
    IS( digest, parts[0] );

    // attempt to sha1sum() nonexistent file throws error
    THROWS_LIKE(sha1sum("/foo/bar/baz"),
                "Failed to open /foo/bar/baz: No such file or directory");

    TEST_END;
}
