// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/core/util.h>
#include <gearbox/core/logger.h>
#include <gearbox/core/Errors.h>
#include <gearbox/core/TempFile.h>

#include <sys/wait.h> //waitpid
#include <stdio.h>

namespace Gearbox {

    // stupid functions so we can stub them out for testing
    int do_system(const std::string & cmd) {
        return system(cmd.c_str());
    }
    int do_execvp(const char* file, char* const argv[]) {
        return execvp(file, argv);
    }

    static void do_exec(const Cmd & args, int out, int err) {
        size_t count = args.size();
        const char* exec_args[count + 1];
        for (size_t i=0; i<count; ++i)
            exec_args[i]=args[i].c_str();
        exec_args[count] = NULL;

        // child
        // don't explicitly close fds, dup2 will close them only if they differ
        dup2(out, 1);
        dup2(err, 2);
        int rc = do_execvp(exec_args[0], const_cast<char * const *>(exec_args));
        // don't log here, higher level functions do the logging
        perror( ("execvp: " + args[0]).c_str() );
        fflush(NULL);
        _exit(WEXITSTATUS(rc) | (rc & 0xFF)); // should never happen
    }


    static void close_io(int stdin, int stdout, int stderr) {
        for( int i=0; i < 256; ++i ) {
            if( i!=stdin && i!=stdout && i!=stderr ) {
                ::close(i);
            }
        }
    }

    pid_t run_bg(const Cmd & args, ReadPipe & p) {
        _DEBUG( "run(): [" << shellquote_list(args) << "]");

        pid_t pid = fork();
        if (pid < 0)
            gbTHROW( ERR_LIBC("unable to fork") );

        if (pid == 0) {
            close_io(-1,p.super::fd(1),p.super::fd(1));
            do_exec(args,p.super::fd(1), p.super::fd(1));
        }
        else {
            p.close(1);
        }

        return pid;
    }

    pid_t run_bg(const Cmd & args, int fd_out, int fd_err) {
        _DEBUG( "run(): [" << shellquote_list(args) << "]");

        pid_t pid = fork();
        if (pid < 0)
            gbTHROW( ERR_LIBC("unable to fork") );

        if (pid == 0) {
            do_exec(args,fd_out,fd_err);
        }

        return pid;
    }

    int run(const Cmd & args, int fd_out, int fd_err) {
        pid_t pid = run_bg(args, fd_out, fd_err);

        int rc = -1;
        waitpid(pid, &rc, 0);
        if( rc ) {
            _ERROR(
                "run(): execvp(" << shellquote_list(args) << ") failed with error "
                << rc << " with stderr: " << slurp(fd_err)
            );
        }
        return (WEXITSTATUS(rc) | (rc & 0xFF));
    }

    int run (const Cmd & args, std::string & std_out)
    {
        TempFile tmp;
        int rv = run(args, tmp.fd(), tmp.fd());
        std_out = slurp(tmp.fd(), true);
        return rv;
    }

    int run (const Cmd & args, std::string & std_out, std::string & std_err)
    {
        TempFile out;
        TempFile err;
        int rv = run(args, out.fd(), err.fd());
        std_out = slurp(out.fd(), true);
        std_err = slurp(err.fd(), true);
        return rv;
    }

    static bool
    has_root_capabilities() {
        return !(getuid() && geteuid());
    }

    static int run (const std::string & cmd, int fout, int ferr)
    {
        _DEBUG( "run(): [" << cmd << "]");
        if ( has_root_capabilities() ) {
            gbTHROW( std::runtime_error("a root process must not run shell-parsed commands") );
        }

        int pid = fork();
        if( pid < 0 ) {
            gbTHROW( ERR_LIBC("unable to fork!") );
        }
        if( pid == 0 ) {
            close_io(-1,fout,ferr);
            // child
            dup2(fout, 1);
            dup2(ferr, 2);
            int rc = do_system(cmd);
            fflush(NULL);
            // use _exit to prevent destructors or atexit routines from running
            // must exit non-zero if process dies from a signal
            _exit(WEXITSTATUS(rc) | (rc & 0xFF));
        }
        int rc = -1; // initialize to a non-zero value in case waitpid fails
        waitpid(pid,&rc,0);
        return (WEXITSTATUS(rc) | (rc & 0xFF));
    }

    int run( const std::string & cmd ) {
        return run(cmd, 1, 2);
    }

    int run (const std::string & cmd, std::string & output)
    {
        TempFile out("/tmp/run-stdout");
        int rv = run(cmd, out.fd(), out.fd());
        output = slurp(out.fd());
        if( rv ) {
            _ERROR( "run(): system(" << cmd << ") failed with error " << rv << " and output: " << output);
        }
        return rv;
    }

    int run (const std::string & cmd, std::string & stdout, std::string & stderr)
    {

        TempFile out("/tmp/run-stdout"), err("/tmp/run-stderr");
        int rv = run(cmd, out.fd(), err.fd());
        stdout = slurp(out.fd());
        stderr = slurp(err.fd());
        if( rv ) {
            _ERROR( "run(): system(" << cmd << ") failed with error " << rv << " and stderr: " << stderr);
        }
        return rv;
    }
}
