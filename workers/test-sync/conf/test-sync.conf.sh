#!/bin/sh
bindir=/usr/bin

cat <<EOF
{
    "component" : "testsync",

    "daemons" : [{
        "name" : "worker",
        "logname": "%{component}",
        "command" : "$bindir/workerTestSync --config $ROOT/conf/gearbox/test-sync.conf",
        "count" : 1,
        "user" : "%{gearbox.user}"
    }]
}
EOF
