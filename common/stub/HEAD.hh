#include <string>

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
namespace bfs=boost::filesystem;

#include <gearbox/core/HttpClient.h>

HttpResponse
HttpClient::HEAD(const Uri & uri) {
    std::ostringstream oss;
    oss << uri.scheme() << "://" << uri.hostname() << '_' << uri.port() << "/" << uri.path();
    std::string url = oss.str();
    bfs::path infile = bfs::path("get") / bfs::path( url.substr(url.find('/',6)+1 ), bfs::no_check );
    if( ! bfs::exists(infile) ) {
        return HttpResponse(404);
    }
    return HttpResponse(200);
}
