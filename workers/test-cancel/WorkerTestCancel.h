#ifndef GEARBOX_WORKER_TEST_CANCEL_H
#define GEARBOX_WORKER_TEST_CANCEL_H

#include <gearbox/worker/Worker.h>
#include <gearbox/core/Json.h>
#include <string>

namespace Gearbox {
    class WorkerTestCancel: public Worker {
        typedef Worker super;
    public:
        WorkerTestCancel(const std::string &config);
        response_t do_post_testcancel_thing_v1( const Job & job, JobResponse & resp );
        response_t do_cancel_testcancel_thing_v1( const Job & job, JobResponse & resp );

        response_t do_post_testcancel_continuation_v1( const Job & job, JobResponse & resp );
        response_t do_run_testcancel_continuation_v1( const Job & job, JobResponse & resp );
        response_t do_finish_testcancel_continuation_v1( const Job & job, JobResponse & resp );
    };
} // namespace

#endif
