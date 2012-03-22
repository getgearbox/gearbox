#!/bin/sh
[ -n "$status_db_type" ] && db_type=$status_db_type

if [ "$db_type" = "mysql" ]; then
   AUTOINCREMENT="AUTO_INCREMENT"
   ENGINE="ENGINE=MyISAM DEFAULT CHARSET=utf8"
else
   AUTOINCREMENT="AUTOINCREMENT"
   ENGINE=
fi

cat <<EOF
DROP TABLE status_cancel;
CREATE TABLE IF NOT EXISTS statusOld (
    name     VARCHAR(64) NOT NULL,
    operation VARCHAR(10) NOT NULL,
    progress INTEGER DEFAULT 0,
    code   INTEGER DEFAULT 0,
    resource_uri VARCHAR(128) NOT NULL,
    parent_uri   VARCHAR(128) DEFAULT "",
    ctime INTEGER DEFAULT 0,
    mtime INTEGER DEFAULT 0,
    state VARCHAR(20) NOT NULL,
    PRIMARY KEY(name),
    UNIQUE(name)
) $ENGINE;
INSERT INTO statusOld (name,operation,progresss,code,resource_uri,parent_uri,ctime,mtime,state) SELECT name,operation,progress,code,resource_uri,parent_uri,ctime,mtime,state FROM status;
DROP TABLE status;
ALTER TABLE statusOld RENAME TO status;
EOF

