// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/core/REST.h>
#include <gearbox/core/Errors.h>

#define LOGCAT "gearbox.rest"
#include <gearbox/core/logger.h>

#include <gearbox/core/HttpClient.h>

namespace Gearbox {

static Headers gheaders;

static long _request (
    HttpClient::method_t method,
    const std::string & url, 
    const std::string & post, 
    const Headers & headers,
    Json & response)
{
    HttpClient c(headers);
    HttpResponse resp;
    
    if( c.get_header("content-type") == "" ) {
        c.set_content_type("application/json");
    }
    
    Headers::const_iterator itr = gheaders.begin();
    Headers::const_iterator end = gheaders.end();
    for( ; itr != end; ++itr ) {
        c.set_header( itr->first, itr->second );
    }
    
    std::string content;
    switch(method) {
    case HttpClient::METHOD_GET:
        resp = c.GET(url, content);
        break;
    case HttpClient::METHOD_DELETE:
        resp = c.DELETE(url, content);
        break;
    case HttpClient::METHOD_POST:
        resp = c.POST(url, post, content);
        break;
    case HttpClient::METHOD_PUT:
        resp = c.PUT(url, post, content);
        break;
    case HttpClient::METHOD_HEAD:
        resp = c.HEAD(url);
        break;
    default:
        gbTHROW( ERR_INTERNAL_SERVER_ERROR("Unknown method type") );
    }
    
    try {
        if( method != HttpClient::METHOD_HEAD )
            response.parse(content);
    }
    catch( const JsonError &  err ) {
        // ignore Json Errors in case server didnt return
        // JSON to us (error condidtion)
        _WARN("received invalid json: " << content);
    }

    if( response.type() == Json::UNDEF ) {
        // set it to an empty object if 
        // the response is null
        response = Json::Object();
    }

    if( resp.code() >= 400 ) {
        std::string meth = 
            method == HttpClient::METHOD_GET ? "GET" :
            method == HttpClient::METHOD_POST ? "POST" :
            method == HttpClient::METHOD_DELETE ? "DELETE" :
            method == HttpClient::METHOD_PUT ? "PUT" :
            method == HttpClient::METHOD_HEAD ? "HEAD" : 
            "UNKNOWN";
        if( response.hasKey("message") ) { 
            throw_from_code(resp.code(), "Failed to " + meth + " " + url + " got: " + response["message"].as<std::string>());
        } else {
            throw_from_code(resp.code(), "Failed to " + meth + " " + url + " got: invalid error response");
        }
    }
    return resp.code();
}

long HEAD   (const std::string &url) {
    Json bogus;
    return _request(HttpClient::METHOD_HEAD, url, "", Headers(), bogus);
}

long GET    (const std::string & url, Json & response) {
    return _request(HttpClient::METHOD_GET, url, "", Headers(), response);
}

long DELETE (const std::string & url, Json & response) {
    return _request(HttpClient::METHOD_DELETE, url, "", Headers(), response);
}

long POST   (const std::string & url, const Json & data, Json & response) {
    return _request(HttpClient::METHOD_POST, url, data.type() == Json::UNDEF ? "{ }" : data.serialize(), Headers(), response);
}

long PUT    (const std::string & url, const Json & data, Json & response) {
    return _request(HttpClient::METHOD_PUT, url, data.type() == Json::UNDEF ? "{ }" : data.serialize(), Headers(), response);
}


long HEAD   (const std::string &url, const Headers & headers) {
    Json bogus;
    return _request(HttpClient::METHOD_HEAD, url, "", headers, bogus);
}

long GET    (const std::string & url, const Headers & headers, Json & response) {
    return _request(HttpClient::METHOD_GET, url, "", headers, response);
}

long DELETE (const std::string & url, const Headers & headers, Json & response) {
    return _request(HttpClient::METHOD_DELETE, url, "", headers, response);
}

long POST   (const std::string & url, const Json & data, const Headers & headers, Json & response) {
    return _request(HttpClient::METHOD_POST, url, data.type() == Json::UNDEF ? "{ }" : data.serialize(), headers, response);
}

long PUT    (const std::string & url, const Json & data, const Headers & headers, Json & response) {
    return _request(HttpClient::METHOD_PUT, url, data.type() == Json::UNDEF ? "{ }" : data.serialize(), headers, response);
}

void REST::global_headers( const Headers & headers ) {
    gheaders = headers;
}

void REST::add_global_header( const std::string & key, const std::string & value ) {
    gheaders[key] = value;
}

void REST::del_global_header( const std::string & key ) {
    gheaders.erase(key);
}

} // namespace
