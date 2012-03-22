#!/bin/sh
bindir=/usr/bin

cat <<EOF
{
    "component" : "testbasic",

    "daemons" : [{
        "name" : "worker",
        "logname": "%{component}",
        "command" : "$bindir/workerTestBasic --config $ROOT/conf/gearbox/test-basic.conf",
        "count" : 1,
        "user" : "%{gearbox.user}"
    }]
}
EOF
