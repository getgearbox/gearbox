#!/bin/sh
[ -n "$delay_db_type" ] && db_type=$delay_db_type

if [ "$db_type" = "mysql" ]; then
   AUTOINCREMENT="AUTO_INCREMENT"
   ENGINE="ENGINE=MyISAM DEFAULT CHARSET=utf8"
else
   AUTOINCREMENT="AUTOINCREMENT"
   ENGINE=
fi

cat <<EOF
CREATE TABLE IF NOT EXISTS jobscopy (
    id		INTEGER PRIMARY KEY $AUTOINCREMENT,
    name    VARCHAR(255) NOT NULL,
    envelope TEXT NOT NULL,
    time    INTEGER NOT NULL,
    ctime   INTEGER NOT NULL
) $ENGINE;

INSERT INTO jobscopy SELECT id, name, envelope, time, ctime FROM jobs;
DROP TABLE jobs;
ALTER TABLE jobscopy RENAME TO jobs;
EOF
