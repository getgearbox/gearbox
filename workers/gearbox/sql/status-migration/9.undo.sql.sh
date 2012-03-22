#!/bin/bash

[ -n "$status_db_type" ] && db_type=$status_db_type

if [ "$db_type" = "mysql" ]; then
    cat <<EOF
ALTER TABLE status ENGINE=MyISAM;
ALTER TABLE status_event ENGINE=MyISAM;
ALTER TABLE messages ENGINE=MyISAM;
ALTER TABLE child_uri ENGINE=MyISAM;
EOF
else
    echo "SELECT 1;"
fi
