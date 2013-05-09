// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include "config.h"
#include <tap/trivial.h>
#include <gearbox/core/logger.h>
#include <gearbox/core/util.h>
using namespace Gearbox;

#include <sys/types.h> /* mkfifo,kill */
#include <sys/stat.h>  /* mkfifo */
#include <signal.h>    /* kill */

int main(int argc, char **argv) {
    chdir(TESTDIR);
    TEST_START(20);
    log_init("./unit.conf");

    IS( run(TRUE_BIN), 0 );
    IS( run(FALSE_BIN), 1 );

    std::string stdout;
    NOTHROW( run("(/bin/echo -n 1) && (/bin/echo -n 2 1>&2) && (/bin/echo -n 1) && (/bin/echo -n 2 1>&2)", stdout) );
    IS(stdout, "1212");

    std::string stderr;
    NOTHROW( run("(/bin/echo -n 1) && (/bin/echo -n 2 1>&2) && (/bin/echo -n 1) && (/bin/echo -n 2 1>&2)", stdout, stderr) );
    IS(stdout, "11");
    IS(stderr, "22");

    std::string base64;
    urandb64(base64);
    OK( base64 != "" );
    IS( base64.size(), 44 );

    base64.clear();
    urandb64(base64, 48);
    OK( base64 != "" );
    IS( base64.size(), 64 );

    base64.clear();
    urandb64(base64, 1024);
    OK( base64 != "" );
    IS( base64.size(), 1368 );

    // test slurp with bogus file:
    THROWS( slurp("/dev/null/bad"), 
            "INTERNAL_SERVER_ERROR [500]: Could not open /dev/null/bad: Not a directory" );

    std::string contents;
    OK( run("cat /etc/resolv.conf", contents) == 0);
    IS( slurp("/etc/resolv.conf"), contents );

    // test slurp on a non-seekable file (named pipe)
    {
        // mkfifo, make sure the pipe is removed at the end
        struct TmpPipe : std::string {
            TmpPipe() : std::string("/tmp/misc-test-p-") {
                std::string uuid;
                uuid_b32c(uuid, false);
                append(uuid);
                int r = mkfifo(c_str(), 0666);
                IS (r, 0);
            }
            ~TmpPipe() {
                unlink(c_str());
            }
        } pipe;
        
        // fork, make sure the child is killed at the end
        struct Fork {
            pid_t pid;
            Fork() : pid(fork()) {}
            ~Fork() { if (pid > 0) kill(pid, 9); }
        } f;
        
        // write down the pipe from the child
        if (f.pid == 0) {
            FILE *f = fopen(pipe.c_str(), "w");
            if (f == NULL)
                exit(1);
            fputs("Hello world\n", f);
            fclose(f);
            _exit(0);
        }
        
        IS( slurp(pipe), "Hello world\n" );
    }

    // test slurp on a large file (this executable)
    {
        std::string filename = argv[0];

        std::vector<std::string> words;
        words.push_back("cat");
        words.push_back(filename);
        std::string contents;
        run(shellquote_list(words), contents);
        OK( contents.size() > 1024 );

        IS( slurp(filename), contents );
    }


    TEST_END;
}
