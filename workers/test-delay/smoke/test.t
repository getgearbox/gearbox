#!/usr/bin/perl
use strict;
use warnings;
use lib qw(../../../perl/Gearbox_Rest_Client);
use Gearbox::Rest::Client;
use Test::Trivial tests => 28;
use Time::HiRes qw(time sleep);

use Log::Log4perl qw(:easy);
Log::Log4perl->easy_init({
    level    => $DEBUG,
    file     => "STDOUT",
    layout   => '# %m%n'
});

use Getopt::Long;
my $TRACE = 0;
GetOptions(
    'trace' => \$TRACE,
);

my $c = Gearbox::Rest::Client->new( trace => $TRACE );

my $PREFIX = $ENV{PREFIX} ? "/$ENV{PREFIX}" : "";
my $URI = "http://" . ($ENV{SMOKE_HOST} || "localhost") . "$PREFIX/test-delay/v1";

my $data;
my $err;

######################################################################
#
# Create a new counter, it will automatically count to 10 via
# delayed continuation jobs with a 1 second delay.  Verify
# that the messages have all the "set to " values and that the 
# create time of the status is at least 10 seconds older than the 
# last modify time.
#
######################################################################
IS $c->req("POST", "$URI/counter") => 10;

my ($id) = ($c->status()->{uri} =~ m{/([^/]+)$});

# check the polled status to make sure it looks normal
IS $c->status()->{messages} => [map { "set to $_"} 1 .. 10];
IS $c->status()->{code} => 0;
IS $c->status()->{progress} => 100;
IS $c->status()->{uri} => "$URI/counter/$id";
IS $c->status()->{state} => "COMPLETED";
warn "MTIME: ", $c->status()->{mtime}, " CTIME: ", $c->status()->{ctime}, "\n";
OK $c->status()->{mtime} >= $c->status()->{ctime} + 10;


######################################################################
#
# Create a new counter, it will automatically count from 5 to 10 via
# delayed continuation jobs with a 2 second delay.  Verify
# that the messages have all the "set to " values and that the 
# create time of the status is at least 10 (5 * 2)  seconds older
# than the last modify time.
#
######################################################################
IS $c->req("POST", "$URI/counter;end=10;start=5;delay=2") => 10;

($id) = ($c->status()->{uri} =~ m{/([^/]+)$});

# check the polled status to make sure it looks normal
IS $c->status()->{messages} => [map { "set to $_"} 6 .. 10];
IS $c->status()->{code} => 0;
IS $c->status()->{progress} => 100;
IS $c->status()->{uri} => "$URI/counter/$id";
IS $c->status()->{state} => "COMPLETED";
warn "MTIME: ", $c->status()->{mtime}, " CTIME: ", $c->status()->{ctime}, "\n";
OK $c->status()->{mtime} >= $c->status()->{ctime} + 10;

######################################################################
#
# Create a new counter, it will automatically count to 5 via
# delayed continuation jobs with a 1 second delay.  Verify
# that the messages have all the "set to " values and that the 
# create time of the status is at least 5 seconds older than the 
# last modify time.
#
######################################################################
IS $c->req("POST", "$URI/counter;end=5") => 5;

($id) = ($c->status()->{uri} =~ m{/([^/]+)$});

# check the polled status to make sure it looks normal
IS $c->status()->{messages} => [map { "set to $_"} 1 .. 5];
IS $c->status()->{code} => 0;
IS $c->status()->{progress} => 100;
IS $c->status()->{uri} => "$URI/counter/$id";
IS $c->status()->{state} => "COMPLETED";
OK $c->status()->{mtime} >= $c->status()->{ctime} + 5;

######################################################################
#
# Create a new counter, it will automatically count to 5 by
# retrying.  Each iteration will wait longer than the previous
# iteration.  
# Should take at least 1+1+2+3=8 seconds 
#
######################################################################
IS $c->req("POST", "$URI/counter;retry=1;end=5") => 5;

($id) = ($c->status()->{uri} =~ m{/([^/]+)$});

# check the polled status to make sure it looks normal
IS $c->status()->{messages} => [
      'set to 1',
      'retry attempt number 1',
      'set to 2',
      'retry attempt number 2',
      'set to 3',
      'retry attempt number 3',
      'set to 4',
      'retry attempt number 4',
      'set to 5'
];
IS $c->status()->{code} => 0;
IS $c->status()->{progress} => 100;
IS $c->status()->{uri} => "$URI/counter/$id";
IS $c->status()->{state} => "COMPLETED";
warn "MTIME: ", $c->status()->{mtime}, " CTIME: ", $c->status()->{ctime}, "\n";
OK $c->status()->{mtime} >= $c->status()->{ctime} + 8;
