// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/core/logger.h>
#include <gearbox/job/JsonStatusImpl.h>
#include <gearbox/core/Uri.h>
#include <gearbox/core/Errors.h>

namespace Gearbox {
    
    JsonStatusImpl::JsonStatusImpl(const ConfigFile & c)
        : super(c) {}

    JsonStatusImpl::JsonStatusImpl( const JsonStatusImpl & other ) 
        : super(other), data(other.data) {}

    JsonStatusImpl::~JsonStatusImpl() {}

    StatusImpl * JsonStatusImpl::clone() const {
        return new JsonStatusImpl(*this);
    }

    void JsonStatusImpl::json( const Json & json ) {
        this->data = json;
    }
    
    void JsonStatusImpl::load() {
        _TRACE("Loading Status from JSON: " << this->data.serialize());
        if ( data.hasKey("operation") )
            this->operation( data["operation"].as<std::string>() );
        if ( data.hasKey("component") )
            this->component( data["component"].as<std::string>() );
        if ( data.hasKey("state") )
            this->state( Status::str2state( data["state"].as<std::string>() ) );
        if ( data.hasKey("uri") )
            this->resource_uri( data["uri"].as<std::string>() );

        
        if ( data.hasKey("status_uri") ) {
            Uri uri(data["status_uri"].as<std::string>());
            this->name( uri.leaf() );
            uri.path_pop(); // remove status name
            uri.path_pop(); // remove resource name "/status/"
            this->base_uri( uri.canonical() );
        }

        if ( data.hasKey("meta") ) {
            this->meta( data["meta"] );
        }

        if( data.hasKey("progress") )
            this->progress( data["progress"].as<int64_t>() );
        
        if( data.hasKey("code") )
            this->code( data["code"].as<int64_t>() );
        
        if( data.hasKey("messages") ) {
            this->messages( Array() );
            for( int i=0; i < data["messages"].length(); ++i ) {
                this->add_message( data["messages"][i].as<std::string>() );
            }
        }
    
        if( data.hasKey("children") ) {
            this->children( Array() );
            for( int i=0; i < data["children"].length(); i++ ) {
                this->add_child( data["children"][i].as<std::string>() );
            }
        }

        if( data.hasKey("parent_uri") ) { 
            this->parent_uri( data["parent_uri"].as<std::string>() );
        }

        if( data.hasKey("failures") ) 
            this->failures( data["failures"].as<int64_t>() );

        if( data.hasKey("ctime") )
            this->ctime( data["ctime"].as<int64_t>() );

        if( data.hasKey("mtime") )
            this->mtime( data["mtime"].as<int64_t>() );

        if( data.hasKey("ytime") )
            this->ytime( data["ytime"].as<int64_t>() );

        if( data.hasKey("concurrency") ) {
            this->concurrency( data["concurrency"].as<uint32_t>() );
        }
    }
    
    void JsonStatusImpl::insert() {
        gbTHROW( std::runtime_error("json status insert not implemented") );
    }
    
    void JsonStatusImpl::sync() {
        this->state(Status::STATE_UNKNOWN);
        this->load();
    }

    const char * JsonStatusImpl::impltype() const {
        return "json";
    }

    void JsonStatusImpl::on(Status::Event e, const Job & job) {
        gbTHROW( ERR_NOT_IMPLEMENTED("cannot set event handlers for remote json status object") );
    }

    JobPtr JsonStatusImpl::on(Status::Event e, const JobManager & jm) const {
        gbTHROW( ERR_NOT_IMPLEMENTED("cannot get event handlers for remote json status object") );
    }

    void JsonStatusImpl::starting() {
        gbTHROW( ERR_NOT_IMPLEMENTED("cannot update concurrency for remote json status object") );
    }

    void JsonStatusImpl::stopping() {
        gbTHROW( ERR_NOT_IMPLEMENTED("cannot update concurrency for remote json status object") );
    }
}
