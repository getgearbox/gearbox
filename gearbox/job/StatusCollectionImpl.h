#ifndef GEARBOX_STATUS_COLLECTION_IMPL_H
#define GEARBOX_STATUS_COLLECTION_IMPL_H

#include <limits.h>
#include <gearbox/job/StatusImpl.h>
#include <gearbox/core/ConfigFile.h>

namespace Gearbox {

    class StatusCollectionImpl {
    public:
        StatusCollectionImpl(const ConfigFile & c);
        StatusCollectionImpl( const StatusCollectionImpl & );
        virtual ~StatusCollectionImpl();
        virtual int version() = 0;

        virtual StatusImpl * pop() = 0;
        virtual bool empty() = 0;

        virtual void filter_progress(unsigned int min=0, unsigned int max=100) = 0;
        virtual void filter_code(int min=0, int max=0) = 0;
        virtual void filter_operation(const std::string & op) = 0;
        virtual void filter_component(const std::string & c) = 0;
        virtual void filter_mtime(time_t min=0, time_t max=UINT_MAX) = 0;
        virtual void filter_ctime(time_t min=0, time_t max=UINT_MAX) = 0;
        virtual void filter_state(const std::string & state) = 0;
        virtual void filter_uri(const std::string & uri) = 0;
        virtual void limit(unsigned int count=0) = 0;


    protected:
        const ConfigFile & cfg() const;

    private:
        StatusCollectionImpl & operator=(const StatusCollectionImpl &);
        class Private;
        Private * impl;
    };
}

#endif
