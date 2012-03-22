// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/core/logger.h>
#include "TestPlugin.h"

struct Goodbye : public TestPlugin {
    Goodbye() {
        _DEBUG("Goodbye constructor");
    }
    ~Goodbye() {
        _DEBUG("Goodbye destructor");
    }
    
    std::string get() {
        return "goodbye";
    }
    
    void set( std::string & data ) {
        data = "goodbye";
    }

};

// plugin interface
extern "C" {
    void * goodbye_new();
    void goodbye_delete(void *);
}

void * goodbye_new() {
    return new Goodbye();
}

void goodbye_delete(void *ptr) {
    delete (Goodbye*)ptr;
}


