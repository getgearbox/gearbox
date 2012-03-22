#!/bin/sh
bindir=/usr/bin

cat <<EOF
{
    "component" : "testagents",

    "daemons" : [{
        "name" : "worker",
        "logname": "%{component}",
        "command" : "$bindir/workerTestAgents --config $ROOT/conf/gearbox/test-agents.conf",
        "count" : 6,
        "user" : "%{gearbox.user}"
    }]
}
EOF
