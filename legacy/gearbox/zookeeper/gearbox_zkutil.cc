// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include "ZooKeeper.h"

#include <zookeeper/zookeeper.h> /* for zoo_set_debug_level */

#include <gearbox/core/logger.h>
#include <gearbox/core/Json.h>
#include <gearbox/zookeeper/ZooKeeperClack.h>

#include <log4cxx/basicconfigurator.h>
#include <log4cxx/consoleappender.h>

#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio/ip/host_name.hpp>


#include <string>
#include <iostream>
#include <vector>

#include <unistd.h>

namespace opt = boost::program_options;
using namespace Gearbox;

typedef std::vector<std::string> strvec_t;

namespace
{
    void del_recursively(ZooKeeper& zk, const std::string& path)
    {
        // /zookeper* is special and we do not want to be deleting it

        if( path.find("/zookeeper") == 0 ) {
            _WARN("Skipping attempted delete of " << path);
            return;
        }

        strvec_t children;
        zk.children(path, children);

        for (strvec_t::const_iterator it = children.begin(); it != children.end(); ++it) {
            std::string path2 = ZooKeeper::join_paths(path, *it);
            del_recursively(zk, path2);
        }

        zk.del(path);
    }

    void ls_recursively(ZooKeeper& zk, const std::string& path, bool long_fmt, const std::string& indent = "")
    {
        strvec_t children;
        zk.children(path, children);

        for (strvec_t::const_iterator it = children.begin(); it != children.end(); ++it) {
            if (long_fmt) {
                std::cout << ZooKeeper::join_paths(path, *it) << std::endl;
            }
            else {
                std::cout << indent << *it << std::endl;
            }
            std::string path2 = ZooKeeper::join_paths(path, *it);
            ls_recursively(zk, path2, long_fmt, indent + " ");
        }

    }
}

int main(int argc, char **argv)
{
    const std::string usage = "\
gearbox_zkutil [options] get PATH\n\
gearbox_zkutil [options] set [-p] PATH VALUE\n\
gearbox_zkutil [options] set [-p] PATH < VALUE_FILE\n\
gearbox_zkutil [options] ls [-R [-l]] PATH\n\
gearbox_zkutil [options] rm [-R] PATH\n\
gearbox_zkutil [options] stat PATH\n\
gearbox_zkutil [options] ack_success DATUM_PATH TX_ID\n\
gearbox_zkutil [options] ack_fail DATUM_PATH TX_ID MESSAGE\n\
gearbox_zkutil [options] ack_fail DATUM_PATH TX_ID < MESSAGE_FILE\n\
gearbox_zkutil [options] ack_txId DATUM_PATH \n\
\n";

    opt::options_description options("Options");

    try {
        options.add_options()
            ("help,h", "produce this help message")
            ("zookeepers,z", opt::value<std::string>()->default_value("localhost:2181"), "List of ZK servers and ports")
            ("timeout,t", opt::value<int>()->default_value(10000), "ZK receive timeout in milliseconds")
            ("parents,p", "create missing parents with empty node values")
            ("recursive,R", "list (ls) or remove (rm) PATH and all its children [*BE CAREFUL*]")
            ("verbose,v", "enable verbose logging from ZK")
            ("long,l", "with ls -R, show a full path for each znode")
            ;

        opt::variables_map args;
        {
            // The option to receive positional args is hidden from the help output
            opt::options_description options2;
            options2.add(options).add_options()("command", opt::value<strvec_t>(), "Command and its arguments");

            opt::positional_options_description positional;
            positional.add("command", -1);

            opt::store(opt::command_line_parser(argc, argv).options(options2).positional(positional).run(), args);
            opt::notify(args);
        }

        if (args.count("help")) {
            std::cout << usage << options << std::endl;
            return 0;
        }

        if (!args.count("command") || args["command"].as<strvec_t>().empty()) {
            throw opt::error("Please specify a command");
        }
        const strvec_t& cmd = args["command"].as<strvec_t>();

        // make log4cxx shut up about its missing config file
        log4cxx::BasicConfigurator::configure();

        // if there is a ConsoleAppender, make it log to stderr
        {
            log4cxx::AppenderList appenders = log4cxx::Logger::getRootLogger()->getAllAppenders();
            for (log4cxx::AppenderList::iterator it = appenders.begin(); it != appenders.end(); ++it) {
                log4cxx::ConsoleAppender *cap = dynamic_cast<log4cxx::ConsoleAppender *>(&**it);
                if (cap != NULL) {
                    cap->setTarget(log4cxx::ConsoleAppender::getSystemErr());
                    // Necessary for setTarget to have an effect.
                    // The Pool is not used, it could even be *NULL.
                    log4cxx::helpers::Pool p;
                    cap->activateOptions(p);
                }
            }
        }

        if (args.count("verbose")) {
            log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getDebug());
        }
        else {
            log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getError());
            // suppress the overly verbose logging in the ZK client
            zoo_set_debug_level(ZOO_LOG_LEVEL_WARN);
        }



        // check if the comand matches and enforce arg count if it does
        class cmd_is_t {
            const strvec_t& cmd_;
        public:
            cmd_is_t(const strvec_t& cmd) : cmd_(cmd) {}

            bool operator () (const std::string& c, size_t req_args, size_t opt_args = 0)
            {
                if (cmd_.front() == c) {
                    size_t nargs = cmd_.size() - 1;
                    if (nargs < req_args) {
                        std::ostringstream err;
                        err << "Too few args for command '" << c
                            << "', need at least " << (unsigned)req_args;
                        throw opt::error( err.str() );
                    }
                    if (nargs > req_args + opt_args) {
                        std::ostringstream err;
                        err << "Too many args for command '" << c
                            << "', need at most " << (unsigned)(req_args+opt_args);
                        throw opt::error( err.str() );
                    }
                    return true;
                }
                else {
                    return false;
                }
            }
        } cmd_is(cmd);

        if (cmd_is("ack_success", 2)) {
            const std::string datumPath = cmd.at(1);
            size_t found=datumPath.find_last_of("/\\");
            const std::string path = datumPath.substr(0,found);
            const std::string datumKey = datumPath.substr(found+1);

            const std::string& datumTxIdString = cmd.at(2);

            int64_t datumTxId = boost::lexical_cast<int64_t>(datumTxIdString);
            std::string hostName = boost::asio::ip::host_name();
            DatumConsumer dc(args["zookeepers"].as<std::string>(),
                             args["timeout"].as<int>(),
                             hostName,
                             path);
            dc.acknowledgeSuccess(datumKey, datumTxId);
        }
        else if (cmd_is("ack_fail", 2, 1)) {
            const std::string datumPath = cmd.at(1);
            size_t found=datumPath.find_last_of("/\\");
            const std::string path = datumPath.substr(0,found);
            const std::string datumKey = datumPath.substr(found+1);

            const std::string& datumTxIdString = cmd.at(2);

            std::string msg;
            if (cmd.size() == 4) {
                msg = cmd.at(3);
            }
            else {
                // this is probably not what the user intended, so warn them
                if (isatty(0)) {
                    std::cerr << "Please type in the desired value, or Ctrl-C yourself out (stdin is a tty)" << std::endl;
                }

                // load the value from stdin
                std::stringstream ss;
                ss << std::cin.rdbuf();
                msg = ss.str();
            }
            int64_t datumTxId = boost::lexical_cast<int64_t>(datumTxIdString);
            std::string hostName = boost::asio::ip::host_name();
            DatumConsumer dc(args["zookeepers"].as<std::string>(),
                             args["timeout"].as<int>(),
                             hostName,
                             path);
            dc.acknowledgeFailure(datumKey, datumTxId, msg);

        } else{
            ZooKeeper zk(args["zookeepers"].as<std::string>(), args["timeout"].as<int>());

            if (cmd_is("get", 1)) {
                const std::string& path = cmd.at(1);

                std::string data;
                zk.get(path, data);

                // be terminal-friendly, add a trailing line-break if missing
                if (data.empty() || data.at(data.size()-1) != '\n') {
                    data += "\n";
                }

                std::cout << data;
            }
            else if (cmd_is("set", 1, 1)) {
                const std::string& path = cmd.at(1);

                std::string data;
                if (cmd.size() == 3) {
                    data = cmd.at(2);
                }
                else {
                    // this is probably not what the user intended, so warn them
                    if (isatty(0)) {
                        std::cerr << "Please type in the desired value, or Ctrl-C yourself out (stdin is a tty)" << std::endl;
                    }

                    // load the value from stdin
                    std::stringstream ss;
                    ss << std::cin.rdbuf();
                    data = ss.str();
                }

                if (args.count("parents")) {
                    // create the parent path if necessary
                    std::string parent = ZooKeeper::normalize_path(path);
                    size_t slash_pos = parent.rfind('/');
                    if (slash_pos != std::string::npos && slash_pos != 0) {
                        parent.resize(slash_pos);
                        if (!zk.exists(parent)) {
                            zk.createFullPath(parent);
                        }
                    }
                }

                if (!zk.exists(path)) {
                    zk.create(path, data);
                }
                else {
                    zk.set(path, data);
                }
            }
            else if (cmd_is("rm", 1)) {
                const std::string& path = cmd.at(1);
                if (args.count("recursive")) {
                    del_recursively(zk, path);
                }
                else {
                    zk.del(path);
                }
            }
            else if (cmd_is("ls", 1)) {
                const std::string& path = cmd.at(1);

                if (args.count("recursive")) {
                    ls_recursively(zk, path, args.count("long"));
                }
                else {
                    strvec_t children;
                    zk.children(path, children);

                    for (strvec_t::const_iterator it = children.begin(); it != children.end(); ++it) {
                        std::cout << *it << std::endl;
                    }
                }
            }
            else if (cmd_is("stat", 1)) {
                const std::string& path = cmd.at(1);
                ZooKeeper::Stat s = zk.stat(path);
                std::cout << "version = " <<  s.version() << std::endl;
                std::cout << "modTxId = " <<  s.modTxId() << std::endl;
                std::cout << "mtime = " <<  s.mtime() << std::endl;
                std::cout << "ctime = " <<  s.ctime() << std::endl;
                std::cout << "dataLength = " <<  s.dataLength() << std::endl;
                std::cout << "numChildren = " <<  s.numChildren() << std::endl;
            }
            else if (cmd_is("ack_txId", 1)) {
                std::string hostName = boost::asio::ip::host_name();
                const std::string datumPath = cmd.at(1);
                size_t found=datumPath.find_last_of("/\\");
                std::string path = datumPath.substr(0,found);
                const std::string datumKey = datumPath.substr(found+1);

                path.append("/ack/").append(datumKey).append("/").append(hostName);
                std::string data;
                zk.get(path, data);

                Json ackJson;
                ackJson.parse(data);
                const int64_t jsonTxId =  ackJson["txId"].as<int64_t>();
                std::cout << jsonTxId << std::endl;
            }
            else {
                throw opt::error( "Unrecognized command '" + cmd.front() + "'");
            }
        }
        return 0;
    }
    catch (const opt::error& err) {
        // opt::error is thrown for usage errors, by the options parser, or by our code
        std::cerr << usage << options << std::endl << "Usage error: " << err.what() << std::endl;
        return 1;
    }
    catch (const std::exception& exc) {
        std::cerr << "ERROR: " << exc.what() << std::endl;
        return 2;
    }
}

/* Style Settings: 
 *
 * Local Variables: 
 *   mode: C++ 
 *   indent-tabs-mode: nil 
 *   c-basic-offset: 4 
 *   c-set-style: "knr"
 * End:
 * vim:set sw=4 ts=4 sts=4 cindent expandtab cinoptions=(0:
 */

