// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>
#include <gearbox/core/REST.h>
#include <gearbox/core/logger.h>
#include <gearbox/core/SystemPair.h>
#include <gearbox/core/util.h>
using namespace Gearbox;

int main() {
    chdir(TESTDIR);
    TEST_START(54);
    
    log_init("./unit.conf");
    std::string uri;
    run("./start-httpd", uri);
    _DEBUG("uri: " << uri);

    SystemPair::cmd init, dest;
    SystemPair sp(
        init << "true",
        dest << "./stop-httpd"
    );

    Json response;
    long result;

    NOTHROW( result = HEAD( uri + "/json/empty" ) );
    IS( result, 200 );

    NOTHROW( result = GET( uri + "/json/empty", response ) );
    IS( result, 200 );
    OK( response.empty() );

    NOTHROW( result = GET( uri + "/json/foo", response ) );
    IS( result, 200 );
    OK( response.hasKey("foo") );
    IS( response["foo"].as<int>(), 1 );

    NOTHROW( result = DELETE( uri + "/json/bar", response ) );
    IS( result, 200 );
    OK( response.hasKey( "bar") );

    Json data;
    data["foo"] = 1;
    NOTHROW( result = POST( uri + "/json/bar", data, response ) );
    IS( result, 200 );
    OK( response.hasKey( "bar") );
    IS( slurp("./http/requests/4"), slurp("./http/expect/rest-post") );

    NOTHROW( result = PUT( uri + "/json/bar", data, response ) );
    IS( result, 200 );
    OK( response.hasKey( "bar") );
    IS( slurp("./http/requests/5"), slurp("./http/expect/rest-put") );

    THROWS ( GET( uri + "/error/400", response ), "BAD_REQUEST [400]: Failed to GET " + uri + "/error/400 got: invalid error response");

    THROWS ( GET( uri + "/error/400/hate", response ), "BAD_REQUEST [400]: Failed to GET " + uri + "/error/400/hate got: I do not like you");

    // // now testing with headers

    Headers headers;
    headers["y-rid"] = "abcdef";
    NOTHROW( REST::global_headers(headers) );
    
    NOTHROW( result = HEAD( uri + "/json/empty" ) );
    IS( result, 200 );
    IS( slurp("./http/requests/8"), slurp("./http/expect/rest-header") );
    
    NOTHROW( REST::add_global_header("y-nimbus-sig", "asdf12-asde") );

    NOTHROW( result = HEAD( uri + "/json/empty" ) );
    IS( result, 200 );
    IS( slurp("./http/requests/9"), slurp("./http/expect/rest-another-header") );

    NOTHROW( REST::del_global_header("y-nimbus-sig") );
    NOTHROW( result = HEAD( uri + "/json/empty" ) );
    IS( result, 200 );
    IS( slurp("./http/requests/10"), slurp("./http/expect/rest-remove-header") );
    
    NOTHROW( REST::del_global_header("y-rid") );

    NOTHROW( result = HEAD( uri + "/json/empty" ) );
    IS( result, 200 );
    IS( slurp("./http/requests/11"), slurp("./http/expect/rest-no-header") );

    NOTHROW( result = GET( uri + "/json/bar", headers, response ) );
    IS( result, 200 );
    OK( response.hasKey("bar") );
    IS( slurp("./http/requests/12"), slurp("./http/expect/rest-get-header") );

    NOTHROW( result = DELETE( uri + "/json/bar", headers, response ) );
    IS( result, 200 );
    OK( response.hasKey("bar") );
    IS( slurp("./http/requests/13"), slurp("./http/expect/rest-delete-header") );

    NOTHROW( result = POST( uri + "/json/bar", data, headers, response ) );
    IS( result, 200 );
    OK( response.hasKey("bar") );
    IS( slurp("./http/requests/14"), slurp("./http/expect/rest-post-header") );

    NOTHROW( result = PUT( uri + "/json/bar", data, headers, response ) );
    IS( result, 200 );
    OK( response.hasKey("bar") );
    IS( slurp("./http/requests/15"), slurp("./http/expect/rest-put-header") );

    TEST_END;
}
