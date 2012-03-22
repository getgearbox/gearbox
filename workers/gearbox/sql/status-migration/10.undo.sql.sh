#!/bin/bash

[ -n "$status_db_type" ] && db_type=$status_db_type

echo "DROP TABLE status_meta;";
