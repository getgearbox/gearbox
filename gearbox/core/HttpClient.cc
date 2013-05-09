// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include "config.h"
#include <sstream>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>

#define LOGCAT "gearbox.http"
#include <gearbox/core/logger.h>
#include <gearbox/core/Errors.h>

#include <gearbox/core/HttpClient.h>
#include <gearbox/core/strlcpy.h>
#include <curl/curl.h>

using namespace std;
using namespace boost::algorithm;
using namespace Gearbox;

namespace bfs=boost::filesystem;
// something somewhere already says using namespace Gearbox

HttpClientError::HttpClientError(const string & msg)
{
    std::ostringstream err;
    err << "HttpClient Error: " << msg;
    strlcpy(buffer, err.str().c_str(), sizeof(buffer));
}
HttpClientError::~HttpClientError() throw () {}

const char *
HttpClientError::what() const throw () 
{
    return buffer;
}

HttpRetryError::HttpRetryError(const string & msg)       : HttpClientError(msg) {}
RetryTimeoutError::RetryTimeoutError(const string & msg) : HttpRetryError(msg) {}
RetryConnectError::RetryConnectError(const string & msg) : HttpRetryError(msg) {}

//////////////////////////////////////////////////////////////////////////////

namespace {
    struct FileAutoPtr {
        FileAutoPtr(const bfs::path & file, const char *mode) {
            fp_ = fopen( file.string().c_str(), mode );
            if( !fp_ ) {
                gbTHROW( ERR_LIBC("failed to open '" + file.string() + "'") );
            }
        }
        FileAutoPtr(FILE*fp) : fp_(fp) {};
        ~FileAutoPtr() { if (fp_) fclose(fp_); }
        FILE * get() { return fp_; }
        FILE * fp_;
    };
}

//////////////////////////////////////////////////////////////////////////////

typedef std::map<std::string, std::string> header_map_t;

struct Gearbox::HttpResponse::Private {
    long code_;
    header_map_t headers_;

    Private() : code_(0) {}
    Private(const Private & src);
};

HttpResponse::HttpResponse::Private::Private(const Private & src)
    : code_(src.code_),
      headers_(src.headers_)
{}

HttpResponse::HttpResponse()
    : impl(new Private())
{}

HttpResponse::HttpResponse(const HttpResponse & src)
    : impl(new Private(*src.impl))
{}

HttpResponse::HttpResponse(const Private & src)
    : impl(new Private(src))
{}

HttpResponse::HttpResponse(long code)
    : impl(new Private())
{
    impl->code_ = code;
}

HttpResponse &
HttpResponse::operator = (const HttpResponse & src)
{
    if (&src != this) {
	delete impl;
	impl = new Private(*src.impl);
    }

    return *this;
}

HttpResponse::~HttpResponse()
{
    delete impl;
}

long
HttpResponse::code() const
{
    return impl->code_;
}

bool
HttpResponse::get_header(const std::string & field_mixed_case, std::string & out) const
{
    std::string field = field_mixed_case;
    to_lower(field);

    header_map_t::const_iterator it = impl->headers_.find(field);
    if (it != impl->headers_.end()) {
	out = it->second;
	return true;
    }
    else {
	out.clear();
	return false;
    }
}

std::string
HttpResponse::header(const std::string & field) const
{
    std::string res;
    (void)get_header(field, res);
    return res;
}

void
HttpResponse::get_headers(map<string, string> & headers) const
{
    header_map_t::const_iterator it = impl->headers_.begin();
    for (; it != impl->headers_.end(); ++it ) {
        headers[it->first] = it->second;
    }
}

//////////////////////////////////////////////////////////////////////////////

struct Gearbox::HttpClient::Private {
    void * curl_;
    struct curl_slist * curl_headers_;
    bool follow_redirects_;
    header_map_t headers_;
    std::string http_proxy_;
    std::string socks_proxy_;
    std::map<HttpClient::method_t, long> timeout_;
    std::map<HttpClient::method_t, int> attempts_;
    std::map<HttpClient::method_t, long> idle_timeout_;
    int connect_attempts_;
    std::map<std::string,std::string> cookies_;
    std::string cookiejar_;
    static std::string global_cookiejar_;
       
    Private() : curl_(NULL), curl_headers_(NULL), follow_redirects_(false), connect_attempts_(10) {
        timeout_[METHOD_GET]      = 15;
        timeout_[METHOD_DELETE]   = 300;
        timeout_[METHOD_POST]     = 300;
        timeout_[METHOD_PUT]      = 300;
        timeout_[METHOD_HEAD]     = 15;
        attempts_[METHOD_GET]     = 3;
        attempts_[METHOD_DELETE]  = 1;
        attempts_[METHOD_POST]    = 1;
        attempts_[METHOD_PUT]     = 1;
        attempts_[METHOD_HEAD]    = 3;
    }
    Private(const Private & src);
    ~Private();

    void setup_curl( HttpClient::method_t method );
    HttpResponse make_request( const Uri & uri);
    void setup_request_to_string( const Uri & uri, std::string & content );
    void setup_request_to_file( const Uri & uri, FILE * fp );
    void setup_request_from_file( const Uri & uri, FILE * fp );
    void setup_request_from_string( const Uri & uri, std::istringstream & body_iss );

private:
    // not implemented
    Private & operator = (const Private & src);
};

std::string HttpClient::Private::global_cookiejar_;

HttpClient::Private::Private(const Private & src)
    : curl_(NULL),
      curl_headers_(NULL),
      follow_redirects_(src.follow_redirects_),
      headers_(src.headers_),
      http_proxy_(src.http_proxy_),
      socks_proxy_(src.socks_proxy_),
      timeout_(src.timeout_),
      attempts_(src.attempts_),
      idle_timeout_(src.timeout_),
      connect_attempts_(10)
{
    if (src.curl_ != NULL) {
	curl_ = curl_easy_duphandle(src.curl_);
    }
}

HttpClient::Private::~Private()
{
    if( curl_headers_ != NULL ) {
        curl_slist_free_all(curl_headers_);
    }
    if (curl_ != NULL) {
        curl_easy_cleanup(curl_);
    }
}

HttpClient::HttpClient()
    : impl(new Private())
{}

HttpClient::HttpClient(const map<string, string> & headers)
    : impl(new Private())
{
    for (map<string, string>::const_iterator it = headers.begin();
         it != headers.end(); ++it) {
         set_header(it->first, it->second);
     }
}

HttpClient::HttpClient(const HttpClient & src)
    : impl(new Private(*src.impl))
{}

HttpClient &
HttpClient::operator = (const HttpClient & src)
{
    if (&src != this) {
	delete impl;
	impl = new Private(*src.impl);
    }

    return *this;
}

HttpClient::~HttpClient()
{
    delete impl;
}

static size_t _read_response(void *ptr, size_t size, size_t nmemb, void * data) {
    string * str = static_cast<string*>(data);
    int len = size * nmemb;
    str->append(static_cast<char*>(ptr), len);
    return len;
}

static size_t _seek_response(void *stream, curl_off_t offset, int origin) {
    _DEBUG("_seek_response called with stream: " << stream << "offset: " << offset << " origin: " << origin);
    return 1;
}

static size_t _write_file(void *ptr, size_t size, size_t nmemb, void * stream) 
{
    return fwrite(ptr, size, nmemb, (FILE*)stream);
}

static size_t _read_file(void * ptr, size_t size, size_t nmemb, void * stream)
{
    return fread(ptr, size, nmemb, (FILE*)stream);
}

static size_t _read_stream(void * ptr, size_t size, size_t nmemb, void * stream)
{
    std::istringstream * iss = static_cast<istringstream *>(stream);
    iss->read(static_cast<char*>(ptr), size * nmemb);
    return iss->gcount();
}

// curl will call this callback once for each line
static size_t _read_response_headers(void *ptr, size_t size, size_t nmemb, void * data)
{
    const size_t len = size * nmemb;
    header_map_t & headers = *(static_cast<header_map_t *>(data));
    std::string line(static_cast<const char *>(ptr), len);

    if (line.substr(0, 5) == "HTTP/") {
	// Start of a response header.  Discard the previous response header,
	// which will be the case if we are following redirects.
	headers.clear();
    }
    else {
	// As long as there is a colon, treat headers and trailers alike.  The
	// empty line after the header (possibly before the trailer) will be
	// skipped on this basis.
	size_t colon = line.find(':');
	if (colon != std::string::npos) {
	    std::string field(line, 0, colon);
	    to_lower(field);

	    std::string value(line, colon+1);
	    trim(value);

	    // Comma-separate values of header fields with the same name
	    std::string & stored = headers[field];
	    if (!stored.empty()) {
		stored.append(",", 1);
	    }
	    stored.append(value);
	}
    }

    return len;
}

string
HttpClient::get_http_proxy() const
{
    return impl->http_proxy_;
}
void
HttpClient::set_http_proxy(const string & hostport)
{
    impl->http_proxy_ = hostport;
}

string
HttpClient::get_socks_proxy() const
{
    return impl->socks_proxy_;
}
void
HttpClient::set_socks_proxy(const string & hostport)
{
    impl->socks_proxy_ = hostport;
}

string
HttpClient::get_header(string header) const
{
     to_lower(header);
     const header_map_t::const_iterator it = impl->headers_.find(header);
     if ( it == impl->headers_.end() ) return ""; // cannot do string(NULL) !
     return it->second;
}
void
HttpClient::set_header(string header, const string & value)
{
    to_lower(header);
    impl->headers_[header] = value;
}

bool
HttpClient::follow_redirects() const
{
    return impl->follow_redirects_;
}
void
HttpClient::follow_redirects(bool follow)
{
    impl->follow_redirects_ = follow;
}

void
HttpClient::set_content_type(const string & type)
{
    set_header("content-type", type);
}

void
HttpClient::set_accept_type(const string & type)
{
    set_header("accept", type);
}

long
HttpClient::get_timeout(const HttpClient::method_t method) const
{
    return impl->timeout_[method];
}

void
HttpClient::set_timeout(const HttpClient::method_t method, long secs)
{
    impl->timeout_[method] = secs;
}

int
HttpClient::get_timeout_attempts(const HttpClient::method_t method) const
{
    return impl->attempts_[method];
}

void
HttpClient::set_timeout_attempts(const HttpClient::method_t method, int attempts)
{
    impl->attempts_[method] = attempts;
}

long
HttpClient::get_idle_timeout(const HttpClient::method_t method) const
{
    return impl->idle_timeout_[method];
}

void
HttpClient::set_idle_timeout(const HttpClient::method_t method, long secs)
{
    impl->idle_timeout_[method] = secs;
}

void HttpClient::set_global_cookiejar(const std::string & file) {
    HttpClient::Private::global_cookiejar_ = file;
}

void HttpClient::set_cookiejar(const std::string & file) {
    impl->cookiejar_ = file;
}

void HttpClient::set_cookie(const std::string & name, const std::string & value) {
    impl->cookies_[name] = value;
}

void HttpClient::write_cookiejar() {
    if( !impl->cookiejar_.empty() || !impl->global_cookiejar_.empty() ) {
        curl_easy_setopt( impl->curl_, CURLOPT_COOKIELIST, "FLUSH" );
    }
}


HttpResponse
HttpClient::DELETE(const Uri & uri, string & content)
{
    int timeout_retry = impl->attempts_[METHOD_DELETE];
    int connect_retry = impl->connect_attempts_;

    while( timeout_retry && connect_retry ) {
        try {
            impl->setup_curl( METHOD_DELETE );
            impl->setup_request_to_string( uri, content);
            return this->make_request( uri );
        }
        catch ( const RetryTimeoutError & err ) {
            if(!--timeout_retry) throw;
        }
        catch ( const RetryConnectError & err ) {
            if(!--connect_retry) throw;
        }
    }
    return HttpResponse(500);
}

HttpResponse
HttpClient::DELETE(const Uri & uri, const bfs::path & file)
{
    FileAutoPtr to_file(file, "w");

    int timeout_retry = impl->attempts_[METHOD_DELETE];
    int connect_retry = impl->connect_attempts_;

    while( timeout_retry && connect_retry ) {
        try {
            impl->setup_curl( METHOD_DELETE );
            impl->setup_request_to_file( uri, to_file.get() );
            return this->make_request( uri );
        }
        catch ( const RetryTimeoutError & err ) {
            if(!--timeout_retry) throw;
        }
        catch ( const RetryConnectError & err ) {
            if(!--connect_retry) throw;
        }
    }
    return HttpResponse(500);
}

HttpResponse
HttpClient::GET(const Uri & uri, string & content)
{
    int timeout_retry = impl->attempts_[METHOD_GET];
    int connect_retry = impl->connect_attempts_;

    while( timeout_retry && connect_retry ) {
        try {
            impl->setup_curl( METHOD_GET );
            impl->setup_request_to_string( uri, content);
            return this->make_request( uri );
        }
        catch ( const RetryTimeoutError & err ) {
            if(!--timeout_retry) throw;
        }
        catch ( const RetryConnectError & err ) {
            if(!--connect_retry) throw;
        }
    }
    return HttpResponse(500);
}

HttpResponse
HttpClient::GET(const Uri & uri, const bfs::path & file)
{
    FileAutoPtr to_file(file, "w");

    int timeout_retry = impl->attempts_[METHOD_GET];
    int connect_retry = impl->connect_attempts_;

    while( timeout_retry && connect_retry ) {
        try {
            impl->setup_curl( METHOD_GET );
            impl->setup_request_to_file( uri, to_file.get() );
            return this->make_request( uri );
        }
        catch ( const RetryTimeoutError & err ) {
            if(!--timeout_retry) throw;
        }
        catch ( const RetryConnectError & err ) {
            if(!--connect_retry) throw;
        }
    }
    return HttpResponse(500);
}

HttpResponse
HttpClient::HEAD(const Uri & uri )
{
    int timeout_retry = impl->attempts_[METHOD_HEAD];
    int connect_retry = impl->connect_attempts_;

    while( timeout_retry && connect_retry ) {
        try {
            impl->setup_curl( METHOD_HEAD );
            return this->make_request( uri );
        }
        catch ( const RetryTimeoutError & err ) {
            if(!--timeout_retry) throw;
        }
        catch ( const RetryConnectError & err ) {
            if(!--connect_retry) throw;
        }
    }
    return HttpResponse(500);
}

HttpResponse
HttpClient::POST(const Uri & uri, const string & body, string & content)
{
    std::istringstream body_iss(body);

    int timeout_retry = impl->attempts_[METHOD_POST];
    int connect_retry = impl->connect_attempts_;

    while( timeout_retry && connect_retry ) {
        try {
            impl->setup_curl( METHOD_POST );
            impl->setup_request_from_string( uri, body_iss );
            impl->setup_request_to_string( uri, content );
            return this->make_request( uri );
        }
        catch ( const RetryTimeoutError & err ) {
            if(!--timeout_retry) throw;
        }
        catch ( const RetryConnectError & err ) {
            if(!--connect_retry) throw;
        }
    }
    return HttpResponse(500);
}

HttpResponse
HttpClient::PUT(const Uri & uri, const string & body, string & content)
{
    std::istringstream body_iss(body);

    int timeout_retry = impl->attempts_[METHOD_PUT];
    int connect_retry = impl->connect_attempts_;

    while( timeout_retry && connect_retry ) {
        try {
            impl->setup_curl( METHOD_PUT );
            impl->setup_request_from_string( uri, body_iss );
            impl->setup_request_to_string( uri, content );
            return this->make_request( uri );
        }
        catch ( const RetryTimeoutError & err ) {
            if(!--timeout_retry) throw;
        }
        catch ( const RetryConnectError & err ) {
            if(!--connect_retry) throw;
        }
    }
    return HttpResponse(500);
}

HttpResponse
HttpClient::PUT(const Uri & uri, const bfs::path & file, string & content)
{
    FileAutoPtr from_file( file, "r" );

    int timeout_retry = impl->attempts_[METHOD_PUT];
    int connect_retry = impl->connect_attempts_;

    while( timeout_retry && connect_retry ) {
        try {
            impl->setup_curl( METHOD_PUT );
            impl->setup_request_from_file( uri, from_file.get() );
            impl->setup_request_to_string( uri, content );
            return this->make_request( uri );
        }
        catch ( const RetryTimeoutError & err ) {
            if(!--timeout_retry) throw;
        }
        catch ( const RetryConnectError & err ) {
            if(!--connect_retry) throw;
        }
    }
    return HttpResponse(500);
}

HttpResponse
HttpClient::make_request(const Uri & uri )
{
    return impl->make_request( uri );
}

void
HttpClient::Private::setup_request_to_file(const Uri & uri, FILE * fp )
{
    curl_easy_setopt( curl_, CURLOPT_WRITEFUNCTION, _write_file );
    curl_easy_setopt( curl_, CURLOPT_WRITEDATA, fp );
}

void
HttpClient::Private::setup_request_to_string(const Uri & uri, std::string & content)
{
    content.clear();
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, _read_response);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, (void *)&content);
}

void
HttpClient::Private::setup_request_from_file( const Uri & uri, FILE * fp )
{
    curl_easy_setopt(curl_, CURLOPT_READFUNCTION, _read_file );
    curl_easy_setopt(curl_, CURLOPT_READDATA, fp );
}

void
HttpClient::Private::setup_request_from_string( const Uri & uri, std::istringstream & body_iss )
{
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, body_iss.str().length());
    curl_easy_setopt(curl_, CURLOPT_READFUNCTION, _read_stream );
    curl_easy_setopt(curl_, CURLOPT_READDATA, &body_iss );
}

HttpResponse
HttpClient::Private::make_request(const Uri & uri)
{
    HttpResponse::Private res;

    string uristr = uri.canonical();
    char error_msg[CURL_ERROR_SIZE];
    curl_easy_setopt( curl_, CURLOPT_ERRORBUFFER, error_msg );
    curl_easy_setopt( curl_, CURLOPT_URL, uristr.c_str() );
    curl_easy_setopt( curl_, CURLOPT_HEADERFUNCTION, _read_response_headers);
    curl_easy_setopt( curl_, CURLOPT_HEADERDATA, (void *)&res.headers_);
    _DEBUG("Requesting '"<< uristr << "'...");
    CURLcode result = curl_easy_perform(curl_);
    if ( result ) {
        std::string error("Error connecting to " + uristr + ": " + error_msg + " [code " + boost::lexical_cast<string>(result) + "]");
        // timeout or NO DATA error
        if ( result == CURLE_OPERATION_TIMEDOUT
             || result == CURLE_GOT_NOTHING
             || result == CURLE_SEND_ERROR
             || result == CURLE_RECV_ERROR
             || result == CURLE_PARTIAL_FILE
        ) {
            _ERROR(error);
            gbTHROW( RetryTimeoutError(error) );
        }
        else if( result == CURLE_COULDNT_CONNECT
                 || result == CURLE_COULDNT_RESOLVE_HOST
        ) {
            _ERROR(error);
            gbTHROW( RetryConnectError(error) );
        } else {
            _ERROR(error);
            gbTHROW( HttpClientError(error) );
        }
    }
    curl_easy_getinfo( curl_, CURLINFO_RESPONSE_CODE, &res.code_ );
    return HttpResponse(res);
}

void
HttpClient::Private::setup_curl(method_t method)
{
    // every curl init has to be balanced with a cleanup, or it leaks
    // memory & fds.  create a session once and reset it before reuse
    if ( !curl_ ) {
        curl_ = curl_easy_init();
    } else {
        curl_easy_reset( curl_ );
    }

    if( curl_headers_ ) {
        curl_slist_free_all(curl_headers_);
        curl_headers_ = NULL;
    }

    if ( curl_ == NULL ) {
        gbTHROW( HttpClientError( "Could not initialize libcurl." ) );
    }

    //curl_easy_setopt(curl_, CURLOPT_VERBOSE, 1);
    
    // setup whether curl should use a proxy and/or automatically follow
    // http redirects
    if ( ! http_proxy_.empty() ) {
        curl_easy_setopt( curl_, CURLOPT_PROXY, http_proxy_.c_str() );
    }

    if ( ! socks_proxy_.empty() ) {
        curl_easy_setopt( curl_, CURLOPT_PROXY, socks_proxy_.c_str() );
        curl_easy_setopt( curl_, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4 );
    }

    if ( follow_redirects_ ) {
        curl_easy_setopt( curl_, CURLOPT_FOLLOWLOCATION, 1 );
    }

    // set request headers
    for (map<string, string>::const_iterator it = headers_.begin();
                  it != headers_.end(); ++it) {
        string header = it->first + ": " + it->second;        
        curl_headers_ = curl_slist_append(curl_headers_, header.c_str());
    }
    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, curl_headers_);

    curl_easy_setopt(curl_, CURLOPT_CAINFO, DATADIR "/ssl/certs/ca-bundle.crt");

    // set the HTTP method to use
    switch(method) {
        case METHOD_GET:
            curl_easy_setopt( curl_, CURLOPT_HTTPGET, 1 );
            _TRACE("Doing HTTP GET (" << method << ")");
            break;
        case METHOD_DELETE:
            curl_easy_setopt( curl_, CURLOPT_CUSTOMREQUEST, "DELETE" );
            _TRACE("Doing HTTP DELETE (" << method << ")");
            break;
        case METHOD_POST:
            curl_easy_setopt( curl_, CURLOPT_POST, 1 );
            _TRACE("Doing HTTP POST (" << method << ")");
            break;
        case METHOD_PUT:
            curl_easy_setopt( curl_, CURLOPT_UPLOAD, 1 );
            _TRACE("Doing HTTP PUT (" << method << ")");
            break;
        case METHOD_HEAD:
            curl_easy_setopt( curl_, CURLOPT_NOBODY, 1 );
            _TRACE("Doing HTTP HEAD (" << method << ")");
            break;
        default:
            gbTHROW( HttpClientError("setup got a nonsensical method!") );
    }

    // set a response timeout if necessary
    if ( timeout_[method] > 0 ) {
        _DEBUG("setting timeout to " << timeout_[method]);
        curl_easy_setopt( curl_, CURLOPT_TIMEOUT, timeout_[method] );
    }

    // if we have not recieved any data for idle_timeout_[method] seconds
    // then abort the request
    if ( idle_timeout_[method] > 0 ) {
        _DEBUG("setting idle timeout to " << idle_timeout_[method]);
        curl_easy_setopt( curl_, CURLOPT_LOW_SPEED_LIMIT, 1);
        curl_easy_setopt( curl_, CURLOPT_LOW_SPEED_TIME, idle_timeout_[method] );
    }

    curl_easy_setopt(curl_, CURLOPT_SEEKFUNCTION, _seek_response);

    // setup cookie jar if used
    if( ! cookiejar_.empty() ) {
        curl_easy_setopt(curl_, CURLOPT_COOKIEFILE, cookiejar_.c_str());
        curl_easy_setopt(curl_, CURLOPT_COOKIEJAR,  cookiejar_.c_str());
    }
    else if( ! global_cookiejar_.empty() ) {
        curl_easy_setopt(curl_, CURLOPT_COOKIEFILE, global_cookiejar_.c_str());
        curl_easy_setopt(curl_, CURLOPT_COOKIEJAR,  global_cookiejar_.c_str());
    }
    
    // setup cookie data
    if( ! cookies_.empty() ) {
        std::map<std::string,std::string>::iterator itr = cookies_.begin();
        std::map<std::string,std::string>::iterator end = cookies_.end();
        std::string cookie_str;
        for( ; itr != end; ++itr ) {
            if( !cookie_str.empty() ) {
                cookie_str += "; ";
            }
            cookie_str += itr->first + "=" + itr->second;
        }
        curl_easy_setopt(curl_, CURLOPT_COOKIE, cookie_str.c_str());
    }

}
