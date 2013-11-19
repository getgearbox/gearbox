#!/bin/sh

[ -n "$gearman_host" ] || gearman_host=localhost
[ -n "$gearman_port" ] || gearman_port=4730

[ -n "$gearman_db_type" ] && db_type=$gearman_db_type
[ -n "$gearman_db_name" ] && db_name=$gearman_db_name
[ -n "$gearman_db_user" ] && db_user=$gearman_db_user
[ -n "$gearman_db_pass" ] && db_pass=$gearman_db_pass
[ -n "$gearman_db_admin_pass" ] && db_admin_pass=$gearman_db_admin_pass
[ -n "$gearman_db_host" ] && db_host=$gearman_db_host
[ -n "$gearman_db_port" ] && db_port=$gearman_db_port
[ -n "$gearman_db_sock" ] && db_sock=$gearman_db_sock

[ -n "$db_type" ] || db_type="sqlite3"
if [ -z "$db_name" ]; then
    [ "$db_type" = "mysql" ] && db_name="gearman" || db_name="$ROOT/var/gearbox/db/gearman.db"
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
    [ -n "$db_host" ] || db_host="localhost";
    [ "$db_host" = "localhost" ] && [[ -z "${db_sock-undefined}" || ${db_sock:=/tmp/mysql.sock} ]]
fi

cat <<EOM
{
    "host" : "$gearman_host",
    "port" : $gearman_port,

    "db_type" : "$db_type",
    "db_name" : "$db_name",
    "db_table" : "queue",
    "db_user" : "$db_user",
    "db_pass" : "$db_pass",
    "db_admin_pass": "$db_admin_pass",
    "db_host" : "$db_host",
    "db_port" : $db_port
}
EOM
