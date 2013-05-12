// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/store/dbconn.h>

#define LOGCAT "gearbox.cc.database"
#include <gearbox/core/logger.h>
#include <gearbox/core/Errors.h>
#include <gearbox/core/ConfigFile.h>

#include <soci/soci.h>

#include <string>
#include <sstream>
#include <stdexcept>

#include <boost/lexical_cast.hpp>

namespace Gearbox {
    void db_init(const char * configFile, const std::string & conn_name ) {
        if( !configFile )
            gbTHROW( std::invalid_argument("passed NULL as configFile to db_init") );
        db_init( std::string(configFile), conn_name);
    }

    void db_init(const std::string & configFile, const std::string & conn_name ) {
        ConfigFile cfg(configFile);

        std::string cn(conn_name);
        std::string db_type = cfg.get_string_default("db_type", "mysql");
        std::string db_user = cfg.get_string_default("db_user", "");        
        std::string db_pass = cfg.get_string_default("db_pass", "");
        std::string db_name = cfg.get_string_default("db_name", "");
        std::string db_host = cfg.get_string_default("db_host", "");
        std::string db_sock = cfg.get_string_default("db_sock", "");
        int db_port = cfg.get_int_default("db_port", 0);
        
        if ( cn.empty() ) {
            std::ostringstream oss;
            oss <<  db_user << ":" << db_type << ":" << db_name << ":"
                << db_host << ":" << db_sock << ":" << db_port;
            cn = oss.str();
        }
        
        Gearbox::Database::Connection & conn =
            Gearbox::Database::Connection::reg(cn, db_type);

        conn.set_user(db_user);
        conn.set_pass(db_pass);
        conn.set_dbname(db_name);    
        conn.set_host(db_host);
        conn.set_sock(db_sock);
        conn.set_port(db_port);
    }

    void db_init(const Json & configData, const std::string & conn_name ) {
        std::string cn(conn_name);
        
        std::string db_type("mysql");
        if( configData.hasKey("db_type") )
            db_type = configData["db_type"].as<std::string>();

        std::string db_user, db_pass, db_name, db_host, db_sock;
        int db_port = 0;

        if( configData.hasKey("db_user") )
            db_user = configData["db_user"].as<std::string>();
        if( configData.hasKey("db_pass") )
            db_pass = configData["db_pass"].as<std::string>();
        if( configData.hasKey("db_name") )
            db_name = configData["db_name"].as<std::string>();
        if( configData.hasKey("db_host") )
            db_host = configData["db_host"].as<std::string>();
        if( configData.hasKey("db_sock") )
            db_sock = configData["db_sock"].as<std::string>();

        if( configData.hasKey("db_port") )
            db_port = configData["db_port"].as<int>();

        if ( cn.empty() ) {
            std::ostringstream oss;
            oss <<  db_user << ":" << db_type << ":" << db_name << ":"
                << db_host << ":" << db_sock << ":" << db_port;
            cn = oss.str();
        }
        
        Gearbox::Database::Connection & conn =
            Gearbox::Database::Connection::reg(cn, db_type);

        conn.set_user(db_user);
        conn.set_pass(db_pass);
        conn.set_dbname(db_name);    
        conn.set_host(db_host);
        conn.set_sock(db_sock);
        conn.set_port(db_port);
    }

    namespace Database {
        typedef std::map<std::string, boost::shared_ptr<Connection> > ConnMap;
        static ConnMap s_conns;
        static std::string s_last_connname;

        struct Connection::Private {
            std::string dbname;
            std::string user;
            std::string pass;
            std::string host;
            std::string port;
            std::string sock;
            soci::session * sess;
            std::string type;
            Private(const std::string & type)
                : dbname("test"),
                  user("root"),
                  pass(""),
                  host("localhost"),
                  port(""),
                  sock(""),
                  sess(NULL),
                  type(type) {}
        };

        Connection::Connection(const std::string & type) : impl(new Private(type)) {
        };

        Connection::~Connection() {
            if( this->impl->sess ) delete this->impl->sess;
            delete impl;
        }
        
        Connection & Connection::reg( const std::string & name, const std::string & type) {
            ConnMap::iterator it = s_conns.find(name);
            if( it != s_conns.end() ) return *(it->second);
            boost::shared_ptr<Connection> c(new Connection(type));
            s_last_connname = name;
            s_conns[name] = c;
            return *c;
        }

        Connection & Connection::get( const std::string & name ) {
            ConnMap::iterator it = s_conns.find(name);
            if( it == s_conns.end() ) {
                std::ostringstream oss;
                oss << "Database connection " << name << " has not been registered!";
                gbTHROW( std::runtime_error(oss.str()) );
            }
            return *s_conns[name];
        }
        
        void Connection::set_dbname(const std::string & dbname) {
            this->impl->dbname = dbname;
        }

        void Connection::set_user(const std::string & user) {
            this->impl->user = user;
        }

        void Connection::set_pass(const std::string & pass) {
            this->impl->pass = pass;
        }

        void Connection::set_host(const std::string & host) {
            this->impl->host = host;
        }

        void Connection::set_port(const std::string & port) {
            this->impl->port = port;
        }

        void Connection::set_port(int port) {
            if( port <= 0 ) return;
            std::stringstream converter;
            converter << port;
            converter >> this->impl->port;
        }
        
        void Connection::set_sock(const std::string & sock) { 
            this->impl->sock = sock;
        }

        std::string Connection::connection_string() const {
            std::ostringstream conn;
            if( this->impl->type == "sqlite3" ) {
                conn << "dbname=" << this->impl->dbname << " timeout=" << 10;
            }
            else {
                conn << "dbname=" << this->impl->dbname << " user=" << this->impl->user;
                if( this->impl->pass.size() )
                    conn << " password=" << this->impl->pass;
                if( this->impl->host.size() )
                    conn << " host=" << this->impl->host;
                if( this->impl->port.size() )
                    conn << " port=" << this->impl->port;
                if( this->impl->sock.size() )
                    conn << " unix_socket=" << this->impl->sock;
            }
            // hide the password for our debug statement
            std::string connsafe = conn.str();
            if( this->impl->pass.size() ) { 
                size_t loc = connsafe.find( this->impl->pass );
                if ( loc != std::string::npos ) { 
                    connsafe.replace( loc, this->impl->pass.size(), "<hidden>" );
                }
            }
            _DEBUG("Using DB connection string: " << connsafe);
            return conn.str();;
        }

        soci::session & Connection::get_session(int test) {
            if( this->impl->sess ) {
                if(!test) return *this->impl->sess;
                // there must be a better way than this
                try {
                    (*this->impl->sess) << "SELECT 1";
                    return *this->impl->sess;
                }
                catch ( const std::exception & err ) {
                    _INFO("Database connection went away: " << err.what());
                    delete this->impl->sess;
                    this->impl->sess = NULL;
                    throw;
                }
            }
                
            this->impl->sess = new soci::session(this->impl->type, this->connection_string());
            return *this->impl->sess;
        }

        void dblock(const std::string & connname) { 
            soci::session & sql = connname.empty() ? getconn() : getconn(connname);
            if( sql.get_backend_name() == "sqlite3" ) {
                _TRACE("GETTING EXCLUSIVE LOCK!");
                sql << "BEGIN EXCLUSIVE";
            }
            else {
                _TRACE("Starting Transaction");
                sql.begin();
            }
        }

        void dbcommit(const std::string & connname) {
            soci::session & sql = connname.empty() ? getconn() : getconn(connname);
            sql.commit();
        }

        void dbrollback(const std::string & connname) {
            try {
                soci::session & sql = connname.empty() ? getconn() : getconn(connname);
                sql.rollback();
            }
            catch ( const std::exception & err ) {
                _ERROR("Failed to rollback: " << err.what());
            }
        }
            
        std::string lastsql(const std::string & connname) {
            try {
                return Connection::get(connname.empty() ? s_last_connname : connname).get_session(0).get_last_query();
            }
            catch ( const std::exception & err ) {
                return "";
            }
        }            

        soci::session & getconn (
            const std::string & connname,
            bool auto_reconnect
        ) {
            while( true ) {
                try {
                    s_last_connname = connname;
                    return Connection::get(connname).get_session();
                }
                catch( const std::exception & err ) {
                    if( auto_reconnect ) {
                        _WARN("failed to get database connection: " << err.what());
                        continue;
                    }
                    else {
                        _ERROR("failed to get database connection: " << err.what());
                        throw;
                    }
                }
            }
        }

        soci::session & getconn (
            const char * connname,
            bool auto_reconnect
        ) {
            return getconn(std::string(connname),auto_reconnect);
        }

        soci::session & getconn (bool auto_reconnect) {
            while( true ) {
                try {
                    return Connection::get(s_last_connname).get_session();
                }
                catch( const std::exception & err ) {
                    if( auto_reconnect ) {
                        _WARN("failed to get database connection: " << err.what());
                        continue;
                    }
                    else {
                        _ERROR("failed to get database connection: " << err.what());
                        throw;
                    }
                }
            }
        }

        soci::session & getconn (const std::string & connname) {
            s_last_connname = connname;
            return Connection::get(connname).get_session();
        }

        soci::session & getconn (const char * connname) {
            return getconn(std::string(connname));
        }

        soci::session & getconn () {
            return Connection::get(s_last_connname).get_session();
        }
    }
}
