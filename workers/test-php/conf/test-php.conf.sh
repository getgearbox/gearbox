#!/bin/sh
bindir=/usr/bin

cat <<EOF
{
    "component" : "testphp",

    "daemons" : [{
        "name" : "worker",
        "logname": "%{component}",
        "command" : "/usr/bin64/php22 $bindir/workerTestPhp.php $ROOT/conf/gearbox/test-php.conf",
        "count" : 1,
        "user" : "%{gearbox.user}"
    }]
}

EOF
