#!/bin/sh
bindir=/usr/bin

cat <<EOF
{
    "component" : "testcancel",

    "daemons" : [{
        "name" : "worker",
        "logname": "%{component}",
        "command" : "$bindir/workerTestCancel --config $ROOT/conf/gearbox/test-cancel.conf",
        "count" : 1,
        "user" : "%{gearbox.user}"
    }]
}
EOF
