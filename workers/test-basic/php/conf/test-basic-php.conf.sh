#!/bin/sh
bindir=/usr/bin

cat <<EOF
{
    "component" : "testbasicphp",

    "daemons" : [{
        "name" : "worker",
        "logname": "%{component}",
        "command" : "$bindir/php22 $ROOT/bin/workerTestBasic.php $ROOT/conf/gearbox/test-basic-php.conf",
        "count" : 1,
        "user" : "%{gearbox.user}"
    }]
}

EOF
