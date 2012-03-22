#!/bin/sh

cat <<EOF
{
    "component" : "client",
    "allow_unknown_jobs": 1,
    "status": {
        "persistence_type": "transient"
    },
    "log": {
        "config_file": "$ROOT/conf/gearbox/client/stdout-logger.conf"
    }
    
}
EOF
