// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include "config.h"
#include <gearbox/core/logger.h>
#include <gearbox/core/Plugin.h>
#include <gearbox/core/Errors.h>

#include <gearbox/job/StatusManager.h>
#include <gearbox/job/JobManager.h>
#include <gearbox/job/TransientStatusImpl.h>
#include <gearbox/job/JobStatusImpl.h>
#include <gearbox/job/JsonStatusImpl.h>

#include <boost/filesystem/path.hpp>
namespace bfs=boost::filesystem;

using std::string;
using std::vector;

namespace Gearbox {

    class StatusPlugin;
    class StatusImpl;
    typedef std::map<std::string,StatusPlugin*> PluginMap;

    class StatusPlugin : public Plugin {
    public:
        static PluginMap statusPlugins;
        static bool loaded;

        static void loadAll(const ConfigFile & cfg) {
            if(loaded) return;
            bfs::path pluginDir( 
                cfg.get_string_default("status", "plugin_path", PLUGINSDIR "/status")
            ); 
            Plugins plugins = Plugin::loadAll(pluginDir);
            for( unsigned int i=0; i < plugins.size(); ++i ) {
                StatusPlugin * p  = static_cast<StatusPlugin*>(plugins[i]);
                // get plugin name
                void * func = p->getFunc("name");
                typedef const char * (*namer)();
                const char * name = ((namer)func)();
                // put new name in map for this plugin
                statusPlugins[name] = p;
            }
            loaded = true;
        }

        static StatusPlugin * find(const std::string type_name, const ConfigFile & cfg) {
            loadAll(cfg);
            PluginMap::iterator it = statusPlugins.find(type_name);
            if( it == statusPlugins.end() )
                return NULL;
            return it->second;
        }

        StatusImpl * status_impl(const ConfigFile & cfg) {
            void * ctor = this->getCtor();
            typedef StatusImpl * (*generator)(const ConfigFile &, bool);
            return ((generator)ctor)(cfg,false);
        }
        
        StatusCollectionImpl * status_collection_impl(const ConfigFile & cfg) {
            void * ctor = this->getCtor();
            typedef StatusCollectionImpl * (*generator)(const ConfigFile &, bool);
            return ((generator)ctor)(cfg,true);
        }
    };

    // static data member ctors
    PluginMap StatusPlugin::statusPlugins;
    bool StatusPlugin::loaded(false);

    struct StatusManager::Private {
        enum StatusType {
            STATUS_NONE,
            STATUS_PLUGIN,
            STATUS_TRANSIENT,
            STATUS_JOB
        };

        ConfigFile cfg;
        std::string base_uri;
        std::string typestr;
        StatusType type;

        static StatusType string_to_type(const std::string & str) {
            if ( str == "transient" ) {
                return STATUS_TRANSIENT;
            }
            else if ( str == "job" ) {
                return STATUS_JOB;
            }
            else {
                return STATUS_PLUGIN;
            }
        }
        
        Private(const ConfigFile & a_cfg) : cfg ( a_cfg ), typestr(cfg.get_string_default("status", "persistence_type", "sql")), type(string_to_type(typestr)) {}

        Private(const std::string & type_str, const ConfigFile & a_cfg) : cfg ( a_cfg ), typestr(type_str), type(string_to_type(typestr)) {
            
        }
        ~Private() {};
        
        StatusImpl * newStatusImpl() {
            StatusPlugin * plugin = NULL;
            switch(type) {
            case STATUS_PLUGIN:
                plugin = StatusPlugin::find(this->typestr, this->cfg);
                if( plugin ) {
                    return plugin->status_impl(this->cfg);
                }
                gbTHROW( ERR_INTERNAL_SERVER_ERROR("no plugin found for status persistence type of " + this->typestr) );
                    
            case STATUS_TRANSIENT:
                return new TransientStatusImpl( this->cfg );
            case STATUS_JOB:
                return new JobStatusImpl( this->cfg );
            case STATUS_NONE:
            default:
                gbTHROW( ERR_INTERNAL_SERVER_ERROR("Unknown Status Implementation Type") );
            }
        }

        StatusCollectionImpl * newStatusCollectionImpl() {
            StatusPlugin * plugin = NULL;
            switch(type) {
            case STATUS_PLUGIN:
                plugin = StatusPlugin::find(this->typestr, this->cfg);
                if( plugin ) {
                    return plugin->status_collection_impl(this->cfg);
                }
                gbTHROW( ERR_INTERNAL_SERVER_ERROR("no plugin found for status persistence type of " + this->typestr) );
            case STATUS_NONE:
            default:
                gbTHROW( ERR_INTERNAL_SERVER_ERROR("Unknown Status Collection Implementation Type") );
            }
        }
    };

    StatusManager::StatusManager( const ConfigFile & cfg ) : impl(new Private(cfg)) {}
    StatusManager::StatusManager( const std::string & type, const ConfigFile & cfg)
        : impl( new Private(type, cfg) ) {}

    StatusManager::StatusManager( const StatusManager & copy )
        : impl( new Private(*copy.impl) ) {}

    StatusManager & StatusManager::operator=(const StatusManager & copy) {
        if( this == &copy ) return *this;
        impl->cfg      = copy.impl->cfg;
        impl->base_uri = copy.impl->base_uri;
        impl->type     = copy.impl->type;
        return *this;
    }
        

    StatusManager::~StatusManager() {
        delete impl;
    }

    void
    StatusManager::base_uri( const std::string & base_uri ) {
        impl->base_uri = base_uri;
    }

    StatusPtr
    StatusManager::create( 
        const string & name,
        const string & operation,
        const string & rsrc_uri,
        const string & component
    ) const {
        std::auto_ptr<StatusImpl> si(impl->newStatusImpl());
        si->base_uri( impl->base_uri );
        si->name(name);
        si->operation(operation);
        si->resource_uri(rsrc_uri);
        si->component(component);
        si->state(Status::STATE_PENDING);
        si->insert();
        si->status_manager( * this );
        return StatusPtr(new Status(si.release()));
    }

    StatusPtr
    StatusManager::fetch( const char * name ) const {
        if( name ) {
            return this->fetch(string(name));
        }
        return StatusPtr();
    }

    StatusPtr
    StatusManager::fetch( const std::string & name ) const {
        std::auto_ptr<StatusImpl> si(impl->newStatusImpl());
        si->base_uri( impl->base_uri );
        si->name(name);
        si->load();
        si->status_manager( * this );
        return StatusPtr(new Status(si.release()));
    }

    StatusPtr
    StatusManager::fetch( const Uri & uri ) const {
        JobManager jm(impl->cfg);
        jm.base_uri( impl->base_uri );
        JobPtr job = jm.job(HttpClient::METHOD_GET, uri.canonical());
        job->type(Job::JOB_SYNC);
        return this->fetch(*job);
    }

    StatusPtr
    StatusManager::fetch( const Job & job ) const {
        std::auto_ptr<JobStatusImpl> si(new JobStatusImpl(impl->cfg));
        si->base_uri( impl->base_uri );
        si->job( job );
        si->load();
        si->status_manager( * this );
        return StatusPtr(new Status(si.release()));
    }

    StatusPtr
    StatusManager::fetch( const Json & status ) const {
        std::auto_ptr<JsonStatusImpl> si(new JsonStatusImpl(impl->cfg));
        si->json( status );
        si->load();
        si->status_manager( * this );
        return StatusPtr(new Status(si.release()));
    }

    StatusCollectionPtr
    StatusManager::collection() const {
        return StatusCollectionPtr(new StatusCollection(impl->newStatusCollectionImpl()));
    }
}

