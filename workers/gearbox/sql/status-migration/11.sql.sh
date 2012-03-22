#!/bin/sh
cat <<EOF
ALTER TABLE status ADD COLUMN concurrency INTEGER DEFAULT 0;
EOF
