// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/core/logger.h>
#include "TestPlugin.h"

struct Hello : public TestPlugin {
    Hello() {
        _DEBUG("Hello constructor");
    }
    ~Hello() {
        _DEBUG("Hello destructor");
    }
    
    std::string get() {
        return "hello";
    }
    
    void set( std::string & data ) {
        data = "hello";
    }

};

// plugin interface
extern "C" {
    void * hello_new();
    void hello_delete(void *);
    std::string greet(const std::string & name);
}

void * hello_new() {
    return new Hello();
}

void hello_delete(void *ptr) {
    delete (Hello*)ptr;
}

std::string greet( const std::string & name ) {
    return "Hello " + name + "!";
}


