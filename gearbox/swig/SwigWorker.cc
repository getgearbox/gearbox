// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include "SwigWorker.h"

#include <gearbox/core/ConfigFile.h>
#include <gearbox/core/Errors.h>
#include <gearbox/core/logger.h>

using namespace Gearbox;

SwigWorker::SwigWorker(const std::string &config) : Worker(config) { }
SwigWorker::~SwigWorker() {}

void SwigWorker::register_handler( const std::string & function_name ) { 
    this->register_handler( 
        function_name,
        static_cast<Worker::handler_t>(&SwigWorker::redispatcher) 
    );
}

Worker::response_t
SwigWorker::redispatcher( const Job  & job, JobResponse & resp ) {
    return this->do_dispatch(job,resp);
}

Worker::response_t
SwigWorker::do_dispatch( const Job & job, JobResponse & resp ) {
    _WARN("do_dispatch should be overridden in target language");
    return WORKER_ERROR;
}
