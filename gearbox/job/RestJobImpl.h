#ifndef GEARBOX_JOB_REST_JOB_IMPL
#define GEARBOX_JOB_REST_JOB_IMPL

#include <gearbox/job/JobImpl.h>

namespace Gearbox 
{
    class RestJobImpl : public JobImpl {
        typedef JobImpl super;
        
    public:
        RestJobImpl(const ConfigFile & cfg);
        RestJobImpl(const RestJobImpl &);
        virtual ~RestJobImpl();
        virtual JobImpl * clone() const;
        virtual JobResponse run() const;
    
        const char * impltype() const;

    protected:
        StatusPtr create_status(const std::string & status_uri = "") const;
    private:
    };
}
#endif
