// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/core/ConfigFile.h>
#include <log4cxx/propertyconfigurator.h>
#include <boost/asio/ip/host_name.hpp>
#include <sstream>
#include <stdexcept>

namespace Gearbox {
    void log_init( const std::string & configFile) {
        ConfigFile cfg(configFile);
        
        std::string log_conf_file = cfg.get_string("log", "config_file");
        log4cxx::PropertyConfigurator::configure( log_conf_file );
        
        std::stringstream converter;
        converter << getpid();
        log4cxx::MDC::put("pid", converter.str());

        std::string hostname = boost::asio::ip::host_name();
        log4cxx::MDC::put("hostname", hostname);
    }
}
