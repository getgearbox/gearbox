// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include "config.h"
#include <gearbox/core/Plugin.h>
#include <gearbox/core/logger.h>
#include <gearbox/core/Errors.h>
#include <boost/filesystem/operations.hpp>
#include <map>
#include <stdexcept>
#include <dlfcn.h>

namespace bfs = boost::filesystem;

using std::string;
using std::map;

// hash of hash:
// pluginMap[pluginDir][pluginName] = Plugin*

namespace Gearbox {

class PrivatePlugin: public Plugin {
public:
    PrivatePlugin();
    PrivatePlugin(const string & file, const string & name);
    ~PrivatePlugin();
    void * getCtor();
    void * getDtor();
    void * getFunc( const string & name );
    string getError();
    string name();
    bool can( const string & name );
    
    void * dl_;
    string error_;
    string file_;
    string name_;
};

typedef boost::shared_ptr<PrivatePlugin> PluginPtr;
typedef map< string, PluginPtr > PluginMap;
typedef map< bfs::path, PluginMap > PluginsMap;
static PluginsMap pluginsMap;

PrivatePlugin::PrivatePlugin() : dl_(NULL) {}
PrivatePlugin::PrivatePlugin(const string & file, const string & name)
  : file_(file), name_(name)  {
    dl_ = dlopen(file_.c_str(), RTLD_NOW);
    if( !dl_ ) error_ = dlerror();
}
PrivatePlugin::~PrivatePlugin() { if(dl_) dlclose(dl_); }

Plugins Plugin::loadAll( const bfs::path & pluginDir )  {
    if( pluginsMap.find(pluginDir) == pluginsMap.end() ) {
        pluginsMap[pluginDir] = PluginMap();
        
        if( ! bfs::exists( pluginDir ) ) {
            _DEBUG("Plugin Directory " << pluginDir.string() << " does not exist.");
            return Plugins();
        }
        bfs::directory_iterator dend;
        bfs::directory_iterator dit(pluginDir);
        _INFO("shlib: " << SHLIB_EXT);
        int extlen = strlen(SHLIB_EXT);
        _INFO("extlen: " << extlen);
        for( ; dit != dend; ++dit ) {
            string file = dit->path().string();
            string name = dit->path().filename().string();
            if( file.substr( file.size() - extlen ) == SHLIB_EXT ) {
                // chop off the ".so"
                name.erase(name.size() - extlen, extlen);
            }
            else if( file.substr( file.size() - 3 ) == ".so" ) {
                // chop off the ".so"
                name.erase(name.size() - 3, 3);
            }
            else {
                continue;
            }

            // it looks like a shared object
            pluginsMap[pluginDir].insert( 
                PluginMap::value_type(name,PluginPtr(new PrivatePlugin(file, name)))
            );
            if( !pluginsMap[pluginDir][name]->dl_ ) {
                string err = pluginsMap[pluginDir][name]->getError();
                pluginsMap.erase(pluginDir);
                gbTHROW( std::runtime_error( err ) );
            }                   
            try {
                pluginsMap[pluginDir][name]->getCtor();
                _DEBUG("loaded plugin: " << file);
            }
            catch( const std::exception & err ) {
                // module does not have a valid ctor, so ignore it
                _WARN("Ignoring plugin: " << err.what())
                    pluginsMap[pluginDir].erase(name);
            }
        }
    }

    Plugins plugins;
    PluginMap::iterator it = pluginsMap[pluginDir].begin();
    PluginMap::iterator end = pluginsMap[pluginDir].end();
    for( ; it != end; ++it ) {
        plugins.push_back( it->second.get() );
    }
    return plugins;
}

Plugin * Plugin::load( const bfs::path & pluginDir, const string & name ) {
    if( pluginsMap.find(pluginDir) == pluginsMap.end() 
        || pluginsMap[pluginDir].find(name) == pluginsMap[pluginDir].end() ) {
        const char * const exts[] = {SHLIB_EXT, ".so"};
        int found = 0;
        for(int i=0; i < 2; i++) {
            bfs::path dlname = pluginDir / (name + exts[i]);
            if( bfs::exists(dlname) ) {
                pluginsMap[pluginDir].insert(
                    PluginMap::value_type(
                        name, PluginPtr(new PrivatePlugin(dlname.string(), name))
                    )
                );
                if( !pluginsMap[pluginDir][name]->dl_ ) {
                    string err = pluginsMap[pluginDir][name]->getError();
                    pluginsMap[pluginDir].erase(name);
                    gbTHROW( std::runtime_error( err ) );
                }
                found=1;
                _DEBUG("loaded plugin: " << dlname.string());
            }
        }
        
        if(!found) {
            gbTHROW( std::runtime_error( "plugin \"" + name + "\" does not exist" ) );
        }
    }
    return pluginsMap[pluginDir][name].get();
}

void * PrivatePlugin::getCtor() {
    string ctor_name = this->name_ + "_new";
    return this->getFunc( ctor_name );
}

void * PrivatePlugin::getDtor() {
    string dtor_name = this->name_ + "_delete";
    return this->getFunc( dtor_name );
}

void * PrivatePlugin::getFunc(const string & name) {
    if( !this->dl_ ) {
        gbTHROW( std::runtime_error( "cannot load function on unloaded plugin: " + this->file_ ) );
    }
    
    dlerror(); // clear error
    void * sym = dlsym(this->dl_, name.c_str());
    const char * error = dlerror();
    if( error ) {
        gbTHROW( std::runtime_error( error ) );
    }
    return sym;
}

string PrivatePlugin::getError() {
    return string("failed to load plugin: ") + error_;
}

bool PrivatePlugin::can( const string & name ) {
    if( !this->dl_ )
        return false;
    dlerror(); // clear error
    dlsym(this->dl_, name.c_str());
    // if there was an error from dlsym this will
    // return a NON-NULL err message
    if( dlerror() ) {
        return false;
    }
    return true;
}

std::string PrivatePlugin::name() {
    return name_;
}

Plugin::Plugin() {}
Plugin::~Plugin() {}

} // namespace
