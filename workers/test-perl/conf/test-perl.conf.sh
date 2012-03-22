#!/bin/sh
bindir=/usr/bin

cat <<EOF
{
    "component" : "testperl",

    "daemons" : [{
        "name" : "worker",
        "logname": "%{component}",
        "command" : "/usr/bin/perl $bindir/workerTestPerl.pl $ROOT/conf/gearbox/test-perl.conf",
        "count" : 1,
        "user" : "%{gearbox.user}"
    }]
}

EOF
