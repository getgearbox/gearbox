#!/bin/sh
cat <<EOF
{
    "component" : "testdelayperl",

    "daemons" : [{
        "name" : "worker",
        "logname": "%{component}",
        "command" : "$ROOT/bin/workerTestDelay.pl $ROOT/conf/gearbox/test-delay-perl.conf",
        "count" : 3,
        "user" : "%{gearbox.user}"
    }]
}

EOF
