// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/core/Json.h>
#include <gearbox/core/JsonGenerator.h>

#include <string>
using std::string;

namespace Gearbox {

string Json::Generator::generate(const Json & json, bool beautify) {

    yajl_gen_config conf = { beautify, "  " };
    yajl_gen g = yajl_gen_alloc(&conf, NULL);

    Json::Generator::generate(g, &json);

    const unsigned char * buf;  
    unsigned int len;  
    yajl_gen_get_buf(g, &buf, &len);  
    string tmp(reinterpret_cast<const char *>(buf), len);
    yajl_gen_free(g);  
    return tmp;
}

void Json::Generator::generate(yajl_gen g, const Json * j) {
    switch(j->impl_->type) {
    case Json::UNDEF:
        yajl_gen_null(g);
        break;
    case Json::BOOL:
        yajl_gen_bool(g, j->as<bool>());
        break;
    case Json::INT:
        yajl_gen_integer(g, j->as<int64_t>());
        break;
    case Json::DOUBLE:
        yajl_gen_double(g, j->as<double>());
        break;
    case Json::STRING:
        yajl_gen_string(
            g, 
            reinterpret_cast<const unsigned char *>(j->as<string>().c_str()), 
            j->as<string>().size()
        );
        break;
    case Json::OBJECT:
        Json::Generator::generateObject(g,j);
        break;
    case Json::ARRAY:
        Json::Generator::generateArray(g,j);
        break;
    }
}
        
void Json::Generator::generateObject(yajl_gen g, const Json * j) {
    yajl_gen_map_open(g);

    Json::Object & o = j->as<Json::Object>();
    Json::Object::iterator i = o.begin();
    Json::Object::iterator e = o.end();
    for( ; i != e; ++i ) {
        yajl_gen_string(g, reinterpret_cast<const unsigned char *>(i->first.c_str()), i->first.size());
        Json::Generator::generate(g, i->second.get());
    }
        
    yajl_gen_map_close(g);
}
        
void Json::Generator::generateArray(yajl_gen g, const Json * j) {
    yajl_gen_array_open(g);
    Json::Array & a = j->as<Json::Array>();
    for( unsigned int i=0; i< a.size(); ++i ) {
        Json::Generator::generate(g, a[i].get());
    }
    yajl_gen_array_close(g);
}

} // namespace
