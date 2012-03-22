#ifndef GEARBOX_WORKER_TEST_BASIC_H
#define GEARBOX_WORKER_TEST_BASIC_H

#include <gearbox/worker/Worker.h>
#include <gearbox/core/Json.h>
#include <string>

namespace Gearbox {
    class WorkerTestBasic: public Worker {
        typedef Worker super;
    public:
        WorkerTestBasic(const std::string &config);
        response_t do_get_testbasic_thing_v1( const Job & job, JobResponse & resp );
        response_t do_put_testbasic_thing_v1( const Job & job, JobResponse & resp );
        response_t do_post_testbasic_thing_v1( const Job & job, JobResponse & resp );
        response_t do_delete_testbasic_thing_v1( const Job & job, JobResponse & resp );
    };
} // namespace

#endif
