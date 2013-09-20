// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>
#include <gearbox/core/util.h>
#include <gearbox/core/logger.h>

#include <sys/types.h>
#include <unistd.h>

#include <string>

#include "config.h"

extern "C" {
    uid_t test_uid;
    uid_t test_euid;

    uid_t getuid(void) { 
        std::cerr << "local getuid called" << std::endl;
        return test_uid;
    }
    uid_t geteuid(void) { return test_euid; }
}

using namespace Gearbox;
using std::string;

using namespace log4cxx;

const std::string security_log_msg = "a root process must not run shell-parsed commands";
int main()
{
    chdir(TESTDIR);
    TEST_START(5);
    Gearbox::log_init("./unit.conf");

    test_uid = test_euid = 1234;
    NOTHROW( run("true") );

    test_uid = 1234;
    test_euid = 0;
    THROWS( run(TRUE_BIN " foo"), security_log_msg );

    test_uid = 0;
    test_euid = 1234;
    THROWS( run(TRUE_BIN " foo"), security_log_msg );

    test_uid = test_euid = 0;
    THROWS( run(TRUE_BIN" foo"), security_log_msg );

    Cmd cmd;
    NOTHROW( run( cmd << TRUE_BIN << "foo" ) );

    TEST_END;
}
