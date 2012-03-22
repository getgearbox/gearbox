#!/bin/sh
[ -n "$delay_db_type" ] && db_type=$delay_db_type

if [ "$db_type" = "mysql" ]; then
   AUTOINCREMENT="AUTO_INCREMENT"
   ENGINE="ENGINE=InnoDB DEFAULT CHARSET=utf8"
   BIGTEXT=LONGBLOB
else
   AUTOINCREMENT="AUTOINCREMENT"
   ENGINE=
   BIGTEXT=BLOB
fi

cat <<EOF
/*
 * delay.sql --- delay worker database
*/

-- 1

CREATE TABLE IF NOT EXISTS version (
  version 	INTEGER NOT NULL,
  revert    TEXT
);
INSERT INTO version (version) VALUES(6);

CREATE TABLE IF NOT EXISTS jobs (
    id		INTEGER PRIMARY KEY $AUTOINCREMENT,
    status_name VARCHAR(64) NULL,
    name    VARCHAR(255) NOT NULL,
    envelope $BIGTEXT NOT NULL,
    time    INTEGER NOT NULL,
    ctime   INTEGER NOT NULL
) $ENGINE;

CREATE INDEX ixJobsTime ON jobs(time);

EOF
