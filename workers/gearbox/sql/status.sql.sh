#!/bin/sh
[ -n "$status_db_type" ] && db_type=$status_db_type

if [ "$db_type" = "mysql" ]; then
   AUTOINCREMENT="AUTO_INCREMENT"
   ENGINE="ENGINE=InnoDB DEFAULT CHARSET=utf8"
   BIGTEXT=MEDIUMTEXT
else
   AUTOINCREMENT="AUTOINCREMENT"
   ENGINE=
   BIGTEXT=TEXT
fi

cat <<EOF
/*
 * status.sql --- worker status database
*/

-- 1
CREATE TABLE IF NOT EXISTS version (
    version INTEGER NOT NULL,
    revert TEXT
);
INSERT INTO version (version) VALUES(11);

CREATE TABLE IF NOT EXISTS status (
    name     VARCHAR(64) NOT NULL,
    operation VARCHAR(64) NOT NULL,
    component VARCHAR(64) NOT NULL,
    progress INTEGER DEFAULT 0,
    code   INTEGER DEFAULT 0,
    resource_uri VARCHAR(128) NOT NULL,
    parent_uri   VARCHAR(128) DEFAULT "",
    ctime INTEGER DEFAULT 0,
    mtime INTEGER DEFAULT 0,
    ytime INTEGER,
    state VARCHAR(20) NOT NULL,
    failures INTEGER DEFAULT 0,
    concurrency INTEGER DEFAULT 0,
    PRIMARY KEY(name),
    UNIQUE(name)
) $ENGINE;

CREATE INDEX ixStatusState ON status(state);

CREATE TABLE IF NOT EXISTS status_meta (
    status_name VARCHAR(64) NOT NULL,
    name VARCHAR(255) NOT NULL,
    value $BIGTEXT NOT NULL,
    UNIQUE( status_name, name )
) $ENGINE;
    

CREATE TABLE IF NOT EXISTS status_event (
    status_name VARCHAR(64) NOT NULL,
    event_type VARCHAR(64)  NOT NULL,
    name VARCHAR(255) NOT NULL,
    envelope $BIGTEXT NOT NULL,
    PRIMARY KEY(status_name, event_type),
    FOREIGN KEY(status_name) REFERENCES status(name)
) $ENGINE;

CREATE TABLE IF NOT EXISTS messages (
    id          INTEGER PRIMARY KEY $AUTOINCREMENT,
    status_name VARCHAR(64) NOT NULL,
    message $BIGTEXT,
    FOREIGN KEY(status_name) REFERENCES status(name)
) $ENGINE;

CREATE TABLE IF NOT EXISTS child_uri (
    id           INTEGER PRIMARY KEY $AUTOINCREMENT,
    status_name  VARCHAR(64) NOT NULL,
    uri          VARCHAR(128) NOT NULL,
    child_id     VARCHAR(32) NOT NULL,
    FOREIGN KEY(status_name) REFERENCES status(name)
) $ENGINE;
EOF
