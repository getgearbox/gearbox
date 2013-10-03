#include <gearbox/core/Json.h>
#include <gearbox/core/Errors.h>
#include <gearbox/core/util.h>
#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
namespace bfs=boost::filesystem;

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
namespace ba=boost::algorithm;


#include <gearbox/core/HttpClient.h>

HttpResponse
HttpClient::GET(const Uri & uri, std::string & content ) {
    std::ostringstream oss;
    std::string path = uri.path();

    // newer boost changes how they deal with trailing /
    // so remove it hear for our stub
    if( path.rfind("/") == path.length()-1 ) {
        path = path.substr(0, path.length()-1);
    }
    oss << uri.scheme() << "://" << uri.hostname() << '_' << uri.port() << "/" << path;
    std::string url = oss.str();

    bfs::path infile = bfs::path("get") / bfs::path( url.substr(url.find('/',6)+1 ));
    
    if( bfs::exists(infile) && bfs::is_directory(infile) ) {
        infile /= "index";
    }
    if( ! bfs::exists(infile) ) {
        // if not in ./get look in ./put to see if we just "uploaded" it
        infile = bfs::path("put") / bfs::path( url.substr(url.find('/',6)+1 ));
        if( !bfs::exists(infile) ) {
            // if this is a status, just make up a success response

            std::vector<std::string> parts;
            ba::split(parts, uri.path(), boost::is_any_of("/"));
            if( parts.size() >= 4 && parts[2] == "status" ) {            
                Json response;
                // make up a status
                response["progress"] = 100;
                response["status"] = 0;
          
                std::string base = uri.scheme() + "://" + uri.hostname() + ":" + boost::lexical_cast<std::string>(uri.port()) + "/";
                
		std::string uuid;
		uuid_b32c(uuid, false);
                response["uri"] = base + parts[0] + "/" + parts[1] + "/unknown/u-" + uuid;
                response["status_uri"] = base + uri.path();
                content = response.serialize();
            }
            else {
                return HttpResponse(404);
            }
        }
        else {
            content = slurp(infile.string());
        }
    }
    else {
        content = slurp(infile.string());       
    }

    return HttpResponse(200);
}

HttpResponse
HttpClient::GET(const Uri & uri, const boost::filesystem::path & file ) {
    std::ostringstream oss;
    oss << uri.scheme() << "://" << uri.hostname() << '_' << uri.port() << "/" << uri.path();
    std::string url = oss.str();

    bfs::path infile = bfs::path("get") / bfs::path( url.substr(url.find('/',6)+1 ));
    if( bfs::exists(infile) && bfs::is_directory(infile) ) {
        infile /= "index";
    }
    if( ! bfs::exists(infile) ) {
        // if not in ./get look in ./put to see if we just "uploaded" it
        infile = bfs::path("put") / bfs::path( url.substr(url.find('/',6)+1 ));
        if( !bfs::exists(infile) ) {
            return HttpResponse(404);
        }
    }
    bfs::copy_file(infile, file);
    return HttpResponse(200);
}
