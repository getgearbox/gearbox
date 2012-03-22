// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/job/StatusImpl.h>
#include <gearbox/core/Uri.h>
#include <gearbox/core/Errors.h>

namespace Gearbox {

    struct StatusImpl::Private {
        std::vector<std::string> messages;
        std::vector<std::string> children;
        std::string name;
        std::string operation;
        Json meta;
        unsigned int progress;
        int code;
        std::string resource_uri;
        std::string parent_uri;
        time_t ctime;
        time_t mtime;
        int64_t ytime;
        ConfigFile cfg;
        std::string base_uri;
        Status::State state;
        uint32_t failures;
        std::string component;
        boost::shared_ptr<StatusManager> sm;
        uint32_t concurrency;
        Private(const ConfigFile & c) : progress(0), code(-1), ctime(time(NULL)), mtime(time(NULL)), ytime(-1), cfg(c), state(Status::STATE_UNKNOWN), failures(0), concurrency(0) {}
    };

    StatusImpl::StatusImpl(const ConfigFile & c) : impl(new Private(c)) {};
    StatusImpl::StatusImpl( const StatusImpl & other ) : impl(new Private(*(other.impl))) {}

    StatusImpl::~StatusImpl() {
        if(impl) delete impl;
    }
   
    const std::vector<std::string> & StatusImpl::messages() const {
        return impl->messages;
    }
    
    void StatusImpl::messages( const std::vector<std::string> & messages ) {
        impl->messages = messages;
    }
    
    void StatusImpl::add_message( const std::string & message ) {
        impl->messages.push_back(message);
        this->mtime(time(NULL));
    }
    
    const std::vector<std::string> & StatusImpl::children() const {
        return impl->children;
    }

    void StatusImpl::children( const std::vector<std::string> & children ) {
        impl->children = children;
    }

    void StatusImpl::add_child( const std::string & child ) {
        impl->children.push_back(child);
        this->mtime(time(NULL));
    }
    
    const std::string & StatusImpl::name() const {
        return impl->name;
    }

    void StatusImpl::name(const std::string & n) {
        impl->name = n;
        this->mtime(time(NULL));
    }
    
    const std::string & StatusImpl::operation() const {
        return impl->operation;
    }

    void StatusImpl::operation(const std::string & op) {
        impl->operation = op;
        this->mtime(time(NULL));
    }

    std::string StatusImpl::uri() const {
        return this->base_uri() + "/status/" + this->name();
    }
    
    const std::string & StatusImpl::resource_uri() const {
        return impl->resource_uri;
    }

    void StatusImpl::resource_uri(const std::string & rsrc_uri) {
        impl->resource_uri = rsrc_uri;
        this->mtime(time(NULL));
    }
    
    const std::string & StatusImpl::component() const {
        return impl->component;
    }

    void StatusImpl::component(const std::string & c) {
        impl->component = c;
        this->mtime(time(NULL));
    }

    const Json & StatusImpl::meta() const {
        return impl->meta;
    }

    void StatusImpl::meta( const Json & meta ) {
        impl->meta = meta;
        this->mtime(time(NULL));
    }

    void StatusImpl::meta( const std::string & key, const Json & value ) {
        impl->meta[key] = value;
        this->mtime(time(NULL));
    }

    unsigned int StatusImpl::progress() const {
        return impl->progress;
    }

    void StatusImpl::progress(unsigned int p) {
        impl->progress = p;
        this->mtime(time(NULL));
    }
    
    void StatusImpl::fail( int code ) {
        if( ! this->state( Status::STATE_COMPLETED ) ) {
            gbTHROW(
                ERR_INTERNAL_SERVER_ERROR(
                    "Invalid status state change from " + Status::state2str(this->state()) 
                    + " to COMPLETED"
                )
            );
        }
        this->mtime(time(NULL));
        this->code(code);
        // update progress last, that is the trigger
        // that clients look for
        this->progress(100);
    }

    void StatusImpl::cancel() {
            if ( this->state() != Status::STATE_CANCELLED ) {

                if( !this->state( Status::STATE_CANCELLED ) ) {
                gbTHROW(
                    ERR_INTERNAL_SERVER_ERROR(
                        "Invalid status state change from " + Status::state2str(this->state())
                        + " to CANCELLED"
                    )
                );
            }

            if ( this->progress() < 100 ) {
                this->code(ERR_CONFLICT().code());
                // update progress last, that is the trigger
                // that clients look for
                this->progress(100);
            }
        }
    }

    void StatusImpl::success() {
        if( ! this->state( Status::STATE_COMPLETED ) ) {
            gbTHROW(
                ERR_INTERNAL_SERVER_ERROR(
                    "Invalid status state change from " + Status::state2str(this->state()) 
                    + " to COMPLETED"
                )
            );
        }
        this->mtime(time(NULL));
        this->code(0);
        // update progress last, that is the trigger
        // that clients look for
        this->progress(100);
    }

    int StatusImpl::code() const {
        return impl->code;
    }

    void StatusImpl::code(int s) {
        impl->code = s;
    }
    
    const std::string & StatusImpl::parent_uri() const {
        return impl->parent_uri;
    }

    void StatusImpl::parent_uri(const std::string & parent) {
        impl->parent_uri = parent;
        this->mtime(time(NULL));
    }
    
    StatusPtr StatusImpl::parent() const {
        if ( !this->parent_uri().empty() && impl->sm.get() ) {
            Uri uri( this->parent_uri() );
            if ( !this->base_uri().empty() && this->parent_uri().find( this->base_uri() ) == 0 ) {
                return impl->sm->fetch( uri.leaf() );
            } else {
                return impl->sm->fetch( uri );
            }
        } else {
            return StatusPtr();
        }
    }

    time_t StatusImpl::ctime() const {
        return impl->ctime;
    }

    void StatusImpl::ctime(time_t t) {
        impl->ctime = t;
    }

    time_t StatusImpl::mtime() const {
        return impl->mtime;
    }

    void StatusImpl::mtime(time_t t) {
        impl->mtime = t;
    }

    int64_t StatusImpl::ytime() const {
        return impl->ytime;
    }

    void StatusImpl::ytime(int64_t t) {
        impl->ytime = t;
    }

    void StatusImpl::base_uri( const std::string & base ) {
        impl->base_uri = base;
    }
    
    const std::string & StatusImpl::base_uri() const {       
        return impl->base_uri;
    }

    bool StatusImpl::state( Status::State state ) {
        bool valid = Status::validStateTransition(impl->state,state);
        
        if( !valid && state != Status::STATE_UNKNOWN ) {
            if( std::string(this->impltype()) != "transient" ) {
                print_trace();
                _WARN( "Attempted Invalid state transition from "
                       << Status::state2str(impl->state)
                       << " to "
                       << Status::state2str(state)
                       << " on status "
                       << impl->name
                );
            }
            return false;
        }
        
        impl->state = state;
        
        // only update mtime if not stoping or cancelling
        switch(state) {
        case Status::STATE_STOPPING:
        case Status::STATE_STOPPED:
        case Status::STATE_CANCELLING:
        case Status::STATE_CANCELLED:
            break;
        default:
            this->mtime(time(NULL));
        }

        return true;
    }

    Status::State StatusImpl::state() const 
    {
        return impl->state;
    }
    
    void StatusImpl::failures(uint32_t count) {
        impl->failures = count;
    }
    
    uint32_t StatusImpl::failures() const {
        return impl->failures;
    }
    
    const ConfigFile & StatusImpl::cfg() const {
        return impl->cfg;
    }

    std::string StatusImpl::serialize() const {
        Json out;

        Uri base_uri(
            this->base_uri().empty() ? "http://localhost:4080/internal" : this->base_uri()
        );
        out["operation"]  = impl->operation;
        out["component"]  = impl->component;
        out["state"]      = Status::state2str( impl->state );
        out["uri"]        = impl->resource_uri;
        out["status_uri"] = base_uri.canonical() + "/status/" + impl->name;
        out["progress"]   = impl->progress;
        if( impl->progress == 100 ) {
            out["code"] = impl->code;
        }
    
        out["messages"] = Json::Array();
        const std::vector<std::string> & messages = impl->messages;
        for(unsigned int i=0; i < messages.size(); i++ ) {
            out["messages"][i] = messages[i];
        }
    
    
        out["children"] = Json::Array();
        const std::vector<std::string> & children = impl->children;
        for(unsigned int i=0; i < children.size(); i++ ) {
            Uri child_uri( children[i] );
            if ( child_uri.hostname() == "localhost" ) {
                // rewrite uris of "internal" jobs so they can be queried externally
                child_uri.hostname( base_uri.hostname() );
            }
            out["children"][i] = child_uri.canonical();
        }

        if ( !impl->meta.empty() )
            out["meta"] = impl->meta;
            
        if ( ! impl->parent_uri.empty() ) {
            out["parent_uri"] = impl->parent_uri;
        }
        out["failures"] = impl->failures;
        out["ctime"] = impl->ctime;
        out["mtime"] = impl->mtime;
        if( impl->ytime > 0 ) {
            out["ytime"] = impl->ytime;
        }
        out["concurrency"] = this->concurrency();
        return out.serialize();
    }

    void StatusImpl::status_manager( const StatusManager & sm ) {
        impl->sm = boost::shared_ptr<StatusManager>( new StatusManager( sm ) );
    }

    uint32_t StatusImpl::concurrency() const {
        return impl->concurrency;
    }

    void StatusImpl::concurrency( uint32_t count ) {
        impl->concurrency = count;
    }
}
