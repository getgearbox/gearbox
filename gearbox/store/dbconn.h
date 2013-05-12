// -*- c++ -*-
#ifndef GEARBOX_DATABASE_H
#define GEARBOX_DATABASE_H

#include <string>
#include <list>
#include <sstream>
#include <soci/soci.h>
#include <gearbox/core/Json.h>
#include <gearbox/core/logger.h>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>

namespace Gearbox {
    void db_init(const char * configFile, const std::string & conn_name = "");
    void db_init(const std::string & configFile, const std::string & conn_name = "");
    void db_init(const Json & configData, const std::string & conn_name = "");

    namespace Database {
        class Connection {
        public:
            static Connection & reg(
                const std::string & name,
                const std::string & type = "mysql"
            );
            static Connection & get(const std::string & name);
            ~Connection();
            
            std::string connection_string() const;
            soci::session & get_session(int test=1);
            
            void set_dbname (const std::string & dbname);
            void set_user   (const std::string & user);
            void set_pass   (const std::string & pass);
            void set_host   (const std::string & host);
            void set_port   (const std::string & port);
            void set_port   (int port);
            void set_sock   (const std::string & sock);

        protected:
            Connection(
                const std::string & type
            );
            // left undefined to make private
            Connection(const Connection &);
            Connection & operator=(const Connection &);

            void _update_connection_string();

            class Private;
            Private * impl;
        };

        class LoggedStatement {
        public:
            LoggedStatement( const std::string & name );
            LoggedStatement( soci::session & s );
            ~LoggedStatement();
            LoggedStatement & operator<<( const std::string & sql );

            LoggedStatement &
            use(const char * d, const std::string & name = std::string() );

            template<typename T>
            LoggedStatement &
            use( const T & d, const std::string & name = std::string() ) {
                if( executed_ ) this->reset();
                std::string key(genKey(name));
                bind[key] = d;
                st_->exchange( soci::use( bind[key].as<typename Gearbox::TypeToJsonType<T>::RealType>(), name ) );
                return *this;
            }
            
            LoggedStatement &
            use_blob( const std::string & d, const std::string & name = std::string() );

            LoggedStatement &
            use( const char * d, soci::indicator & ind, const std::string & name = std::string() );

            template<typename T>
            LoggedStatement &
            use( const T & d, soci::indicator & ind, const std::string & name = std::string() ) {
                if( executed_ ) this->reset();
                std::string key(genKey(name));
                bind[key] = d;
                st_->exchange( soci::use( bind[key].as<typename Gearbox::TypeToJsonType<T>::RealType>(), ind, name ) );
                return *this;
            }     
            
            LoggedStatement &
            use_blob( const std::string & d, soci::indicator & ind, const std::string & name = std::string() );

            template<typename T>        
            LoggedStatement &
            into( T & d ) {
                if( executed_ ) this->reset();
                into_exchange_ = true;
                st_->exchange( soci::into( d ) );
                return *this;
            }
            
            template<typename T>        
            LoggedStatement &
            into( T & d, soci::indicator & ind ) {
                if( executed_ ) this->reset();
                into_exchange_ = true;
                st_->exchange( soci::into( d, ind ) );
                return *this;
            }

            LoggedStatement & prepare();
            LoggedStatement & prepare( const std::string & sql );
            LoggedStatement & execute( const std::string & sql );
            LoggedStatement & execute( const std::string & sql, bool withDataExchange );
            LoggedStatement & execute();
            LoggedStatement & execute( bool withDataExchange );
            bool fetch();
            void reset();
            bool got_data();
        private:
            LoggedStatement( const LoggedStatement & copy );
            std::string genKey(const std::string & name) {
                if( name.empty() )
                    return boost::lexical_cast<std::string>(bind.as<Json::Object>().size());
                return name;
            }
            soci::session & sess_;
            soci::statement * st_;
            std::ostringstream sql_;
            Json bind;
            bool prepared_;
            bool executed_;
            bool into_exchange_;
            std::list< boost::shared_ptr<soci::blob> > blobs_;
        };

        void dblock(const std::string & connname = "");
        void dbcommit( const std::string & connname = "");
        void dbrollback(const std::string & connname = "");

        std::string lastsql(const std::string & connname = "");

        soci::session & getconn(
            const std::string & connname
        );
        soci::session & getconn(
            const std::string & connname,
            bool auto_reconnect
        );
        
        soci::session & getconn(
            const char * connname
        );
        soci::session & getconn(
            const char * connname,
            bool auto_reconnect
        );

        soci::session & getconn();
        soci::session & getconn(bool auto_reconnect);
        
    }
}

// namespace soci {
//     namespace details {
//         template <>
//         struct exchange_traits<unsigned int>
//         {
//             typedef basic_type_tag type_family;
//             enum { x_type = x_unsigned_long };
//         };
// #ifndef __i386__
//         template <>
//         struct exchange_traits<int64_t>
//         {
//             typedef basic_type_tag type_family;
//             enum { x_type = x_long_long };
//         };
// #else
//         template <>
//         struct exchange_traits<time_t>
//         {
//             typedef basic_type_tag type_family;
//             enum { x_type = x_unsigned_long };
//         };
// #endif
//     }
// }



#endif
