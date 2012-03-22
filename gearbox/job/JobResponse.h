#ifndef GEARBOX_JOB_JOB_RESPONSE_H
#define GEARBOX_JOB_JOB_RESPONSE_H

#include <gearbox/job/StatusManager.h>
#include <gearbox/core/Hash.h>

namespace Gearbox {
    typedef std::vector<std::string> Array;

    class JobResponse {
    public:
        JobResponse();
        JobResponse(const JobResponse & copy);
        JobResponse & operator=(const JobResponse & copy);
        ~JobResponse();
        
        void content( const std::string & content );
        const std::string & content() const;
        
        void headers( const Hash & headers );
        void add_header( const std::string & name, const std::string & value );
        const Hash & headers() const;
        
        void status( const Gearbox::Status & status );
        const StatusPtr & status() const;

        void code( int code );
        int code() const;

        void job( const JobPtr & job );
        const JobPtr & job() const;
        
    private:
        class Private;
        Private * impl;
    };
}

#endif
