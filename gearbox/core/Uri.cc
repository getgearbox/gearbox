// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <iostream>
#include <sstream>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem/operations.hpp>
#include <gearbox/core/Errors.h>

#include <gearbox/core/Uri.h>
#include <gearbox/core/strlcpy.h>

#include <curl/curl.h> // easy_escape

using std::string;
using std::vector;
using namespace boost::algorithm;

namespace Gearbox {

#define REQUIRE_HTTPX(name)                                          \
    if (impl->scheme != "http" && impl->scheme != "https") {                   \
        gbTHROW( UriError("attribute '" #name "' unsupported by '" +    \
                       impl->scheme + "' URIs") );                          \
    }

#define REQUIRE_FILE(name)                                          \
    if (impl->scheme != "file") {                                        \
        gbTHROW( UriError("attribute '" #name "' unsupported by '" +   \
                       impl->scheme + "' URIs") );                         \
    }

UriError::UriError(const string msg)
{
    std::ostringstream err;
    err << "Uri Error: " << msg;
    strlcpy(buffer, err.str().c_str(), sizeof(buffer));
}
UriError::~UriError() throw () {}
const char * UriError::what() const throw () 
{
    return buffer;
}

class Uri::Private {
public:
    std::string uristr;
    std::string scheme;
    std::string username;
    std::string password;
    std::string hostname;
    std::string path;
    std::string query;
    std::string fragment;
    int port;
    Private() : port(0) {}
};
    
Uri::Uri() : impl(new Private()) {
}

Uri::Uri(const char * uristr) : impl(new Private()) {
    impl->uristr = uristr;
    try {
        this->parse_uristr(impl->uristr);
    }
    catch(...) {
        delete impl;
        throw;
    }
}

Uri::Uri(const string & uristr)
    : impl(new Private()) {
    impl->uristr = uristr;
    try {
        this->parse_uristr(impl->uristr);
    }
    catch(...) {
        delete impl;
        throw;
    }

}

Uri::Uri(const boost::filesystem::path & bfspath)
    : impl(new Private()) {
    try {
        if (! bfspath.is_complete() ) {
            gbTHROW( UriError("Uris of scheme 'file' may not be relative.") );
        }
        impl->scheme = "file";
        impl->path = bfspath.string();
        impl->uristr = canonical();
    }
    catch(...) {
        delete impl;
        throw;
    }
        
}

Uri::Uri( const Uri & other) : impl(new Private(*other.impl)) {
}

Uri & Uri::operator=(const Uri & other) {
    if( this == &other ) return *this;
    if( impl ) delete impl;
    impl = new Private(*other.impl);
    return *this;
}

Uri::~Uri() {
    if(impl) delete impl;
}

const std::string &
Uri::uristr() const {
    return impl->uristr;
}

const string &
Uri::scheme() const
{
    return impl->scheme;
}
void
Uri::scheme(const string & scheme)
{
    impl->scheme = scheme;
    to_lower(impl->scheme);
    if (impl->scheme != "file"  &&
        impl->scheme != "http"  &&
        impl->scheme != "https"
        ) gbTHROW( UriError("Unsupported uri scheme '" + impl->scheme + "'.") );
}

bool
Uri::is_http() const
{
    return ( impl->scheme == "http" );
}

bool
Uri::is_https() const
{
    return ( impl->scheme == "https" );
}

bool
Uri::is_httpx() const
{
    return ( is_http() || is_https() );
}

bool
Uri::is_file() const
{
    return ( impl->scheme == "file" );
}

const string &
Uri::username() const
{
    REQUIRE_HTTPX(username);
    return impl->username;
}
void
Uri::username(const string & username)
{
    REQUIRE_HTTPX(username);
    impl->username = username;
}

const string &
Uri::password() const
{
    REQUIRE_HTTPX(password);
    return impl->password;
}
void
Uri::password(const string & password)
{
    REQUIRE_HTTPX(password);
    impl->password = password;
}

const string &
Uri::hostname() const
{
    REQUIRE_HTTPX(hostname);
    return impl->hostname;
}
void
Uri::hostname(const string & hostname)
{
    REQUIRE_HTTPX(hostname);
    impl->hostname = hostname;
    to_lower(impl->hostname);
}

int
Uri::port() const
{
    REQUIRE_HTTPX(port);
    return impl->port;
}

void
Uri::port(const int port)
{
    REQUIRE_HTTPX(port);
    impl->port = port;
}

const string &
Uri::path() const
{
    return impl->path;
}

// static member function
string
Uri::canonicalize_path(const string & path)
{
    std::ostringstream cpath;

    if (path.empty()) {
        return "/";
    }
    
    vector<string> parts;
    split(parts, path, is_any_of("/"));
    vector<string>::iterator p = parts.begin();
    while( p != parts.end()) {
        string & part = *p;
        if ( part.empty() || part == ".") {
            p = parts.erase(p);
        }
        else if (part == "..") {
            p = parts.erase(p);
            if (p != parts.begin() ) {
                p = parts.erase(--p);
            }
        }
        else {
            p++;
        }
    }
    p = parts.begin();
    while ( p != parts.end() ) {
        cpath << "/" << (*p);
        p++;
    }
    if (path.substr(path.size() - 1, 1) == "/") cpath << "/";
    return cpath.str();
}

string 
Uri::urlescape_part( const std::string & part ) 
{
    CURL *curl = curl_easy_init();
    if ( curl == NULL ) {
        gbTHROW( UriError( "Could not initialize libcurl." ) );
    }
    char *escc = curl_easy_escape( curl, part.c_str(), part.size() );
    string escs( escc );
    curl_free( escc ); 
    curl_easy_cleanup( curl );
    return escs;
}

const string
Uri::canonical_path() const
{
    return Uri::canonicalize_path(impl->path);
}

void
Uri::path(const string & path)
{
    impl->path = path;
}
void
Uri::path_push(const string & part)
{
    if ( impl->path.empty() ) { 
        path(part);
    } else {
        path(impl->path + "/" + part);
    }
}

void
Uri::path_pop()
{
    if ( impl->path.empty() ) { 
        return;
    }

    size_t slash = impl->path.rfind('/');
    if( slash != std::string::npos ) {
        path(impl->path.substr( 0, slash ));
    }
}

Uri &
Uri::operator /=(const string & part)
{
    path_push(part);
    return *this;
}
Uri
Uri::operator /(const string & part) const
{
    return Uri(*this) /= part;
}

string
Uri::leaf() const
{
    std::string leaf;
    size_t pos = impl->path.find_last_of('/');
    if (pos != string::npos)
        leaf = impl->path.substr(pos + 1);
    else
        leaf = impl->path;
    return leaf;
}

const string &
Uri::query() const
{
    REQUIRE_HTTPX(query);
    return impl->query;
}
void
Uri::query(const string & query)
{
    REQUIRE_HTTPX(query);
    impl->query = query;
}
void
Uri::query_push(const string & part)
{
    REQUIRE_HTTPX(query);
    if (impl->query.empty()) {
        query(part);
    }
    else {
        query(impl->query + "&" + part);
    }
}

const string &
Uri::fragment() const
{
    REQUIRE_HTTPX(fragment);
    return impl->fragment;
}
void
Uri::fragment(const string & fragment)
{
    REQUIRE_HTTPX(fragment);
    impl->fragment = fragment;
}

const string Uri::authority() const
{
    if (impl->scheme == "file") return "";
    std::ostringstream authority;
    if (! impl->username.empty() ) {
        authority << impl->username;
        if (! impl->password.empty() ) {
            authority << ":" << impl->password;
        }
        authority << "@";
    }
    authority << hostport();
    return authority.str();
}

const string Uri::hostport() const
{
    if (impl->scheme == "file") return "";
    if ( impl->scheme == "http"  && impl->port == 80  ) return hostname();
    if ( impl->scheme == "https" && impl->port == 443 ) return hostname();
    std::ostringstream hostport;
    hostport << hostname() << ":" << port();
    return hostport.str();
}

const string
Uri::str() const
{
    std::ostringstream uri;
    string p = path();
    
    uri << impl->scheme << "://" << authority();
    if ( p.substr(0,1) != "/" ) uri << "/";
    uri << p;

    if (! impl->query.empty() ) {
        uri << "?" << impl->query;
    }
    if (! impl->fragment.empty() ) {
        uri << "#" << impl->fragment;
    }
    return uri.str();
}

const string
Uri::canonical() const
{
    std::ostringstream curi;
    curi << impl->scheme << "://" << authority() << canonical_path();
    if (! impl->query.empty() ) {
        curi << "?" << impl->query;
    }
    if (! impl->fragment.empty() ) {
        curi << "#" << impl->fragment;
    }
    return curi.str();
}

void
Uri::parse_authority(const string & authority)
{
    if (authority.empty()) {
        gbTHROW( UriError("Authority must be present for '" +
                       impl->scheme + "' uris.") );
    }
    string nl(authority);
    size_t pos = nl.find("@");
    if (pos != string::npos) {
        username(nl.substr(0, pos));
        nl = nl.substr(pos+1);
        pos = impl->username.find(":");
        if (pos != string::npos) {
            password(impl->username.substr(pos+1));
            username(impl->username.substr(0, pos));
        }
    }
    pos = nl.find(":");
    if (pos != string::npos) {
        std::string host = nl.substr(0, pos);
        this->hostname(host);
        std::string port = nl.substr(pos+1);
        this->port(atoi(port.c_str()));
    } else {
        hostname(nl);
    }
}

void
Uri::parse_uristr(const string & uristr)
{
    impl->port = 0;
    string authority;
    string uri(uristr);

    size_t pos = uristr.find("://");
    if (pos == string::npos) {
        gbTHROW( UriError("The string '" + uristr + "' is not " +
                       "parseable as a uri.") );
    }
    scheme(uri.substr(0, pos));
    uri = uri.substr(pos+3);

    if (impl->scheme != "file") {    
        pos = uri.find_first_of("/?#");
        if (pos != string::npos) {
            authority = uri.substr(0, pos);
            uri = uri.substr(pos+1);
        }
        else {
            authority = uri;
            uri = "";
        }
        
        parse_authority(authority);
        pos = uri.find("#");
        if (pos != string::npos) {
            fragment(uri.substr(pos+1));
            uri = uri.substr(0, pos);
        }
        pos = uri.find("?");
        if (pos != string::npos) {
            query(uri.substr(pos+1));
            uri = uri.substr(0, pos);
        }
        if (impl->port == 0) {
            if (impl->scheme == "http" ) port(80);
            if (impl->scheme == "https") port(443);
        }
    }
    path(uri);
}

bool
operator ==(const Uri & lhs, const Uri & rhs)
{
    return ( lhs.canonical() == rhs.canonical() );
}

bool
operator !=(const Uri & lhs, const Uri & rhs)
{
    return ! ( lhs.canonical() == rhs.canonical() );
}

} // namespace
