#!/usr/bin/env python

import setup
from testtrivial import *

from gearbox import *

PLAN(494)

err = (OK(ERR_MULTIPLE_CHOICES("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 300 )
IS( err.name, "MULTIPLE_CHOICES" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_300("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_MULTIPLE_CHOICES) )
IS( bycode.code, 300 )
IS( bycode.name, "MULTIPLE_CHOICES" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_MOVED_PERMANENTLY("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 301 )
IS( err.name, "MOVED_PERMANENTLY" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_301("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_MOVED_PERMANENTLY) )
IS( bycode.code, 301 )
IS( bycode.name, "MOVED_PERMANENTLY" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_MOVED_TEMPORARILY("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 302 )
IS( err.name, "MOVED_TEMPORARILY" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_302("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_MOVED_TEMPORARILY) )
IS( bycode.code, 302 )
IS( bycode.name, "MOVED_TEMPORARILY" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_SEE_OTHER("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 303 )
IS( err.name, "SEE_OTHER" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_303("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_SEE_OTHER) )
IS( bycode.code, 303 )
IS( bycode.name, "SEE_OTHER" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_NOT_MODIFIED("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 304 )
IS( err.name, "NOT_MODIFIED" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_304("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_NOT_MODIFIED) )
IS( bycode.code, 304 )
IS( bycode.name, "NOT_MODIFIED" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_USE_PROXY("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 305 )
IS( err.name, "USE_PROXY" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_305("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_USE_PROXY) )
IS( bycode.code, 305 )
IS( bycode.name, "USE_PROXY" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_TEMPORARY_REDIRECT("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 307 )
IS( err.name, "TEMPORARY_REDIRECT" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_307("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_TEMPORARY_REDIRECT) )
IS( bycode.code, 307 )
IS( bycode.name, "TEMPORARY_REDIRECT" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_BAD_REQUEST("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 400 )
IS( err.name, "BAD_REQUEST" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_400("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_BAD_REQUEST) )
IS( bycode.code, 400 )
IS( bycode.name, "BAD_REQUEST" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_UNAUTHORIZED("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 401 )
IS( err.name, "UNAUTHORIZED" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_401("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_UNAUTHORIZED) )
IS( bycode.code, 401 )
IS( bycode.name, "UNAUTHORIZED" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_PAYMENT_REQUIRED("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 402 )
IS( err.name, "PAYMENT_REQUIRED" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_402("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_PAYMENT_REQUIRED) )
IS( bycode.code, 402 )
IS( bycode.name, "PAYMENT_REQUIRED" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_FORBIDDEN("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 403 )
IS( err.name, "FORBIDDEN" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_403("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_FORBIDDEN) )
IS( bycode.code, 403 )
IS( bycode.name, "FORBIDDEN" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_NOT_FOUND("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 404 )
IS( err.name, "NOT_FOUND" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_404("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_NOT_FOUND) )
IS( bycode.code, 404 )
IS( bycode.name, "NOT_FOUND" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_METHOD_NOT_ALLOWED("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 405 )
IS( err.name, "METHOD_NOT_ALLOWED" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_405("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_METHOD_NOT_ALLOWED) )
IS( bycode.code, 405 )
IS( bycode.name, "METHOD_NOT_ALLOWED" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_NOT_ACCEPTABLE("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 406 )
IS( err.name, "NOT_ACCEPTABLE" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_406("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_NOT_ACCEPTABLE) )
IS( bycode.code, 406 )
IS( bycode.name, "NOT_ACCEPTABLE" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_PROXY_AUTHENTICATION_REQUIRED("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 407 )
IS( err.name, "PROXY_AUTHENTICATION_REQUIRED" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_407("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_PROXY_AUTHENTICATION_REQUIRED) )
IS( bycode.code, 407 )
IS( bycode.name, "PROXY_AUTHENTICATION_REQUIRED" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_REQUEST_TIME_OUT("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 408 )
IS( err.name, "REQUEST_TIME_OUT" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_408("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_REQUEST_TIME_OUT) )
IS( bycode.code, 408 )
IS( bycode.name, "REQUEST_TIME_OUT" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_CONFLICT("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 409 )
IS( err.name, "CONFLICT" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_409("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_CONFLICT) )
IS( bycode.code, 409 )
IS( bycode.name, "CONFLICT" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_GONE("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 410 )
IS( err.name, "GONE" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_410("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_GONE) )
IS( bycode.code, 410 )
IS( bycode.name, "GONE" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_LENGTH_REQUIRED("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 411 )
IS( err.name, "LENGTH_REQUIRED" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_411("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_LENGTH_REQUIRED) )
IS( bycode.code, 411 )
IS( bycode.name, "LENGTH_REQUIRED" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_PRECONDITION_FAILED("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 412 )
IS( err.name, "PRECONDITION_FAILED" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_412("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_PRECONDITION_FAILED) )
IS( bycode.code, 412 )
IS( bycode.name, "PRECONDITION_FAILED" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_REQUEST_ENTITY_TOO_LARGE("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 413 )
IS( err.name, "REQUEST_ENTITY_TOO_LARGE" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_413("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_REQUEST_ENTITY_TOO_LARGE) )
IS( bycode.code, 413 )
IS( bycode.name, "REQUEST_ENTITY_TOO_LARGE" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_REQUEST_URI_TOO_LARGE("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 414 )
IS( err.name, "REQUEST_URI_TOO_LARGE" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_414("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_REQUEST_URI_TOO_LARGE) )
IS( bycode.code, 414 )
IS( bycode.name, "REQUEST_URI_TOO_LARGE" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_UNSUPPORTED_MEDIA_TYPE("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 415 )
IS( err.name, "UNSUPPORTED_MEDIA_TYPE" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_415("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_UNSUPPORTED_MEDIA_TYPE) )
IS( bycode.code, 415 )
IS( bycode.name, "UNSUPPORTED_MEDIA_TYPE" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_RANGE_NOT_SATISFIABLE("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 416 )
IS( err.name, "RANGE_NOT_SATISFIABLE" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_416("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_RANGE_NOT_SATISFIABLE) )
IS( bycode.code, 416 )
IS( bycode.name, "RANGE_NOT_SATISFIABLE" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_EXPECTATION_FAILED("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 417 )
IS( err.name, "EXPECTATION_FAILED" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_417("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_EXPECTATION_FAILED) )
IS( bycode.code, 417 )
IS( bycode.name, "EXPECTATION_FAILED" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_UNPROCESSABLE_ENTITY("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 422 )
IS( err.name, "UNPROCESSABLE_ENTITY" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_422("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_UNPROCESSABLE_ENTITY) )
IS( bycode.code, 422 )
IS( bycode.name, "UNPROCESSABLE_ENTITY" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_LOCKED("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 423 )
IS( err.name, "LOCKED" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_423("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_LOCKED) )
IS( bycode.code, 423 )
IS( bycode.name, "LOCKED" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_FAILED_DEPENDENCY("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 424 )
IS( err.name, "FAILED_DEPENDENCY" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_424("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_FAILED_DEPENDENCY) )
IS( bycode.code, 424 )
IS( bycode.name, "FAILED_DEPENDENCY" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_UPGRADE_REQUIRED("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 426 )
IS( err.name, "UPGRADE_REQUIRED" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_426("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_UPGRADE_REQUIRED) )
IS( bycode.code, 426 )
IS( bycode.name, "UPGRADE_REQUIRED" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_INTERNAL_SERVER_ERROR("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 500 )
IS( err.name, "INTERNAL_SERVER_ERROR" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_500("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_INTERNAL_SERVER_ERROR) )
IS( bycode.code, 500 )
IS( bycode.name, "INTERNAL_SERVER_ERROR" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_NOT_IMPLEMENTED("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 501 )
IS( err.name, "NOT_IMPLEMENTED" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_501("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_NOT_IMPLEMENTED) )
IS( bycode.code, 501 )
IS( bycode.name, "NOT_IMPLEMENTED" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_BAD_GATEWAY("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 502 )
IS( err.name, "BAD_GATEWAY" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_502("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_BAD_GATEWAY) )
IS( bycode.code, 502 )
IS( bycode.name, "BAD_GATEWAY" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_SERVICE_UNAVAILABLE("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 503 )
IS( err.name, "SERVICE_UNAVAILABLE" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_503("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_SERVICE_UNAVAILABLE) )
IS( bycode.code, 503 )
IS( bycode.name, "SERVICE_UNAVAILABLE" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_GATEWAY_TIME_OUT("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 504 )
IS( err.name, "GATEWAY_TIME_OUT" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_504("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_GATEWAY_TIME_OUT) )
IS( bycode.code, 504 )
IS( bycode.name, "GATEWAY_TIME_OUT" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_VERSION_NOT_SUPPORTED("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 505 )
IS( err.name, "VERSION_NOT_SUPPORTED" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_505("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_VERSION_NOT_SUPPORTED) )
IS( bycode.code, 505 )
IS( bycode.name, "VERSION_NOT_SUPPORTED" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_VARIANT_ALSO_VARIES("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 506 )
IS( err.name, "VARIANT_ALSO_VARIES" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_506("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_VARIANT_ALSO_VARIES) )
IS( bycode.code, 506 )
IS( bycode.name, "VARIANT_ALSO_VARIES" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_INSUFFICIENT_STORAGE("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 507 )
IS( err.name, "INSUFFICIENT_STORAGE" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_507("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_INSUFFICIENT_STORAGE) )
IS( bycode.code, 507 )
IS( bycode.name, "INSUFFICIENT_STORAGE" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

err = (OK(ERR_NOT_EXTENDED("err")))[1]
OK( isinstance(err, Error) )
IS( err.code, 510 )
IS( err.name, "NOT_EXTENDED" )
IS( err.msg, "err" )
try: raise err;
except Exception as e: IS(str(e), str(err))
bycode = (OK(ERR_CODE_510("err")))[1]
OK( isinstance(bycode, Error) )
OK( isinstance(bycode, ERR_NOT_EXTENDED) )
IS( bycode.code, 510 )
IS( bycode.name, "NOT_EXTENDED" )
IS( bycode.msg, "err" )
try: raise bycode;
except Exception as e: IS(str(e), str(err))

