// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/core/TempFile.h>
#include <gearbox/core/Errors.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <boost/filesystem/operations.hpp>
namespace bfs=boost::filesystem;

namespace Gearbox {

    TempFile::TempFile(const std::string & prefix) : name_(prefix + "XXXXXX") {
        mode_t prev = umask(0077);
        this->fd_ = mkstemp( const_cast<char*>(name_.data()) );
        umask(prev);
        if (this->fd_ < 0) {
            gbTHROW( ERR_LIBC("failed to create tempfile") );
        }
    }
    
    TempFile::~TempFile() {
        this->unlink();
    }

    void TempFile::release() {
        this->fd_ = -1;
        this->name_.clear();
    }
    
    void TempFile::unlink() const {
        if( !name_.empty() && bfs::exists(name_) ) {
            bfs::remove(name_);
        }
    }

    const std::string & TempFile::name() const {
        return name_;
    }
}
