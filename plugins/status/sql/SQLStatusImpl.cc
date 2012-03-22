// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/core/logger.h>

#include "SQLStatusImpl.h"
#include <gearbox/store/dbconn.h>
#include <gearbox/core/Errors.h>
#include <gearbox/core/Uri.h>

#include <gearbox/job/JobManager.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <gearbox/core/logger.h>

using namespace Gearbox;
using namespace Gearbox::Database;
using namespace soci;
using std::string;
using std::vector;

// Plugin Interface
void * sql_status_new(const Gearbox::ConfigFile & cfg, bool collection) {
    if( collection ) {
        return new Gearbox::SQLStatusCollectionImpl(cfg);
    }
    return new Gearbox::SQLStatusImpl(cfg);
}

const char * name() {
    return "sql";
}

#include <soci/exchange-traits.h>
namespace soci
{
    template<> struct type_conversion<Gearbox::SQLStatusImpl>
    {
        typedef values base_type;
        static void from_base(values const & v, indicator /* ind */, Gearbox::SQLStatusImpl & stat) {
            stat.super::name(v.get<std::string>("name"));
            stat.super::operation(v.get<std::string>("operation"));
            stat.super::component(v.get<std::string>("component"));
            stat.super::progress(v.get<int>("progress"));
            stat.super::code(v.get<int>("code"));
            stat.super::resource_uri(v.get<std::string>("resource_uri"));
            stat.super::parent_uri(v.get<std::string>("parent_uri"));
            stat.super::ctime(v.get<int>("ctime"));
            stat.super::ytime(v.get<int>("ytime"));
            stat.super::state( Gearbox::Status::str2state(v.get<std::string>("state")) );
            stat.super::failures( v.get<int>("failures") );

            // this must be set last because any previous assignment
            // will also reset the mtime to "now", so we need this
            // override to happen last
            stat.super::mtime(v.get<int>("mtime"));
        }

        static void to_base( const Gearbox::SQLStatusImpl & stat, values & v, indicator & ind) {
            v.set("name", stat.super::name());
            v.set("operation", stat.super::operation());
            v.set("component", stat.super::component());
            v.set("progress", (int)stat.super::progress());
            v.set("code", stat.super::code());
            v.set("resource_uri", stat.super::resource_uri());
            v.set("parent_uri", stat.super::parent_uri());
            v.set("ctime", (int)stat.super::ctime());
            v.set("mtime", (int)stat.super::mtime());
            v.set("ytime", (int)stat.super::ytime());
            v.set("state", Gearbox::Status::state2str( stat.super::state() ));
            v.set("failures", (int)stat.super::failures());
        }
    };
}

namespace Gearbox {

    SQLStatusImpl::SQLStatusImpl(const ConfigFile & c)
        : super(c),
          messages_loaded(false),
          children_loaded(false),
          meta_loaded(false),
          loaded(false),
          should_load(false) {
        if ( c.as_json().hasKey("status") ) {
            db_init( c.get_json("status"), "status");
        }
    }

    SQLStatusImpl::SQLStatusImpl( const SQLStatusImpl & other )
        : super(other),
          messages_loaded(other.messages_loaded),
          children_loaded(other.children_loaded),
          meta_loaded(other.meta_loaded),
          loaded(other.loaded),
          should_load(other.should_load) {}


    SQLStatusImpl::~SQLStatusImpl() {}

    StatusImpl * SQLStatusImpl::clone() const {
        return new SQLStatusImpl(*this);
    }

    // the load impl is just a stub. we will delay
    // the actual data population for lazy-loading
    // via really_load()
    void SQLStatusImpl::load() {
        should_load = true;
    }

    void SQLStatusImpl::really_load() {
        while( true ) {
            try {
                std::string name,op,comp,uri,parent,state;
                int progress,code,ctime,mtime,ytime,failures;
                LoggedStatement lst("status");
                lst <<
                    "SELECT name, operation, component, progress, code, resource_uri, parent_uri, ctime, mtime, ytime, state, failures " <<
                    "  FROM status " <<
                    " WHERE name = :name ";
                lst.use(this->super::name());
                lst.into(name).into(op).into(comp).into(progress).into(code).into(uri).into(parent).into(ctime).into(mtime).into(ytime).into(state).into(failures);
                lst.execute(true);

                if( !lst.got_data() ) {
                    gbTHROW( ERR_NOT_FOUND("status " + this->name() + " not found") );
                }
                _DEBUG("got:" << " name=" << name<< " op=" << op<< " comp=" << comp<< " progress=" << progress<< " code=" << code<< " uri=" << uri<< " parent=" << parent<< " ctime=" << ctime<< " mtime=" << mtime<< " ytime=" << ytime<< " state=" << state<< " failures=" << failures);
                this->super::name(name);
                this->super::operation(op);
                this->super::component(comp);
                this->super::progress(progress);
                this->super::code(code);
                this->super::resource_uri(uri);
                this->super::parent_uri(parent);
                this->super::ctime(ctime);
                this->super::ytime(ytime);
                this->super::state(Gearbox::Status::str2state(state));
                this->super::failures(failures);

                // this must be set last because any previous assignment
                // will also reset the mtime to "now", so we need this
                // override to happen last
                this->super::mtime(mtime);

                this->loaded = true;
                this->should_load = true;
                break;
            }
            catch( const ERR_NOT_FOUND & err ) {
                throw;
            }
            catch( const std::exception & err ) {
                _WARN("Failed to load status: " << err.what());
                sleep(1);
            }
        }
    }

    void
    SQLStatusImpl::end(int code, Status::State s) {
        if( !this->loaded ) {
            this->really_load();
        }
        else {
            this->sync();
        }

        if( ! this->super::state( s ) ) {
            gbTHROW(
                ERR_INTERNAL_SERVER_ERROR(
                    "Invalid status state change from " + Status::state2str(this->state())
                    + " to " + Status::state2str(s)
                )
            );
        }

        if( this->progress() < 100 ) {
            this->super::code(code);
            this->super::progress(100);
        }

        // state updates are critical, so
        // we keep trying until there are not exceptions
        // which should be pretty rare anyway.
        while( true ) {
            try {
                session & sess = getconn("status");
                dblock("status");

                std::string query_suffix;
                if( sess.get_backend_name() == "mysql" ) {
                    query_suffix = " FOR UPDATE";
                }

                LoggedStatement lst(sess);
                // for mysql we SELECT .. FOR UPDATE to lock the row, with sqlite
                // the dblock() above will do an exclusive database lock
                std::string current_state;
                lst << "SELECT state FROM status WHERE name = :name" << query_suffix;
                lst.use(this->super::name()).into(current_state).execute(true);
                
                if( ! Status::validStateTransition(Status::str2state(current_state), s) ) {
                    _ERROR("Invalid status state change from " << current_state
                           + " to " + Status::state2str(s));
                    dbrollback("status");
                    return;
                }

                lst << "UPDATE status "
                    << "   SET progress=:progress, "
                    << "       code=:code, "
                    << "       state=:state "
                    << " WHERE name=:name ";
                lst.use(this->super::progress()).use(this->super::code()).use(Status::state2str(this->super::state())).use(this->super::name()).execute(true);

                dbcommit("status");

                return;
            }
            catch( const std::exception & err ) {
                _WARN(err.what());
                dbrollback("status");
                sleep(1);
            }
        }
    }

    void
    SQLStatusImpl::fail( int code ) {
        this->end(code,Status::STATE_COMPLETED);
    }

    void
    SQLStatusImpl::cancel() {
        if ( this->state() != Status::STATE_CANCELLED ) {
            int code = ( this->code() == -1 ) ? ERR_CONFLICT().code() : this->code();
            this->end( code, Status::STATE_CANCELLED );
        }
    }

    void
    SQLStatusImpl::success() {
        this->end(0,Status::STATE_COMPLETED);
    }

    void SQLStatusImpl::on(Status::Event e, const Job & job) {
        while( true ) {
            try {
                LoggedStatement lst("status");
                lst << "REPLACE INTO status_event (status_name, event_type, name, envelope) "
                    << " VALUES (:status_name, :event_type, :name, :envelope)";
                lst.use(this->super::name()).use(Status::event2str(e)).use(job.name()).use(job.serialize()).execute(true);
                break;
            }
            catch( const std::exception & err ) {
                _WARN("Failed to save status event job: " << err.what());
                sleep(1);
            }
        }
    }

    JobPtr SQLStatusImpl::on(Status::Event e, const JobManager & jm) const {
        string name, envelope;
        while( true ) {
            try {
                LoggedStatement lst("status");
                lst << "SELECT name, envelope "
                    << "  FROM status_event "
                    << " WHERE status_name=:status_name "
                    << "   AND event_type=:type ";
                lst.into(name).into(envelope).use(this->super::name()).use(Status::event2str(e)).execute(true);
                
                if( lst.got_data() ) {
                    return jm.job(name, envelope);
                }
                return JobPtr();
            }
            catch( const std::exception & err ) {
                _WARN("Failed to load status event: " << err.what());
                sleep(1);
            }
        }
    }


    void SQLStatusImpl::insert() {
        while( true ) {
            try {
                LoggedStatement lst( "status" );
                lst << "INSERT INTO status ( "
                    << "    name, operation, component, progress, code, resource_uri, parent_uri, ctime, mtime, ytime, state, failures "
                    << ") VALUES ( "
                    << "    :name, :operation, :component, :progress, :code, :resource_uri, :parent_uri, :ctime, :mtime, :ytime, :state, :failures "
                    << ") ";
                lst.use(this->super::name());
                lst.use(this->super::operation());
                lst.use(this->super::component());
                lst.use(this->super::progress());
                lst.use(this->super::code());
                lst.use(this->super::resource_uri());
                lst.use(this->super::parent_uri());
                lst.use(this->super::ctime());
                lst.use(this->super::mtime());
                lst.use(this->super::ytime());
                lst.use(Status::state2str(this->super::state()));
                lst.use(this->super::failures());
                lst.execute(true);
                return;
            }
            catch( const std::exception & err ) {
                _WARN("failed to insert status: " << err.what());
                sleep(1);
            }
        }
        this->loaded = true;
    }

    const char * SQLStatusImpl::impltype() const {
        return "sql";
    }

    const std::vector<std::string> & SQLStatusImpl::messages() const {
        // if messages are not loaded, we need to load them first
        if( ! this->messages_loaded ) {
            while( true ) {
                try {
                    // make sure messages are empty
                    const_cast<SQLStatusImpl*>(this)->super::messages( std::vector<std::string>() );
                    string tmp;
                    LoggedStatement lst( "status" );
                    lst << "SELECT message FROM messages WHERE status_name = :name ORDER BY id";
                    lst.use(this->super::name()).into(tmp).execute();

                    // cast away constness since we are lazy loading here.
                    SQLStatusImpl * self = const_cast<SQLStatusImpl*>(this);

                    // super::add_message updates mtime.
                    // cache mtime and update the status object
                    time_t mtime = self->super::mtime();

                    while( lst.fetch() ) {
                        self->super::add_message(tmp);
                    }

                    // update with cached mtime
                    self->super::mtime(mtime);

                    self->messages_loaded = true;
                    break;
                }
                catch( const std::exception & err ) {
                    _WARN("Failed to load status messages: " << err.what());
                    sleep(1);
                }
            }
        }
        return this->super::messages();
    }

    void SQLStatusImpl::add_message( const string & message ) {
        if( this->messages_loaded ) {
            // load messages if not loaded yet
            this->messages();
        }
        this->super::add_message(message);
        while( true ) {
            try {
                LoggedStatement lst( "status" );
                lst <<
                    "INSERT INTO messages (status_name, message) " <<
                    "VALUES (:name, :message) ";
                lst.use(this->super::name()).use(message).execute(true);
                
                lst << "UPDATE status SET mtime = :mtime WHERE name = :name";
                lst.use(this->super::mtime()).use(this->super::name()).execute(true);

                return;
            }
            catch( const std::exception & err ) {
                _WARN("Failed to insert status message: " << err.what());
                sleep(1);
            }
        }
    }

    const Json & SQLStatusImpl::meta() const {
        if( ! this->meta_loaded ) {
            while( true ) {
                try {
                    string name, value_str;
                    LoggedStatement lst( "status" );
                    lst << "SELECT name, value FROM status_meta WHERE status_name = :name";
                    lst.use(this->super::name()).into(name).into(value_str).execute();

                    // cast away constness for lazy loading
                    SQLStatusImpl * self = const_cast<SQLStatusImpl*>(this);

                    // super::meta( name, value) updates mtime.
                    // cache mtime and update the status object
                    time_t mtime = self->super::mtime();

                    while( lst.fetch() ) {
                        Json value;
                        value.parse( value_str );
                        self->super::meta( name, value );
                    }

                    // update with cached mtime
                    self->super::mtime(mtime);

                    self->meta_loaded = true;
                    break;
                }
                catch( const std::exception & err ) {
                    _WARN("Failed to load status meta: " << err.what());
                    sleep(1);
                }
            }
        }
        return this->super::meta();
    }

    const std::vector<std::string> & SQLStatusImpl::children() const {
        // if children are not loaded, then we need to load them first
        if( ! this->children_loaded ) {
            while( true ) {
                try {
                    // make sure children are empty
                    const_cast<SQLStatusImpl*>(this)->super::children( std::vector<std::string>() );
                    string tmp;
                    LoggedStatement lst( "status" );
                    lst << "SELECT uri FROM child_uri WHERE status_name = :name ORDER BY id";
                    lst.use(this->super::name()).into(tmp).execute();

                    // cast away constness for lazy loading
                    SQLStatusImpl * self = const_cast<SQLStatusImpl*>(this);

                    // super::add_child updates mtime.
                    // cache mtime and update the status object
                    time_t mtime = self->super::mtime();

                    while( lst.fetch() ) {
                        self->super::add_child( tmp );
                    }

                    // update with cached mtime
                    self->super::mtime(mtime);

                    self->children_loaded = true;
                    break;
                }
                catch( const std::exception & err ) {
                    _WARN("Failed to load status children: " << err.what());
                    sleep(1);
                }
            }
        }
        return this->super::children();
    }

    void SQLStatusImpl::add_child(const string & child_uri) {
        if( ! this->children_loaded ) {
            // load children if not loaded
            this->children();
        }
        this->super::add_child(child_uri);
        while( true ) {
            try {
                Uri tmp(child_uri);
                string child_path = tmp.path();
                string child_id = tmp.leaf();
                LoggedStatement lst( "status" );
                lst << 
                    "INSERT INTO child_uri (status_name, uri, child_id) " <<
                    "VALUES (:status_name, :uri, :child_id) ";
                lst.use(this->super::name()).use(child_uri).use(child_id).execute(true);

                lst << "UPDATE status SET mtime = :mtime WHERE name = :name";
                lst.use(this->super::mtime()).use(this->super::name()).execute(true);

                return;
            }
            catch( const std::exception & err ) {
                _WARN("Failed to save status child: " << err.what());
                sleep(1);
            }
        }
    }

    void SQLStatusImpl::parent_uri(const string & parent) {
        this->super::parent_uri(parent);
        while( true ) {
            try {
                LoggedStatement lst( "status" );
                lst <<
                    "UPDATE status SET parent_uri=:parent_uri, mtime=:mtime " <<
                    "WHERE name = :name ";
                lst.use(this->super::parent_uri()).use(this->super::mtime()).use(this->super::name()).execute(true);
                return;
            }
            catch( const std::exception & err ) {
                _WARN("Failed to update status parent_uri: " << err.what());
                sleep(1);
            }
        }
    }

    void SQLStatusImpl::meta( const std::string & name, const Json & value ) {
        this->super::meta( name, value );
        while( true ) {
            try {
                LoggedStatement lst( "status" );
                lst <<
                    "REPLACE INTO status_meta (status_name, name, value) " <<
                    "VALUES(:status_name, :name, :value) ";
                lst.use(this->super::name()).use(name).use(value.serialize()).execute(true);

                lst << "UPDATE status SET mtime = :mtime WHERE name = :name";
                lst.use(this->super::mtime()).use(this->super::name()).execute(true);
                return;
            }
            catch( const std::exception & err ) {
                _WARN("Failed to update status meta: " << err.what());
                sleep(1);
            }
        }
    }
    void SQLStatusImpl::meta( const Json & meta ) {
        while( true ) {
            try {
                LoggedStatement lst( "status" );
                lst <<
                    "DELETE FROM status_meta WHERE status_name = :status_name";
                lst.use(this->super::name()).execute(true);

                lst << "UPDATE status SET mtime = :mtime WHERE name = :name";
                lst.use(this->super::mtime()).use(this->super::name()).execute(true);
                break;
            }
            catch( const std::exception & err ) {
                _WARN("Failed to update status meta: " << err.what());
                sleep(1);
            }
        }

        Json::Object & o = meta.as<Json::Object>();
        Json::Object::iterator it = o.begin();
        for( ; it != o.end(); ++it ) {
            this->meta( it->first, *(it->second) );
        }
    }

    void SQLStatusImpl::progress(unsigned int progress) {
        this->super::progress(progress);
        while( true ) {
            try {
                LoggedStatement lst( "status" );
                lst <<
                    "UPDATE status SET progress=:progress, mtime=:mtime " <<
                    "WHERE name = :name AND progress < :progress";
                lst.use(this->super::progress(),"progress").use(this->super::mtime(),"mtime").use(this->super::name(),"name").execute(true);
                return;
            }
            catch( const std::exception & err ) {
                _WARN("Failed to update status progress: " << err.what());
                sleep(1);
            }
        }
    }

    void SQLStatusImpl::code(int code) {
        this->super::code(code);
        while( true ) {
            try {
                LoggedStatement lst( "status" );
                lst <<
                    "UPDATE status SET code=:code, mtime=:mtime " <<
                    "WHERE name = :name ";
                lst.use(this->super::code()).use(this->super::mtime()).use(this->super::name()).execute(true);
                return;
            }
            catch( const std::exception & err ) {
                _WARN("Failed to update status code: " << err.what());
                sleep(1);
            }
        }
    }

    void SQLStatusImpl::ytime( int64_t ytime ) {
        this->super::ytime( ytime );
        while( true ) {
            try {
                LoggedStatement lst( "status" );
                // update ytime unless already in a final ytime
                lst <<
                    "UPDATE status SET ytime=:ytime, mtime=:mtime " <<
                    "WHERE name = :name ";
                lst.use(this->super::ytime()).use(this->super::mtime()).use(this->super::name()).execute(true);
                return;
            }
            catch( const std::exception & err ) {
                _WARN("Failed to update status ytime: " << err.what());
                sleep(1);
            }
        }
    }

    bool SQLStatusImpl::state( Status::State state ) {
        if ( this->should_load && !this->loaded ) this->really_load();

        if( !this->super::state( state ) ) return false;

        // state updates are critical, so
        // we keep trying until there are not exceptions
        // which should be pretty rare anyway.
        while( true ) {
            try {
                session & sess = getconn( "status" );
                LoggedStatement lst(sess);
                dblock("status");

                std::string query_suffix;
                if( sess.get_backend_name() == "mysql" ) {
                    query_suffix = " FOR UPDATE";
                }

                // for mysql we SELECT .. FOR UPDATE to lock the row, with sqlite
                // the dblock() above will do an exclusive database lock
                std::string current_state;
                lst << "SELECT state FROM status WHERE name = :name" << query_suffix;
                lst.use(this->super::name()).into(current_state).execute(true);

                if( ! Status::validStateTransition(Status::str2state(current_state), state) ) {
                    dbrollback("status");
                    return false;
                }

                // update state unless already in a final state
                lst <<
                    "UPDATE status SET state=:state, mtime=:mtime " <<
                    "WHERE name = :name";
                lst.use(Status::state2str(this->super::state())).use(this->super::mtime()).use(this->super::name()).execute(true);

                dbcommit("status");
                return true;
            }
            catch( const std::exception & err ) {
                _WARN(err.what());
                dbrollback("status");
                sleep(1);
            }
        }
    }

    void SQLStatusImpl::failures( uint32_t count )
    {
        this->super::failures( count );
        while( true ) {
            try {
                LoggedStatement lst( "status" );
                // update failures unless already in a final failures
                lst <<
                    "UPDATE status SET failures=:failures, mtime=:mtime " <<
                    "WHERE name = :name";
                lst.use(this->super::failures()).use(this->super::mtime()).use(this->super::name()).execute(true);
                return;
            }
            catch( const std::exception & err ) {
                _WARN("Failed to update status failures: " << err.what());
                sleep(1);
            }
        }
    }

    void SQLStatusImpl::sync() {
        this->super::state(Status::STATE_UNKNOWN);
        this->really_load();
        if( ! this->messages().empty() ) this->super::messages( std::vector<std::string>() );
        this->messages_loaded = false;
        if( ! this->children().empty() ) this->super::children( std::vector<std::string>() );
        this->children_loaded = false;
        if( ! this->meta().empty() ) this->super::meta( Json() );
        this->meta_loaded = false;
    }

    const std::string & SQLStatusImpl::operation() const {
        SQLStatusImpl * self = const_cast<SQLStatusImpl*>(this);
        if( !self->loaded ) self->really_load();
        return this->super::operation();
    }
    const std::string & SQLStatusImpl::component() const {
        SQLStatusImpl * self = const_cast<SQLStatusImpl*>(this);
        if( !self->loaded ) self->really_load();
        return this->super::component();
    }
    const std::string & SQLStatusImpl::resource_uri() const {
        SQLStatusImpl * self = const_cast<SQLStatusImpl*>(this);
        if( !self->loaded ) self->really_load();
        return this->super::resource_uri();
    }
    unsigned int SQLStatusImpl::progress() const {
        SQLStatusImpl * self = const_cast<SQLStatusImpl*>(this);
        if( !self->loaded ) self->really_load();
        return this->super::progress();
    }
    int SQLStatusImpl::code() const {
        SQLStatusImpl * self = const_cast<SQLStatusImpl*>(this);
        if( !self->loaded ) self->really_load();
        return this->super::code();
    }
    const std::string & SQLStatusImpl::parent_uri() const {
        SQLStatusImpl * self = const_cast<SQLStatusImpl*>(this);
        if( !self->loaded ) self->really_load();
        return this->super::parent_uri();
    }
    time_t SQLStatusImpl::ctime() const {
        SQLStatusImpl * self = const_cast<SQLStatusImpl*>(this);
        if( !self->loaded ) self->really_load();
        return this->super::ctime();
    }
    time_t SQLStatusImpl::mtime() const {
        SQLStatusImpl * self = const_cast<SQLStatusImpl*>(this);
        if( !self->loaded ) self->really_load();
        return this->super::mtime();
    }
    int64_t SQLStatusImpl::ytime() const {
        SQLStatusImpl * self = const_cast<SQLStatusImpl*>(this);
        if( !self->loaded ) self->really_load();
        return this->super::ytime();
    }
    Status::State SQLStatusImpl::state() const {
        SQLStatusImpl * self = const_cast<SQLStatusImpl*>(this);
        if( !self->loaded ) self->really_load();
        return this->super::state();
    }
    uint32_t SQLStatusImpl::failures() const {
        SQLStatusImpl * self = const_cast<SQLStatusImpl*>(this);
        if( !self->loaded ) self->really_load();
        return this->super::failures();
    }

    std::string SQLStatusImpl::serialize() const {
        SQLStatusImpl * self = const_cast<SQLStatusImpl*>(this);
        if( !self->loaded ) self->really_load();
        // force lazy loading of messages, children and meta data
        this->messages();
        this->children();
        this->meta();
        return this->super::serialize();
    }

    uint32_t SQLStatusImpl::concurrency() const {
        uint32_t count;
        // update concurrency unless already in a final concurrency
        LoggedStatement lst( "status" );
        lst <<
            "SELECT concurrency FROM status WHERE name = :name";
        lst.into(count).use(this->super::name()).execute(true);
        return count;
    }

    void SQLStatusImpl::starting() {
        while( true ) {
            try {
                LoggedStatement lst( "status" );
                // update concurrency unless already in a final concurrency
                Gearbox::Database::dblock("status");
                lst << "UPDATE status SET concurrency = concurrency + 1 WHERE name = :name";
                lst.use(this->super::name()).execute(true);
                Gearbox::Database::dbcommit("status");
                return;
            }
            catch( const std::exception & err ) {
                _WARN("Failed to update status concurrency: " << err.what());
                sleep(1);
            }
        }
    }

    void SQLStatusImpl::stopping() {
        while( true ) {
            try {
                LoggedStatement lst( "status" );
                // update concurrency unless already in a final concurrency
                Gearbox::Database::dblock("status");
                lst << "UPDATE status SET concurrency = concurrency - 1 WHERE name = :name";
                lst.use(this->super::name()).execute(true);
                Gearbox::Database::dbcommit("status");
                return;
            }
            catch( const std::exception & err ) {
                _WARN("Failed to update status concurrency: " << err.what());
                sleep(1);
            }
        }
    }

    SQLStatusCollectionImpl::SQLStatusCollectionImpl(const ConfigFile & c) :super(c), lst(getconn("status",true)), cursor(c), fetched(false), executed(false), count(10) {
        lst << "SELECT * FROM status WHERE 1=1 ";
    }

    // SQLStatusCollectionImpl::SQLStatusCollectionImpl(
    //     const SQLStatusCollectionImpl & other
    // ) : super(other),
    //     lst(other.lst),
    //     cursor(other.cursor),
    //     fetched(other.fetched),
    //     executed(other.executed),
    //     count(other.count) {}

    SQLStatusCollectionImpl::~SQLStatusCollectionImpl() {
    }

    void
    SQLStatusCollectionImpl::run_query() {
        if( ! this->executed ) {
            this->lst << " ORDER BY ctime DESC ";

            if (this->count) {
                this->lst << " LIMIT :count ";
                this->lst.use(this->count, "count");
            }
            
            this->lst.into(this->cursor);
            this->lst.execute();
            this->fetched = this->lst.fetch();
            this->executed = true;
        }
    }

    StatusImpl * SQLStatusCollectionImpl::pop() {
        if( ! this->executed ) this->run_query();
        if( ! this->fetched )
            gbTHROW( std::overflow_error("called pop() on empty status collection") );

        StatusImpl * status = new SQLStatusImpl(cursor);
        this->fetched = this->lst.fetch();
        return status;
    }

    bool SQLStatusCollectionImpl::empty() {
        if( ! this->executed ) this->run_query();
        return !this->fetched;
    }

    void SQLStatusCollectionImpl::filter_progress(unsigned int min, unsigned int max) {
        if( min == max ) {
            lst << "AND progress = :progress_min ";
            lst.use(min, "progress_min");
        } else {
            lst << "AND progress >= :progress_min AND progress <= :progress_max ";
            lst.use(min, "progress_min").use(max, "progress_max");
        }
    }

    void SQLStatusCollectionImpl::filter_code(int min, int max) {
        if( min == max ) {
            lst << "AND code = :code_min ";
            lst.use(min, "code_min");
        } else {
            lst << "AND code >= :code_min and code <= :code_max ";
            lst.use(min, "code_min").use(max, "code_max");
        }
    }

    void SQLStatusCollectionImpl::filter_operation(const std::string & op) {
        lst << "AND operation = :op ";
        lst.use(op, "op");
    }

    void SQLStatusCollectionImpl::filter_component(const std::string & c) {
        lst << "AND component = :component ";
        lst.use(c, "component");
    }


    void SQLStatusCollectionImpl::filter_mtime(time_t min, time_t max) {
        if( min == max ) {
            lst << "AND mtime = :mtime_min ";
            lst.use(min, "mtime_min");
        } else {
            lst << "AND mtime >= :mtime_min AND mtime <= :mtime_max ";
            lst.use(min, "mtime_min").use(max, "mtime_max");
        }
    }

    void SQLStatusCollectionImpl::filter_ctime(time_t min, time_t max) {
        if( min == max ) {
            lst << "AND ctime = :ctime_min ";
            lst.use(min, "ctime_min");
        } else {
            lst << "AND ctime >= :ctime_min AND ctime <= :ctime_max ";
            lst.use(min, "ctime_min").use(max, "ctime_max");
        }
    }

    void SQLStatusCollectionImpl::filter_state(const std::string & state) {
        lst << "AND state = :state ";
        lst.use(state, "state");
    }

    void SQLStatusCollectionImpl::filter_uri(const std::string & uri) {
        lst << "AND resource_uri LIKE :uri ";
        lst.use( "%" + uri + "%", "uri");
    }

    void SQLStatusCollectionImpl::limit(unsigned int count) {
        this->count = count;
    }
}
