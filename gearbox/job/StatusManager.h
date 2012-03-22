#ifndef STATUS_MANAGER_H
#define STATUS_MANAGER_H

#include <gearbox/core/ConfigFile.h>
#include <gearbox/job/Status.h>
#include <gearbox/job/StatusCollection.h>
#include <gearbox/core/Uri.h>
#include <string>

namespace Gearbox {

    class Job;
    typedef boost::shared_ptr<Job> JobPtr;

    typedef boost::shared_ptr<Status> StatusPtr;
    typedef boost::shared_ptr<StatusCollection> StatusCollectionPtr;

    class StatusManager {
    public:
        StatusManager(const ConfigFile & cfg);
        StatusManager(const std::string & type, const ConfigFile & cfg);
        StatusManager(const StatusManager & copy);
        StatusManager & operator=(const StatusManager & copy);
        ~StatusManager();
        void base_uri( const std::string & base_uri );

        StatusPtr create(
            const std::string & name,
            const std::string & operation,
            const std::string & resource_uri = "",
            const std::string & component = "internal"
        ) const;
        
        StatusPtr fetch(
            const char * name
        ) const;

        StatusPtr fetch(
            const std::string & name
        ) const;
        
        StatusPtr fetch(
            const Uri & uri
        ) const;

        StatusPtr fetch(
            const Job & job
        ) const;

        StatusPtr fetch(
            const Json & status
        ) const;

        StatusCollectionPtr collection() const;

    private:
        class Private;
        Private *impl;
    };
}
#endif
