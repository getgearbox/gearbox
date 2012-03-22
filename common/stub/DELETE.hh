#include <gearbox/core/Json.h>
#include <gearbox/core/Errors.h>
#include <string>

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <gearbox/core/HttpClient.h>
namespace bfs=boost::filesystem;

int DELETE_removes_files = 0;

long DELETE (const std::string & url, Json & response) {
    Uri uri(url);
    std::ostringstream oss;
    oss << uri.scheme() << "://" << uri.hostname() << '_' << uri.port() << "/" << uri.path();
    std::string urldir = oss.str();

    bfs::path file = bfs::path("delete") / bfs::path( urldir.substr(urldir.find('/',6)+1 ), bfs::no_check );
    if( bfs::exists(file) && bfs::is_directory(file) ) {
        file /= "index";
    }
    if( ! bfs::exists(file) ) {
        throw ERR_NOT_FOUND("Failed to find " + file.string());
    }
    response.parseFile(file.string());
    return 200;
}


// the httpclient implementation actually deletes the file 
HttpResponse HttpClient::DELETE(const Uri& uri, std::string& content) { 
    std::ostringstream oss;
    oss << uri.scheme() << "://" << uri.hostname() << '_' << uri.port() << "/" << uri.path();
    std::string url = oss.str();

    bfs::path file = bfs::path("delete") / bfs::path( url.substr(url.find('/',6)+1 ), bfs::no_check );

    if( ! bfs::exists(file) ) {
        throw ERR_NOT_FOUND("Failed to find " + file.string());
    }

    content = slurp(file.string());
    if( DELETE_removes_files ) {
        bfs::remove_all(file);
    }

    return HttpResponse(200);
}

