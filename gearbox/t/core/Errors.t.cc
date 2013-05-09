// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>
#include <gearbox/core/Errors.h>
using namespace Gearbox;

int main() {
    chdir(TESTDIR);
    TEST_START(156);
    log_init("./unit.conf");

    THROWS( gbTHROW( ERR_MULTIPLE_CHOICES("error") ), "MULTIPLE_CHOICES [300]: error" );
    THROWS( gbTHROW( ERR_MOVED_PERMANENTLY("error") ), "MOVED_PERMANENTLY [301]: error" );
    THROWS( gbTHROW( ERR_MOVED_TEMPORARILY("error") ), "MOVED_TEMPORARILY [302]: error" );
    THROWS( gbTHROW( ERR_SEE_OTHER("error") ), "SEE_OTHER [303]: error" );
    THROWS( gbTHROW( ERR_NOT_MODIFIED("error") ), "NOT_MODIFIED [304]: error" );
    THROWS( gbTHROW( ERR_USE_PROXY("error") ), "USE_PROXY [305]: error" );
    THROWS( gbTHROW( ERR_TEMPORARY_REDIRECT("error") ), "TEMPORARY_REDIRECT [307]: error" );
    THROWS( gbTHROW( ERR_BAD_REQUEST("error") ), "BAD_REQUEST [400]: error" );
    THROWS( gbTHROW( ERR_UNAUTHORIZED("error") ), "UNAUTHORIZED [401]: error" );
    THROWS( gbTHROW( ERR_PAYMENT_REQUIRED("error") ), "PAYMENT_REQUIRED [402]: error" );
    THROWS( gbTHROW( ERR_FORBIDDEN("error") ), "FORBIDDEN [403]: error" );
    THROWS( gbTHROW( ERR_NOT_FOUND("error") ), "NOT_FOUND [404]: error" );
    THROWS( gbTHROW( ERR_METHOD_NOT_ALLOWED("error") ), "METHOD_NOT_ALLOWED [405]: error" );
    THROWS( gbTHROW( ERR_NOT_ACCEPTABLE("error") ), "NOT_ACCEPTABLE [406]: error" );
    THROWS( gbTHROW( ERR_PROXY_AUTHENTICATION_REQUIRED("error") ), "PROXY_AUTHENTICATION_REQUIRED [407]: error" );
    THROWS( gbTHROW( ERR_REQUEST_TIME_OUT("error") ), "REQUEST_TIME_OUT [408]: error" );
    THROWS( gbTHROW( ERR_CONFLICT("error") ), "CONFLICT [409]: error" );
    THROWS( gbTHROW( ERR_GONE("error") ), "GONE [410]: error" );
    THROWS( gbTHROW( ERR_LENGTH_REQUIRED("error") ), "LENGTH_REQUIRED [411]: error" );
    THROWS( gbTHROW( ERR_PRECONDITION_FAILED("error") ), "PRECONDITION_FAILED [412]: error" );
    THROWS( gbTHROW( ERR_REQUEST_ENTITY_TOO_LARGE("error") ), "REQUEST_ENTITY_TOO_LARGE [413]: error" );
    THROWS( gbTHROW( ERR_REQUEST_URI_TOO_LARGE("error") ), "REQUEST_URI_TOO_LARGE [414]: error" );
    THROWS( gbTHROW( ERR_UNSUPPORTED_MEDIA_TYPE("error") ), "UNSUPPORTED_MEDIA_TYPE [415]: error" );
    THROWS( gbTHROW( ERR_RANGE_NOT_SATISFIABLE("error") ), "RANGE_NOT_SATISFIABLE [416]: error" );
    THROWS( gbTHROW( ERR_EXPECTATION_FAILED("error") ), "EXPECTATION_FAILED [417]: error" );
    THROWS( gbTHROW( ERR_UNPROCESSABLE_ENTITY("error") ), "UNPROCESSABLE_ENTITY [422]: error" );
    THROWS( gbTHROW( ERR_LOCKED("error") ), "LOCKED [423]: error" );
    THROWS( gbTHROW( ERR_FAILED_DEPENDENCY("error") ), "FAILED_DEPENDENCY [424]: error" );
    THROWS( gbTHROW( ERR_UPGRADE_REQUIRED("error") ), "UPGRADE_REQUIRED [426]: error" );
    THROWS( gbTHROW( ERR_INTERNAL_SERVER_ERROR("error") ), "INTERNAL_SERVER_ERROR [500]: error" );
    THROWS( gbTHROW( ERR_NOT_IMPLEMENTED("error") ), "NOT_IMPLEMENTED [501]: error" );
    THROWS( gbTHROW( ERR_BAD_GATEWAY("error") ), "BAD_GATEWAY [502]: error" );
    THROWS( gbTHROW( ERR_SERVICE_UNAVAILABLE("error") ), "SERVICE_UNAVAILABLE [503]: error" );
    THROWS( gbTHROW( ERR_GATEWAY_TIME_OUT("error") ), "GATEWAY_TIME_OUT [504]: error" );
    THROWS( gbTHROW( ERR_VERSION_NOT_SUPPORTED("error") ), "VERSION_NOT_SUPPORTED [505]: error" );
    THROWS( gbTHROW( ERR_VARIANT_ALSO_VARIES("error") ), "VARIANT_ALSO_VARIES [506]: error" );
    THROWS( gbTHROW( ERR_INSUFFICIENT_STORAGE("error") ), "INSUFFICIENT_STORAGE [507]: error" );
    THROWS( gbTHROW( ERR_NOT_EXTENDED("error") ), "NOT_EXTENDED [510]: error" );
    
    try { gbTHROW( ERR_MULTIPLE_CHOICES("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 300 );
        IS( err.name(), "MULTIPLE_CHOICES" );
    }
    try { gbTHROW( ERR_MOVED_PERMANENTLY("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 301 );
        IS( err.name(), "MOVED_PERMANENTLY" );
    }
    try { gbTHROW( ERR_MOVED_TEMPORARILY("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 302 );
        IS( err.name(), "MOVED_TEMPORARILY" );
    }
    try { gbTHROW( ERR_SEE_OTHER("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 303 );
        IS( err.name(), "SEE_OTHER" );
    }
    try { gbTHROW( ERR_NOT_MODIFIED("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 304 );
        IS( err.name(), "NOT_MODIFIED" );
    }
    try { gbTHROW( ERR_USE_PROXY("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 305 );
        IS( err.name(), "USE_PROXY" );
    }
    try { gbTHROW( ERR_TEMPORARY_REDIRECT("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 307 );
        IS( err.name(), "TEMPORARY_REDIRECT" );
    }
    try { gbTHROW( ERR_BAD_REQUEST("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 400 );
        IS( err.name(), "BAD_REQUEST" );
    }
    try { gbTHROW( ERR_UNAUTHORIZED("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 401 );
        IS( err.name(), "UNAUTHORIZED" );
    }
    try { gbTHROW( ERR_PAYMENT_REQUIRED("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 402 );
        IS( err.name(), "PAYMENT_REQUIRED" );
    }
    try { gbTHROW( ERR_FORBIDDEN("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 403 );
        IS( err.name(), "FORBIDDEN" );
    }
    try { gbTHROW( ERR_NOT_FOUND("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 404 );
        IS( err.name(), "NOT_FOUND" );
    }
    try { gbTHROW( ERR_METHOD_NOT_ALLOWED("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 405 );
        IS( err.name(), "METHOD_NOT_ALLOWED" );
    }
    try { gbTHROW( ERR_NOT_ACCEPTABLE("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 406 );
        IS( err.name(), "NOT_ACCEPTABLE" );
    }
    try { gbTHROW( ERR_PROXY_AUTHENTICATION_REQUIRED("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 407 );
        IS( err.name(), "PROXY_AUTHENTICATION_REQUIRED" );
    }
    try { gbTHROW( ERR_REQUEST_TIME_OUT("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 408 );
        IS( err.name(), "REQUEST_TIME_OUT" );
    }
    try { gbTHROW( ERR_CONFLICT("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 409 );
        IS( err.name(), "CONFLICT" );
    }
    try { gbTHROW( ERR_GONE("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 410 );
        IS( err.name(), "GONE" );
    }
    try { gbTHROW( ERR_LENGTH_REQUIRED("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 411 );
        IS( err.name(), "LENGTH_REQUIRED" );
    }
    try { gbTHROW( ERR_PRECONDITION_FAILED("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 412 );
        IS( err.name(), "PRECONDITION_FAILED" );
    }
    try { gbTHROW( ERR_REQUEST_ENTITY_TOO_LARGE("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 413 );
        IS( err.name(), "REQUEST_ENTITY_TOO_LARGE" );
    }
    try { gbTHROW( ERR_REQUEST_URI_TOO_LARGE("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 414 );
        IS( err.name(), "REQUEST_URI_TOO_LARGE" );
    }
    try { gbTHROW( ERR_UNSUPPORTED_MEDIA_TYPE("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 415 );
        IS( err.name(), "UNSUPPORTED_MEDIA_TYPE" );
    }
    try { gbTHROW( ERR_RANGE_NOT_SATISFIABLE("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 416 );
        IS( err.name(), "RANGE_NOT_SATISFIABLE" );
    }
    try { gbTHROW( ERR_EXPECTATION_FAILED("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 417 );
        IS( err.name(), "EXPECTATION_FAILED" );
    }
    try { gbTHROW( ERR_UNPROCESSABLE_ENTITY("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 422 );
        IS( err.name(), "UNPROCESSABLE_ENTITY" );
    }
    try { gbTHROW( ERR_LOCKED("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 423 );
        IS( err.name(), "LOCKED" );
    }
    try { gbTHROW( ERR_FAILED_DEPENDENCY("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 424 );
        IS( err.name(), "FAILED_DEPENDENCY" );
    }
    try { gbTHROW( ERR_UPGRADE_REQUIRED("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 426 );
        IS( err.name(), "UPGRADE_REQUIRED" );
    }
    try { gbTHROW( ERR_INTERNAL_SERVER_ERROR("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 500 );
        IS( err.name(), "INTERNAL_SERVER_ERROR" );
    }
    try { gbTHROW( ERR_NOT_IMPLEMENTED("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 501 );
        IS( err.name(), "NOT_IMPLEMENTED" );
    }
    try { gbTHROW( ERR_BAD_GATEWAY("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 502 );
        IS( err.name(), "BAD_GATEWAY" );
    }
    try { gbTHROW( ERR_SERVICE_UNAVAILABLE("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 503 );
        IS( err.name(), "SERVICE_UNAVAILABLE" );
    }
    try { gbTHROW( ERR_GATEWAY_TIME_OUT("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 504 );
        IS( err.name(), "GATEWAY_TIME_OUT" );
    }
    try { gbTHROW( ERR_VERSION_NOT_SUPPORTED("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 505 );
        IS( err.name(), "VERSION_NOT_SUPPORTED" );
    }
    try { gbTHROW( ERR_VARIANT_ALSO_VARIES("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 506 );
        IS( err.name(), "VARIANT_ALSO_VARIES" );
    }
    try { gbTHROW( ERR_INSUFFICIENT_STORAGE("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 507 );
        IS( err.name(), "INSUFFICIENT_STORAGE" );
    }
    try { gbTHROW( ERR_NOT_EXTENDED("error") ); } 
    catch ( const Error & err ) {
        IS( err.code(), 510 );
        IS( err.name(), "NOT_EXTENDED" );
    }

    THROWS( throw_from_code(300, "error"), "MULTIPLE_CHOICES [300]: error" );
    THROWS( throw_from_code(301, "error"), "MOVED_PERMANENTLY [301]: error" );
    THROWS( throw_from_code(302, "error"), "MOVED_TEMPORARILY [302]: error" );
    THROWS( throw_from_code(303, "error"), "SEE_OTHER [303]: error" );
    THROWS( throw_from_code(304, "error"), "NOT_MODIFIED [304]: error" );
    THROWS( throw_from_code(305, "error"), "USE_PROXY [305]: error" );
    THROWS( throw_from_code(307, "error"), "TEMPORARY_REDIRECT [307]: error" );
    THROWS( throw_from_code(400, "error"), "BAD_REQUEST [400]: error" );
    THROWS( throw_from_code(401, "error"), "UNAUTHORIZED [401]: error" );
    THROWS( throw_from_code(402, "error"), "PAYMENT_REQUIRED [402]: error" );
    THROWS( throw_from_code(403, "error"), "FORBIDDEN [403]: error" );
    THROWS( throw_from_code(404, "error"), "NOT_FOUND [404]: error" );
    THROWS( throw_from_code(405, "error"), "METHOD_NOT_ALLOWED [405]: error" );
    THROWS( throw_from_code(406, "error"), "NOT_ACCEPTABLE [406]: error" );
    THROWS( throw_from_code(407, "error"), "PROXY_AUTHENTICATION_REQUIRED [407]: error" );
    THROWS( throw_from_code(408, "error"), "REQUEST_TIME_OUT [408]: error" );
    THROWS( throw_from_code(409, "error"), "CONFLICT [409]: error" );
    THROWS( throw_from_code(410, "error"), "GONE [410]: error" );
    THROWS( throw_from_code(411, "error"), "LENGTH_REQUIRED [411]: error" );
    THROWS( throw_from_code(412, "error"), "PRECONDITION_FAILED [412]: error" );
    THROWS( throw_from_code(413, "error"), "REQUEST_ENTITY_TOO_LARGE [413]: error" );
    THROWS( throw_from_code(414, "error"), "REQUEST_URI_TOO_LARGE [414]: error" );
    THROWS( throw_from_code(415, "error"), "UNSUPPORTED_MEDIA_TYPE [415]: error" );
    THROWS( throw_from_code(416, "error"), "RANGE_NOT_SATISFIABLE [416]: error" );
    THROWS( throw_from_code(417, "error"), "EXPECTATION_FAILED [417]: error" );
    THROWS( throw_from_code(422, "error"), "UNPROCESSABLE_ENTITY [422]: error" );
    THROWS( throw_from_code(423, "error"), "LOCKED [423]: error" );
    THROWS( throw_from_code(424, "error"), "FAILED_DEPENDENCY [424]: error" );
    THROWS( throw_from_code(426, "error"), "UPGRADE_REQUIRED [426]: error" );
    THROWS( throw_from_code(500, "error"), "INTERNAL_SERVER_ERROR [500]: error" );
    THROWS( throw_from_code(501, "error"), "NOT_IMPLEMENTED [501]: error" );
    THROWS( throw_from_code(502, "error"), "BAD_GATEWAY [502]: error" );
    THROWS( throw_from_code(503, "error"), "SERVICE_UNAVAILABLE [503]: error" );
    THROWS( throw_from_code(504, "error"), "GATEWAY_TIME_OUT [504]: error" );
    THROWS( throw_from_code(505, "error"), "VERSION_NOT_SUPPORTED [505]: error" );
    THROWS( throw_from_code(506, "error"), "VARIANT_ALSO_VARIES [506]: error" );
    THROWS( throw_from_code(507, "error"), "INSUFFICIENT_STORAGE [507]: error" );
    THROWS( throw_from_code(510, "error"), "NOT_EXTENDED [510]: error" );

    errno = EPERM;
    THROWS( gbTHROW( ERR_LIBC("kaboom") ), "INTERNAL_SERVER_ERROR [500]: kaboom: Operation not permitted" );

    errno = ENOENT;
    THROWS( gbTHROW( ERR_LIBC("kaboom") ), "INTERNAL_SERVER_ERROR [500]: kaboom: No such file or directory" );

    errno = ESRCH;
    THROWS( gbTHROW( ERR_LIBC("kaboom", EPERM) ), "INTERNAL_SERVER_ERROR [500]: kaboom: Operation not permitted" );

    try {
        gbTHROW( ERR_LIBC("kaboom", EPERM) );
    } catch ( const ERR_LIBC & e ) {
        IS(e.errnum(), EPERM);
    }

    TEST_END;
}
