// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>
#include <gearbox/core/strlcpy.h>
using namespace Gearbox;


int main() {
    chdir(TESTDIR);
    TEST_START(12);

    const int BUFSIZE = 32;
    char buf[BUFSIZE];

    using Gearbox::strlcpy;

    // normal copy, ret value strlen(of the source)
    IS( strlcpy(buf, "1234567890", (size_t)sizeof buf), 10UL );
    
    // verify contents
    IS( buf, "1234567890");

    // deal with truncated strings - ret value still strlen(source)
    IS( strlcpy(buf, "1234567890", 5), 10UL);

    // verify we copied all we could, and didn't overflow
    IS( buf, "1234");

    // size - 1 should be copied
    IS( strlcpy(buf, "1234567890", 10), 10UL );
    IS( buf, "123456789" );

    // deal with 1 byte strings properly
    IS( strlcpy(buf, "1", sizeof buf), 1UL );
    IS( buf, "1" );

    // deal with empty strings properly
    IS( strlcpy(buf, "", sizeof buf), 0UL );
    IS( buf, "" );

    // deal with 0 sized buffers
    strncpy(buf, "test", sizeof(buf));
    IS( strlcpy(buf, "1234", 0), 4UL );
    IS( buf, "test" );

    TEST_END;
}
