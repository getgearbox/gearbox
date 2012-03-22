/* configuration file handling */

#ifndef GEARBOX_CONFIG_FILE_H
#define GEARBOX_CONFIG_FILE_H

#include <string>
#include <gearbox/core/Json.h>

namespace Gearbox {
    
    class ConfigFile {
    public:
        ConfigFile( const std::string & file);
        ConfigFile( const ConfigFile & copy );
        ConfigFile & operator=(const ConfigFile & copy);
        ~ConfigFile();
        
        const std::string &
        get_string(
            const std::string & section,
            const std::string & key
        ) const;

        const std::string &
        get_string_default(
            const std::string & section,
            const std::string & key,
            const std::string & dflt
        ) const;

        int64_t
        get_int(
            const std::string & section,
            const std::string & key
        ) const;

        int64_t
        get_int_default(
            const std::string & section,
            const std::string & key,
            int64_t dflt
        ) const;

        const std::string &
        get_string(
            const std::string & key
        ) const;
        
        const std::string &
        get_string_default(
            const std::string & key,
            const std::string & dflt
        ) const;

        int64_t
        get_int(
            const std::string & key
        ) const;

        int64_t
        get_int_default(
            const std::string & key,
            int64_t dflt
        ) const;

        const Json &
        get_json(
            const std::string & key
        ) const;

        const Json &
        as_json() const;

    private:
        class Private;
        Private * impl;
    };

} // namespace

#endif

