#ifndef GEARBOX_WORKER_GEARBOX_H
#define GEARBOX_WORKER_GEARBOX_H

#include <gearbox/worker/Worker.h>
#include <gearbox/core/Json.h>
#include <string>

namespace Gearbox {
    class WorkerGearbox: public Worker {
    public:
        WorkerGearbox(const std::string &config, bool withSync=true, bool withAsync=true);
        response_t do_put_delay_job_v1( const Job & job, JobResponse & resp );
        response_t do_get_global_status_v1( const Job & job, JobResponse & resp );
        response_t do_post_global_status_v1( const Job & job, JobResponse & resp );
        response_t do_create_global_status_v1( const Job & job, JobResponse & resp );
        response_t do_update_global_status_v1( const Job & job, JobResponse & resp );
        
        response_t do_stop_global_status_v1( const Job & job, JobResponse & resp );
        response_t do_cancelwatch_global_status_v1( const Job & job, JobResponse & resp );
        response_t do_pollstate_global_status_v1( const Job & job, JobResponse & resp );

        // for agent dispatch
        response_t do_run_global_agents_v1( const Job & job, JobResponse & resp );
        response_t do_runlevel_global_agents_v1( const Job & job, JobResponse & resp );
        response_t do_decrement_global_counter_v1( const Job & job, JobResponse & resp );        

    private:
        void poke_delay_daemon();
        std::string make_counter(unsigned int start);

    };
} // namespace

#endif // GEARBOX_WORKER_GEARBOX_H
