#!/bin/sh
bindir=/usr/bin
sbindir=/usr/bin
libdir=/usr/lib

[ -n "$gearman_db_type" ] && db_type=$gearman_db_type

if [ "$db_type" = "mysql" ]; then
    persistence="-q libdrizzle --libdrizzle-user=%{gearman.db_user} --libdrizzle-password=\\\"\$(pw=\\\"%{gearman.db_pass}\\\" && [ \\\"\${pw:0:8}\\\" = \\\"keydb://\\\" ] && keydbgetkey \${pw:8} || echo -n \$pw)\\\" --libdrizzle-db=%{gearman.db_name} --libdrizzle-host=%{gearman.db_host} --libdrizzle-port=%{gearman.db_port} \$([ -n \\\"%{gearman.db_sock}\\\" ] && echo \\\"--libdrizzle-uds=%{gearman.db_sock}\\\") --libdrizzle-table=%{gearman.db_table}  --libdrizzle-mysql"
else
    persistence="-q libsqlite3 --libsqlite3-table=%{gearman.db_table} --libsqlite3-db=%{gearman.db_name}"
fi

[ -n "$async_workers" ] || async_workers=2
[ -n "$sync_workers" ] || sync_workers=10

[ -n "$exception_stack_trace" ] || exception_stack_trace=1

ENV=""

case $exception_stack_trace in
    yes|true|on|1)
        ENV="$ENV GEARBOX_STACK_TRACE=1";;
esac

cat <<EOF
{
    "component" : "gearbox",
    "config_dir" : "$ROOT/conf/gearbox/mock-config.d",

    "daemons" : [{
        "name" : "gearmand",
        "logname" : "%{name}",
        "command" : "$ENV $sbindir/gearmand -v $persistence --user=%{gearbox.user} --listen=localhost --port=%{gearman.port} --time-order"
    }, {
        "name" : "sync-worker",
        "logname": "%{component}",
        "command" : "$ENV $bindir/workerGearbox --config $ROOT/conf/gearbox/mock-gearbox.conf --no-async --max-requests=5000",
        "count" : $sync_workers,
        "user" : "%{gearbox.user}"
    }, {
        "name" : "async-worker",
        "logname": "%{component}",
        "command" : "$ENV $bindir/workerGearbox --config $ROOT/conf/gearbox/mock-gearbox.conf --no-sync --max-requests=5000",
        "count" : $async_workers,
        "user" : "%{gearbox.user}"
    }, {
        "name" : "delay",
        "logname": "%{name}",
        "command" : "$ENV $bindir/delayDaemon --config $ROOT/conf/gearbox/mock-gearbox.conf",
        "count" : 1,
        "user" : "%{gearbox.user}"
    }]
}
EOF
