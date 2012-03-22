#ifndef GEARBOX_HASH_H
#define GEARBOX_HASH_H

#include <map>
#include <string>
#include <boost/lexical_cast.hpp>

namespace Gearbox {
    class Hash : public std::map<std::string,std::string> {
        typedef std::map<std::string,std::string> super;

    public:
        bool has_key(const std::string & key) const;
        const std::string & operator[](const std::string & key) const;
        using super::operator[];

        template<typename T>
        T get_default(const std::string & key, T dflt) const {
            if( this->has_key(key) ) {
                return boost::lexical_cast<T>( (*this)[key] );
            }
            return dflt;
        }
    };
}
#endif
