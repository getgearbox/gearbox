#ifndef GEARBOX_WORKER_TEST_DELAY_H
#define GEARBOX_WORKER_TEST_DELAY_H

#include <gearbox/worker/Worker.h>
#include <gearbox/core/Json.h>
#include <string>

namespace Gearbox {
    class WorkerTestDelay: public Worker {
        typedef Worker super;
    public:
        WorkerTestDelay(const std::string &config);
        response_t do_get_testdelay_counter_v1( const Job & job, JobResponse & resp );
        response_t do_post_testdelay_counter_v1( const Job & job, JobResponse & resp );
        response_t do_delete_testdelay_counter_v1( const Job & job, JobResponse & resp );
        response_t do_increment_testdelay_counter_v1( const Job & job, JobResponse & resp );
    };
} // namespace

#endif
