#!/bin/sh
[ -n "$gearman_db_type" ] && db_type=$gearman_db_type
      
if [ "$db_type" = "mysql" ]; then
    cat <<EOF
ALTER TABLE queue ENGINE=MyISAM;
EOF
else
    echo "SELECT 1;"
fi
