#!/bin/sh
bindir=/usr/bin

cat <<EOF
{
    "component" : "testcancelphp",

    "daemons" : [{
        "name" : "worker",
        "logname": "%{component}",
        "command" : "$bindir/php22 $ROOT/bin/workerTestCancel.php $ROOT/conf/gearbox/test-cancel-php.conf",
        "count" : 1,
        "user" : "%{gearbox.user}"
    }]
}

EOF
