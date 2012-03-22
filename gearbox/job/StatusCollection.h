#ifndef GEARBOX_STATUS_COLLECTION_H
#define GEARBOX_STATUS_COLLECTION_H

#include <limits.h>
#include <boost/shared_ptr.hpp>

namespace Gearbox {

    class StatusCollectionImpl; // forward declare
    typedef boost::shared_ptr<Status> StatusPtr;

    class StatusCollection {
    public:
        StatusPtr pop();
        bool empty();

        StatusCollection & filter_progress(unsigned int min=0, unsigned int max=100);
        StatusCollection & filter_code(int min=0, int max=0);
        StatusCollection & filter_operation(const std::string & op);
        StatusCollection & filter_component(const std::string & c);
        StatusCollection & filter_mtime(time_t min=0, time_t max=UINT_MAX);
        StatusCollection & filter_ctime(time_t min=0, time_t max=UINT_MAX);
        StatusCollection & filter_state(const std::string & state);
        StatusCollection & filter_uri(const std::string & uri);
        StatusCollection & limit(unsigned int count=0);

        ~StatusCollection();
    private:
        friend class StatusManager;
        StatusCollection( StatusCollectionImpl * impl );
        StatusCollection( const StatusCollection & );
        StatusCollection & operator=(const StatusCollection &);
        StatusCollectionImpl * impl;
    };
}

#endif
