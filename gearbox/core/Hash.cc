// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/core/Hash.h>
#include <gearbox/core/Errors.h>

#include <stdexcept>

namespace Gearbox {
    
    bool Hash::has_key(const std::string & key) const {
        return this->find(key) != this->end();
    }
    
    const std::string & Hash::operator[](const std::string & key) const {
        Hash::const_iterator it = this->find(key);
        if( it == this->end() ) {
            gbTHROW( std::invalid_argument("key " + key + " not found in const Hash") );
        }
        return it->second;
    }

    template<>
    const std::string &
    Hash::get_default<const std::string &>(const std::string & key, const std::string & dflt) const {
        if( this->has_key(key) ) {
            return (*this)[key];
        }
        return dflt;
    }
}
