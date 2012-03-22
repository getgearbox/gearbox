// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/job/JobResponse.h>

namespace Gearbox {
    struct JobResponse::Private {
        std::string content;
        Hash headers;
        StatusPtr status;
        int code;
        JobPtr job;
        Private() : code(-1) {}
        Private(const Private & other)
            : content(other.content),
              headers(other.headers),
              status(other.status),
              code(other.code),
              job(other.job) {}
        
    };
        
    JobResponse::JobResponse() : impl(new Private()) {}

    JobResponse::JobResponse(const JobResponse & copy) : impl( new Private(*(copy.impl)) ) {}

    JobResponse & JobResponse::operator=(const JobResponse & copy) {
        if( this == &copy ) return *this;
        this->impl->content = copy.impl->content;
        this->impl->headers = copy.impl->headers;
        this->impl->status  = copy.impl->status;
        this->impl->code    = copy.impl->code;
        this->impl->job     = copy.impl->job;
        return *this;
    }

    JobResponse::~JobResponse() {
        if( impl ) delete impl;
    }
    
    void JobResponse::content( const std::string & content ) {
        this->impl->content = content;
    }

    const std::string & JobResponse::content() const {
        return this->impl->content;
    }

    void JobResponse::headers( const Hash & headers ) {
        this->impl->headers = headers;
    }

    void JobResponse::add_header( const std::string & name, const std::string & value ) {
        this->impl->headers[name] = value;
    }

    const Hash & JobResponse::headers() const {
        return this->impl->headers;
    }

    void JobResponse::status( const Status & status ) {
        this->impl->status.reset(new Status(status));
    }

    const StatusPtr & JobResponse::status() const {
        return this->impl->status;
    }
    
    void JobResponse::code( int code ) {
        this->impl->code = code;
    }

    int JobResponse::code() const {
        return this->impl->code;
    }

    void JobResponse::job( const JobPtr & job ) {
        this->impl->job = job;
    }

    const JobPtr & JobResponse::job() const {
        return this->impl->job;
    }
}
