#ifndef GEARBOX_STATUS_IMPL_V1_H
#define GEARBOX_STATUS_IMPL_V1_H

#include <vector>
#include <string>
#include <time.h>
#include <gearbox/job/StatusImpl.h>
#include <gearbox/core/ConfigFile.h>

namespace Gearbox {

    class StatusImplV1 : public StatusImpl {
        typedef StatusImpl super;
    public:
        StatusImplV1(const ConfigFile & cfg);
        virtual int version() const;
    };
}

#endif
