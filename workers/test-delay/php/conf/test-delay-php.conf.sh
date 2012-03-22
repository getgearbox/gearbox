#!/bin/sh
bindir=/usr/bin

cat <<EOF
{
    "component" : "testdelayphp",

    "daemons" : [{
        "name" : "worker",
        "logname": "%{component}",
        "command" : "$bindir/php22 $ROOT/bin/workerTestDelay.php $ROOT/conf/gearbox/test-delay-php.conf",
        "count" : 3,
        "user" : "%{gearbox.user}"
    }]
}

EOF
