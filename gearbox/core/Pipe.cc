// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/core/Pipe.h>
#include <gearbox/core/Errors.h>
#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <unistd.h>

namespace Gearbox {
    Pipe::Pipe() {
        if ( pipe(this->filedes) < 0 ) {
            gbTHROW( ERR_LIBC("pipe") );
        }
    }

    Pipe::~Pipe() {
        // prevent throwing out of dtor, uncatchable
        if( filedes[0] ) ::close(filedes[0]);
        if( filedes[1] ) ::close(filedes[1]);
    }

    int Pipe::fd(int ix) {
        if( ix < 0 || ix > 1 ) {
            gbTHROW( std::out_of_range("invalid index for pipe close: " + boost::lexical_cast<std::string>(ix)) );
        }
        return filedes[ix];
    }

    void Pipe::close(int ix) {
        if( ix < 0 || ix > 1 ) {
            gbTHROW( std::out_of_range("invalid index for pipe close: " + boost::lexical_cast<std::string>(ix)) );
        }

        if( filedes[ix] ) {
            if ( ::close(filedes[ix]) < 0 ) {
                gbTHROW( ERR_LIBC("close filedes " + boost::lexical_cast<std::string>(ix)) );
            }
            filedes[ix] = 0;
        }
    }

    ReadPipe::ReadPipe() {}
    int ReadPipe::fd() {
        return this->super::fd(0);
    }
}
