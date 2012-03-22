// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/core/logger.h>
#include <gearbox/core/Errors.h>
#include <gearbox/job/TransientStatusImpl.h>
#include <gearbox/job/JobManager.h>

namespace Gearbox {
    const static unsigned int MAX_QUEUE=100;

    struct TransientStatusData {
        std::string name;
        std::vector<std::string> messages;
        std::vector<std::string> children;
        std::string operation;
        Json meta;
        unsigned int progress;
        int code;
        std::string resource_uri;
        std::string parent_uri;
        time_t ctime;
        time_t mtime;
        int64_t ytime;
        Status::State state;
        uint32_t failures;
        std::string cancel_job_name;
        std::string cancel_envelope;
        std::map<Status::Event,Hash> event_handlers;
        std::string component;
        uint32_t concurrency;
        TransientStatusData() : progress(0), code(-1), ctime(time(NULL)), mtime(time(NULL)), ytime(-1), state(Status::STATE_UNKNOWN), failures(0), concurrency(0) {}

    };

    static TransDB db;

    TransientStatusImpl::TransientStatusImpl(const ConfigFile & c)
        : super(c) {}

    TransientStatusImpl::TransientStatusImpl( const TransientStatusImpl & other )
        : super(other), data(other.data) {}

    TransientStatusImpl::~TransientStatusImpl() {}

    StatusImpl * TransientStatusImpl::clone() const {
        return new TransientStatusImpl(*this);
    }

    void TransientStatusImpl::load() {
        TransDB::iterator it = db.begin();
        for( ; it != db.end(); ++it ) {
            if( (*it)->name == this->super::name() )
                this->data = (*it);
        }
        if( !this->data.get() ) {
            gbTHROW( ERR_NOT_FOUND("status " + this->super::name() + " not found") );
        }

        // cache the mtime of the status since some operations will update the mtime
        time_t mtime =  data->mtime;

        this->super::messages( data->messages );
        this->super::children( data->children );
        this->super::operation( data->operation );
        this->super::resource_uri( data->resource_uri );
        this->super::component( data->component );
        this->super::state( data->state );
        this->super::meta( data->meta );
        this->super::progress( data->progress );
        this->super::code( data->code );
        this->super::parent_uri( data->parent_uri );
        this->super::ctime( data->ctime );
        this->super::ytime( data->ytime );
        this->mtime( mtime );
        this->super::concurrency( data->concurrency );
    }

    void TransientStatusImpl::insert() {
        if( db.size() == MAX_QUEUE ) {
            db.pop_front();
        }

        // TransDB::iterator it = db.find( this->name() );
        // if( it != db.end() ) {
        //     throw ERR_BAD_REQUEST("status " + this->name() + " already exists!");
        // }
        this->data = boost::shared_ptr<TransientStatusData>(new TransientStatusData());
        db.push_back(this->data);

        // reset this b/c it was not synced do the TransientStatusData object yet
        this->name(this->super::name());
        this->operation(this->super::operation());
        this->resource_uri(this->super::resource_uri());
        this->component( this->super::component() );
        this->state( this->super::state() );
        this->meta( this->super::meta() );
    }

    void TransientStatusImpl::sync() {
        this->super::state(Status::STATE_UNKNOWN);
        this->load();
    }

    void TransientStatusImpl::messages( const std::vector<std::string> & messages ) {
        data->messages = messages;
        this->super::messages(messages);
    }
    void TransientStatusImpl::add_message( const std::string & message ) {
        data->messages.push_back(message);
        this->super::add_message(message);
    }
    void TransientStatusImpl::children( const std::vector<std::string> & children ) {
        data->children = children;
        this->super::children(children);
    }
    void TransientStatusImpl::add_child( const std::string & child ) {
        data->children.push_back(child);
        this->super::add_child(child);
    }
    void TransientStatusImpl::name(const std::string & n) {
        if( data.get() ) {
            data->name = n;
        }
        this->super::name(n);
    }
    void TransientStatusImpl::operation(const std::string & op) {
        if( data.get() ) {
            data->operation = op;
        }
        this->super::operation(op);
    }
    void TransientStatusImpl::resource_uri(const std::string & rsrc_uri) {
        if( data.get() ) {
            data->resource_uri = rsrc_uri;
        }
        this->super::resource_uri(rsrc_uri);
    }
    void TransientStatusImpl::component(const std::string & c) {
        if( data.get() ) {
            data->component = c;
        }
        this->super::component(c);
    }
    void TransientStatusImpl::meta(const std::string & key, const Json & value) {
        if ( data.get() ) {
            data->meta[key] = value;
        }
        this->super::meta( key, value );
    }
    void TransientStatusImpl::meta(const Json & meta) {
        if ( data.get() ) {
            data->meta = meta;
        }
        this->super::meta( meta );
    }
    void TransientStatusImpl::progress(unsigned int p) {
        data->progress = p;
        this->super::progress(p);
    }
    void TransientStatusImpl::code(int c) {
        data->code = c;
        this->super::code(c);
    }
    bool TransientStatusImpl::state(Status::State state) {
        if( data.get() ) {
            if( !Status::validStateTransition(data->state,state) ) {
                return false;
            }
            data->state = state;
        }
        return this->super::state(state);
    }
    void TransientStatusImpl::parent_uri(const std::string & parent) {
        data->parent_uri = parent;
        this->super::parent_uri(parent);
    }
    void TransientStatusImpl::ctime(time_t t) {
        data->ctime = t;
        this->super::ctime(t);
    }
    void TransientStatusImpl::mtime(time_t t) {
        if( data.get() )
            data->mtime = t;
        this->super::mtime(t);
    }

    void TransientStatusImpl::ytime(int64_t t) {
        if( data.get() )
            data->ytime = t;
        this->super::ytime(t);
    }

    void TransientStatusImpl::failures(uint32_t count) {
        if( data.get() )
            data->failures = count;
        this->super::failures(count);
    }

    const char * TransientStatusImpl::impltype() const {
        return "transient";
    }

    static std::string static_base_uri("http://localhost:4080/transient");

    const std::string & TransientStatusImpl::base_uri() const {
        return static_base_uri;
    }

    void TransientStatusImpl::on(Status::Event e, const Job & job) {
        data->event_handlers[e]["name"] = job.name();
        data->event_handlers[e]["envelope"] = job.serialize();
    }

    JobPtr TransientStatusImpl::on(Status::Event e, const JobManager & jm) const {
        if( data->event_handlers[e].empty() )
            return JobPtr();
        return jm.job(data->event_handlers[e]["name"], data->event_handlers[e]["envelope"]);
    }

    void TransientStatusImpl::concurrency(uint32_t count) {
        if( data.get() )
            data->concurrency = count;
        this->super::concurrency(count);
    }

    void TransientStatusImpl::starting() {
        this->concurrency( this->concurrency() + 1 );
    }

    void TransientStatusImpl::stopping() {
        this->concurrency( this->concurrency() - 1 );
    }
}
