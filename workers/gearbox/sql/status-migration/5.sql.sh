#!/bin/sh
[ -n "$status_db_type" ] && db_type=$status_db_type

if [ "$db_type" = "mysql" ]; then
   ENGINE="ENGINE=MyISAM DEFAULT CHARSET=utf8"
else
   ENGINE=
fi

cat <<EOF
CREATE TABLE IF NOT EXISTS status_event (
    status_name VARCHAR(64) NOT NULL,
    event_type  VARCHAR(64) NOT NULL,
    name VARCHAR(255) NOT NULL,
    envelope TEXT NOT NULL,
    PRIMARY KEY(status_name, event_type),
    FOREIGN KEY(status_name) REFERENCES status(name)
) $ENGINE;
INSERT INTO status_event SELECT status_name, "CANCEL", name, envelope FROM status_cancel;
DROP TABLE status_cancel;
EOF