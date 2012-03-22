#!/home/y/bin/perl

use Test::Trivial tests => 21;

OK( system("rm -rf .db/*.db") == 0 );

#
# create sqlite.db at Version 1
#
IS qx{../dbsetup -c ./conf/sqlite.conf -u $ENV{USER} ./sql/db1.sql 2>&1}, <<EOM;
------------------------------------------------------------------------
.db/sqlite.db Database Setup...

- Checking that sqlite3 client (sqlite3): OK
- Checking for existing database: Nope!
- Creating database .db/sqlite.db: OK

Setup was successful.

------------------------------------------------------------------------
EOM
OK( $? == 0 );

#
# Upgrade sqlite.db to Version 2
#
IS qx{../dbsetup -c ./conf/sqlite.conf -u $ENV{USER} ./sql/db2.sql 2>&1}, <<EOM;
------------------------------------------------------------------------
.db/sqlite.db Database Setup...

- Checking that sqlite3 client (sqlite3): OK
- Checking for existing database: OK
- Upgrading .db/sqlite.db to version 2: OK
- Updating db version table: OK
- Verify DB version == 2: OK

Setup was successful.

------------------------------------------------------------------------
EOM
OK( $? == 0 );

#
# Downgrade sqlite.db to Version 1
#
IS qx{../dbsetup -c ./conf/sqlite.conf -u $ENV{USER} ./sql/db1.sql 2>&1}, <<EOM;
------------------------------------------------------------------------
.db/sqlite.db Database Setup...

- Checking that sqlite3 client (sqlite3): OK
- Checking for existing database: OK
- Downgrading .db/sqlite.db to version 2: OK
- Updating db version table: OK
- Verify DB version == 1: OK

Setup was successful.

------------------------------------------------------------------------
EOM
OK( $? == 0 );

#
# Upgrade sqlite.db again to Version 2
#
IS qx{../dbsetup -c ./conf/sqlite.conf -u $ENV{USER} ./sql/db2.sql 2>&1}, <<EOM;
------------------------------------------------------------------------
.db/sqlite.db Database Setup...

- Checking that sqlite3 client (sqlite3): OK
- Checking for existing database: OK
- Upgrading .db/sqlite.db to version 2: OK
- Updating db version table: OK
- Verify DB version == 2: OK

Setup was successful.

------------------------------------------------------------------------
EOM
OK( $? == 0 );

#
# Verify no changes to sqlite.db since we are already at Version 2
#
IS qx{../dbsetup -c ./conf/sqlite.conf -u $ENV{USER} ./sql/db2.sql 2>&1}, <<EOM;
------------------------------------------------------------------------
.db/sqlite.db Database Setup...

- Checking that sqlite3 client (sqlite3): OK
- Checking for existing database: OK
- Verify DB version == 2: OK

Setup was successful.

------------------------------------------------------------------------
EOM
OK( $? == 0 );

########################################################################
# Testing dbsetup without using -c config file
########################################################################

#
# Create new sqlite2.db
#
IS qx{../dbsetup -u $ENV{USER} -n .db/sqlite2.db ./sql/db1.sql 2>&1}, <<EOM;
------------------------------------------------------------------------
.db/sqlite2.db Database Setup...

- Checking that sqlite3 client (sqlite3): OK
- Checking for existing database: Nope!
- Creating database .db/sqlite2.db: OK

Setup was successful.

------------------------------------------------------------------------
EOM
OK( $? == 0 );

#
# Upgrade sqlite2.db to Version 2
#
IS qx{../dbsetup -u $ENV{USER} -n .db/sqlite2.db ./sql/db2.sql 2>&1}, <<EOM;
------------------------------------------------------------------------
.db/sqlite2.db Database Setup...

- Checking that sqlite3 client (sqlite3): OK
- Checking for existing database: OK
- Upgrading .db/sqlite2.db to version 2: OK
- Updating db version table: OK
- Verify DB version == 2: OK

Setup was successful.

------------------------------------------------------------------------
EOM
OK( $? == 0 );

#
# Downgrade sqlite2.db back to Version 1
#
IS qx{../dbsetup -u $ENV{USER} -n .db/sqlite2.db ./sql/db1.sql 2>&1}, <<EOM;
------------------------------------------------------------------------
.db/sqlite2.db Database Setup...

- Checking that sqlite3 client (sqlite3): OK
- Checking for existing database: OK
- Downgrading .db/sqlite2.db to version 2: OK
- Updating db version table: OK
- Verify DB version == 1: OK

Setup was successful.

------------------------------------------------------------------------
EOM
OK( $? == 0 );

#
# Upgrade sqlite2.db again back to Version 2
#
IS qx{../dbsetup -u $ENV{USER} -n .db/sqlite2.db ./sql/db2.sql 2>&1}, <<EOM;
------------------------------------------------------------------------
.db/sqlite2.db Database Setup...

- Checking that sqlite3 client (sqlite3): OK
- Checking for existing database: OK
- Upgrading .db/sqlite2.db to version 2: OK
- Updating db version table: OK
- Verify DB version == 2: OK

Setup was successful.

------------------------------------------------------------------------
EOM
OK( $? == 0 );

#
# Verify no changes to sqlite2.db since we are already at Version 2
#
IS qx{../dbsetup -u $ENV{USER} -n .db/sqlite2.db ./sql/db2.sql 2>&1}, <<EOM;
------------------------------------------------------------------------
.db/sqlite2.db Database Setup...

- Checking that sqlite3 client (sqlite3): OK
- Checking for existing database: OK
- Verify DB version == 2: OK

Setup was successful.

------------------------------------------------------------------------
EOM
OK( $? == 0 );
