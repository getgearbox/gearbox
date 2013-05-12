// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>
#include <gearbox/core/Json.h>
#include <iostream>
using namespace std;
using namespace Gearbox;

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
namespace bfs=boost::filesystem;

int main() {
    chdir(TESTDIR);
    TEST_START(2)
    
    Json s;
    bfs::path schemadir("../../worker/schemas");
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
