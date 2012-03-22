// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/job/StatusCollectionImplV1.h>
namespace Gearbox {
    StatusCollectionImplV1::StatusCollectionImplV1(const ConfigFile & cfg) : super(cfg) {};
    int StatusCollectionImplV1::version() { return 1; }
}
