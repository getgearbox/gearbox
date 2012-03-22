#ifndef GEARBOX_WORKER_TEST_SYNC_H
#define GEARBOX_WORKER_TEST_SYNC_H

#include <gearbox/worker/Worker.h>
#include <gearbox/core/Json.h>
#include <string>

namespace Gearbox {
    class WorkerTestSync: public Worker {
        typedef Worker super;
    public:
        WorkerTestSync(const std::string &config);
        response_t do_get_testsync_thing_v1( const Job & job, JobResponse & resp );
        response_t do_put_testsync_thing_v1( const Job & job, JobResponse & resp );
        response_t do_post_testsync_thing_v1( const Job & job, JobResponse & resp );
        response_t do_delete_testsync_thing_v1( const Job & job, JobResponse & resp );
    };
} // namespace

#endif
