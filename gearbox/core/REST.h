// -*- c++ -*-
#ifndef GEARBOX_REST_H
#define GEARBOX_REST_H

#include <gearbox/core/Json.h>

namespace Gearbox {

typedef std::map<std::string,std::string> Headers;

long HEAD   (const std::string & url);
long GET    (const std::string & url, Json & response);
long DELETE (const std::string & url, Json & response);
long POST   (const std::string & url, const Json & data, Json & response);
long PUT    (const std::string & url, const Json & data, Json & response);

long HEAD   (const std::string & url, const Headers & headers);
long GET    (const std::string & url, const Headers & headers, Json & response);
long DELETE (const std::string & url, const Headers & headers, Json & response);
long POST   (const std::string & url, const Json & data, const Headers & headers, Json & response);
long PUT    (const std::string & url, const Json & data, const Headers & headers, Json & response);

class REST {
public:    
    static void global_headers( const Headers & headers );
    static void add_global_header( const std::string & key, const std::string & value );
    static void del_global_header( const std::string & key );
private:
    REST();
    ~REST();
};

} // namespace
#endif // GEARBOX_REST_H
