// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include "TestPlugin.h"
#include <gearbox/core/logger.h>

TestPlugin::TestPlugin() {
    _DEBUG("TestPlugin constructed");
}

TestPlugin::~TestPlugin() {
    _DEBUG("TestPlugin destructed");
}
