#!/bin/sh
[ -n "$delay_db_type" ] && db_type=$delay_db_type

if [ "$db_type" = "mysql" ]; then
    echo "ALTER TABLE jobs ENGINE=MyISAM;";
else
    echo "SELECT 1;"
fi
