#!/bin/sh
cat <<EOF
{
    "component" : "testchainedperl",

    "daemons" : [{
        "name" : "worker",
        "logname": "%{component}",
        "command" : "$ROOT/bin/workerTestChained.pl $ROOT/conf/gearbox/test-chained-php.conf",
        "count" : 10,
        "user" : "%{gearbox.user}"
    }]
}

EOF
