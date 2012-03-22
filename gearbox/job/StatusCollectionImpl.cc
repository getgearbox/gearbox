// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/job/StatusCollectionImpl.h>


namespace Gearbox {
    struct StatusCollectionImpl::Private {
        ConfigFile cfg;
        Private(const ConfigFile & c) : cfg(c) {};
        Private( const Private & other ) : cfg(other.cfg) {}
    };

    StatusCollectionImpl::StatusCollectionImpl(const ConfigFile & c) : impl(new Private(c)) {}
    StatusCollectionImpl::StatusCollectionImpl( const StatusCollectionImpl & other ) : impl(new Private(*(other.impl))) {}

    StatusCollectionImpl::~StatusCollectionImpl() {
        if(impl) delete impl;
    }

    const ConfigFile & StatusCollectionImpl::cfg() const {
        return impl->cfg;
    }
}

