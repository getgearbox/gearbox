#ifndef GEARBOX_JOB_STATUS_IMPL_H_
#define GEARBOX_JOB_STATUS_IMPL_H_

#include <gearbox/job/JsonStatusImpl.h>
#include <gearbox/job/Job.h>

namespace Gearbox {
    class JobStatusImpl : public JsonStatusImpl {
        typedef JsonStatusImpl super;
    public:
        JobStatusImpl(const ConfigFile & cfg);
        JobStatusImpl(const JobStatusImpl &);
        virtual ~JobStatusImpl();
        virtual StatusImpl * clone() const;
        virtual void job(const Job & job);
        virtual void sync();
        virtual void insert();
        virtual void load();
        virtual const char * impltype() const;

        virtual void   on(Status::Event e, const Job & job);
        virtual JobPtr on(Status::Event e, const JobManager & jm) const;

    private:
        JobStatusImpl & operator=(const JobStatusImpl &); // unimplemented
        JobPtr status_job;
    };
}

#endif
