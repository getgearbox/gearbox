#!/bin/sh
cat <<EOM
CREATE TABLE IF NOT EXISTS version (
    version 	INTEGER NOT NULL,
    revert    TEXT
);
INSERT INTO version (version) VALUES(2);

EOM

[ -n "$gearman_db_type" ] && db_type=$gearman_db_type
[ -z "$db_type" ] && db_type="sqlite3"

if [ "$db_type" = "sqlite3" ]; then
   cat <<EOM
CREATE TABLE queue (
    unique_key TEXT,
    function_name TEXT,
    priority INTEGER,
    data BLOB,
    PRIMARY KEY(unique_key,function_name)
);
EOM
else
    # mysql
    cat <<EOM
CREATE TABLE queue (
    unique_key VARCHAR(64),
    function_name VARCHAR(255),
    priority INT,
    data LONGBLOB,
    unique key(unique_key, function_name)
) ENGINE=InnoDB DEFAULT CHARSET=utf8
EOM
fi
