// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/core/Json.h>
#include <gearbox/core/JsonSchema.h>
using namespace Gearbox;

#include <string>
#include <iostream>
#include <fstream>

#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

namespace opt = boost::program_options;
#include <unistd.h>

int main(int argc, char **argv)
{
    opt::options_description options("Options");

    try {
        options.add_options()
            ("help,h", "produce this help message")
            ("schema,s", opt::value<std::string>(), "Validate against the specified schema")
            ("pretty,p", "pretty-print (indent) the output")
            ("data,d", "operate on the embedded \"data\" json string")
            ;

        opt::variables_map args;
        {
            // The option to receive positional args is hidden from the help output
            opt::options_description options2;
            options2.add(options).add_options()("input", opt::value<std::string>(), "Input file");

            opt::positional_options_description positional;
            positional.add("input", -1);

            opt::store(opt::command_line_parser(argc, argv).options(options2).positional(positional).run(), args);
            opt::notify(args);
        }

        if (args.count("help")) {
            std::cout << options << std::endl;
            return 0;
        }

        std::string input;

        if (args.count("input")) {
            input = args["input"].as<std::string>();
        }
        else {
            input = "/dev/stdin";

            // this is probably not what the user intended, so warn them
            if (isatty(0)) {
                std::cerr << "Please type in JSON or hit Ctrl-C (stdin is a tty)" << std::endl;
            }
        }

        Json json;
        json.parseFile(input);

        if (args.count("data")) {
            Json tmp;
            tmp.parse(json["data"].as<std::string>());
            json = tmp;
        }

        if (args.count("schema")) {
            JsonSchema schema;
            schema.parseFile(args["schema"].as<std::string>());

            if (!json.validate(&schema)) {
                std::cerr << "ERROR: Schema validation failed" << std::endl;
                return 2;
            }
        }

        bool pretty = args.count("pretty");
        std::string res = json.serialize(pretty);
        if (!res.empty() && res.at(res.size()-1) != '\n') {
            res += "\n";
        }
        std::cout << res;

        return 0;
    }
    catch (const opt::error& err) {
        // opt::error is thrown for usage errors, by the options parser, or by our code
        std::cerr << options << std::endl << "Usage error: " << err.what() << std::endl;
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
