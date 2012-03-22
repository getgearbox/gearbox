#ifndef GEARBOX_JOB_MANAGER
#define GEARBOX_JOB_MANAGER

#include <gearbox/job/Job.h>
#include <gearbox/core/ConfigFile.h>
#include <gearbox/core/Json.h>
#include <gearbox/core/HttpClient.h>
#include <gearbox/core/Uri.h>

#include <string>
#include <vector>

namespace Gearbox {
    
    typedef boost::shared_ptr<Job> JobPtr;
    typedef std::vector< std::vector<JobPtr> > JobQueue;

    class JobImpl;
    class JsonSchema;

    class JobManager {

    public:
        JobManager(const ConfigFile & cfg);
        JobManager(const JobImpl * j);
        JobManager(const JobManager & copy);
        JobManager & operator=(const JobManager & copy);
        ~JobManager();

        void parent_uri( const std::string & parent_uri );
        void base_uri( const std::string & base_uri );
        
        void cfg( const ConfigFile & cfg);

        bool known_job_name( const std::string & name ) const;
        
        JobPtr job(const std::string & name) const;
        JobPtr job(HttpClient::method_t method, const Uri & uri) const;

        // create job from envelope
        JobPtr job(const std::string & name, const std::string & envelope) const;
        JobPtr job(const std::string & name, const Json & envelope) const;

        // schedule a job to be run some number of seconds from "now"
        void delay(const Job & job, int seconds ) const;
        void retry(const Job & job, int max_delay=300, int max_jitter=15);

        static std::string gen_id(const std::string & prefix);

        JobQueue    job_queue( const Json & job_config ) const;
        static void job_queue_run( JobQueue & jobs );

        // template wrapper to apply attributes to jobs returned from job_queue:
        // example: JobManager::job_queue_apply(jobs, &Job::content, my_content);
        template<typename T>
        static void 
        job_queue_apply(
            JobQueue & jobs,
            Job & (Job::*func)(const T &),
            const T & data
        ) {
            unsigned int imax = jobs.size();
            for( unsigned int i=0; i < imax; i++ ) {
                unsigned int jmax = jobs[i].size();
                for( unsigned int j=0; j < jmax; j++ ) {
                    Job & job = *jobs[i][j];
                    (job.*func)(data);
                }
            }
        }
        
        static JsonSchema * getSchema(const JobImpl * ji, const ConfigFile & cfg );
        static void requireSchemas(bool);

        private:
        static bool require_schemas;
        class Private;
        Private * impl;
    };
}

#endif
