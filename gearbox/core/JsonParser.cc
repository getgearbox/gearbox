// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/core/Json.h>
#include <gearbox/core/JsonParser.h>
#include <gearbox/core/JsonSchema.h>
#include <gearbox/core/Errors.h>

#include "JsonPrivate.h"

#include <boost/lexical_cast.hpp>

#include <string>
using std::string;

namespace {
    struct YajlHandleAutoPtr {
        YajlHandleAutoPtr(yajl_handle handle) : handle_(handle) {}
        ~YajlHandleAutoPtr() { if(handle_) yajl_free(handle_); }
        yajl_handle get() { return handle_; }
        yajl_handle handle_;
    };
}

namespace Gearbox {

void Json::Parser::parse(
    const string & content,
    Json & json,
    JsonSchema * schema
) {
    json.freeData();
    Json::Parser::Context ctx;
    ctx.cursor = &json;
    ctx.schema = schema;
    
    YajlHandleAutoPtr yhap(yajl_alloc(&Json::Parser::callbacks, NULL, &ctx));
    if( !yhap.get() ) {
        gbTHROW( JsonError(json.context(), "failed to create parser") );
    }
    yajl_config(yhap.get(), yajl_allow_comments, 1);
    yajl_status stat = yajl_parse(yhap.get(), reinterpret_cast<const unsigned char *>(content.c_str()), content.size()); 
    if( stat == yajl_status_ok ) {
        stat = yajl_complete_parse(yhap.get());
    }
    
    if( !ctx.exception.empty() ) {
        gbTHROW( JsonError(ctx.exception) );
    }

    if (stat != yajl_status_ok) {  
        unsigned char * str = yajl_get_error(yhap.get(), 1, reinterpret_cast<const unsigned char *>(content.c_str()), content.size());  
        string err(reinterpret_cast<const char *>(str));
        yajl_free_error(yhap.get(), str);
        _WARN("failed to parse content <<EOM\n" << content << "\nEOM");
        gbTHROW( JsonError(json.context(), "parse error: " + err) );
    }
}

Json * Json::Parser::getJson(void * ctx) {
    return static_cast< Json::Parser::Context *>(ctx)->cursor;
}

void Json::Parser::setJson(void * ctx, Json * j) {
    static_cast< Json::Parser::Context *>(ctx)->cursor = j;
}    

void Json::Parser::setJsonUp(void * ctx) {
    static_cast< Json::Parser::Context *>(ctx)->cursor
        = static_cast< Json::Parser::Context *>(ctx)->cursor->impl_->parent;
    setSchemaUp(ctx);
}

JsonSchema * Json::Parser::getSchema(void * ctx) {
    return static_cast< Json::Parser::Context *>(ctx)->schema;
}

void Json::Parser::setSchema(void * ctx, JsonSchema * s) {
    assert(s);
    static_cast< Json::Parser::Context *>(ctx)->schema = s;
}

void Json::Parser::setSchemaUp(void * ctx) {
    if( static_cast< Json::Parser::Context *>(ctx)->schema ) {
        static_cast< Json::Parser::Context *>(ctx)->schema = 
            static_cast<JsonSchema*>(static_cast< Json::Parser::Context *>(ctx)->schema->impl_->parent);
    }
}

Json * Json::Parser::getObj(void * ctx) {
    Json * cur = getJson(ctx);
    if( cur->impl_->type == Json::ARRAY ) {
        boost::shared_ptr<Json> j(new Json);
        j->impl_->parent = cur;
        cur->as<Json::Array>().push_back( j );
        JsonSchema * s = getSchema(ctx); 
        if( s ) {
            int newIdx = cur->as<Json::Array>().size();
            if( ! s->validMaxSize(*cur) ) return NULL;
            JsonSchema * child = s->getChildSchema(newIdx - 1, *cur);
            if( !child ) {
                return NULL;
            }
            setSchema(ctx, child);
        }        
        setJson(ctx, j.get());
    }
    return getJson(ctx);
}

int Json::Parser::parse_null(void * ctx) {
    try { 
        Json * cur = getObj(ctx);
        if( !cur ) return 0;
        assert(cur->impl_->type == Json::UNDEF);
        if( getSchema(ctx) ) {
            if( ! getSchema(ctx)->validType(*cur) ) return 0;
        }
        setJsonUp(ctx);
        return 1;
    }
    catch( const std::exception & err ) {
        static_cast< Json::Parser::Context *>(ctx)->exception= err.what();
        return 0;
    }
}

int Json::Parser::parse_boolean(void * ctx, int boolVal) {
    try {
        Json * cur = getObj(ctx);
        if( !cur ) return 0;
        assert(cur->impl_->type == Json::UNDEF);
        cur->setData(new bool(boolVal), Json::BOOL);
        JsonSchema * s = getSchema(ctx);
        if( s ) {
            bool cont = s->validType(*cur) 
                && s->validValue(static_cast<bool>(boolVal));
            if(!cont) return 0;
        }
        setJsonUp(ctx);
        return 1;
    }
    catch( const std::exception & err ) {
        static_cast< Json::Parser::Context *>(ctx)->exception= err.what();
        return 0;
    }
}

int Json::Parser::parse_number(void *ctx, const char *numberVal, size_t numberLen) {
    try {
        Json * cur = getObj(ctx);
        if(!cur) return 0;
        string strnum(numberVal, numberLen);
        if( strnum.find('.') == string::npos ) {
            // no dot, so assume it is integer with optional exponent
            cur->setData( new int64_t( boost::lexical_cast<int64_t>(strnum) ), Json::INT );
        }
        else {
            // has dot so assume double
            cur->setData( new double( boost::lexical_cast<double>(strnum) ), Json::DOUBLE );
        }
        JsonSchema * s = getSchema(ctx);
        if( s ) {
            bool cont = s->validType(*cur)
                && s->validValue(*cur);
            if( !cont ) {
                return 0;
            } else { 
                s->typeFixup( *cur );
            }
        }
        setJsonUp(ctx);
        return 1;
    }
    catch( const std::exception & err ) {
        static_cast< Json::Parser::Context *>(ctx)->exception= err.what();
        return 0;
    }
}

int Json::Parser::parse_string(void * ctx, const unsigned char * stringVal,
                        size_t stringLen) {
    try {
        Json * cur = getObj(ctx);
        if( !cur ) return 0;
        assert(cur->impl_->type == Json::UNDEF);
        std::auto_ptr<std::string> value(new string(reinterpret_cast<const char*>(stringVal), stringLen));
        cur->setData(value.release(), Json::STRING);
        JsonSchema * s = getSchema(ctx);
        if( s ) {
            bool cont = s->validType(*cur)
                && s->validValue(*cur);
            if(!cont) return 0;
        }
        setJsonUp(ctx);
        return 1;
    }
    catch( const std::exception & err ) {
        static_cast< Json::Parser::Context *>(ctx)->exception= err.what();
        return 0;
    }
}

int Json::Parser::parse_start_map(void * ctx) {
    try {
        Json * cur = getObj(ctx);
        if( !cur ) return 0;
        assert(cur->impl_->type == Json::UNDEF);
        cur->setData(new Json::Object(), Json::OBJECT);
        if( getSchema(ctx) ) {
            if( !getSchema(ctx)->validType(*cur) ) return 0;
        }
        setJson(ctx, cur);
        return 1;
    }
    catch( const std::exception & err ) {
        static_cast< Json::Parser::Context *>(ctx)->exception = err.what();
        return 0;
    }
}

int Json::Parser::parse_map_key(void * ctx, const unsigned char * key,
                         size_t stringLen) {
    try {
        assert(getJson(ctx)->impl_->type == Json::OBJECT);
        string name(reinterpret_cast<const char*>(key), stringLen);
        JsonSchema * s = getSchema(ctx); 
        if( s ) {
            JsonSchema * child = s->getChildSchema(name, *(getJson(ctx)));
            if( !child ) {
                return 0;
            }
            setSchema(ctx,child);
        }        
        boost::shared_ptr<Json> j(new Json);
        Json::Object &map = getJson(ctx)->as<Json::Object>();
        if ( map.find( name ) != map.end() ) { 
            gbTHROW( std::runtime_error("duplicate key name found: " + name) );
        }
        map.insert ( Json::Object::value_type(name, j) );
        j->impl_->parent = getJson(ctx);
        setJson(ctx, j.get());
        return 1;
    }
    catch( const std::exception & err ) {
        static_cast< Json::Parser::Context *>(ctx)->exception = err.what();
        return 0;
    }
}

int Json::Parser::parse_end_map(void * ctx) {
    try {
        Json * cur = getJson(ctx);
        assert(cur->impl_->type == Json::OBJECT);
        
        JsonSchema * s = getSchema(ctx);
        if( s ) {
            bool cont = s->setDefaults(*cur) 
                && s->validRequires(*cur);
            if(!cont) return 0;
        }
        
        setJsonUp(ctx);
        return 1;
    }
    catch( const std::exception & err ) {
        static_cast< Json::Parser::Context *>(ctx)->exception = err.what();
        return 0;
    }
}

int Json::Parser::parse_start_array(void * ctx) {
    try {
        Json * cur = getObj(ctx);
        if( !cur ) return 0;
        assert(cur->impl_->type == Json::UNDEF);
        cur->setData(new Json::Array(), Json::ARRAY);
        // check is array is allowed
        if( getSchema(ctx) ) {
            if( ! getSchema(ctx)->validType(*cur) ) return 0;
        }
        setJson(ctx, cur);
        return 1;
    }
    catch( const std::exception & err ) {
        static_cast< Json::Parser::Context *>(ctx)->exception = err.what();
        return 0;
    }
}

int Json::Parser::parse_end_array(void * ctx) {
    try {
        Json * cur = getJson(ctx);
        assert(cur->impl_->type == Json::ARRAY);
        
        JsonSchema * s = getSchema(ctx);
        if( s ) {
            // check min  array length
            if( ! s->validMinSize( *cur ) ) return 0;
        }
        
        setJsonUp(ctx);
        return 1;
    }
    catch( const std::exception & err ) {
        static_cast< Json::Parser::Context *>(ctx)->exception = err.what();
        return 0;
    }
}
    
// yajl_parser_config Json::Parser::cfg = { 1, 1 };
yajl_callbacks     Json::Parser::callbacks = {  
    Json::Parser::parse_null,  
    Json::Parser::parse_boolean,  
    NULL,
    NULL,
    Json::Parser::parse_number,  
    Json::Parser::parse_string,  
    Json::Parser::parse_start_map,  
    Json::Parser::parse_map_key,  
    Json::Parser::parse_end_map,  
    Json::Parser::parse_start_array,  
    Json::Parser::parse_end_array  
};

} // namespace
