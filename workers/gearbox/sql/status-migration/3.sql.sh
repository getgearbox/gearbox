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
CREATE TABLE IF NOT EXISTS messagesNew (
    id          INTEGER PRIMARY KEY $AUTOINCREMENT,
    status_name VARCHAR(64) NOT NULL,
    message  TEXT,
    FOREIGN KEY(status_name) REFERENCES status(name)
) $ENGINE;
INSERT INTO messagesNew (status_name, message) SELECT status_name, message FROM messages;
DROP TABLE messages;
ALTER TABLE messagesNew RENAME TO messages;
EOF