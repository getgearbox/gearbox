#!/bin/sh
bindir=/usr/bin
sbindir=/usr/sbin
libdir=/usr/lib

[ -n "$gearman_db_type" ] && db_type=$gearman_db_type

if [ "$db_type" = "mysql" ]; then
    persistence="-q mysql --mysql-user=%{gearman.db_user} --mysql-password=%{gearman.db_pass} --mysql-db=%{gearman.db_name} --mysql-host=%{gearman.db_host} --mysql-port=%{gearman.db_port} --mysql-table=%{gearman.db_table}"
else
    persistence="-q libsqlite3 --libsqlite3-table=%{gearman.db_table} --libsqlite3-db=%{gearman.db_name} --store-queue-on-shutdown"
fi

[ -n "$config_dir" ] || config_dir=$ROOT/etc/gearbox/config.d

[ -n "$async_workers" ] || async_workers=2
[ -n "$sync_workers" ] || sync_workers=10
[ -n "$allow_unknown_jobs" ] || allow_unknown_jobs=0

[ -n "$exception_stack_trace" ] || exception_stack_trace=0

ENV=""

case $exception_stack_trace in
    yes|true|on|1)
        ENV="$ENV GEARBOX_STACK_TRACE=1";;
esac

cat <<EOF
{
    "component" : "gearbox",
    "allow_unknown_jobs": $allow_unknown_jobs,
    "config_dir": "$config_dir",

    "daemons" : [{
        "name" : "gearmand",
        "logname" : "%{name}",
        "command" : "$ENV $sbindir/gearmand --verbose WARNING $persistence --user=%{gearbox.user} --listen=localhost --port=%{gearman.port}"
    }, {
        "name" : "sync-worker",
        "logname": "%{component}",
        "command" : "$ENV $bindir/workerGearbox --config $ROOT/etc/gearbox/gearbox.conf --no-async --max-requests=5000",
        "count" : $sync_workers,
        "user" : "%{gearbox.user}"
    }, {
        "name" : "async-worker",
        "logname": "%{component}",
        "command" : "$ENV $bindir/workerGearbox --config $ROOT/etc/gearbox/gearbox.conf --no-sync --max-requests=5000",
        "count" : $async_workers,
        "user" : "%{gearbox.user}"
    }, {
        "name" : "delay",
        "logname": "%{name}",
        "command" : "$ENV $bindir/delayDaemon --config $ROOT/etc/gearbox/gearbox.conf",
        "count" : 1,
        "user" : "%{gearbox.user}"
    }]
}
EOF
