#ifndef GEARBOX_STATUS_COLLECTION_IMPL_V1_H
#define GEARBOX_STATUS_COLLECTION_IMPL_V1_H

#include <gearbox/job/StatusCollectionImpl.h>

namespace Gearbox {

    class StatusCollectionImplV1 : public StatusCollectionImpl {
        typedef StatusCollectionImpl super;
    public:
        StatusCollectionImplV1(const ConfigFile & c);
        int version();
    };
}

#endif
