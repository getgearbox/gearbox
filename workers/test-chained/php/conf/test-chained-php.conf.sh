#!/bin/sh
bindir=/usr/bin

cat <<EOF
{
    "component" : "testchainedphp",

    "daemons" : [{
        "name" : "worker",
        "logname": "%{component}",
        "command" : "$bindir/php22 $ROOT/bin/workerTestChained.php $ROOT/conf/gearbox/test-chained-php.conf",
        "count" : 10,
        "user" : "%{gearbox.user}"
    }]
}

EOF
