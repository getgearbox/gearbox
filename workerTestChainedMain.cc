// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include "WorkerTestChained.h"
#include <gearbox/core/logger.h>

#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
namespace opt = boost::program_options;
using opt::value;

#include <string>
#include <iostream>

int main (int argc, char *argv[]) {

    try {
        std::string config;
        opt::options_description options("Options",100);
        options.add_options()
            ("help,h", "this help message")
            ("config,f", value<std::string>(&config)->default_value(SYSCONFDIR "/gearbox/test-chained.conf"), "specify path to config file")
            ;
        
        opt::variables_map args;
        try {
            opt::store(parse_command_line(argc, argv, options), args);
            opt::notify(args);
        }
        catch ( const opt::error & err ) {
            std::cerr << err.what() << std::endl << options << std::endl;
            return 1;
        }
        
        if (args.count("help")) {
            std::cout << options << std::endl;
            return 0;
        }
        
        Gearbox::WorkerTestChained w(config);
        w.run();
    }
    catch ( const std::exception & err ) {
        _FATAL(err.what());
        return 1;
    }
    return 0;
}

