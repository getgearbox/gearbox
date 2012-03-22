#!/bin/sh

[ -n "$status_db_type" ] && db_type=$status_db_type

if [ "$db_type" = "mysql" ]; then
   ENGINE="ENGINE=InnoDB DEFAULT CHARSET=utf8"
   BIGTEXT=MEDIUMTEXT
else
   ENGINE=
   BIGTEXT=TEXT
fi

cat <<EOF
CREATE TABLE IF NOT EXISTS status_meta (
    status_name VARCHAR(64) NOT NULL,
    name VARCHAR(255) NOT NULL,
    value $BIGTEXT NOT NULL,
    UNIQUE( status_name, name )
) $ENGINE;
EOF
