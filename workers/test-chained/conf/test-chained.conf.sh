#!/bin/sh
bindir=/usr/bin

cat <<EOF
{
    "component" : "testchained",

    "daemons" : [{
        "name" : "worker",
        "logname": "%{component}",
        "command" : "$bindir/workerTestChained --config $ROOT/conf/gearbox/test-chained.conf",
        "count" : 10,
        "user" : "%{gearbox.user}"
    }]
}
EOF
