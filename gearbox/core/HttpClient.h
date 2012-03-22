#ifndef GEARBOX_HTTPCLIENT_H
#define GEARBOX_HTTPCLIENT_H

#include <gearbox/core/Uri.h>

#include <boost/filesystem/operations.hpp>
#include <map>
#include <string>

namespace Gearbox { /*}*/

class HttpClientError : public std::exception 
{
  public:
    HttpClientError(const std::string & msg);
    virtual ~HttpClientError() throw ();
    const char * what() const throw ();

  private:
    char buffer[512];
};

class HttpRetryError : public HttpClientError {
public:
    HttpRetryError(const std::string & msg);
};

class RetryTimeoutError : public HttpRetryError {
public:
    RetryTimeoutError(const std::string & msg);
};

class RetryConnectError: public HttpRetryError {
public:
    RetryConnectError(const std::string & msg);
};

class HttpResponse
{
  public:
    HttpResponse(long code);
    HttpResponse();
    HttpResponse(const HttpResponse & src);
    HttpResponse & operator = (const HttpResponse & src);
    ~HttpResponse();
    long code() const;
    bool get_header(const std::string & field, std::string & out) const;
    std::string header(const std::string & field) const;
    void get_headers(std::map<std::string, std::string> & headers) const;
    class Private;
    HttpResponse(const Private & src);
  private:
    Private * impl;
};

class HttpClient
{
  public:
    enum method_t 
    {
        METHOD_GET,
        METHOD_DELETE,
        METHOD_POST,
        METHOD_PUT,
        METHOD_HEAD,
        METHOD_UNKNOWN
    };
        
    HttpClient();
    HttpClient(const std::map<std::string, std::string> & headers);
    virtual ~HttpClient();
    HttpClient(const HttpClient & src);
    HttpClient & operator = (const HttpClient & src);
    
    std::string get_http_proxy() const;
    void set_http_proxy(const std::string & hostport);
    std::string get_socks_proxy() const;
    void set_socks_proxy(const std::string & hostport);
    std::string get_header(std::string header) const;
    void set_header(std::string header, const std::string & value);
    bool follow_redirects() const;
    void follow_redirects(bool follow);
    void set_content_type(const std::string & type);
    void set_accept_type(const std::string & type);
    long get_timeout(const method_t method) const;
    void set_timeout(const method_t method, long secs);
    int get_timeout_attempts(const method_t method) const;
    void set_timeout_attempts(const method_t method, int attempts);
    long get_idle_timeout(const method_t method) const;
    void set_idle_timeout(const method_t method, long secs);

    static void set_global_cookiejar(const std::string & file);
    void set_cookiejar(const std::string & file);
    void set_cookie(const std::string & name, const std::string & value);
    void write_cookiejar();
    
    HttpResponse DELETE(const Uri & uri, std::string & content);
    HttpResponse DELETE(const Uri & uri, const boost::filesystem::path & file);
    HttpResponse GET(const Uri & uri, std::string & content);
    HttpResponse GET(const Uri & uri, const boost::filesystem::path & file);
    HttpResponse HEAD(const Uri & uri);
    HttpResponse POST(const Uri & uri, const std::string & body, std::string & content);
    HttpResponse PUT(const Uri & uri, const std::string & body, std::string & content);
    HttpResponse PUT(const Uri & uri, const boost::filesystem::path & file, std::string & content);

  protected:
    virtual HttpResponse make_request(const Uri & uri);

  private:
    class Private;
    Private * impl;
};

/*{*/ } /* end namespace Gearbox */

#ifndef GEARBOX_HTTPCLIENT_NO_IMPORT
// imported in order to maintain source-compatibility with the old API
using Gearbox::HttpResponse;
using Gearbox::HttpClient;
#endif // GEARBOX_HTTPCLIENT_NO_IMPORT

#endif // GEARBOX_HTTPCLIENT_H
