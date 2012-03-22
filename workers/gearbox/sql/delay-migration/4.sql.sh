#!/bin/sh
[ -n "$delay_db_type" ] && db_type=$delay_db_type

if [ "$db_type" = "mysql" ]; then
   AUTOINCREMENT="AUTO_INCREMENT"
   ENGINE="ENGINE=MyISAM DEFAULT CHARSET=utf8"
   BIGTEXT=MEDIUMTEXT
else
   AUTOINCREMENT="AUTOINCREMENT"
   ENGINE=
   BIGTEXT=TEXT
fi

cat <<EOF
CREATE TABLE IF NOT EXISTS jobs_new (
    id		INTEGER PRIMARY KEY $AUTOINCREMENT,
    status_name VARCHAR(64) NULL,
    name    VARCHAR(255) NOT NULL,
    envelope $BIGTEXT NOT NULL,
    time    INTEGER NOT NULL,
    ctime   INTEGER NOT NULL
) $ENGINE;
INSERT INTO jobs_new SELECT id,status_name,name,envelope,time,ctime FROM jobs;
DROP TABLE jobs;
ALTER TABLE jobs_new RENAME TO jobs;
EOF