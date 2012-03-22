#include <gearbox/core/Json.h>
#include <gearbox/core/Errors.h>
#include <gearbox/core/util.h>
#include <string>

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
namespace bfs=boost::filesystem;

#include <gearbox/core/HttpClient.h>

HttpResponse
HttpClient::PUT(const Uri & uri, const std::string & input, std::string & output ) {
    std::ostringstream oss;
    oss << uri.scheme() << "://" << uri.hostname() << '_' << uri.port() << "/" << uri.path();
    std::string url = oss.str();

    bfs::path outfile = bfs::path("put") / bfs::path( url.substr(url.find('/',6)+1 ), bfs::no_check );
    
    std::string filename = outfile.leaf();
    if( filename == "bogus" ) {
        return HttpResponse(500);
    }
    bfs::path parent_dir(outfile.branch_path());
    if( !bfs::exists(parent_dir) ) {
        bfs::create_directories(parent_dir);
    }
    bfs::ofstream out(outfile, std::ios_base::out | std::ios_base::binary);
    out << input;
    out.close();
    output = "OK";
    return HttpResponse(200);
}

HttpResponse
HttpClient::PUT(const Uri & uri, const bfs::path & file, std::string & output ) {
    std::ostringstream oss;
    oss << uri.scheme() << "://" << uri.hostname() << '_' << uri.port() << "/" << uri.path();
    std::string url = oss.str();

    bfs::path outfile = bfs::path("put") / bfs::path( url.substr(url.find('/',6)+1 ), bfs::no_check );
    
    std::string filename = outfile.leaf();
    if( filename == "bogus" ) {
        return HttpResponse(500);
    }
    bfs::path parent_dir(outfile.branch_path());
    if( !bfs::exists(parent_dir) ) {
        bfs::create_directories(parent_dir);
    }
    bfs::copy_file(file, outfile);
    output = "OK";
    return HttpResponse(200);
}
