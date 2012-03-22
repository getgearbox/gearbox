#!/bin/sh
[ -n "$status_db_type" ] && db_type=$status_db_type

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
CREATE TABLE IF NOT EXISTS status_event_new (
    status_name VARCHAR(64) NOT NULL,
    event_type VARCHAR(64)  NOT NULL,
    name VARCHAR(255) NOT NULL,
    envelope $BIGTEXT NOT NULL,
    PRIMARY KEY(status_name, event_type),
    FOREIGN KEY(status_name) REFERENCES status(name)
) $ENGINE;

CREATE TABLE IF NOT EXISTS messages_new (
    id          INTEGER PRIMARY KEY $AUTOINCREMENT,
    status_name VARCHAR(64) NOT NULL,
    message $BIGTEXT,
    FOREIGN KEY(status_name) REFERENCES status(name)
) $ENGINE;

INSERT INTO status_event_new SELECT status_name,event_type,name,envelope FROM status_event;
INSERT INTO messages_new SELECT id,status_name,message FROM messages;

DROP TABLE status_event;
DROP TABLE messages;

ALTER TABLE status_event_new RENAME TO status_event;
ALTER TABLE messages_new RENAME TO messages;
EOF
