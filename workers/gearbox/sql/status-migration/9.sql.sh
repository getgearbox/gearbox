#!/bin/sh

[ -n "$status_db_type" ] && db_type=$status_db_type

if [ "$db_type" = "mysql" ]; then
    cat <<EOF
ALTER TABLE status ENGINE=InnoDB;
ALTER TABLE status_event ENGINE=InnoDB;
ALTER TABLE messages ENGINE=InnoDB;
ALTER TABLE child_uri ENGINE=InnoDB;
EOF
else
    echo "SELECT 1;"
fi
