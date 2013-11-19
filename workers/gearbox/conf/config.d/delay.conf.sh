#!/bin/sh

[ -n "$delay_db_type" ] && db_type=$delay_db_type
[ -n "$delay_db_name" ] && db_name=$delay_db_name
[ -n "$delay_db_user" ] && db_user=$delay_db_user
[ -n "$delay_db_pass" ] && db_pass=$delay_db_pass
[ -n "$delay_db_admin_pass" ] && db_admin_pass=$delay_db_admin_pass
[ -n "$delay_db_host" ] && db_host=$delay_db_host
[ -n "$delay_db_port" ] && db_port=$delay_db_port
[ -n "$delay_db_sock" ] && db_sock=$delay_db_sock

[ -n "$db_type" ] || db_type="sqlite3"
if [ -z "$db_name" ]; then
    [ "$db_type" = "mysql" ] && db_name="delay" || db_name="$ROOT/var/gearbox/db/delay.db"
fi
if [ -z "$db_user" ]; then
    if [ "$db_type" = "mysql" ]; then
        db_user="gearbox"
    else
        db_user="nobody"
    fi
fi
if [ -z "$db_port" ]; then
    [ "$db_type" = "mysql" ] && db_port=3306 || db_port=0
fi
if [ "$db_type" = "mysql" ]; then
    [ -n "$db_host" ] || db_host="localhost"
    [ "$db_host" = "localhost" ] && [[ -z "${db_sock-undefined}" || ${db_sock:=/tmp/mysql.sock} ]]
fi
cat <<EOM
{
    "fifo" : "$ROOT/var/gearbox/delay.fifo",
    "db_type" : "$db_type",
    "db_name" : "$db_name",
    "db_user" : "$db_user",
    "db_pass" : "$db_pass",
    "db_admin_pass": "$db_admin_pass",
    "db_host" : "$db_host",
    "db_port" : $db_port,
    "db_sock" : "$db_sock"
}
EOM
