// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>
#include <gearbox/core/Json.h>
using namespace Gearbox;
#include <iostream>
using namespace std;

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
namespace bfs=boost::filesystem;

int main() {
    TEST_START(5)
    
    Json s;
    bfs::path schemadir("../../gearbox/schemas");
    bfs::directory_iterator di( schemadir );    
    bfs::directory_iterator end;
    for (; di != end; ++di ) {
        if( !bfs::is_directory(*di) ) {
            cout << "# schema: " << di->string() << endl;
            NOTHROW( s.parseFile( di->string() ) );
        }
    }
    TEST_END;
}
