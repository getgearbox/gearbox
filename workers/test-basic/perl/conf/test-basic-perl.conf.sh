#!/bin/sh
cat <<EOF
{
    "component" : "testbasicperl",

    "daemons" : [{
        "name" : "worker",
        "logname": "%{component}",
        "command" : "$ROOT/bin/workerTestBasic.pl $ROOT/conf/gearbox/test-basic-perl.conf",
        "count" : 1,
        "user" : "%{gearbox.user}"
    }]
}

EOF
