#!/bin/sh

cat <<EOF
{
    "component" : "testagentsperl",

    "daemons" : [{
        "name" : "worker",
        "logname": "%{component}",
        "command" : "$ROOT/bin/workerTestAgents.pl $ROOT/conf/gearbox/test-agents-perl.conf",
        "count" : 6,
        "user" : "%{gearbox.user}"
    }]
}

EOF
