// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>
#include <gearbox/core/logger.h>
#include <gearbox/core/util.h>
#include "log4cxx/propertyconfigurator.h"


using namespace Gearbox;

int main() {
    chdir(TESTDIR);
    log4cxx::PropertyConfigurator::configure("../../../common/conf/stdout-logger.conf");
    TEST_START(9);
    // encode/decode
    std::string s;
    IS( encode_b32c( 1234567890123456789LL, s ), 0 );
    IS( s, "128ggyhyyk08n" );
    uint64_t n;
    IS( decode_b32c( s, n ), 0 );
    IS( n, 1234567890123456789LL );
    // illegal characters
    IS( decode_b32c("i", n), 1 );
    IS( decode_b32c("l", n), 1) ;
    IS( decode_b32c("o", n), 1 );
    IS( decode_b32c("u", n), 1 );
    IS( decode_b32c("=", n), 1 );
    TEST_END;
}
