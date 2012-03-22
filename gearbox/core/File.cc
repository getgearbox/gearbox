// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/core/File.h>

#include <gearbox/core/Errors.h>
#include <sys/stat.h>

namespace bfs=boost::filesystem;

namespace Gearbox {
    File::File() : fd_(-1) {}

    File::File(const bfs::path & pathname, int flags, mode_t mode) : fd_(-1) {
        this->open(pathname,flags,mode);
    }

    File::File(int fd) : fd_(fd) {}

    File::~File() {
        this->close();
    }

    int File::fd() {
        return this->fd_;
    }

    void File::open(const bfs::path & pathname, int flags, mode_t mode) {
        if( fd_ >= 0 ) this->close();
        this->fd_ = ::open(pathname.string().c_str(), flags, mode);
        if( this->fd_ == -1 ) {
            gbTHROW( ERR_LIBC("could not open file " + pathname.string()) );
        }
    }

    void File::close() {
        if( fd_ >= 0 ) {
            ::close(fd_);
        }
    }

}
