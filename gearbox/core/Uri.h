#ifndef GEARBOX_URI_H
#define GEARBOX_URI_H

#include <vector>
#include <string>
#include <boost/filesystem/operations.hpp>

namespace Gearbox {

class UriError : public std::exception 
{
  public:
    UriError(const::std::string msg);
    virtual ~UriError() throw ();
    const char * what() const throw ();

  private:
    char buffer[512];
};

class Uri
{

  public:
    Uri();
    Uri(const char * uristr);
    Uri(const std::string & uristr);
    Uri(const boost::filesystem::path & bfspath);
    Uri(const Uri &);
    Uri & operator=(const Uri &);
    ~Uri();

    // a reusable helper
    static std::string canonicalize_path(const std::string& path);
    static std::string urlescape_part(const std::string & part );

    // accessors
    const std::string & uristr() const;

    const std::string & scheme() const;
    void scheme(const std::string & scheme);

    const std::string & username() const;
    void username(const std::string & username);

    const std::string & password() const;
    void password(const std::string & password);

    const std::string & hostname() const;
    void hostname(const std::string & hostname);

    int port() const;
    void port(const int port);

    const std::string & path() const;
    std::string leaf() const;
    const std::string canonical_path() const;
    void path(const std::string & path);
    void path_push(const std::string & part);
    void path_pop();

    const std::string & query() const;
    void query(const std::string & query);
    void query_push(const std::string & part);

    const std::string & fragment() const;
    void fragment(const std::string & fragment);

    const std::string authority() const;
    const std::string hostport() const;
    const std::string str() const;
    const std::string canonical() const;

    // path sugar
    Uri & operator /=(const std::string & part);
    Uri operator /(const std::string & part) const;

    bool is_http() const;
    bool is_https() const;
    bool is_httpx() const;
    bool is_file() const;

  protected:
    void parse_authority(const std::string & authority);
    void parse_uristr(const std::string & uristr);
    
  private:
    class Private;
    Private * impl;
};

bool operator ==(const Uri & lhs, const Uri & rhs);
bool operator !=(const Uri & lhs, const Uri & rhs);

} // namespace

#endif // GEARBOX_URI_
