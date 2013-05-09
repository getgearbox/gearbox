// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/core/ConfigFile.h>

#include "config.h"
#include <gearbox/core/logger.h>
#include <glob.h>
#include <map>

#include <boost/filesystem.hpp>
namespace bfs=boost::filesystem;

namespace Gearbox {

    static std::map< std::string, boost::shared_ptr<Json> > CONFIG_MAP;

    struct ConfigFile::Private {
        boost::shared_ptr<Json> content;
        std::string file;
        Private(const std::string & f) : content(CONFIG_MAP[f]), file(f)  {
            if( ! content.get() ) {
                content = CONFIG_MAP[file] = boost::shared_ptr<Json>(new Json());
                content->parseFile(file);
                std::string config_dir(SYSCONFDIR "/gearbox/config.d");
                if( content->hasKey("config_dir") ) {
                    config_dir = content->get("config_dir").as<std::string>();
                }

                glob_t globbuf;
                glob( std::string(config_dir + "/*.conf").c_str(), 0, NULL, &globbuf);
                for( unsigned int i=0; i < globbuf.gl_pathc; i++ ) {
                    std::string base = bfs::basename(globbuf.gl_pathv[i]);
                    try {
                        // if key exists in top-level conf file then
                        // attempt to merge in the config.d files
                        // however the top-level conf settings
                        // take precedence
                        if( content->hasKey(base) ) {
                            Json tmp;
                            tmp.parseFile(globbuf.gl_pathv[i]);
                            Json::Object::iterator it  = tmp.as<Json::Object>().begin();
                            Json::Object::iterator end = tmp.as<Json::Object>().end();
                            for( ; it != end; ++it ) {
                                if( ! content->get(base).hasKey(it->first) ) {
                                    content->get(base).get(it->first) = *(it->second);
                                }
                            }
                        } else {
                            content->get(base).parseFile(globbuf.gl_pathv[i]);
                        }
                    }
                    catch(const std::exception & e) {
                        _WARN(
                            "ConfigFile parse error for file " << 
                            config_dir << "/" << globbuf.gl_pathv[i] << 
                            ": " << e.what()
                        );
                    }
                }
                globfree(&globbuf);
            }
        }
    };
    
    ConfigFile::ConfigFile( const std::string & file) : impl(new Private(file)) {}
    ConfigFile::ConfigFile( const ConfigFile & copy ) : impl(new Private(*copy.impl)) {}
    ConfigFile & ConfigFile::operator=(const ConfigFile & copy) {
        if( this == &copy ) return *this;
        if( impl ) delete impl;
        impl = new Private(*copy.impl);
        return *this;
    }

    ConfigFile::~ConfigFile() {
        if( impl ) delete impl;
    }
        

    static const std::string EMPTY_STRING;

    const std::string &
    ConfigFile::get_string(
        const std::string & section,
        const std::string & key
    ) const {
        if( impl->content->hasKey(section) && impl->content->get(section).hasKey(key) ) {
            return impl->content->get(section).get(key).as<std::string>();
        }
        return EMPTY_STRING;
    }

    const std::string &
    ConfigFile::get_string_default(
        const std::string & section,
        const std::string & key,
        const std::string & dflt
    ) const {
        if( impl->content->hasKey(section) && impl->content->get(section).hasKey(key) ) {
            return impl->content->get(section).get(key).as<std::string>();
        }
        return dflt;
    }

    int64_t
    ConfigFile::get_int(
        const std::string & section,
        const std::string & key
    ) const {
        if( impl->content->hasKey(section) && impl->content->get(section).hasKey(key) ) {
            return impl->content->get(section).get(key).as<int64_t>();
        }
        return 0;
    }

    int64_t
    ConfigFile::get_int_default(
        const std::string & section,
        const std::string & key,
        int64_t dflt
    ) const {
        if( impl->content->hasKey(section) && impl->content->get(section).hasKey(key) ) {
            return impl->content->get(section).get(key).as<int64_t>();
        }
        return dflt;
    }

    const std::string &
    ConfigFile::get_string(
        const std::string & key
    ) const {
        if( impl->content->hasKey(key) ) {
            return impl->content->get(key).as<std::string>();
        }
        return EMPTY_STRING;
    }
        
    const std::string &
    ConfigFile::get_string_default(
        const std::string & key,
        const std::string & dflt
    ) const {
        if( impl->content->hasKey(key) ) {
            return impl->content->get(key).as<std::string>();
        }
        return dflt;
    }

    int64_t
    ConfigFile::get_int(
        const std::string & key
    ) const {
        if( impl->content->hasKey(key) ) {
            return impl->content->get(key).as<int64_t>();
        }
        return 0;
    }

    int64_t
    ConfigFile::get_int_default(
        const std::string & key,
        int64_t dflt
    ) const {
        if( impl->content->hasKey(key) ) {
            return impl->content->get(key).as<int64_t>();
        }
        return dflt;
    }

    static const Json EMPTY_JSON;

    const Json & ConfigFile::get_json(
        const std::string & key
    ) const {
        if( impl->content->hasKey(key) ) {
            return impl->content->get(key);
        }
        return EMPTY_JSON;
    }

    const Json & ConfigFile::as_json() const {
        return *(impl->content);
    }

} // namespace
