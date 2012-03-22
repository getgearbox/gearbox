#!/bin/sh
cat <<EOF
{
    "component" : "testcancelperl",

    "daemons" : [{
        "name" : "worker",
        "logname": "%{component}",
        "command" : "$ROOT/bin/workerTestCancel.pl $ROOT/conf/gearbox/test-cancel-perl.conf",
        "count" : 1,
        "user" : "%{gearbox.user}"
    }]
}

EOF
