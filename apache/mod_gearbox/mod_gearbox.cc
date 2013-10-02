// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/core/ConfigFile.h>

#ifdef HAVE_CONFIG_H
// Apache's ap_config_auto.h and gearbox's config.h conflict.
// This is questionable workaround.  Alternatives?
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#endif

#define LOGCAT "gearbox.httpd"
#include <gearbox/core/logger.h>
#include "log4cxx/propertyconfigurator.h"
#include <gearbox/core/Errors.h>
#include <gearbox/core/strlcpy.h>
#include <gearbox/job/JobManager.h>

#include "httpd.h"
#include "http_config.h"
#include "http_core.h"
#include "http_protocol.h"
#include "ap_config.h"

#include <boost/filesystem/operations.hpp>
namespace bfs=boost::filesystem;
#include <boost/algorithm/string.hpp>

using namespace Gearbox;

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
using std::string;

typedef struct {
    ConfigFile * cfg;
    JobManager * jm;
    std::string * component;
    const char * path_prefix;
    int sync_put;
    int sync_post;
    int sync_delete;
} gearbox_cfg;

static const char *cmd_gearbox_logconfig(cmd_parms *cmd, void *mconfig, const char *arg);
static const char *cmd_gearbox_config_file(cmd_parms *cmd, void *mconfig, const char *arg);
static const char *cmd_gearbox_sync(cmd_parms *cmd, void *mconfig, int argc, const char **argv);
static void read_content(request_rec * r, string & content);
static void *gearbox_create_dir_config(apr_pool_t *p, char *dirspec);
static void gearbox_register_hooks(apr_pool_t *p);

static const command_rec gearbox_cmds[] =
{
    AP_INIT_TAKE1(
        "GearboxConfigFile",                        /* directive name */
        (const char* (*)())cmd_gearbox_config_file, /* config action routine */
        NULL,                                       /* argument to include in call */
        OR_OPTIONS,                                 /* where available */
        "Gearbox Config File - one argument"        /* directive description */
    ),
    AP_INIT_TAKE1(
        "GearboxLogConfig",                                 /* directive name */
        (const char* (*)())cmd_gearbox_logconfig,           /* config action routine */
        NULL,                                               /* argument to include in call */
        OR_OPTIONS,                                         /* where available */
        "GearboxLogConfig File <path to log4j config file>" /* directive description */
    ),
    AP_INIT_TAKE_ARGV(
        "GearboxSync",
        (const char* (*)())cmd_gearbox_sync,                /* config action routine */
        NULL,                                               /* argument to include in call */
        OR_OPTIONS,                                         /* where available */
        "GearboxSync <PUT POST DELETE>"                     /* directive description */
    ),
    {NULL}
};

module AP_MODULE_DECLARE_DATA gearbox_module =
{
    STANDARD20_MODULE_STUFF,
    gearbox_create_dir_config,  /* per-directory config creator */
    NULL,                       /* dir config merger */
    NULL,                       /* server config creator */
    NULL,                       /* server config merger */
    gearbox_cmds,               /* command table */
    gearbox_register_hooks,     /* set up other request processing hooks */
};

static gearbox_cfg *our_dconfig(const request_rec *r) {
    return (gearbox_cfg *) ap_get_module_config(r->per_dir_config, &gearbox_module);
}

static void strtolower(string & str) {
    std::transform(str.begin(), str.end(), str.begin(), tolower);
}

static const char *cmd_gearbox_sync(cmd_parms *cmd, void *mconfig, int argc, const char **argv ) {
    if( cmd->directive && cmd->directive->parent && cmd->directive->parent->directive ) {
        string directive(cmd->directive->parent->directive);
        strtolower(directive);
        if( directive.find("location") == string::npos ) {
            std::cerr << "The GearboxSync variable must be specified in a <Location> block"
                      << std::endl;
            exit(1);
        }
    }
    gearbox_cfg * cfg = (gearbox_cfg *) mconfig;
    for ( int i=0; i < argc; i++ ) {
        string method(argv[i]);
        strtolower(method);
        if ( method == "post" ) {
            cfg->sync_post = 1;
        }
        else if ( method == "put" ) {
            cfg->sync_put = 1;
        }
        else if ( method == "delete" ) {
            cfg->sync_delete = 1;
        }
        else {
            std::cerr << "Acceptable arguments for GearboxSync are PUT, POST, DELETE"
                      <<std::endl;
            exit(1);
        }
    }
    return NULL;
}

static const char *cmd_gearbox_logconfig(cmd_parms *cmd, void *mconfig, const char *arg) {
    log4cxx::PropertyConfigurator::configure( arg );
    _DEBUG("Logger Initialized");
    return NULL;
}

static const char *cmd_gearbox_config_file(cmd_parms *cmd, void *mconfig, const char *arg) {
    gearbox_cfg * cfg = (gearbox_cfg *) mconfig;
    cfg->cfg = new ConfigFile(arg);
    cfg->jm = new JobManager( *cfg->cfg );
    cfg->path_prefix = cmd->path;
    std::string component = cfg->cfg->get_string("component");
    if( !component.empty() ) {
        if( cfg->component ) delete cfg->component;
        cfg->component = new std::string(component);
    }

    // so in case where location block is something like
    // /rm/v1/resource  we want the /v1/resource removed
    // and path_prefix is just /rm, so we look for hte /vXXX
    // version pattern an remove it
    std::string path(cmd->path);
    std::string ver("/v");
    size_t ix = 0;
    while( (ix = path.find(ver,ix+1)) != string::npos ) {
        // get full segment up to next '/'
        size_t nx = path.find('/',ix+1);
        // verify the ver part is all digits
        // like /v123 and not /verbose
        std::string verpart = path.substr(ix+2, nx-ix-2);
        int allver = 1;
        for( unsigned int i=0; i < verpart.size(); ++i ) {
            if( ! std::isdigit(verpart[i]) ) {
                allver = 0;
                break;
            }
        }
        // must have found path like /very that does not match our pattern
        if( !allver ) {
            continue;
        }
        std::string subpath = path.substr(0,ix);
        char * path_prefix = (char*)apr_pcalloc(cmd->pool, sizeof(char) * subpath.size() + 1);
        Gearbox::strlcpy(path_prefix, subpath.c_str(), subpath.size()+1);
        cfg->path_prefix = path_prefix;
        break;
    }
    return NULL;
}

/* random helper stuff here */

static int is_sync_method( const string & method, void *mconfig ) {
    gearbox_cfg * cfg = (gearbox_cfg *) mconfig;
    if ( method == "head" || method == "get" ) return 1;
    if ( method == "post" && cfg->sync_post ) return 1;
    if ( method == "put" && cfg->sync_put ) return 1;
    if ( method == "delete" && cfg->sync_delete ) return 1;
    return 0;
}

static int copy_table_tolower(void *dest, const char *key, const char *value) {
     Hash & m = *(static_cast<Hash*>(dest));
     string lkey(key);
     strtolower(lkey);
     m[lkey] = value;
     return 1;
}

static int copy_table_raw(void *dest, const char *key, const char *value) {
     Hash & m = *(static_cast<Hash*>(dest));
     string lkey(key);
     if( value )
         m[lkey] = value;
     else
         m[lkey] = "";
     return 1;
}

static void split_path_args(
    const string & path_args,
    std::vector<string> & parts,
    Hash & matrix_args
) {
    if( path_args.empty() ) 
        return;

    // path_args always starts with /, so split after first char
    // so first element of parts[] is not an empty string
    string pathfixup = path_args.substr(1);

    // last arg might be like nodes;tier_id=t-1234
    if( !pathfixup.empty() && pathfixup.find(';') != string::npos ) {
        std::vector<string> matrix;
        boost::split(matrix, pathfixup, boost::is_any_of(";="));
        if( matrix.size() % 2 == 0 ) {
            //size is even which means there is a bogus matrix arg
            gbTHROW( ERR_BAD_REQUEST("malformed matrix arguments in '" + pathfixup + "'") );
        }

        if( !matrix[0].empty() ) {
            pathfixup = matrix[0];
        }
        for( unsigned int i=1; i< matrix.size(); i+=2 ) {
            matrix_args[matrix[i]] = matrix[i+1];
        }
    }

    boost::split(parts, pathfixup, boost::is_any_of("/"));
    if( parts.back().empty() ) {
        // last part is an empty string:
        parts.pop_back();
    }
}

static std::string hosturi ( request_rec * r ) {
    std::ostringstream uri;
    uri << ap_http_scheme(r) << "://" << ap_get_server_name(r);
    if( !ap_is_default_port(ap_get_server_port(r),r) ) {
        uri << ":" << ap_get_server_port(r);
    }
    return uri.str();
}

static int error_response( request_rec * r, int code, const string & message ) {
    Json status;
    status["uri"] = hosturi(r) + r->uri;
    status["progress"] = 100;
    status["state" ] = "COMPLETED";
    string meth = r->method;
    strtolower(meth);
    if( meth == "post" ) {
        meth = "update";
    }
    else if( meth == "put" ) {
        meth = "create";
    }
    status["operation"] = meth;
    
    try {
        _ERROR(message);
        status["code"] = code;
        status["messages"][0] = message;
    }
    catch( ... ) {
        std::cerr << "Error while trying to log message: \"" << message << "\". Disk Full?" << std::endl;
        status["code"] = 500;
        status["messages"][0] = "Internal Server Error.  Disk Full?";
    }

    r->status = status["code"].as<int>();
    string err = status.serialize();
    apr_table_set(r->notes, "gearbox-output", err.c_str());
    ap_rputs(err.c_str(), r);
    return DONE;
}

static void read_content( request_rec * r, string & content)
{   
    content.clear();
    ap_setup_client_block(r, REQUEST_CHUNKED_DECHUNK);
    if( ap_should_client_block(r) ) {
        // there is a body, so lets read it
        long read;
        char buffer[512];
        while( (read = ap_get_client_block(r, buffer, 512)) > 0 ) {
            content.append(buffer, read);
        }
    }
    // stuff the content in the notes for any other module
    // to poke at it
    apr_table_set(r->notes,"gearbox-input",content.c_str());
}

static int gearbox_handler(request_rec *r) {
    if (strcmp(r->handler, "gearbox-handler")) {
        return DECLINED;
    }
    if ( bfs::exists( r->filename ) ) {
        if ( bfs::is_directory( r->filename ) ) {
            r->filename = ap_make_full_path( r->pool, r->filename, "index.html" );
            apr_stat( &(r->finfo), r->filename, APR_FINFO_TYPE | APR_FINFO_SIZE | APR_FINFO_CSIZE, r->pool );
            ap_set_content_type( r, "text/html" );       
        }
        return DECLINED;
    }
    try {
        gearbox_cfg * dcfg(our_dconfig(r));

        _DEBUG("uri: " << r->uri);
        _DEBUG("filename: " << r->filename);
        _DEBUG("path_info: " << r->path_info);
        _DEBUG("args: " << r->args);	
        _DEBUG("path prefix: " << dcfg->path_prefix);

        ap_no2slash(r->uri);
        ap_set_content_type(r, "application/json");
        
        string req_uri(r->uri);
        string path_info = req_uri.erase(0, string(dcfg->path_prefix).size());
        
        std::vector<string> path;
        Hash matrix_args;
        split_path_args(path_info, path, matrix_args);
        
        for(unsigned int i=0; i< path.size(); i++) {
            _DEBUG("part[" << i << "] = " << path[i]);
        }

        Hash::iterator itr = matrix_args.begin();
        for( ; itr != matrix_args.end(); ++itr ) {
            _DEBUG("matrix arg: " << itr->first << " => " << itr->second);
        }
        
        if( path.size() < 2 || path[0][0] != 'v' || !std::isdigit(path[0][1]) ) {
            gbTHROW( ERR_BAD_REQUEST("Invalid API version designation.") );
        }
        string version = path[0];
        // shift
        path.erase(path.begin());

        if( path.size() < 1 || path[0].empty() ) {
            gbTHROW( ERR_BAD_REQUEST("Incomplete API uri specified - no resource.") );
        }
        string resource = path[0];
        // shift
        path.erase(path.begin());

        std::string base_uri = hosturi(r);
        base_uri += dcfg->path_prefix;
        base_uri += "/";
        base_uri += version;


        if (r->header_only) {
            ap_rputs("",r);
            return DONE;
        }

        // this is the gearman function we are going to call
        string method(r->method);
        strtolower(method);
        
        string function = "do_" + method + "_" + *dcfg->component + "_" + resource + "_" + version;

        _DEBUG("worker trying function: ->" << function << "<-" );
        if ( ! dcfg->jm->known_job_name( function ) ) {
            function = "do_" + method + "_global_" + resource + "_" + version;
            // check for a 'global' handler function for this request
            if ( ! dcfg->jm->known_job_name( function ) ) {
                gbTHROW( ERR_BAD_REQUEST( "No valid handler found!" ) );
            }
        }
        _DEBUG("found function: " << function);

        Json output;

        // copy headers, environ data into job data
        Hash headers, environ;
        apr_table_do(copy_table_tolower, (void*)&headers, r->headers_in, NULL);
        apr_table_do(copy_table_raw, (void*)&environ, r->subprocess_env, NULL);

        // FIXME remote_ip should probably be in env
        string remote_ip;
        
        dcfg->jm->base_uri(base_uri);
        JobPtr job = dcfg->jm->job(function);
        job->remote_ip(remote_ip);
        job->headers(headers);
        job->environ(environ);
        job->arguments(path);
        job->matrix_arguments(matrix_args);

        if( r->args ) {
            // we have query_params so set them
            std::vector<std::string> args;
            boost::split(args, r->args, boost::is_any_of(";&"));
            for( unsigned int i=0; i < args.size(); ++i ) {
                size_t eqpos;
                if( (eqpos = args[i].find('=')) == std::string::npos ) {
                    // no = so just assume value "1" for key/value pair
                    job->add_query_param(args[i], "1");
                }
                else {
                    std::string key, value;
                    job->add_query_param(
                        args[i].substr(0,eqpos),
                        args[i].substr(eqpos+1)
                    );
                }
            }
        }

        if( method == "post" && path.empty() ) {
            job->operation("create");
            job->resource_name( JobManager::gen_id( string(1,job->resource_type()[0]) ) );
        }
        else if( !path.empty() ) {
            job->resource_name( path[0] );
        }
        
        string input;
        read_content(r, input);

        if( !input.empty() )
            job->content(input);

        if( is_sync_method( method, dcfg ) ) {
            job->type(Job::JOB_SYNC);
        }

        // execute the job (blocking operation)
        JobResponse resp = job->run();

        // process and set any HTTP output headers set by the worker function
        const Hash & headers_out = resp.headers();
        Hash::const_iterator it  = headers_out.begin();
        Hash::const_iterator end = headers_out.end();
        apr_table_t * reqheaders = resp.code() >= 400 ? r->err_headers_out : r->headers_out;
        for( ; it != end; ++it ) {
            apr_table_set(reqheaders, it->first.c_str(), it->second.c_str());
        }

        if ( resp.code() >= 400 ) {
            throw_from_code( resp.code(), resp.status()->messages()[0] );
        }

        if( ! job->resource_name().empty() ) {
            string loc = base_uri + "/" + job->resource_type() + "/" + job->resource_name();
            apr_table_set(r->headers_out,"location",loc.c_str());
        }

        if( job->type() == Job::JOB_SYNC ) {
            _DEBUG("OUTPUT: " << resp.content());
            apr_table_add(r->notes, "gearbox-output", resp.content().c_str());
            ap_rputs(resp.content().c_str(), r);
            r->status = resp.code() == 0 ? 200 : resp.code();
        }
        else {
            const std::string & status = resp.status()->serialize();
            _DEBUG("OUTPUT: " << status);
            apr_table_add(r->notes, "gearbox-output", status.c_str());
            ap_rputs(status.c_str(), r);
            r->status = HTTP_ACCEPTED;
        }
        return DONE;
    }
    catch (const Error & err ) {
        return error_response(r, err.code(), err.what());
    }
    catch (const JsonError & err ) {
        return error_response(r, HTTP_BAD_REQUEST, err.what());
    }
    catch ( const std::exception & err ) {
        return error_response(r, HTTP_INTERNAL_SERVER_ERROR, err.what());
    }
    catch ( ... ) {
        // only reach this point in the event of an uncaught exception
        return error_response(r, HTTP_INTERNAL_SERVER_ERROR,
                              "Uncaught exception in handler! This should not happen.");
    }
}

static void *gearbox_create_dir_config(apr_pool_t *p, char *dirspec) {
    gearbox_cfg *cfg = (gearbox_cfg *) apr_pcalloc(p, sizeof(gearbox_cfg));

    if( bfs::exists( SYSCONFDIR "/gearbox/logger/httpd-logger.conf" ) ) {
        log4cxx::PropertyConfigurator::configure( SYSCONFDIR "/gearbox/logger/httpd-logger.conf" );
        log4cxx::MDC::put("pid", boost::lexical_cast<std::string>(getpid()));
        _DEBUG("Logger Initialized");
    }
        
    cfg->cfg = NULL;
    cfg->jm = NULL;
    cfg->component = new string("gearbox");
    cfg->path_prefix = "";
    cfg->sync_put = 0;
    cfg->sync_post = 0;
    cfg->sync_delete = 0;

    return (void *) cfg;
}

static void gearbox_register_hooks(apr_pool_t *p) {
    ap_hook_handler(gearbox_handler, NULL, NULL, APR_HOOK_MIDDLE);
}

