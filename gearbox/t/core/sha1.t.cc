// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <string>

#include "tap/trivial.h"

#include "gearbox/core/logger.h"
#include "gearbox/core/util.h"

using std::string;
using namespace Gearbox;

int
main(int argc, char *argv[])
{
    TEST_START(3);
    log_init("./unit.conf");

    const string filename("/bin/sh");
    string digest;
    NOTHROW(digest = sha1sum(filename));

    // verify generated digest with sha1sum command line util
    OK(system(("echo \"" + digest + "  " + filename + "\" | sha1sum -c").c_str()) == 0);

    // attempt to sha1sum() nonexistent file throws error
    THROWS_LIKE(sha1sum("/foo/bar/baz"),
                "Failed to open /foo/bar/baz: No such file or directory");

    TEST_END;
}
