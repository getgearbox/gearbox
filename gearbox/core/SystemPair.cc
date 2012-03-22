// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/core/SystemPair.h>
#include <gearbox/core/util.h>
#include <gearbox/core/Errors.h>


namespace Gearbox {
    SystemPair::SystemPair(
        const cmd & on_init,
        const cmd & on_destroy
    ) : on_destroy_(on_destroy) {
        std::string output;
        int rc = run(on_init, output);
        if( rc ) {
            gbTHROW( ERR_INTERNAL_SERVER_ERROR("failed to run \"" + shellquote_list(on_init) + "\": " + output) );
        }
    }
    
    SystemPair::~SystemPair() {
        std::string output;
        int rc = run(on_destroy_, output);
        if( rc ) {
            gbTHROW( ERR_INTERNAL_SERVER_ERROR("failed to run \"" + shellquote_list(on_destroy_) + "\": " + output) );
        }
    }
}
