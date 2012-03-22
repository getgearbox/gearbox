#!/bin/sh
[ -n "$status_db_type" ] && db_type=$status_db_type

if [ "$db_type" = "mysql" ]; then
   ENGINE="ENGINE=MyISAM DEFAULT CHARSET=utf8"
else
   ENGINE=
fi

cat <<EOF
CREATE TABLE IF NOT EXISTS status_cancel (
    status_name VARCHAR(64) PRIMARY KEY,
    name VARCHAR(255) NOT NULL,
    envelope TEXT NOT NULL,
    FOREIGN KEY(status_name) REFERENCES status(name)
) $ENGINE;
INSERT INTO status_cancel SELECT status_name, name, envelope FROM status_event WHERE event_type="CANCEL";
DROP TABLE status_event;
EOF