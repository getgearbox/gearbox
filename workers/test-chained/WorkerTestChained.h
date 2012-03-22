#ifndef GEARBOX_WORKER_TEST_CHAINED_H
#define GEARBOX_WORKER_TEST_CHAINED_H

#include <gearbox/worker/Worker.h>
#include <gearbox/core/Json.h>
#include <string>

namespace Gearbox {
    class WorkerTestChained: public Worker {
        typedef Worker super;
    public:
        WorkerTestChained(const std::string &config);
        response_t do_get_testchained_hello_v1( const Job & job, JobResponse & resp );
        response_t do_get_internal_hello1_v1( const Job & job, JobResponse & resp );
        response_t do_post_testchained_hello2_v1( const Job & job, JobResponse & resp );

        response_t do_get_testchained_goodbye_v1( const Job & job, JobResponse & resp );
        response_t do_post_testchained_goodbye_v1( const Job & job, JobResponse & resp );
        response_t do_append_internal_goodbye1_v1( const Job & job, JobResponse & resp );
        response_t do_append_internal_goodbye2_v1( const Job & job, JobResponse & resp );

        response_t do_get_testchained_thing_v1( const Job & job, JobResponse & resp );
        response_t do_post_testchained_thing_v1( const Job & job, JobResponse & resp );
        response_t do_reg_internal_service1_v1( const Job & job, JobResponse & resp );
        response_t do_post_testchained_service2_v1( const Job & job, JobResponse & resp );

        response_t do_delete_testchained_thing_v1( const Job & job, JobResponse & resp );
        response_t do_unreg_internal_service1_v1( const Job & job, JobResponse & resp );
        response_t do_delete_testchained_service2_v1( const Job & job, JobResponse & resp );
    };
} // namespace

#endif
