#include <libgearman/gearman.h>
#include <gearbox/core/strlcpy.h>
#include <gearbox/core/Json.h>
#include <gearbox/core/util.h>
#include <map>

std::map<std::string, bool> error;
std::map<std::string,Gearbox::Json> response;
static std::string gFunctionName;
static std::string gWorkload = "{}";
static gearman_worker_fn * oneWorker = NULL;
static std::map<std::string, void *> workerArgs;

#define ERROR(retval) { if( error[__func__] ) { return retval; } }

extern "C" {

gearman_worker_st *
gearman_worker_create(gearman_worker_st *worker) {
    ERROR(NULL);
    return worker;
}

gearman_return_t 
gearman_worker_add_server(gearman_worker_st *worker,
                          const char *host, in_port_t port) {
    ERROR(GEARMAN_SERVER_ERROR);
    return GEARMAN_SUCCESS;
}

void
gearman_worker_free(gearman_worker_st *worker) {
}

gearman_return_t
gearman_worker_add_function(gearman_worker_st *worker,
                            const char *function_name,
                            uint32_t timeout,
                            gearman_worker_fn *worker_fn,
                            void *fn_arg) {
    ERROR(GEARMAN_SERVER_ERROR);
    oneWorker = worker_fn;
    workerArgs[function_name] = const_cast<void*>(fn_arg);
    return GEARMAN_SUCCESS;
}

const char *
gearman_worker_error(const gearman_worker_st *worker) {
    return "gearmand error";
}

gearman_return_t
gearman_worker_unregister(gearman_worker_st *worker,
                          const char *function_name) {
    ERROR(GEARMAN_SERVER_ERROR);
    return GEARMAN_SUCCESS;
}

gearman_return_t
gearman_worker_work(gearman_worker_st *worker) {
    ERROR(GEARMAN_SERVER_ERROR);
    if( oneWorker ) {
        size_t rs;
        gearman_return_t ret;
        void * result = oneWorker(reinterpret_cast<gearman_job_st*>(1), workerArgs[gFunctionName], &rs, &ret);
        free(result);
    }
    return GEARMAN_SUCCESS;
}

const void *
gearman_job_workload(const gearman_job_st *job) {
    ERROR(NULL);
    return gWorkload.c_str();
}

size_t 
gearman_job_workload_size(const gearman_job_st *job) {
    ERROR(0);
    return gWorkload.size();
}

gearman_client_st *
gearman_client_create(gearman_client_st *client) {
    ERROR(NULL);
    return reinterpret_cast<gearman_client_st*>(1);
}

gearman_return_t
gearman_client_add_server(gearman_client_st *client,
                          const char *host, in_port_t port) {
    ERROR(GEARMAN_SERVER_ERROR);
    return GEARMAN_SUCCESS;
}

gearman_return_t
gearman_client_echo(gearman_client_st *client,
                    const void *workload,
                                     size_t workload_size) {
    ERROR(GEARMAN_SERVER_ERROR);
    return GEARMAN_SUCCESS;
}


void*
gearman_client_do(gearman_client_st *client,
                  const char *function_name,
                  const char *unique,
                  const void *workload,
                  size_t workload_size,
                  size_t *result_size,
                  gearman_return_t *ret_ptr
) {

    //define ERROR(retval) { if( error[__func__] ) { return retval; } }
    gFunctionName = function_name;
    gWorkload.clear();
    Gearbox::zlib_decompress(std::string((char*)workload,workload_size),gWorkload);
    
    

    if( error[__func__] ) {
        Gearbox::Json err;
        err["message"] = "error";
        err["status"] = "ERR_INTERNAL_SERVER_ERROR";
        err["code"] = 500;
        err["result"] = "";
        *result_size = err.serialize().size();
        *ret_ptr = GEARMAN_SERVER_ERROR;
        char * ret = (char*)malloc(*result_size+1);
        Gearbox::strlcpy(ret, err.serialize().c_str(), *result_size+1);
        _DEBUG("Output: " << err.serialize());
        return ret;
    }
    
    Gearbox::Json in;
    in.parse( gWorkload );
    _DEBUG("Input: " << in.serialize());
    in["params"] = Gearbox::Json::Object();
    if( in.hasKey("content") ) {
        in["params"].parse( in["content"].as<std::string>() );
    }
    
    Gearbox::Json out;
    out["message"] = "";
    out["status"]  = "OK";
    out["code"]    = 200;

    if( in.hasKey("resource") && in["resource"].hasKey("type") ) {
        if( in["resource"]["type"].as<std::string>() == "status" ) {
            if( in["operation"].as<std::string>() == "create" ) {
                Gearbox::Json status;
                status["uri"] = in["params"]["uri"];
                status["operation"] = in["params"]["operation"];
                status["status_uri"] = "http://localhost:4080/test/status/" + in["params"]["name"].as<std::string>();
                status["progress"] = 0;
                out["content"] = status.serialize();
            }
        }
    }
    
    if( ! out.hasKey("content") ) {
        if( ! response[function_name].empty() ) {
            out["content"] = response[function_name].serialize();
        }
        else {
            Gearbox::Json msg;
            msg["key"] = "value";
            out["content"] = msg.serialize();
        }
    }

    *result_size = out.serialize().size();
    *ret_ptr = GEARMAN_SUCCESS;
    char * ret = (char*)malloc(*result_size+1);
    Gearbox::strlcpy(ret, out.serialize().c_str(), *result_size+1);
    _DEBUG("Output: " << out.serialize());
    return ret;
}

gearman_return_t
gearman_client_do_background(gearman_client_st *client,
                             const char *function_name,
                             const char *unique,
                             const void *workload,
                             size_t workload_size,
                             char *job_handle) {
    ERROR(GEARMAN_SERVER_ERROR);
    gFunctionName = function_name;
    gWorkload = std::string((char*)workload,workload_size);
    return GEARMAN_SUCCESS;
}
    

const char *gearman_client_error(const gearman_client_st *client) {
    return "gearman client error";
}

}
