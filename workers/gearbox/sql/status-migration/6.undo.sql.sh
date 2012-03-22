#!/bin/sh
[ -n "$status_db_type" ] && db_type=$status_db_type

if [ "$db_type" = "mysql" ]; then
   ENGINE="ENGINE=MyISAM DEFAULT CHARSET=utf8"
else
   ENGINE=
fi

cat <<EOF
CREATE TABLE IF NOT EXISTS status_new (
    name     VARCHAR(64) NOT NULL,
    operation VARCHAR(64) NOT NULL,
    progress INTEGER DEFAULT 0,
    code   INTEGER DEFAULT 0,
    resource_uri VARCHAR(128) NOT NULL,
    parent_uri   VARCHAR(128) DEFAULT "",
    ctime INTEGER DEFAULT 0,
    mtime INTEGER DEFAULT 0,
    ytime INTEGER,
    state VARCHAR(20) NOT NULL,
    failures INTEGER DEFAULT 0,
    PRIMARY KEY(name),
    UNIQUE(name)
) $ENGINE;

INSERT INTO status_new SELECT name, operation, progress, code, resource_uri, parent_uri, ctime, mtime, ytime, state, failures FROM status;
DROP TABLE status_event;
ALTER TABLE status_new RENAME TO status;
EOF