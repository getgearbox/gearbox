#!/bin/bash
if [ "$db_type" = "mysql" ]; then
   IXNAME="ixStatusState ON status";
else
   IXNAME="ixStatusState"
fi

cat <<EOF
DROP INDEX $IXNAME;
EOF
