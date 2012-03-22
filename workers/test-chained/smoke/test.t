#!/usr/bin/perl
use strict;
use warnings;
use lib qw(../../../perl/Gearbox_Rest_Client);
use Gearbox::Rest::Client;
use Test::Trivial tests => 60;

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

$ENV{PREFIX} ||= "";
my $PREFIX = $ENV{PREFIX} ? "/$ENV{PREFIX}" : "";
my $URI = "http://" . ($ENV{SMOKE_HOST} || "localhost:4080") . "$PREFIX/test-chained/v1";

my $data;
my $err;

######################################################################
#
# do a GET where 2 other sync calls are made
#
######################################################################
OK $data = $c->req("GET", "$URI/hello");

IS $data => "Hello from job and job1 and job2";
IS $c->res()->header("Job1-Header") => "1";
IS $c->res()->header("Job2-Header") => "1";

######################################################################
#
# do an async POST where 2 other jobs all called via 
# continuations (status object is shared between all jobs)
#
######################################################################
OK $data = $c->req("POST", "$URI/goodbye", "{}");

IS $data => "Goodbye from job and job1 and job2";

IS $c->status()->{messages} => [
   "processing from do_post_testchained$ENV{PREFIX}_goodbye_v1",
   "processing from do_append_internal$ENV{PREFIX}_goodbye1_v1",
   "processing from do_append_internal$ENV{PREFIX}_goodbye2_v1"
];
IS $c->status()->{progress} => 100;
IS $c->status()->{children} => [];
IS $c->status()->{state}   => "COMPLETED";
IS $c->status()->{operation} => "create";
IS $c->status()->{code} => 0;

######################################################################
#
# do an async POST where 2 other sub jobs are called async.
# each sub job has it own status which is visiable via
# children attribute in the status object
#
######################################################################
OK $data = $c->req("POST", "$URI/thing", "{}");

my $id = $data->{id};
LIKE $id, qr/t-\S+/;

IS $c->status()->{messages}->[0] => "processing from do_post_testchained$ENV{PREFIX}_thing_v1";
# LIKE $c->status()->{messages}->[1] => qr{Dispatched child job '$URI/service1/'};
# LIKE $c->status()->{messages}->[2] => qr{Dispatched child job '$URI/service2/'};
IS @{$c->status()->{messages}}, 1;

IS $c->status()->{progress} => 100;
# should be 2 child statuses
LIKE $c->status()->{children}->[0] => qr{$URI/status/s-\S};
LIKE $c->status()->{children}->[1] => qr{$URI/status/s-\S};
IS @{$c->status()->{children}}, 2;
IS $c->status()->{state}   => "COMPLETED";
IS $c->status()->{operation} => "create";
IS $c->status()->{code} => 0;

# check the child statuses
my @status_uris = @{$c->status()->{children}};

OK $data = $c->req("GET", $status_uris[0]);
IS $data->{messages} => ["service1 registered"];
IS $data->{progress} => 100;
IS $data->{children} => [];
IS $data->{state}   => "COMPLETED";
IS $data->{operation} => "reg";
IS $data->{code} => 0;

OK $data = $c->req("GET", $status_uris[1]);
IS $data->{messages} => ["service2 registered"];
IS $data->{progress} => 100;
IS $data->{children} => [];
IS $data->{state}   => "COMPLETED";
IS $data->{operation} => "create";
IS $data->{code} => 0;

######################################################################
#
# do an async DELETE where 2 other sub jobs are called async.
# each sub job has it own status which is visiable via
# children attribute in the status object
#
######################################################################
OK $data = $c->req("DELETE", "$URI/thing/$id");

IS @{$c->status()->{messages}}, 0;

IS $c->status()->{progress} => 100;
# should be 2 child statuses
LIKE $c->status()->{children}->[0] => qr{$URI/status/s-\S};
LIKE $c->status()->{children}->[1] => qr{$URI/status/s-\S};
IS @{$c->status()->{children}}, 2;
IS $c->status()->{state}   => "COMPLETED";
IS $c->status()->{operation} => "delete";
IS $c->status()->{code} => 0;

# check the child statuses
@status_uris = @{$c->status()->{children}};

OK $data = $c->req("GET", $status_uris[0]);
IS $data->{messages} => ["service1 unregistered"];
IS $data->{progress} => 100;
IS $data->{children} => [];
IS $data->{state}   => "COMPLETED";
IS $data->{operation} => "unreg";
IS $data->{code} => 0;

OK $data = $c->req("GET", $status_uris[1]);
IS $data->{messages} => ["service2 unregistered"];
IS $data->{progress} => 100;
IS $data->{children} => [];
IS $data->{state}   => "COMPLETED";
IS $data->{operation} => "delete";
IS $data->{code} => 0;
