#include <gearbox/core/Json.h>
#include <gearbox/core/Errors.h>
#include <gearbox/job/JobManager.h>
#include <string>
#include <sstream>
#include <gearbox/core/Uri.h>
#include <map>
#include <gearbox/core/HttpClient.h>

#include <boost/lexical_cast.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
namespace bfs=boost::filesystem;

static int counter = 1;

// If negative results or non-200 http codes are wanted, fill in the response
// map to map URIs to http codes (exceptions will be thrown for codes >= 400)
// e.g. response_map["http://localhost:4080/deny"] = 403;
static std::map<std::string, long> response_map;


long POST(const std::string & url, const Json & data, Json & response) {
    long response_code = 200;
    Uri uri(url);
    std::ostringstream oss;
    oss << uri.scheme() << "://" << uri.hostname() << '_' << uri.port() << "/" << uri.path();
    std::string urldir = oss.str();

    if (response_map.count(url)) {
        if (response_map[url] >= 400) {
            throw_from_code(response_map[url], "Error from " + url);
        }
        else {
            response_code = response_map[url];
        }
    }

    bfs::path file = bfs::path("post") / bfs::path( urldir.substr(urldir.find('/',6)+1 ), bfs::no_check );
    if( bfs::exists(file) && bfs::is_directory(file) ) {
        std::stringstream ss;
        ss << counter++;
        std::string num;
        ss >> num;
        file /= num;
    }

    if( ! bfs::exists(file) ) {
        // make up a status
        response["progress"] = 0;
        std::string base = uri.scheme() + "://" + uri.hostname() + ":" + boost::lexical_cast<std::string>(uri.port()) + "/";

        std::vector<std::string> parts;
        split(uri.path(), "/", parts);
        std::string resource;
        if( parts.size() >= 3 ) {
            resource = parts[2];
        }

        std::string rid;
        Gearbox::JobManager::get_rsrcid(resource.substr(0,1), rid );
        if( parts.size() >= 4 ) {
            rid = parts[3];
        }
        std::string uuid;
        uuid_b32c(uuid, false);

        response["uri"] = base + parts[0] + "/" + parts[1] + "/" + parts[2] + "/" + rid;
        response["status_uri"] = base + parts[0] + "/" + parts[1] + "/status/s-" + uuid;
    }
    else {
        response.parseFile(file.string());
    }
    return response_code;
}

HttpResponse HttpClient::POST(const Uri & uri, const std::string & body, std::string & resp) {
    long response_code = 200;
    std::string error_msg;
    std::ostringstream oss;
    oss << uri.scheme() << "://" << uri.hostname() << '_' << uri.port() << "/" << uri.path();
    std::string urldir = oss.str();

    Json response;

    if (response_map.count(uri.canonical())) {
        response_code = response_map[uri.canonical()];
        if (response_map[uri.canonical()] >= 400) {
            response["code"] = response_code;
            response["message"] = "Error from " + uri.canonical();
            response["path"] = uri.path();
            response["method"] = "POST";
        }
        resp = response.serialize();
        return HttpResponse(response_code);
    }

    bfs::path file = bfs::path("post") / bfs::path( urldir.substr(urldir.find('/',6)+1 ), bfs::no_check );
    if( bfs::exists(file) && bfs::is_directory(file) ) {
        std::stringstream ss;
        ss << counter++;
        std::string num;
        ss >> num;
        file /= num;
    }

    if( ! bfs::exists(file) ) {
        // make up a status
        response["progress"] = 0;
        std::string base = uri.scheme() + "://" + uri.hostname() + ":" + boost::lexical_cast<std::string>(uri.port()) + "/";

        std::vector<std::string> parts;
        split(uri.path(), "/", parts);
        std::string resource;
        if( parts.size() >= 3 ) {
            resource = parts[2];
        }

        std::string rid;
        Gearbox::JobManager::get_rsrcid(resource.substr(0,1), rid );
        if( parts.size() >= 4 ) {
            rid = parts[3];
        }
        std::string uuid;
        uuid_b32c(uuid, false);

        response["uri"] = base + parts[0] + "/" + parts[1] + "/" + parts[2] + "/" + rid;
        response["status_uri"] = base + parts[0] + "/" + parts[1] + "/status/s-" + uuid;
    }
    else {
        response.parseFile(file.string());
    }
    resp = response.serialize();

    return HttpResponse(response_code);
}
