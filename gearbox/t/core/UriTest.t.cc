// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>

#include <gearbox/core/logger.h>
#include <gearbox/core/Uri.h>
using namespace Gearbox;

#include <boost/filesystem/path.hpp>
namespace bfs=boost::filesystem;

// section numbers in ()s refer to RFC3986

int main() {
    chdir(TESTDIR);
    TEST_START(90);
    log_init("./unit.conf");
    
    // string constructor must begin with a scheme
    THROWS( Uri("monkeychow"), "Uri Error: The string 'monkeychow' is not "
            "parseable as a uri.");
    THROWS( Uri(""), "Uri Error: The string '' is not parseable as a uri.");

    // and it needs to be a scheme we understand: file:// http:// https://
    THROWS( Uri("jaybuff://"), "Uri Error: Unsupported uri scheme "
            "'jaybuff'." );
    
    // http(s) uris must include an authority
    THROWS( Uri("http://"), "Uri Error: Authority must be present for "
            "'http' uris." );
    THROWS( Uri("https://"), "Uri Error: Authority must be present for "
            "'https' uris." );

    // work out the accessors
    Uri uri1("http://a.b.c/"); 
    Uri uri2("https://a.b.c/d/e");
    Uri uri3("http://a.b.c:4080/?d=foo&e=baz");
    Uri uri4("http://a:b@c/");
    Uri uri5("http://a.b.c/d#e");

    IS ( uri1.scheme(), "http" );
    IS ( uri2.scheme(), "https" );             // scheme (3.1)
    OK ( uri1.is_http() );
    NOK ( uri1.is_https() );
    OK ( uri1.is_httpx() );
    NOK (uri1.is_file() );
    OK ( uri2.is_https() );
    NOK ( uri2.is_http() );
    OK ( uri2.is_httpx() );
    NOK ( uri2.is_file() );

    IS ( uri1.username(), "" );
    IS ( uri4.username(), "a" );               // username (3.2.1)

    IS ( uri1.password(), "" );
    IS ( uri4.password(), "b" );               // password (3.2.1)

    IS ( uri1.authority(), "a.b.c" );
    IS ( uri4.authority(), "a:b@c" );          // authority (3.2)

    IS ( uri1.hostname(), "a.b.c" );
    IS ( uri3.hostname(), "a.b.c" );
    IS ( uri4.hostname(), "c" );               // hostname (3.2.2)

    IS ( uri1.port(), 80 );
    IS ( uri2.port(), 443 );
    IS ( uri3.port(), 4080 );                  // port (3.2.3)

    IS ( uri1.hostport(), "a.b.c" );
    IS ( uri2.hostport(), "a.b.c" );
    IS ( uri3.hostport(), "a.b.c:4080" );      // hostport
    
    IS ( uri1.path(), "" );
    IS ( uri2.path(), "d/e" );
    IS ( uri3.path(), "" );
    IS ( uri5.path(), "d" );                   // path (3.3)

    IS ( uri1.leaf(), "" );

    NOTHROW( uri1.path_push("f") );
    NOTHROW( uri2.path_push("f") );
    NOTHROW( uri3.path_push("f") );
    NOTHROW( uri5.path_push("f") );
    
    IS ( uri1.leaf(), "f" );
    IS ( uri2.leaf(), "f" );
    IS ( uri3.leaf(), "f" );
    IS ( uri5.leaf(), "f" );

    // make sure the path_push really added the f and leaf didn't remove it
    IS ( uri1.path(), "f" );
    IS ( uri2.path(), "d/e/f" );
    IS ( uri3.path(), "f" );
    IS ( uri5.path(), "d/f" );

    IS ( uri1.query(), "" );
    IS ( uri3.query(), "d=foo&e=baz" );        // query (3.4)

    uri1.query_push("foo=bar");
    IS ( uri1.query(), "foo=bar" );
    uri1.query_push("baz=bif");
    IS ( uri1.query(), "foo=bar&baz=bif" );
    IS ( uri1.canonical(), "http://a.b.c/f?foo=bar&baz=bif" );
    
    IS ( uri1.fragment(), "" );
    IS ( uri5.fragment(), "e" );               // fragments (3.5)

    // check comparisons
    Uri uri(std::string("http://a.b.c/d/e/f/g/")); // string constructor
    OK ( uri == "http://a.b.c/d/e/f/g/" );     // preserve trailing slash
    OK ( uri != "http://a.b.c/d/e/f/g" );      // preserve !trailing slash
    OK ( uri == "http://a.b.c/d//e//f//g//" ); // forward slash dedup   
    OK ( uri == "HtTp://a.b.c/d/e/f/g/" );     // schemes case insensitive
    OK ( uri == "http://A.b.C/d/e/f/g/" );     // hostnames case insensitive
    OK ( uri != "http://a.b.c/D/e/F/g/" );     // paths case sensitive
    
    // overloaded /= appends to the uri path
    uri /= "h";
    IS ( uri.canonical(), "http://a.b.c/d/e/f/g/h");

    // path trailing slashes are preserved
    uri /= "i/";
    IS ( uri.canonical(), "http://a.b.c/d/e/f/g/h/i/");

    // any leading slashes are merged
    uri /= "/j";
    IS ( uri.canonical(), "http://a.b.c/d/e/f/g/h/i/j");

    // canonical() will dedup exta path delimiters
    uri = Uri("http://a.b.c/d//e///f////g");
    IS ( uri.canonical(), "http://a.b.c/d/e/f/g" );
    // str() will not
    IS ( uri.str(), "http://a.b.c/d//e///f////g" );

    // canonical() will remove /./ path sequences
    uri = Uri("http://a.b.c/d/./e/./f");
    IS ( uri.canonical(), "http://a.b.c/d/e/f" );
    // str() will not
    IS ( uri.str(), "http://a.b.c/d/./e/./f" );   

    // canonical() will resolve /../ path sequences
    uri = Uri("http://a.b.c/d/../e/../f/../");
    IS ( uri.canonical(), "http://a.b.c/" );
    // str() will not
    IS ( uri.str(), "http://a.b.c/d/../e/../f/../" );

    // canonical() respects query strings
    OK ( uri3.canonical() == "http://a.b.c:4080/f?d=foo&e=baz" );
    OK ( uri3.str() == "http://a.b.c:4080/f?d=foo&e=baz" );

    // canonical() respects fragments
    OK ( uri5.canonical() == "http://a.b.c/d/f#e" );
    OK ( uri5.str() == "http://a.b.c/d/f#e" );

    uri = Uri("file:///etc");
    uri = uri/"passwd";

    IS ( uri.scheme(), "file" );
    OK ( uri.is_file() );
    NOK ( uri.is_httpx() );
    THROWS ( uri.hostname(), "Uri Error: attribute 'hostname' unsupported "
            "by 'file' URIs" );
    THROWS ( uri.port(), "Uri Error: attribute 'port' unsupported by "
            "'file' URIs" );
    THROWS ( uri.username(), "Uri Error: attribute 'username' unsupported "
            "by 'file' URIs" );
    THROWS ( uri.password(), "Uri Error: attribute 'password' unsupported "
            "by 'file' URIs" );
    THROWS ( uri.query(), "Uri Error: attribute 'query' unsupported " 
	    "by 'file' URIs" );
    THROWS ( uri.fragment(), "Uri Error: attribute 'fragment' unsupported " 
	    "by 'file' URIs" );
    IS ( uri.canonical(), "file:///etc/passwd" );

    OK ( uri == bfs::path("/etc/passwd") );
    OK ( uri != bfs::path("/etc/haircuts") );
    THROWS ( Uri(bfs::path("relative/path")), "Uri Error: Uris of scheme "
             "'file' may not be relative." );

    uri = Uri(bfs::path("/usr/local/giblets"));
    IS( uri.scheme(), "file");
    IS( uri.canonical(), "file:///usr/local/giblets" );
    IS( uri.str(), "file:///usr/local/giblets" );

    IS( Uri::urlescape_part("http://foo?bar=baz;bif"), "http%3A%2F%2Ffoo%3Fbar%3Dbaz%3Bbif") ;

    TEST_END;
}
