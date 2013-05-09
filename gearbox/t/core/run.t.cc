// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>
#include <string>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <boost/filesystem/operations.hpp>
#include <gearbox/core/logger.h>
#include <gearbox/core/util.h>
#include <gearbox/core/TempFile.h>
namespace bfs=boost::filesystem;

using namespace Gearbox;
using std::string;

using namespace log4cxx;
using Gearbox::operator<<;

int main()
{
    chdir(TESTDIR);
    if (geteuid() == 0) {
        std::cerr << "This test should not be run as root" << std::endl;
        exit(1);
    }

    TEST_START(35);
    Gearbox::log_init("./unit.conf");

    std::vector<std::string> args;

    // test the fancy operator<< for adding things to the vector
    args << "ls" << "-l";
    IS( shellquote_list(args), "ls -l" );

    // this time with lexical_cast
    args.clear();
    args << "head" << "-n" << 10;
    IS( shellquote_list(args), "head -n 10" );

    // test execvp functionality
    args.clear();
    
    args << "cat" << (bfs::exists("/etc/shadow") ? "/etc/shadow" : "/etc/master.passwd");
    // we shouldn't be able to cat that file
    string out, err;
    int rv = run(args, out, err);
    OK( rv != 0 );     // error
    OK( !err.empty() ); // some error logged to stderr
    OK( out.empty() ); // and nothing on our stdout

    out.clear();
    err.clear();
    rv = run(args, out); // single argument: gets both stdout + stderr
    OK( rv != 0 );     // error
    OK( !out.empty() ); // some error logged to stderr

    TempFile stdout_test;
    string test_str("This is a test\n");
    bfs::path path(stdout_test.name());
    write_file(path, test_str);

    args.clear();
    args << "cat" << path.string();

    rv = run(args, out, err);
    IS( rv, 0 ); // no error
    OK( err.empty() ); // nothing on stderr
    IS( out, test_str );

    TempFile fd_test;
    fd_test.unlink();
    // redirect the output to a file descriptor
    rv = run(args, fd_test.fd(), fd_test.fd());
    IS( rv, 0 );
    string res = slurp(fd_test.fd(), true);
    IS( test_str, res );

    // run works with shell metacharacters
    args.clear(); out.clear(); err.clear();
    args << "touch" << "<";
    rv = run(args, out, err);
    IS( rv, 0 );
    OK( out.empty() );
    OK( err.empty() );
    OK( bfs::exists("<") );

    // run doesn't mess with shell commands
    args.clear(); out.clear(); err.clear();
    args << "sh" << "-c" << "rm '<' < /dev/null | true";
    rv = run(args, out, err);
    IS( rv, 0 );
    OK( out.empty() );
    OK( err.empty() );
    OK( !bfs::exists("<") );

    args.clear(); out.clear(); err.clear();
    args << "sh" << "-c" << "echo '<' < /dev/null";
    rv = run(args, out, err);
    IS( rv, 0 );
    IS( out, "<\n" );
    OK( err.empty() );

    {
        ReadPipe pipe;
        args.clear();
        args << "echo" << "hello";
        pid_t pid = run_bg(args, pipe);
        OK( pid != -1 );
        IS( waitpid(pid, &rv, 0), pid );
        OK( !rv );
        out = slurp(pipe.fd());
        IS( out, "hello\n" );
    }

    {
        TempFile f;
        f.unlink();
        args.clear();
        args << "echo" << "hello";
        pid_t pid = run_bg(args, f.fd(), f.fd());
        OK( pid != -1 );
        IS( waitpid(pid, &rv, 0), pid );
        OK( !rv );
        out = slurp(f.fd());
        IS( out, "hello\n" );
    }

    {
        TempFile f;
        f.unlink();
        args.clear();
        args << "/dev/null";
        pid_t pid = run_bg(args, f.fd(), f.fd());
        OK( pid != -1 );
        IS( waitpid(pid, &rv, 0), pid );
        OK( rv );
        out = slurp(f.fd());
        IS( out, "execvp: /dev/null: Permission denied\n" );
    }

    TEST_END;
}
