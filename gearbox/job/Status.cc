// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/job/Status.h>
#include <gearbox/job/StatusImpl.h>
#include <gearbox/worker/Worker.h>
#include <gearbox/core/Errors.h>
#include <gearbox/core/logger.h>
#include <stdexcept>

using namespace Gearbox;

const std::vector<std::string> & Status::messages() const {
    return impl->messages();
}

void Status::add_message( const std::string & message ) {
    impl->add_message(message);
}

const std::vector<std::string> & Status::children() const {
    return impl->children();
}

void Status::add_child( const std::string & child ) {
    impl->add_child(child);
}

const std::string & Status::name() const {
    return impl->name();
}

const std::string & Status::operation() const {
    return impl->operation();
}

const std::string & Status::resource_uri() const {
    return impl->resource_uri();
}

const std::string & Status::component() const {
    return impl->component();
}

std::string Status::uri() const {
    return impl->uri();
}

unsigned int Status::progress() const {
    return impl->progress();
}

const Json & Status::meta() const {
    return impl->meta();
}

void Status::meta( const std::string & key, const Json & value ) {
    impl->meta( key, value );
}

void Status::meta( const Json & meta ) {
    impl->meta( meta );
}

void Status::progress(unsigned int p) {
    unsigned int cp = impl->progress();
    if ( p > impl->progress() ) {
        impl->progress(p);
    }
    else if( p < cp ) {
        _WARN("Ignoring attempt to decrease progress from " << cp << " to " << p);
    }
}

void Status::fail( int code ) {
    impl->fail(code);
}

void Status::cancel() {
    impl->cancel();
}

void Status::success() {
    impl->success();
}

void Status::on(Status::Event e, const Job & job) {
    impl->on(e,job);
}

JobPtr Status::on(Status::Event e, const JobManager & jm) const {
    return impl->on(e,jm);
}

Status::Event Status::str2event( const std::string & str ) {
    return
        str == "PRECANCEL" ? EVENT_PRECANCEL :
        str == "CANCEL"    ? EVENT_CANCEL    :
                             EVENT_UNKNOWN;
}

std::string Status::event2str( Status::Event event ) {
    switch( event ) {
        case EVENT_PRECANCEL: return "PRECANCEL"; break;
        case EVENT_CANCEL:    return "CANCEL";    break;
        default:              return "UNKNOWN";    break;
    }
}

int Status::code() const {
    return impl->code();
}

const std::string & Status::parent_uri() const {
    return impl->parent_uri();
}

StatusPtr Status::parent() const {
    return impl->parent();
}

void Status::parent_uri(const std::string & p) {
    impl->parent_uri(p);
}

time_t Status::ctime() const {
    return impl->ctime();
}

time_t Status::mtime() const {
    return impl->mtime();
}

int64_t Status::ytime() const {
    return impl->ytime();
}

void Status::ytime(int64_t t) {
    impl->ytime(t);
}

void Status::sync() {
    return impl->sync();
}

const char * Status::impltype() const {
    return impl->impltype();
}

bool Status::has_completed() const {
    return impl->progress() == 100;
}

bool Status::is_success() const {
    if( impl->progress() != 100 )
        gbTHROW( std::logic_error("is_success is invalid on in-complete status") );
    return impl->code() == 0;
}

uint32_t Status::failures() const {
    return impl->failures();
}

void Status::failures(uint32_t count) {
    impl->failures(count);
}

std::string Status::serialize() const {
    return impl->serialize();
}

bool
Status::state( Status::State state )
{
    return impl->state( state );
}

Status::State
Status::state() const 
{
    return impl->state();
}

void
Status::checkpoint()
{
    this->sync();
    switch( this->state() ) {
    case STATE_STOPPING:
    case STATE_STOPPED:
    case STATE_CANCELLING:
    case STATE_CANCELLED:
        if( this->impl->state( STATE_STOPPED ) ) {
            this->impl->code( ERR_GONE().code() );
            this->impl->progress(100);
        }
        gbTHROW( WorkerStop() );
        break;
    default:
        break;
    }
}

Status::Status(const Status & copy) : impl(copy.impl->clone()) {}

Status & Status::operator=(const Status & copy) {
    if( this == &copy ) return *this;
    if(impl) delete impl;
    impl = copy.impl->clone();
    return *this;
}

Status::~Status() {
    if(impl) delete this->impl;
};

Status::Status( StatusImpl * i ) : impl(i) {}

Status::State Status::str2state( const std::string & str ) {
    std::string istr;
    // resize so istr can fit the upper case str
    istr.resize(str.size());
    std::transform( str.begin(), str.end(), istr.begin(), toupper );
    return
        istr == "PENDING"    ? STATE_PENDING    :
        istr == "RUNNING"    ? STATE_RUNNING    :
        istr == "CANCELLING" ? STATE_CANCELLING :
        istr == "CANCELLED"  ? STATE_CANCELLED  :
        istr == "COMPLETED"  ? STATE_COMPLETED  :
        istr == "STOPPING"   ? STATE_STOPPING   :
        istr == "STOPPED"    ? STATE_STOPPED    :
                               STATE_UNKNOWN;
}

std::string Status::state2str( Status::State state ) {
    switch( state ) {
        case STATE_PENDING:    return "PENDING";    break;
        case STATE_RUNNING:    return "RUNNING";    break;
        case STATE_CANCELLING: return "CANCELLING"; break;
        case STATE_CANCELLED:  return "CANCELLED";  break;
        case STATE_COMPLETED:  return "COMPLETED";  break;
        case STATE_STOPPING:   return "STOPPING";   break;
        case STATE_STOPPED:    return "STOPPED";    break;
        default:               return "UNKNOWN";    break;
    }
}

bool Status::validStateTransition( Status::State from, Status::State to ) {

    if( to == Status::STATE_UNKNOWN ) return true;
    if( to == from ) return true;

    bool valid = false;
    switch(from) { 
    case Status::STATE_UNKNOWN:
        valid = true;
        break;
    case Status::STATE_PENDING:
        if ( to == Status::STATE_RUNNING || to == Status::STATE_CANCELLED || to == Status::STATE_COMPLETED ) 
            valid = true;
        break;
    case Status::STATE_RUNNING:
        if( to == Status::STATE_COMPLETED || to == Status::STATE_STOPPING )
            valid = true;
        break;
    case Status::STATE_STOPPING:
        if( to == Status::STATE_STOPPED || to == Status::STATE_COMPLETED )
            valid = true;
        break;
    case Status::STATE_STOPPED:
        if( to == Status::STATE_CANCELLING )
            valid = true;
        break;
    case Status::STATE_COMPLETED:
        if( to == Status::STATE_CANCELLING )
            valid = true;
        break;
    case Status::STATE_CANCELLING:
        // if on-cancel job fails then state will move
        // from CANCELLING to COMPLETED so that the cancel
        // operation can be reattempted
        if( to == Status::STATE_CANCELLED || to == Status::STATE_COMPLETED )
            valid = true;
        break;
    case Status::STATE_CANCELLED:
        break;
    }
    return valid;
}

uint32_t Status::concurrency() const {
    return impl->concurrency();
}

void Status::starting() {
    return impl->starting();
}

void Status::stopping() {
    return impl->stopping();
}
