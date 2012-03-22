// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/core/TempDir.h>
#include <gearbox/core/Errors.h>

#include <stdlib.h>

#include <boost/filesystem.hpp>

namespace Gearbox {
    TempDir::TempDir(const std::string & prefix) : name_(prefix + "XXXXXX") {
        if( ! mkdtemp(const_cast<char*>(name_.data())) ) {
            gbTHROW( ERR_LIBC("mkdtemp failed for path: " + name_) );
        }
    }

    TempDir::~TempDir() {
        boost::filesystem::remove_all(name_);
    }

    const std::string & TempDir::name() const {
        return name_;
    }
}
