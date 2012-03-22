#!/bin/sh
bindir=/usr/bin

cat <<EOF
{
    "component" : "barn",

    "daemons" : [{
        "name" : "worker",
        "logname": "%{component}",
        "command" : "$bindir/php22 $ROOT/bin/workerBarn.php $ROOT/conf/gearbox/barn.conf",
        "count" : 1,
        "user" : "%{gearbox.user}"
    }]
}

EOF
