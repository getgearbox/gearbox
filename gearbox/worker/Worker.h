// -*- Mode: C++ -*-

#ifndef GEARBOX_WORKER_H
#define GEARBOX_WORKER_H

#include <gearbox/core/Json.h>
#include <gearbox/core/ConfigFile.h>
#include <gearbox/job/JobManager.h>
#include <gearbox/job/StatusManager.h>
#include <string>
#include <map>

namespace Gearbox {

struct callback_args;

#define WORKER_REGISTER(CLASS, FUNC)                 \
    this->register_handler(                          \
        #FUNC,                                       \
        static_cast<Worker::handler_t>(&CLASS::FUNC) \
    )

    class WorkerStop : public std::runtime_error {
    public:
        WorkerStop() : std::runtime_error("Worker Stop") {}
    };

class Worker {
public:

    Worker(const std::string &config);
    virtual ~Worker();
    
    virtual void run();
    
    virtual void pre_request( const Job & job );
    virtual void post_request( const Job & job );
    
    enum response_t {
        WORKER_SUCCESS,
        WORKER_ERROR,
        WORKER_CONTINUE,
        WORKER_RETRY
    };
    
    typedef  Worker::response_t (Worker::*handler_t)(const Job & job, JobResponse & response);

    void afterwards(const Job & job, int delay=0);
    void afterwards(const Job & job, const char * name, int delay=0);
    void afterwards(const Job & job, const std::string & name, int delay=0);
    void clear_afterwards();

    Gearbox::JobManager    & job_manager();
    Gearbox::StatusManager & status_manager();
    const ConfigFile & cfg() const;
    void max_requests(int max);
    int request_count() const;

protected:
    
    virtual void
    register_handler(
        const std::string & name,
        handler_t handler
    );
    
    virtual void
    deregister_handler(
        const std::string & name
    );

private:
    class Private;
    Private *impl;
};

} // namespace

#endif // GEARBOX_WORKER_H
