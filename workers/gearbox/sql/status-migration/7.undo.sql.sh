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
CREATE TABLE IF NOT EXISTS status_event_old (
    status_name VARCHAR(64) NOT NULL,
    event_type VARCHAR(64)  NOT NULL,
    name VARCHAR(255) NOT NULL,
    envelope TEXT NOT NULL,
    PRIMARY KEY(status_name, event_type),
    FOREIGN KEY(status_name) REFERENCES status(name)
) $ENGINE;

CREATE TABLE IF NOT EXISTS messages_old (
    id          INTEGER PRIMARY KEY $AUTOINCREMENT,
    status_name VARCHAR(64) NOT NULL,
    message TEXT,
    FOREIGN KEY(status_name) REFERENCES status(name)
) $ENGINE;

INSERT INTO status_event_old SELECT status_name,event_type,name,envelope FROM status_event;
INSERT INTO messages_old SELECT id,status_name,message FROM messages;

DROP TABLE status_event;
DROP TABLE messages;

ALTER TABLE status_event_old RENAME TO status_event;
ALTER TABLE messages_old RENAME TO messages;
EOF
