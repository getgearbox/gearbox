// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/store/dbconn.h>

#define LOGCAT "gearbox.cc.database"
#include <gearbox/core/logger.h>
#include <iomanip>
#include <sys/time.h>

using namespace std;

namespace Gearbox {
    namespace Database {
        
        LoggedStatement::LoggedStatement( const std::string & name )
            : sess_(getconn(name)), st_(new soci::statement(sess_)), prepared_(false), executed_(false), into_exchange_(false)
        {
            bind.parse("{}");
        }

        LoggedStatement::LoggedStatement( soci::session & s )
            : sess_(s), st_(new soci::statement(s)), prepared_(false), executed_(false), into_exchange_(false)
        {
            bind.parse("{}");
        }

        LoggedStatement::~LoggedStatement()
        {
            if(st_) delete st_;
        }
        
        LoggedStatement &
        LoggedStatement::operator<<( const string & sql )
        {
            if( executed_ ) this->reset();
            sql_  << std::endl << sql;
            return *this;
        }

        LoggedStatement &
        LoggedStatement::use(const char * d, const std::string & name ) {
            if( executed_ ) this->reset();
            std::string key(genKey(name));
            bind[key] = d;
            st_->exchange( soci::use( bind[key].as<std::string>(), name ) );
            return *this;
        }

        LoggedStatement &
        LoggedStatement::use_blob( const std::string & d, const std::string & name) {
            if( executed_ ) this->reset();
            std::string key(genKey(name));
            bind[key] = d;
            blobs_.push_back(boost::shared_ptr<soci::blob>(new soci::blob(sess_)));
            blobs_.back()->append(d.data(),d.size());
            st_->exchange( soci::use( *(blobs_.back()), name ) );
            return *this;
        }     
        
        LoggedStatement &
        LoggedStatement::use( const char * d, soci::indicator & ind, const std::string & name ) {
            if( executed_ ) this->reset();
            std::string key(genKey(name));
            bind[key] = d;
            st_->exchange( soci::use( bind[key].as<std::string>(), ind, name ) );
            return *this;
        }     

        LoggedStatement &
        LoggedStatement::use_blob( const std::string & d, soci::indicator & ind, const std::string & name) {
            if( executed_ ) this->reset();
            std::string key(genKey(name));
            bind[key] = d;
            blobs_.push_back(boost::shared_ptr<soci::blob>(new soci::blob(sess_)));
            blobs_.back()->append(d.data(),d.size());
            st_->exchange( soci::use( *(blobs_.back()), ind, name ) );
            return *this;
        }     

        LoggedStatement &
        LoggedStatement::prepare()
        {
            return this->prepare(sql_.str());
        }
        
        LoggedStatement &
        LoggedStatement::prepare(const string & sql) {
            if( executed_ ) this->reset();
            _TRACE( "SQL: " << sql );
            st_->alloc();
            st_->prepare(sql);
            prepared_=true;
            return *this;
        }

        LoggedStatement &
        LoggedStatement::execute( const std::string & sql ) {
            return this->execute(sql, !into_exchange_);
        }

        LoggedStatement &
        LoggedStatement::execute( const std::string & sql, bool withDataExchange )
        {
            if( !prepared_ )
                this->prepare(sql);

            if( !bind.empty()) _TRACE( "BIND: " << bind.serialize() );
            st_->define_and_bind();
            timeval start, end;
            gettimeofday(&start, NULL);
            executed_ = true;
            st_->execute( withDataExchange );
            gettimeofday(&end,NULL);
            double lapse = end.tv_sec - start.tv_sec;
            lapse += (end.tv_usec - start.tv_usec)/1000000.0;
            if( lapse > 1 ) {
                _WARN("SQL: " << sql);
                if( !bind.empty() ) _WARN("BIND: " << bind.serialize());
                _WARN("Slow Query.  Lapse: " << std::fixed << std::setprecision(5) << lapse);
            }
            return *this;
        }

        LoggedStatement &
        LoggedStatement::execute() {
            return this->execute( sql_.str(), !into_exchange_ );
        }

        LoggedStatement &
        LoggedStatement::execute( bool withDataExchange ) {
            return this->execute( sql_.str(), withDataExchange );
        }

        bool
        LoggedStatement::fetch() {
            return st_->fetch();
        }
            
        void
        LoggedStatement::reset()
        {
            sql_.clear();
            sql_.str(string());
            blobs_.clear();
            prepared_ = false;
            executed_ = false;
            into_exchange_ = false;
            bind.parse("{}");
            if (st_) delete st_;
            st_ = new soci::statement(sess_);
        }
        
        bool
        LoggedStatement::got_data() {
            return sess_.got_data();
        }
    }
}
