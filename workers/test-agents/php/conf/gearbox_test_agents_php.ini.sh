#!/bin/sh

cat<<EOM
open_basedir = \${open_basedir}:$ROOT/var/gearbox/db/test-agents-php/:$ROOT/conf/gearbox/
EOM