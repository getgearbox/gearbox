#ifndef GEARBOX_WORKER_TEST_AGENTS_H
#define GEARBOX_WORKER_TEST_AGENTS_H

#include <gearbox/worker/Worker.h>
#include <gearbox/core/Json.h>
#include <string>

namespace Gearbox {
    class WorkerTestAgents: public Worker {
        typedef Worker super;
    public:
        WorkerTestAgents(const std::string &config);
        response_t thing_handler( const Job & job, JobResponse & resp );
        response_t dummy_handler( const Job & job, JobResponse & resp );
    };
} // namespace

#endif
