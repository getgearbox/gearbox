#ifndef GEARBOX_JSON_STATUS_IMPL_H_
#define GEARBOX_JSON_STATUS_IMPL_H_

#include <gearbox/job/StatusImplV1.h>
#include <gearbox/job/Job.h>

namespace Gearbox {
    class JsonStatusImpl : public StatusImplV1 {
        typedef StatusImplV1 super;
    public:
        JsonStatusImpl(const ConfigFile & cfg);
        JsonStatusImpl(const JsonStatusImpl &);
        virtual ~JsonStatusImpl();
        virtual StatusImpl * clone() const;
        virtual void json(const Json & json);
        virtual void sync();
        virtual void insert();
        virtual void load();
        virtual const char * impltype() const;

        virtual void   on(Status::Event e, const Job & job);
        virtual JobPtr on(Status::Event e, const JobManager & jm) const;

        using StatusImplV1::concurrency;
        virtual void starting();
        virtual void stopping();

    private:
        JsonStatusImpl & operator=(const JsonStatusImpl &); // unimplemented
        Json data;
    };
}

#endif
