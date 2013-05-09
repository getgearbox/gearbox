// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>
#include <gearbox/core/HttpClient.h>
#include <gearbox/core/logger.h>
#include <gearbox/core/SystemPair.h>
#include <gearbox/core/TempFile.h>
#include <gearbox/core/util.h>
using namespace Gearbox;

#include <boost/filesystem/path.hpp>
namespace bfs=boost::filesystem;

int main() {
    chdir(TESTDIR);
    TEST_START(53);
    
    log_init("./unit.conf");
    
    std::string uri;
    run("./start-httpd", uri);
    _DEBUG("uri: " << uri);
    
    SystemPair::cmd init, dest;
    SystemPair sp(
        init << "true",
        dest << "./stop-httpd"
    );


    HttpClient hc = HttpClient();
    HttpResponse hr;
    std::string result;
    NOTHROW( hr = hc.GET(uri + "/a-man", result) );
    IS( hr.code(), 200 );
    IS( result, "a man, a plan, a canal, panama!" );
    IS( slurp("./http/requests/0"), slurp("./http/expect/get-a-man") );


    NOTHROW( hr = hc.GET(uri + "/curse", result ) );
    IS( hr.code(), 200 );
    IS( result, "Dammit, I'm mad!" );
    IS( hr.header("CONTENT-TYPE"), "text/x-clamation" );
    IS( hr.header("connection"), "close" );
    IS( hr.header("absent"), "" );
    std::string field_value = "garbage";
    OK( !hr.get_header("absent", field_value) );
    IS( field_value, "" );
    OK( hr.get_header("Content-language", field_value) );
    IS( field_value, "en_US" );
    
    std::map<std::string, std::string> headers;
    headers["X-FooBar"] = "yes";
    headers["X-BarFoo"] = "no";
    hc = HttpClient(headers);
    IS( hc.get_header("X-FOOBAR"), "yes" );
    IS( hc.get_header("absent"), "" );
    NOTHROW( hr = hc.GET(uri + "/OK", result) );
    IS( hr.code(), 200 );
    IS( slurp("./http/requests/2"), slurp("./http/expect/get-headers") );

    hc = HttpClient();
    hc.set_accept_type( "text/plain; q=0.5, text/html" );
    NOTHROW( hr = hc.GET(uri + "/OK", result) );
    IS( hr.code(), 200 );
    IS( slurp("./http/requests/3"), slurp("./http/expect/get-accept-header") );

    OK( run( "dd bs=1048576 count=8 if=/dev/urandom of=./http/get/random 2> /dev/null") == 0 );
    bfs::path outf("./download");
    NOTHROW( hr = hc.GET( uri + "/random", outf ) );
    IS( hr.code(), 200 );
    OK( run( "diff -u ./download ./http/get/random") == 0 );
    
    hc = HttpClient();
    hc.set_http_proxy(Uri(uri).hostport());
    NOTHROW( hr = hc.GET( uri + "/OK", result) );
    IS( slurp("./http/requests/5"), slurp("./http/expect/get-proxy") );
        
    hc = HttpClient();
    // we dont have a socks server to test with ... so just 
    // testing the setter/getter
    hc.set_socks_proxy(Uri(uri).hostport());
    IS( hc.get_socks_proxy(), Uri(uri).hostport() );

    
    hc = HttpClient();
    hc.set_content_type( "text/plain" );
    NOTHROW( hr = hc.POST(uri, "Yessir, I am POSTed content!\n", result) );
    IS( hr.code(), 200 );
    IS( slurp("./http/requests/6"), slurp("./http/expect/post-content") );
        
    hc = HttpClient();
    hc.set_content_type( "text/plain" );
    NOTHROW( hr = hc.PUT(uri, bfs::path("./http/junk"), result) );
    IS( hr.code(), 200 );
    // note the data in http/requests/7 is a bit of a cheat.  The
    // perl http::daemon will do the Continue-100 bits automatically
    // the important part is to note the Expect: 100-continue in
    // the request
    IS( slurp("./http/requests/7"), slurp("./http/expect/put-file") );

    hc = HttpClient();
    NOTHROW( hr = hc.HEAD( uri + "/OK" ) );
    IS( hr.code(), 200 );
    IS( slurp("./http/requests/8"), slurp("./http/expect/head") );

    NOTHROW( hr = hc.HEAD( uri + "/Forbidden" ) );
    IS( hr.code(), 403 );

    hc = HttpClient();
    NOTHROW( hr = hc.DELETE( uri + "/OK", result ) );
    IS( hr.code(), 200 );
    IS( slurp("./http/requests/10"), slurp("./http/expect/delete") );
    
    TempFile tmp;
    hr = hc.DELETE( uri + "/OK", tmp.name() );
    IS( hr.code(), 200 );
    IS( slurp(tmp.name()), "OK" );

    THROWS( hc.GET( uri, bfs::path("/dev/null/always_fail") ),
            "INTERNAL_SERVER_ERROR [500]: failed to open '/dev/null/always_fail': Not a directory");

    THROWS( hc.PUT( uri, bfs::path("/dev/null/always_fail"), result ),
            "INTERNAL_SERVER_ERROR [500]: failed to open '/dev/null/always_fail': Not a directory");

    
    hc.set_cookie("cookie1", "value1");
    NOTHROW( hr = hc.GET(uri + "/OK", result) );
    IS( slurp("./http/requests/12"), slurp("./http/expect/cookie1") );
    
    
    hc.set_cookie("cookie2", "value2");
    NOTHROW( hr = hc.GET(uri + "/OK", result) );
    IS( slurp("./http/requests/13"), slurp("./http/expect/cookie2") );

    hc.set_cookiejar("./http/requests/cookiejar");
    NOTHROW( hr = hc.GET(uri + "/cookies", result) );
    hc.write_cookiejar();
    IS( slurp("./http/requests/cookiejar"), slurp("./http/expect/cookiejar") );
    
    TEST_END;
}
