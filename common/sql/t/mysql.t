#!/usr/bin/perl

use Test::Trivial tests => 35;

OK( system("./mkdb") == 0 );

$ENV{MYSQLDPIDFILE} = ".db/sandbox/data/mysql_sandbox8935.pid";

sub login {
    my ($user,$pw,$db,$ver) = @_;
    my $args = "-u $user";
    $args .= " -p'$pw'" if $pw;
    $args .= " $db";
    IS qx{mysql -N -S /tmp/mysql_sandbox8935.sock $args -e "SELECT version FROM version"} => $ver;
}

#
# create testdb at Version 1
#
IS qx{../dbsetup -c ./conf/mysql.conf ./sql/db1.sql 2>&1}, <<EOM;
------------------------------------------------------------------------
testdb Database Setup...

- Checking for mysql client (mysql): OK
- Checking that mysqld is running: OK
- Checking for access to mysql: OK
- Checking for existing database: Nope!
- Creating database testdb: OK
- Creating testdb mysql user (gearboxAdmin) [no password]: OK
- Granting access for gearboxAdmin to testdb: OK
- Creating testdb mysql user (gearbox) [no password]: OK
- Granting access for gearbox to testdb: OK

Setup was successful.

------------------------------------------------------------------------
EOM
OK( $? == 0 );

login("gearboxAdmin","","testdb","1\n");
login("gearbox","","testdb","1\n");

#
# Upgrade testdb to Version 2
#
IS qx{../dbsetup -c ./conf/mysql.conf ./sql/db2.sql 2>&1}, <<EOM;
------------------------------------------------------------------------
testdb Database Setup...

- Checking for mysql client (mysql): OK
- Checking that mysqld is running: OK
- Checking for access to mysql: OK
- Checking for existing database: OK
- Upgrading testdb to version 2: OK
- Updating db version table: OK
- Verify DB version == 2: OK
- Granting access for gearboxAdmin to testdb: OK
- Granting access for gearbox to testdb: OK

Setup was successful.

------------------------------------------------------------------------
EOM
OK( $? == 0 );

login("gearboxAdmin","","testdb","1\n2\n");
login("gearbox","","testdb","1\n2\n");

#
# Downgrade testdb back to Version 1
#
IS qx{../dbsetup -c ./conf/mysql.conf ./sql/db1.sql 2>&1}, <<EOM;
------------------------------------------------------------------------
testdb Database Setup...

- Checking for mysql client (mysql): OK
- Checking that mysqld is running: OK
- Checking for access to mysql: OK
- Checking for existing database: OK
- Downgrading testdb to version 2: OK
- Updating db version table: OK
- Verify DB version == 1: OK
- Granting access for gearboxAdmin to testdb: OK
- Granting access for gearbox to testdb: OK

Setup was successful.

------------------------------------------------------------------------
EOM
OK( $? == 0 );

#
# Upgrade testdb again back to Version 2
#
IS qx{../dbsetup -c ./conf/mysql.conf ./sql/db2.sql 2>&1}, <<EOM;
------------------------------------------------------------------------
testdb Database Setup...

- Checking for mysql client (mysql): OK
- Checking that mysqld is running: OK
- Checking for access to mysql: OK
- Checking for existing database: OK
- Upgrading testdb to version 2: OK
- Updating db version table: OK
- Verify DB version == 2: OK
- Granting access for gearboxAdmin to testdb: OK
- Granting access for gearbox to testdb: OK

Setup was successful.

------------------------------------------------------------------------
EOM
OK( $? == 0 );

#
# No changes should be made, already at Version 2
# 

IS qx{../dbsetup -c ./conf/mysql.conf ./sql/db2.sql 2>&1}, <<EOM;
------------------------------------------------------------------------
testdb Database Setup...

- Checking for mysql client (mysql): OK
- Checking that mysqld is running: OK
- Checking for access to mysql: OK
- Checking for existing database: OK
- Verify DB version == 2: OK
- Granting access for gearboxAdmin to testdb: OK
- Granting access for gearbox to testdb: OK

Setup was successful.

------------------------------------------------------------------------
EOM
OK( $? == 0 );

########################################################################
# Testing dbsetup without using -c config file
########################################################################

#
# Create testdb2 at Version 1
#
IS qx{../dbsetup -t mysql -n testdb2 -s /tmp/mysql_sandbox8935.sock -p secret -a '\$3cr37' ./sql/db1.sql 2>&1}, <<EOM;
------------------------------------------------------------------------
testdb2 Database Setup...

- Checking for mysql client (mysql): OK
- Checking that mysqld is running: OK
- Checking for access to mysql: OK
- Checking for existing database: Nope!
- Creating database testdb2: OK
- Updating password for gearboxAdmin: OK
- Granting access for gearboxAdmin to testdb2: OK
- Updating password for gearbox: OK
- Granting access for gearbox to testdb2: OK

Setup was successful.

------------------------------------------------------------------------
EOM
OK( $? == 0 );

login("gearboxAdmin","\$3cr37","testdb2","1\n");
login("gearbox","secret","testdb2","1\n");

#
# Upgrade testdb2 to Version 2
#
IS qx{../dbsetup -t mysql -n testdb2 -s /tmp/mysql_sandbox8935.sock -p secret -a '\$3cr37' ./sql/db2.sql 2>&1}, <<EOM;
------------------------------------------------------------------------
testdb2 Database Setup...

- Checking for mysql client (mysql): OK
- Checking that mysqld is running: OK
- Checking for access to mysql: OK
- Checking for existing database: OK
- Upgrading testdb2 to version 2: OK
- Updating db version table: OK
- Verify DB version == 2: OK
- Granting access for gearboxAdmin to testdb2: OK
- Granting access for gearbox to testdb2: OK

Setup was successful.

------------------------------------------------------------------------
EOM
OK( $? == 0 );

login("gearboxAdmin","\$3cr37","testdb2","1\n2\n");
login("gearbox","secret","testdb2","1\n2\n");

#
# Downgrade testdb2 to Version 1
#
IS qx{../dbsetup -t mysql -n testdb2 -s /tmp/mysql_sandbox8935.sock -p secret -a '\$3cr37' ./sql/db1.sql 2>&1}, <<EOM;
------------------------------------------------------------------------
testdb2 Database Setup...

- Checking for mysql client (mysql): OK
- Checking that mysqld is running: OK
- Checking for access to mysql: OK
- Checking for existing database: OK
- Downgrading testdb2 to version 2: OK
- Updating db version table: OK
- Verify DB version == 1: OK
- Granting access for gearboxAdmin to testdb2: OK
- Granting access for gearbox to testdb2: OK

Setup was successful.

------------------------------------------------------------------------
EOM
OK( $? == 0 );

#
# Upgrade testdb2 again back to Version 2
#
IS qx{../dbsetup -t mysql -n testdb2 -s /tmp/mysql_sandbox8935.sock -p secret -a '\$3cr37' ./sql/db2.sql 2>&1}, <<EOM;
------------------------------------------------------------------------
testdb2 Database Setup...

- Checking for mysql client (mysql): OK
- Checking that mysqld is running: OK
- Checking for access to mysql: OK
- Checking for existing database: OK
- Upgrading testdb2 to version 2: OK
- Updating db version table: OK
- Verify DB version == 2: OK
- Granting access for gearboxAdmin to testdb2: OK
- Granting access for gearbox to testdb2: OK

Setup was successful.

------------------------------------------------------------------------
EOM
OK( $? == 0 );

#
# Verify no changes since we are already at Version 2
#
IS qx{../dbsetup -t mysql -n testdb2 -s /tmp/mysql_sandbox8935.sock -p secret -a '\$3cr37' ./sql/db2.sql 2>&1}, <<EOM;
------------------------------------------------------------------------
testdb2 Database Setup...

- Checking for mysql client (mysql): OK
- Checking that mysqld is running: OK
- Checking for access to mysql: OK
- Checking for existing database: OK
- Verify DB version == 2: OK
- Granting access for gearboxAdmin to testdb2: OK
- Granting access for gearbox to testdb2: OK

Setup was successful.

------------------------------------------------------------------------
EOM
OK( $? == 0 );

########################################################################
# Testing dbsetup with config file passwords
########################################################################

#
# create testdb4 at Version 1
#
IS qx{../dbsetup -c ./conf/mysql-passwd.conf -n testdb4 ./sql/db1.sql 2>&1}, <<EOM;
------------------------------------------------------------------------
testdb4 Database Setup...

- Checking for mysql client (mysql): OK
- Checking that mysqld is running: OK
- Checking for access to mysql: OK
- Checking for existing database: Nope!
- Creating database testdb4: OK
- Updating password for gearboxAdmin: OK
- Granting access for gearboxAdmin to testdb4: OK
- Updating password for gearbox: OK
- Granting access for gearbox to testdb4: OK

Setup was successful.

------------------------------------------------------------------------
EOM
OK( $? == 0 );

login("gearboxAdmin",'$up3r$3cr37',"testdb4","1\n");
login("gearbox","supersecret","testdb4", "1\n");

#
# Upgrade testdb4 to Version 2
#
IS qx{../dbsetup -c ./conf/mysql-passwd.conf -n testdb4 ./sql/db2.sql 2>&1}, <<EOM;
------------------------------------------------------------------------
testdb4 Database Setup...

- Checking for mysql client (mysql): OK
- Checking that mysqld is running: OK
- Checking for access to mysql: OK
- Checking for existing database: OK
- Upgrading testdb4 to version 2: OK
- Updating db version table: OK
- Verify DB version == 2: OK
- Granting access for gearboxAdmin to testdb4: OK
- Granting access for gearbox to testdb4: OK

Setup was successful.

------------------------------------------------------------------------
EOM
OK( $? == 0 );

login("gearboxAdmin",'$up3r$3cr37',"testdb4","1\n2\n");
login("gearbox","supersecret","testdb4", "1\n2\n");
