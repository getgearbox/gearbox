#!/bin/sh
bindir=/usr/bin

cat <<EOF
{
    "component" : "testdelay",

    "daemons" : [{
        "name" : "worker",
        "logname": "%{component}",
        "command" : "$bindir/workerTestDelay --config $ROOT/conf/gearbox/test-delay.conf",
        "count" : 10,
        "user" : "%{gearbox.user}"
    }]
}
EOF
