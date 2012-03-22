#!/bin/sh
bindir=/usr/bin

cat <<EOF
{
    "component" : "testsyncphp",

    "daemons" : [{
        "name" : "worker",
        "logname": "%{component}",
        "command" : "$bindir/php22 $ROOT/bin/workerTestSync.php $ROOT/conf/gearbox/test-sync-php.conf",
        "count" : 1,
        "user" : "%{gearbox.user}"
    }]
}
EOF
