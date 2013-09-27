// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>
#include <gearbox/core/Json.h>
using namespace Gearbox;
#include <iostream>
#include <libgen.h>
#include <unistd.h>

using namespace std;

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
namespace bfs=boost::filesystem;

int main(int argc, char *argv[]) {
    TEST_START(5)

    string basedir = string(dirname(argv[0])) + "/../";
    chdir(basedir.c_str());

    Json s;
    bfs::path schemadir("../../gearbox/schemas");
    bfs::directory_iterator di( schemadir );    
    bfs::directory_iterator end;
    for (; di != end; ++di ) {
        if( !bfs::is_directory(*di) ) {
            cout << "# schema: " << di->path().string() << endl;
            NOTHROW( s.parseFile( di->path().string() ) );
        }
    }
    TEST_END;
}
