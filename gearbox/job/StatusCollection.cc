// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/job/Status.h>
#include <gearbox/job/StatusCollection.h>
#include <gearbox/job/StatusCollectionImpl.h>

namespace Gearbox {

    StatusPtr StatusCollection::pop() {
        return StatusPtr(new Status(impl->pop()));
    }

    bool StatusCollection::empty() {
        return impl->empty();
    }

    StatusCollection & StatusCollection::filter_progress(unsigned int min, unsigned int max) {
        impl->filter_progress(min,max);
        return *this;
    }
    StatusCollection & StatusCollection::filter_code(int min, int max) {
        impl->filter_code(min,max);
        return *this;
    }
    StatusCollection & StatusCollection::filter_operation(const std::string & op) {
        impl->filter_operation(op);
        return *this;
    }
    StatusCollection & StatusCollection::filter_component(const std::string & c) {
        impl->filter_component(c);
        return *this;
    }
    StatusCollection & StatusCollection::filter_mtime(time_t min, time_t max) {
        impl->filter_mtime(min,max);
        return *this;
    }
    StatusCollection & StatusCollection::filter_ctime(time_t min, time_t max) {
        impl->filter_ctime(min,max);
        return *this;
    }
    StatusCollection & StatusCollection::filter_state(const std::string & state) {
        impl->filter_state(state);
        return *this;
    }
    StatusCollection & StatusCollection::filter_uri(const std::string & uri) {
        impl->filter_uri(uri);
        return *this;
    }
    StatusCollection & StatusCollection::limit(unsigned int count) {
        impl->limit(count);
        return *this;
    }

    StatusCollection::StatusCollection( StatusCollectionImpl * i ) : impl(i) {};
    StatusCollection::~StatusCollection() {
        if(impl) delete impl;
    };
}
