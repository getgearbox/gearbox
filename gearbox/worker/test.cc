// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/worker/Worker.h>
#include <gearbox/core/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <string>
using std::string;


class TestWorker : public Worker {
public:
    TestWorker();
    response_t hello ( const Json & in, Json & out );
    response_t goodbye ( const Json & in, Json & out );
};

TestWorker::TestWorker() : Worker("test.cf") {
    this->register_handler("hello",  static_cast<handler_t>(&TestWorker::hello));
    this->register_handler("goodbye",  static_cast<handler_t>(&TestWorker::goodbye));
}

Worker::response_t 
TestWorker::goodbye ( const Json & in, Json & out ) {
    this->deregister_handler("goodbye");
    this->deregister_handler("hello");

    std::ostringstream outData;
    outData <<  "Goodbye " << in["params"]["name"].as<string>();
    out["result"] = outData.str();

    return WORKER_SUCCESS;
}

Worker::response_t 
TestWorker::hello ( const Json & in, Json & out ) {
    std::ostringstream outData;
    outData <<  "Hello " << in["params"]["name"].as<string>();
    
    out["result"] = outData.str();

    return WORKER_SUCCESS;
}

int main () {
    log4cxx::PropertyConfigurator::configure( "../tools/logging.conf" );

    try {
        TestWorker w;
        w.run();
    }
    catch ( const std::exception & err ) {
        _FATAL(err.what());
        return 1;
    }

}

