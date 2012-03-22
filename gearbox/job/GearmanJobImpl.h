#ifndef GEARBOX_JOB_GEARMAN_JOB_IMPL
#define GEARBOX_JOB_GEARMAN_JOB_IMPL

#include <gearbox/job/JobImpl.h>

struct gearman_client_st; // foward declare
namespace Gearbox 
{
    class GearmanJobImpl : public JobImpl {
        typedef JobImpl super;
        
    public:
        GearmanJobImpl(const ConfigFile & cfg);
        GearmanJobImpl(const GearmanJobImpl &);
        virtual JobImpl * clone() const;
        void init();
        virtual ~GearmanJobImpl();
        virtual JobResponse run() const;
    
        const char * impltype() const;

    protected:
        virtual void check_connection() const;
        virtual StatusPtr create_status() const;
        virtual JobResponse run_sync(Json & job_data) const;
        virtual JobResponse run_async(Json & job_data) const;
        
    private:
        std::string gm_host;
        int gm_port;
        gearman_client_st* gm;
    };
}
#endif
