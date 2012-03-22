#ifndef GEARBOX_PLUGIN_H
#define GEARBOX_PLUGIN_H

#include <vector>
#include <string>
#include <boost/filesystem/path.hpp>

namespace Gearbox {

class Plugin;
typedef std::vector<Plugin*> Plugins;

class Plugin {
public:
    static Plugins  loadAll(const boost::filesystem::path & pluginDir);
    static Plugin * load(const boost::filesystem::path & pluginDir, const std::string & name );

    virtual std::string name() = 0;
    virtual ~Plugin();

    template<typename T> 
    T * create() {
        void * ctor = this->getCtor();
        typedef T * (*generator)();
        generator g = (generator)ctor;
        return g();
    }
    template<typename T, typename TT> 
    T * create(const TT & arg) {
        void * ctor = this->getCtor();
        typedef T * (*generator)(const TT &);
        generator g = (generator)ctor;
        return g(arg);
    }
    template<typename T>
    void destroy(T * obj) {
        void * dtor =  this->getDtor();
        typedef void (*destroyer)(T *);
        destroyer d = (destroyer)dtor;
        d(obj);
    }
    virtual void * getCtor() = 0;
    virtual void * getDtor() = 0;
    virtual void * getFunc( const std::string & name ) = 0;
    virtual bool can( const std::string & name ) = 0;

protected:
    Plugin();
    Plugin( const Plugin & plugin );
};

} // namespace
#endif
