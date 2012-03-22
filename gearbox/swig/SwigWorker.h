#ifndef GEARBOX_SWIG_WORKER_H
#define GEARBOX_SWIG_WORKER_H

#include <gearbox/worker/Worker.h>
#include <string>

class SwigWorker: public Gearbox::Worker {
public:
    SwigWorker(const std::string &config);
    virtual ~SwigWorker();
    using Worker::register_handler;
    void register_handler( const std::string & function_name );
    virtual response_t redispatcher( const Gearbox::Job & job, Gearbox::JobResponse & resp );
    virtual response_t do_dispatch( const Gearbox::Job & job, Gearbox::JobResponse & resp );
};

#endif
