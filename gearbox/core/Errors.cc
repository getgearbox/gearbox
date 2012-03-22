// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/core/Errors.h>

#include <sstream>
#include <vector>
#include <gearbox/core/strlcpy.h>
#include <gearbox/core/util.h>
#include <gearbox/core/TempFile.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

namespace Gearbox {

    // this is an odd hack to dump out a useful stack trace
    // while your program is running.  It is not fast or
    // efficient, but is accurate assuming you use
    // -g with gcc
    // code taken with modifications from:
    // http://stackoverflow.com/questions/3151779/how-its-better-to-invoke-gdb-from-program-to-print-its-stacktrace
    void print_trace() {
        if( ! getenv("GEARBOX_STACK_TRACE") ) return;
        char name_buf[512];
        int bytes = readlink("/proc/self/exe", name_buf, 511);
        if( bytes == -1  ) 
            throw ERR_LIBC("failed to readlink /proc/self/exe");
        name_buf[bytes]=0;
        TempFile tmp;
        write_file(tmp.name(), "bt\n");
        Cmd cmd;
        cmd << "/usr/bin/gdb" << "-batch" << "-n" << "-x" << tmp.name() << name_buf << getpid();
        run(cmd,2,2);
    }

Error::Error() : _code(500), _name("Error")  {
    memset(buffer, 0, sizeof(buffer));
}

Error::Error(const std::string & msg, int code, const char * name)
    : _code(code), _name(name) {
    std::ostringstream err;
    if( msg.find(this->name()) != 0 ) {
        err << this->name() << " [" << this->code() << "]: ";
    }
    if( msg.size() ) {
        err << msg;
    }
    else {
        err << "Unknown Error";
    }

    strlcpy(buffer, err.str().c_str(), sizeof(buffer));
}

Error::~Error() throw() {};

const char * Error::what() const throw() {
    return buffer;
}

int Error::code() const throw() {
    return _code;
}

const char * Error::name() const throw() {
    return _name;
}

void throw_from_code(int code, const std::string & msg) {
    switch(code) {
    case 300: gbTHROW( ERR_CODE_300(msg) );
    case 301: gbTHROW( ERR_CODE_301(msg) );
    case 302: gbTHROW( ERR_CODE_302(msg) );
    case 303: gbTHROW( ERR_CODE_303(msg) );
    case 304: gbTHROW( ERR_CODE_304(msg) );
    case 305: gbTHROW( ERR_CODE_305(msg) );
    case 307: gbTHROW( ERR_CODE_307(msg) );
    case 400: gbTHROW( ERR_CODE_400(msg) );
    case 401: gbTHROW( ERR_CODE_401(msg) );
    case 402: gbTHROW( ERR_CODE_402(msg) );
    case 403: gbTHROW( ERR_CODE_403(msg) );
    case 404: gbTHROW( ERR_CODE_404(msg) );
    case 405: gbTHROW( ERR_CODE_405(msg) );
    case 406: gbTHROW( ERR_CODE_406(msg) );
    case 407: gbTHROW( ERR_CODE_407(msg) );
    case 408: gbTHROW( ERR_CODE_408(msg) );
    case 409: gbTHROW( ERR_CODE_409(msg) );
    case 410: gbTHROW( ERR_CODE_410(msg) );
    case 411: gbTHROW( ERR_CODE_411(msg) );
    case 412: gbTHROW( ERR_CODE_412(msg) );
    case 413: gbTHROW( ERR_CODE_413(msg) );
    case 414: gbTHROW( ERR_CODE_414(msg) );
    case 415: gbTHROW( ERR_CODE_415(msg) );
    case 416: gbTHROW( ERR_CODE_416(msg) );
    case 417: gbTHROW( ERR_CODE_417(msg) );
    case 422: gbTHROW( ERR_CODE_422(msg) );
    case 423: gbTHROW( ERR_CODE_423(msg) );
    case 424: gbTHROW( ERR_CODE_424(msg) );
    case 426: gbTHROW( ERR_CODE_426(msg) );
    case 500: gbTHROW( ERR_CODE_500(msg) );
    case 501: gbTHROW( ERR_CODE_501(msg) );
    case 502: gbTHROW( ERR_CODE_502(msg) );
    case 503: gbTHROW( ERR_CODE_503(msg) );
    case 504: gbTHROW( ERR_CODE_504(msg) );
    case 505: gbTHROW( ERR_CODE_505(msg) );
    case 506: gbTHROW( ERR_CODE_506(msg) );
    case 507: gbTHROW( ERR_CODE_507(msg) );
    case 510: gbTHROW( ERR_CODE_510(msg) );
    }
    gbTHROW( ERR_CODE_500(msg) );
}

} // namespace;
    
    
    
