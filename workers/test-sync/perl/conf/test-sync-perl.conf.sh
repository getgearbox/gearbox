#!/bin/sh
cat <<EOF
{
    "component" : "testsyncperl",

    "daemons" : [{
        "name" : "worker",
        "logname": "%{component}",
        "command" : "$ROOT/bin/workerTestSync.pl $ROOT/conf/gearbox/test-sync-perl.conf",
        "count" : 1,
        "user" : "%{gearbox.user}"
    }]
}
EOF
