// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <workers/gearbox/WorkerGearbox.h>

#define LOGCAT "gearbox.worker.gearbox"
#include <gearbox/core/logger.h>

#include <gearbox/store/dbconn.h>
using namespace Gearbox::Database;
using namespace Gearbox;
using namespace soci;

#include <iostream>
#include <stdexcept>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
namespace opt = boost::program_options;
using opt::value;
using std::string;
using namespace soci;

int main (int argc, char *argv[]) {

    try {
        std::string config;
        int max_requests;
        opt::options_description options("Options", 100);
        options.add_options()
            ("help,h", "this help message")
            ("config,f", value<string>(&config)->default_value(SYSCONFDIR "/gearbox/gearbox.conf"), "specify path to config file")
            ("no-sync,a", "This worker should only work on asynchronous jobs")
            ("no-async,s", "This worker should only work on synchronous jobs")
            ("max-requests,r", value<int>(&max_requests)->default_value(0), "maximum requests to handle before exiting")
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
        
        WorkerGearbox w(config, !args.count("no-sync"), !args.count("no-async"));
        w.max_requests(max_requests);
        w.run();
    }
    catch ( const soci::soci_error & err ) {
        _FATAL("SOCI Error: " << err.what() << " from statement:\n" << lastsql());
        dbrollback();
        return 1;
    }
    catch ( const std::exception & err ) {
        _FATAL(err.what());
        return 1;
    }
    return 0;
}
