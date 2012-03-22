#!/bin/sh
bindir=/usr/bin

cat <<EOF
{
    "component" : "testagentsphp",

    "daemons" : [{
        "name" : "worker",
        "logname": "%{component}",
        "command" : "$bindir/php22 $ROOT/bin/workerTestAgents.php $ROOT/conf/gearbox/test-agents-php.conf",
        "count" : 6,
        "user" : "%{gearbox.user}"
    }]
}

EOF
