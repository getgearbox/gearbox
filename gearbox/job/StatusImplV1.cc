// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/job/StatusImplV1.h>
namespace Gearbox {
    StatusImplV1::StatusImplV1(const ConfigFile & cfg) : super(cfg) {};
    int StatusImplV1::version() const { return 1; }
}
