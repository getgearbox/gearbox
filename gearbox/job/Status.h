#ifndef GEARBOX_JOB_STATUS_H
#define GEARBOX_JOB_STATUS_H

#include <vector>
#include <string>
#include <time.h>
#include <boost/shared_ptr.hpp>
#include <gearbox/core/Json.h>

namespace Gearbox {

    // forward declares
    class StatusImpl;
    class Job;
    class Status;
    typedef boost::shared_ptr<Job> JobPtr;
    typedef boost::shared_ptr<Status> StatusPtr;
    class JobManager;

    class Status {
      public:
        enum State 
        {
            STATE_UNKNOWN = 0,
            STATE_PENDING,
            STATE_RUNNING,
            STATE_STOPPING,
            STATE_STOPPED,
            STATE_CANCELLING,
            STATE_CANCELLED,
            STATE_COMPLETED
        };

        enum Event {
            EVENT_UNKNOWN,
            EVENT_PRECANCEL,
            EVENT_CANCEL
        };
    
        Status( const Status & );
        Status & operator=(const Status &);

        const std::vector<std::string> & messages() const;
        void add_message( const std::string & message );
        
        const std::vector<std::string> & children() const;
        void add_child( const std::string & child );

        const std::string & name() const;
        const std::string & operation() const;
        const std::string & resource_uri() const;
        const std::string & component() const;
        std::string uri() const;

        const Json & meta() const;
        void meta( const std::string & key, const Json & value );
        void meta( const Json & meta );
        
        void progress(unsigned int);
        unsigned int progress() const;
        
        void fail( int code );
        void cancel();
        void success();
        int code() const;

        // can schedule call backs on running job
        // for async callbacks
        void on(Status::Event e, const Job & job);
        JobPtr on(Status::Event e, const JobManager & jm) const;

        static Status::Event str2event( const std::string & str );
        static std::string event2str( Status::Event event );

        void parent_uri(const std::string & parent);
        const std::string & parent_uri() const;
        StatusPtr parent() const;

        time_t ctime() const;
        time_t mtime() const;
        
        void ytime(int64_t t);
        int64_t ytime() const;

        // make sure object in memory is in sync
        // with data in datastore
        void sync();

        // name of status impl
        const char * impltype() const;

        bool has_completed() const;

        // precondition: progress == 100
        bool is_success() const;

        void failures(uint32_t count);
        uint32_t failures() const;

        // for debugging
        std::string serialize() const;

        // handle states, checkpoint for cancellation
        bool state( State state );
        State state() const;
        void checkpoint();
        static State str2state( const std::string & str );
        static std::string state2str( State state );
        static bool validStateTransition(State from, State to);

        // how many processes are currently working on 
        // this status?
        uint32_t concurrency() const;

        // worker core will automatically call starting()
        // before work begins and stopping() when work ends
        // both of which increment and decrement the
        // concurrency count()
        void starting();
        void stopping();
        
        ~Status();
      private:
        friend class StatusManager;
        friend class StatusCollection;
        Status( StatusImpl * impl );
        StatusImpl * impl;
    };
}
#endif
