// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <string>
#include <sstream>
#include <stdexcept>
#include <iostream>

#include <gearbox/core/strlcpy.h>
#include <gearbox/core/Json.h>
#include <gearbox/core/JsonParser.h>
#include <gearbox/core/JsonGenerator.h>
#include <gearbox/core/util.h>
#include <gearbox/core/Errors.h>

#include "JsonPrivate.h"

using std::string;

namespace Gearbox {

typedef Json::Object JObject;
typedef Json::Array JArray;

JsonError::JsonError(const string ctx, const string msg) {
    std::ostringstream err;
    err << "Json Exception: " << msg;
    if( ctx.size() ) {
        err << " at: " << ctx;
    }
    strlcpy(buffer, err.str().c_str(), sizeof(buffer));
}
JsonError::JsonError(const string msg) {
    strlcpy(buffer, msg.c_str(), sizeof(buffer));
}

JsonError::~JsonError() throw () {}
const char * JsonError::what() const throw () { 
    return buffer;
}

void Json::freeData() {
    if(this->data_) {
        switch(this->impl_->type) {
        case BOOL:    delete &(this->as<bool>()); break;
        case INT:     delete &(this->as<int64_t>()); break;
        case DOUBLE:  delete &(this->as<double>()); break;
        case STRING:  delete &(this->as<string>()); break;
        case OBJECT:  delete &(this->as<JObject>()); break;
        case ARRAY:   delete &(this->as<JArray>()); break;
        default: break;
        }
        data_ = NULL;
       this->impl_->type = Json::UNDEF;
    }
}

Json::Json()
    : impl_(new Private()), data_(NULL) {}

Json::Json(bool value)
    :impl_(new Private(Json::BOOL)),
     data_(new bool(value)) {}

Json::Json(int value)
    :impl_(new Private(Json::INT)),
     data_(new int64_t(value)) {}

Json::Json(int64_t value)
    :impl_(new Private(Json::INT)),
     data_(new int64_t(value)) {}

Json::Json(double value)
    :impl_(new Private(Json::DOUBLE)),
     data_(new double(value)) {}
    
Json::Json(const char * value)
    : impl_(new Private(Json::STRING)), 
      data_(new string(value)) {}
    
Json::Json(const std::string & value)
    : impl_(new Private(Json::STRING)), 
      data_(new string(value)) {}

Json::Json(const Object & value)
    : impl_(new Private(Json::OBJECT)),
      data_(new Object(value)) {}

Json::Json(const Array & value)
    : impl_(new Private(Json::ARRAY)),
      data_(new Array(value)) {};
        
bool Json::equals( const JObject & other ) const { 
    if ( this->as<JObject>().size() != other.size() ) return false;

    JObject::const_iterator itr = other.begin();
    for( ; itr != other.end(); ++itr ) {
        string key = itr->first;
        if ( !this->hasKey( key ) ) return false;
        if ( !this->get( key ).equals( *(itr->second) ) ) return false;
    }

    return true;
}

bool Json::equals(const JArray & a) const {
    if( this->impl_->type != Json::ARRAY ) return false;

    const JArray & b = this->as<JArray>();
    if ( b.size() != a.size() ) return false;
    for( unsigned int i=0; i < a.size(); i++ ) {
        if ( !a[i]->equals( *b[i] ) ) return false;
    }

    return true;
}

bool Json::equals( const Json & other ) const { 
    if ( other.empty() && this->empty() ) return true;
    if ( other.empty() || this->empty() ) return false;
    if ( this->impl_->type != other.impl_->type ) return false;

    switch(other.impl_->type) {
    case OBJECT:
        if ( !this->equals( other.as<JObject>() ) ) return false;
        break;
    case ARRAY:
        if ( !this->equals( other.as<JArray>() ) ) return false;
        break;
    case BOOL:   
        if ( this->as<bool>() != other.as<bool>() ) return false;
        break;
    case INT:  
        if ( this->as<int64_t>() != other.as<int64_t>() ) return false;
        break;
    case DOUBLE: 
        if ( this->as<double>() != other.as<double>() ) return false;
        break;
    case STRING: 
        if ( this->as<string>() != other.as<string>() ) return false;
        break;
    case UNDEF: 
        if ( this->impl_->type != UNDEF ) return false;
    }

    return true;
}

bool Json::operator==(const Json &other) const {
    return this->equals( other );
}

bool Json::operator!=(const Json &other) const {
    return !(*this == other);
}

void Json::copy(const Json & other) {
    // dont copy self
    if( this == &other ) return;
    freeData();
    this->impl_->parent = NULL;
    this->impl_->errHandler = other.impl_->errHandler;
    if(other.data_) {
        switch(other.impl_->type) {
        case UNDEF: break;
        case BOOL:
            data_ = new bool(other.as<bool>());
           this->impl_->type = Json::BOOL;
            break;
        case INT:
            data_ = new int64_t(other.as<int64_t>());
           this->impl_->type = Json::INT;
            break;
        case DOUBLE:
            data_ = new double(other.as<double>());
           this->impl_->type = Json::DOUBLE;
            break;
        case STRING:
            data_ = new string(other.as<string>());
           this->impl_->type = Json::STRING;
            break;
        case OBJECT:
            data_ = copy(other.as<JObject>());
           this->impl_->type = Json::OBJECT;
            break;
        case ARRAY:
            data_ = copy(other.as<JArray>());
           this->impl_->type = Json::ARRAY;
            break;
        }
    }
}

void * Json::copy(const JObject & o) {
    JObject::const_iterator i = o.begin();
    JObject::const_iterator e = o.end();
    JObject * jop = new JObject();
    for( ; i != e; i++ ) {
        boost::shared_ptr<Json> j(new Json());
        j->copy(*(i->second));
        j->impl_->parent = this;
        jop->insert( JObject::value_type(i->first, j) );
    }
    return jop;
}

void * Json::copy(const JArray & a) {
    JArray * jap = new JArray();
    for(unsigned int i=0; i<a.size(); i++) {
        boost::shared_ptr<Json> j(new Json());
        j->copy(*(a[i]));
        j->impl_->parent = this;
        jap->push_back(j);
    }
    return jap;
}

void Json::setData(void * data, int type) {
    // dont free data if assigning to ourselves
    // ie: json["key"] = json["key"].as<std::string>() 
    if( data_ == data ) return;
    this->freeData();
    data_ = data;
    this->impl_->type = static_cast<Json::Type>(type);
}

Json::Json(const Json & other) : impl_(new Private()), data_(NULL) {
    copy(other);
}

Json::~Json() { 
    freeData();
    delete impl_;
}

Json & Json::parse(const string & content) {
    Json::Parser::parse(content, *this, this->impl_->schema);
    return *this;
}

Json & Json::parseFile( const string & file ) {
    string content = slurp(file);
    this->parse(content);
    return *this;
}

Json::Type Json::type() const {
    return this->impl_->type;
}

string Json::typeName() const {
    return this->typeToName(this->impl_->type);
}

string Json::typeToName(int type) {
    switch(type) {
    case UNDEF:  return "null";
    case BOOL:   return "boolean";
    case INT:    return "integer";
    case DOUBLE: return "double";
    case STRING: return "string";
    case OBJECT: return "object";
    case ARRAY:  return "array";
    }
    return "unknown"; 
}

void Json::assertType(int type) const {
    if( this->impl_->type != type ) {
        this->handleError("cannot convert " + this->typeName() + " to " + this->typeToName(type), this->context());
    }
}

int Json::length() const {
    if( this->impl_->type == Json::ARRAY ) {
        JArray & a  = this->as<JArray>();
        return a.size();
    }    
    return -1;
}

void Json::clear() {
    freeData();
}

bool Json::empty() const {
    if( this->impl_->type == Json::UNDEF ) {
        return true;
    }
    if( this->impl_->type == Json::STRING ) {
        return this->as<string>().empty();
    }
    if( this->impl_->type == Json::OBJECT ) {
        return this->as<JObject>().empty();
    }
    else if( this->impl_->type == Json::ARRAY ) {
        return this->as<JArray>().empty();
    }

    return false;
}

bool Json::hasKey(const string & key) const {
    if( this->impl_->type == Json::OBJECT ) {
        JObject & obj = this->as<JObject>();
        if( obj.find(key) == obj.end() ) {
            return false;
        }
        return true;
    }
    return false;
}

const Json & Json::operator[](const string & key) const {
    if( this->impl_->type == Json::OBJECT ) {
        JObject & o = this->as<JObject>();
        JObject::iterator i = o.find(key);
        if( i == o.end() ) {
            handleError("key " + key + " not found in object",this->context());
            return *this;
        }
        return *(i->second);
    }
    handleError("cannot access object key \"" + key + "\" on type " + this->typeName(), this->context() );
    return *this;
}

const Json & Json::operator[](int idx) const {
    if( this->impl_->type == Json::ARRAY ) {
        JArray & a = this->as<JArray>();
        if( a.size() > static_cast<unsigned int>(idx) ) {
            return *(a[idx]);
        }
        handleError("index out of range", this->context());
        return *this;
    }
    handleError("cannot access array index on type " + this->typeName(),this->context());
    return *this;
}

Json & Json::operator[](const string & key) {
    if( this->impl_->type == Json::OBJECT ) {
        JObject & o = this->as<JObject>();
        JObject::iterator i = o.find(key);
        if( i == o.end() ) {
            // autovivify
            return this->addKey(key);
        }
        else {
            return *(i->second);
        }
    }
    if( this->impl_->type == Json::UNDEF ) {
        // null object being accessed as an object, so autovivify
        // it to become an object
        this->setData(new JObject(), Json::OBJECT);
        return this->addKey(key);
    }
    handleError("cannot access object key \"" + key + "\" on type " + this->typeName(),this->context());
    return *this;
}

const Json & Json::get( const string & key ) const {
    return (*this)[key];
}

const Json & Json::at( int i ) const {
    return (*this)[i];
}

Json & Json::get( const string & key ) {
    return (*this)[key];
}

Json & Json::at( int i ) {
    return (*this)[i];
}

void Json::setSchema(JsonSchema * schema) {
    this->impl_->schema = schema;
}

const Json * Json::parent() const {
    return this->impl_->parent;
}

Json * Json::parent() {
    return this->impl_->parent;
}

bool Json::validate(JsonSchema * schema) {
    if( ! schema->validType(*this) )
        return false;

    schema->typeFixup(*this);
    
    switch(this->impl_->type) {
    case Json::UNDEF:
        return true;
    case Json::BOOL:
    case Json::INT:
    case Json::DOUBLE:
    case Json::STRING:
        return schema->validValue(*this);
    case Json::OBJECT:
        return this->validateObject(schema);
    case Json::ARRAY:
        return this->validateArray(schema);
    }
    return false;
}

bool Json::validateObject(JsonSchema * schema) {
    JObject & o = this->as<JObject>();
    JObject::iterator it = o.begin();
    JObject::iterator end = o.end();
    for( ; it != end; ++it ) {
        if( !it->second->validate( schema->getChildSchema( it->first, *(it->second.get()) ) ) ) {
            return false;
        }
    }
    if( !schema->setDefaults(*this) )
        return false;
    return schema->validRequires(*this);
}

bool Json::validateArray(JsonSchema * schema) {
    JArray & a = this->as<JArray>();
    unsigned int size = a.size();
    if( !schema->validMinSize( a ) )
        return false;
    if( !schema->validMaxSize( a ) ) 
        return false;
    for( unsigned int i=0; i<size; ++i ) {
        if( ! a[i]->validate( schema->getChildSchema(i, *(a[i])) ) ) {
            return false;
        }
    }
    return true;
}

Json & Json::addKey(const string & key) {
    if( this->impl_->type != Json::OBJECT ) {
        handleError("cannot add key \"" + key + "\" to type " + this->typeName(),this->context());
        return *this;
    }
    Json * j = new Json();
    j->impl_->parent = this;
    this->as<JObject>().insert( JObject::value_type(key, boost::shared_ptr<Json>(j)) );
    return *j;
}

bool Json::deleteKey(const string & key) 
{
    if( this->impl_->type != Json::OBJECT ) {
        return handleError("delKey() called on type " + this->typeName(), this->context() );
    }
    JObject & o = this->as<JObject>();
    JObject::iterator i = o.find(key);
    if( i == o.end() ) return false;
    o.erase(i);
    return true;
}

Json & Json::operator[](int idx) {
    if( this->impl_->type == Json::ARRAY ) {
        this->growArray(idx+1);
        return *(this->as<JArray>()[idx]);
    }
    if( this->impl_->type == Json::UNDEF ) {
        this->setData(new JArray(), Json::ARRAY);
        this->growArray(idx+1);
        return *(this->as<JArray>()[idx]);
    }
    handleError("cannot access array index on type " + this->typeName(),this->context());
    return *this;
}

void Json::growArray(int size) {
    if( this->impl_->type != Json::ARRAY ) {
        handleError("cannot grow a type " + this->typeName(),this->context());
        return;
    }
    JArray & a = this->as<JArray>();
    if( a.size() < static_cast<unsigned int>(size) ) {
        for( int i=a.size(); i<size; i++ ) {
            Json * j = new Json();
            j->impl_->parent = this;
            a.push_back(boost::shared_ptr<Json>(j));
        }
    }
}    

Json & Json::operator=(const Json & value) {
    copy(value);
    return *this;
}

Json & Json::operator=(const char * value) {
    freeData();
    this->data_ = new string(value);
    this->impl_->type = Json::STRING;
    return *this;
}

Json & Json::operator=(char * value) {
    freeData();
    this->data_ = new string(value);
    this->impl_->type = Json::STRING;
    return *this;
}

string Json::serialize(bool beautify) const {
    return Json::Generator::generate(*this, beautify);
}

string Json::context() const {
    std::ostringstream ctx;
    if( this->impl_->parent ) {
        ctx << this->impl_->parent->context();
        if( this->impl_->parent->impl_->type == Json::ARRAY ) {
            // got to loop through the array to figure out
            // which index we are, slow and lame, but context() is only 
            // used upon throw() anyway
            JArray & a = this->impl_->parent->as<JArray>();
            for( unsigned int i=0; i<a.size(); i++ ) {
                if( a[i].get() == this ) {
                    ctx << "[" << i << "]";
                    return ctx.str();
                }
            }
            ctx << "[?]";
            return ctx.str();
        }
        if( this->impl_->parent->impl_->type == Json::OBJECT ) {
            JObject & o = this->impl_->parent->as<JObject>();
            JObject::iterator it = o.begin();
            for( ; it != o.end(); it++ ) {
                if( it->second.get() == this ) {
                    ctx << "[\"" << it->first << "\"]";
                    return ctx.str();
                }
            }
            ctx << "[\"?\"]";
            return ctx.str();
        }
    }
    return ctx.str();
}

bool Json::handleError(const string & error, const string & context) const {
    this->impl_->error = error;
    if( impl_->errHandler ) {
        return impl_->errHandler(error, context);
    }
    return true;
}

void Json::setErrorHandler(ErrorHandler err) {
    impl_->errHandler = err;
}

string Json::errorMessage() const {
    return impl_->error;
}

bool Json::onErrorThrow(const string & error, const string & context ) {
    gbTHROW( JsonError(context, error) );
}

bool Json::onErrorWarn(const string & error, const string & context ) {
    std::cerr << "Json Parser Warning: " << error;
    if( !context.empty() ) {
        std::cerr << " at " << context;
    }
    std::cerr << std::endl;
    return true;
}

std::ostream & operator<<(std::ostream & os, const Json & json) {
    switch(json.type()) {
    case Json::UNDEF:
        os << "null";
        break;
    case Json::BOOL:
        os << (json.as<bool>() ? "true" : "false");
        break;
    case Json::INT:
        os << json.as<int64_t>();
        break;
    case Json::DOUBLE:
        os << json.as<double>();
        break;
    case Json::STRING:
        os << json.as<string>();
        break;
    case Json::OBJECT:
        os << json.serialize();
        break;
    case Json::ARRAY:
        os << json.serialize();
    }
    return os;
}

} // namespace
